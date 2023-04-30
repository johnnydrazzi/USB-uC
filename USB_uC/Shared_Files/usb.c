/**
 * @file usb.c
 * @brief Contains the USB stack core variables and  functions.
 * @author John Izzard
 * @date 30/04/2023
 * 
 * USB uC - USB Stack.
 * Copyright (C) 2017-2023  John Izzard
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <xc.h>
#include <stdint.h>
#include <stdbool.h>
#include "usb.h"
#include "usb_app.h"

#define RESET_CONDITION_FLAG      UIRbits.URSTIF
#define ERROR_CONDITION_FLAG      UIRbits.UERRIF
#define ACTIVITY_DETECT_FLAG      UIRbits.ACTVIF
#define TRANSACTION_COMPLETE_FLAG UIRbits.TRNIF
#define IDLE_DETECT_FLAG          UIRbits.IDLEIF
#define STALL_CONDITION_FLAG      UIRbits.STALLIF
#define SOF_FLAG                  UIRbits.SOFIF

#define ACTIVITY_DETECT_ENABLE  UIEbits.ACTVIE

#define USB_CONTROL_REGISTER                UCON
#define USB_CONFIGURATION_REGISTER          UCFG
#define USB_EP0_CONTROL_REGISTER            UEP0
#define USB_EP1_CONTROL_REGISTER            UEP1
#define USB_EP2_CONTROL_REGISTER            UEP2
#define USB_EP3_CONTROL_REGISTER            UEP3
#define USB_EP4_CONTROL_REGISTER            UEP4
#define USB_EP5_CONTROL_REGISTER            UEP5
#define USB_EP6_CONTROL_REGISTER            UEP6
#define USB_EP7_CONTROL_REGISTER            UEP7
#define USB_INTERRUPT_STAT_REGISTER         UIR
#define USB_INTERRUPT_ENABLE_REGISTER       UIE
#define USB_ERROR_INTERRUPT_STAT_REGISTER   UEIR
#define USB_ERROR_INTERRUPT_ENABLE_REGISTER UEIE
#define PACKET_TRANSFER_DISABLE             UCONbits.PKTDIS
#define USB_MODULE_ENABLE                   UCONbits.USBEN
#define SINGLR_ENDED_ZERO                   UCONbits.SE0
#define PPB_RESET                           UCONbits.PPBRST
#define USB_SUSPEND                         UCONbits.SUSPND

#if defined(_18F13K50) || defined(_18F14K50)
struct // PIC18F14K50.h is outdated
{
    unsigned      :3;
    unsigned ENDP :4;
}_USTATbits __at(0xF63);
#else
#define _USTATbits USTATbits
#endif

/* ************************************************************************** */
/* ************************* GLOBAL VARIABLES ******************************* */
/* ************************************************************************** */

usb_ustat_t             g_usb_last_USTAT;
ch9_setup_t             g_usb_setup             __at(SETUP_DATA_ADDR);
ch9_get_descriptor_t    g_usb_get_descriptor    __at(SETUP_DATA_ADDR);
ch9_set_configuration_t g_usb_set_configuration __at(SETUP_DATA_ADDR);
ch9_set_interface_t     g_usb_set_interface     __at(SETUP_DATA_ADDR);
ch9_get_interface_t     g_get_interface         __at(SETUP_DATA_ADDR);

usb_ep_stat_t       g_usb_ep_stat[NUM_ENDPOINTS][2];
uint16_t            g_usb_bytes_available;
uint16_t            g_usb_bytes_2_send;
uint16_t            g_usb_bytes_2_recv;
bool                g_usb_send_short;
uint8_t             g_usb_sending_from;
const uint8_t      *g_usb_rom_ptr;
uint8_t            *g_usb_ram_ptr;
bd_t                g_usb_bd_table[NUM_BD] __at(BDT_BASE_ADDR);

// The following are from: usb_descriptors.c
extern const ch9_device_descriptor_t g_device_descriptor;
extern const uint16_t                g_config_descriptors[];
extern const uint16_t                g_string_descriptors[];
extern const uint8_t                 g_size_of_sd;

/* ************************************************************************** */


/* ************************************************************************** */
/* ************************* LOCAL VARIABLES ******************************** */
/* ************************************************************************** */

#if (PINGPONG_MODE == PINGPONG_DIS) || (PINGPONG_MODE == PINGPONG_1_15)
static uint8_t m_ep0_out[EP0_SIZE]      __at(EP0_OUT_BUFFER_BASE_ADDR);
static uint8_t m_ep0_in[EP0_SIZE]       __at(EP0_IN_BUFFER_BASE_ADDR);

#elif (PINGPONG_MODE == PINGPONG_0_OUT)
static uint8_t m_ep0_out_even[EP0_SIZE] __at(EP0_OUT_EVEN_BUFFER_BASE_ADDR);
static uint8_t m_ep0_out_odd[EP0_SIZE]  __at(EP0_OUT_ODD_BUFFER_BASE_ADDR);
static uint8_t m_ep0_in[EP0_SIZE]       __at(EP0_IN_BUFFER_BASE_ADDR);

#else
static uint8_t m_ep0_out_even[EP0_SIZE] __at(EP0_OUT_EVEN_BUFFER_BASE_ADDR);
static uint8_t m_ep0_out_odd[EP0_SIZE]  __at(EP0_OUT_ODD_BUFFER_BASE_ADDR);
static uint8_t m_ep0_in_even[EP0_SIZE]  __at(EP0_IN_EVEN_BUFFER_BASE_ADDR);
static uint8_t m_ep0_in_odd[EP0_SIZE]   __at(EP0_IN_ODD_BUFFER_BASE_ADDR);
#endif

static usb_dev_settings_t  m_dev_settings;
static uint8_t             m_saved_address;
static bool                m_update_address;
static uint8_t             m_usb_state = STATE_DETACHED;
static uint8_t             m_control_stage;
static uint8_t             m_current_configuration;

static ch9_get_status_t        m_get_status        __at(SETUP_DATA_ADDR);
static ch9_set_address_t       m_set_address       __at(SETUP_DATA_ADDR);
static ch9_set_clear_feature_t m_set_clear_feature __at(SETUP_DATA_ADDR);

/* ************************************************************************** */


/* ************************************************************************** */
/* ************************ LOCAL FUNCTION DECLARATIONS ********************* */
/* ************************************************************************** */

/**
 * @fn void usb_restart(void);
 * 
 * @brief Resets variables and registers to default values when a reset 
 * condition occurs.
 */
static void usb_restart(void);

/**
 * @fn void arm_setup(void)
 * 
 * @brief Arms the OUT Control Endpoint (EP0 OUT) for <i>Setup Transaction</i>.
 * 
 * The <i>arm_setup()</i> function sets up the EP0 OUT buffer descriptor 
 * values in preparation for a <i>Setup Transaction</i>. The <i>Setup Packet</i> 
 * is 8 bytes long and will be stored in the m_ep0_out buffer following the 
 * successful completion of the transaction. The setup data can then accessed 
 * via the g_usb_setup structure.
 */
static void arm_setup(void);

/**
 * @fn void process_setup(void)
 * 
 * @brief Processes the setup transaction.
 * 
 * The function parses the setup packet stored in g_usb_setup and sets up for the 
 * execution of Standard and Class Requests.
 */
static void process_setup(void);

/**
 * @fn void get_status(void)
 * 
 * @brief Handles GET_STATUS requests.
 */
static void get_status(void);

/**
 * @fn void set_clear_feature(void)
 * 
 * @brief Handles SET_FEATURE and CLEAR_FEATURE requests.
 */
static void set_clear_feature(void);

/**
 * @fn void set_address(void)
 * 
 * @brief Handles SET_ADDRESS requests.
 */
static void set_address(void);

/**
 * @fn void get_descriptor(void)
 * 
 * @brief Handles GET_DESCRIPTOR requests.
 */
static void get_descriptor(void);

/**
 * @fn void set_descriptor(void)
 * 
 * @brief Handles SET_DESCRIPTOR requests.
 */
static void set_descriptor(void);

/**
 * @fn void get_configuration(void)
 * 
 * @brief Handles GET_CONFIGURATION requests.
 */
static void get_configuration(void);

/**
 * @fn void set_configuration(void)
 * 
 * @brief Handles SET_CONFIGURATION requests.
 */
static void set_configuration(void);

/**
 * @fn void get_interface(void)
 * 
 * @brief Handles GET_INTERFACE requests.
 */
static void get_interface(void);

/**
 * @fn void set_interface(void)
 * 
 * @brief Handles SET_INTERFACE requests.
 */
static void set_interface(void);

/**
 * @fn void sync_frame(void)
 * 
 * @brief Handles SYNC_FRAME requests.
 */
static void sync_frame(void);

/* ************************************************************************** */


/* ************************************************************************** */
/* ************************ GLOBAL FUNCTIONS ******************************** */
/* ************************************************************************** */

void usb_init(void)
{
    USB_INTERRUPT_ENABLE_REGISTER = _URSTIE;
    RESET_CONDITION_FLAG = 1; // Force a reset so that initialization happens
                              // inside the interrupt context (if interrupts 
                              // are used). Saves the compiler from creating
                              // duplicate code.
}

void usb_close(void)
{
    
    USB_CONTROL_REGISTER       = 0;
    USB_CONFIGURATION_REGISTER = 0;
    USB_EP0_CONTROL_REGISTER   = 0; // Disable EP0
    
    // Disable EPs > EP0 
    #if NUM_ENDPOINTS > 1 
    USB_EP1_CONTROL_REGISTER = 0;
    #endif
    #if NUM_ENDPOINTS > 2
    USB_EP2_CONTROL_REGISTER = 0;
    #endif
    #if NUM_ENDPOINTS > 3
    USB_EP3_CONTROL_REGISTER = 0;
    #endif
    #if NUM_ENDPOINTS > 4
    USB_EP4_CONTROL_REGISTER = 0;
    #endif
    #if NUM_ENDPOINTS > 5
    USB_EP5_CONTROL_REGISTER = 0;
    #endif
    #if NUM_ENDPOINTS > 6
    USB_EP6_CONTROL_REGISTER = 0;
    #endif
    #if NUM_ENDPOINTS > 7
    USB_EP7_CONTROL_REGISTER = 0;
    #endif
    
    while(TRANSACTION_COMPLETE_FLAG) TRANSACTION_COMPLETE_FLAG = 0; // Clear USTAT
    USB_INTERRUPT_ENABLE_REGISTER       = 0; // USB interrupts disabled
    USB_ERROR_INTERRUPT_ENABLE_REGISTER = 0;
    
    USB_INTERRUPT_STAT_REGISTER       = 0;   // USB interrupt flags cleared
    USB_ERROR_INTERRUPT_STAT_REGISTER = 0;
    
    m_usb_state = STATE_DETACHED;
}

uint8_t usb_get_state(void)
{
    return m_usb_state;
}

uint8_t usb_get_configuration(void)
{
    return m_current_configuration;
}

void usb_set_control_stage(uint8_t control_stage)
{
    m_control_stage = control_stage;
}

void usb_tasks(void)
{
    static uint8_t usb_state_prev;
    
    if(ACTIVITY_DETECT_FLAG && ACTIVITY_DETECT_ENABLE)
    {
        #ifdef USE_ACTIVITY
        usb_activity();
        #endif
        ACTIVITY_DETECT_ENABLE = 0;
        
        if(m_usb_state == STATE_SUSPENDED)
        {
            USB_SUSPEND = 0;
            m_usb_state = usb_state_prev;
        }
        
        while(ACTIVITY_DETECT_FLAG) ACTIVITY_DETECT_FLAG = 0;
    }
    
    if(m_usb_state == STATE_SUSPENDED) return;
    
    if(RESET_CONDITION_FLAG)
    {
        if(m_usb_state != STATE_POWERED) usb_restart();
        m_usb_state = STATE_DEFAULT;
        #ifdef USE_RESET
        usb_reset();
        #endif
        RESET_CONDITION_FLAG = 0;
    }
    
    if(IDLE_DETECT_FLAG)
    {
        ACTIVITY_DETECT_ENABLE = 1;
        USB_SUSPEND = 1;
        usb_state_prev = m_usb_state;
        m_usb_state = STATE_SUSPENDED;
        #ifdef USE_IDLE
        usb_idle();
        #endif
        IDLE_DETECT_FLAG = 0;
    }
    
    #ifdef USE_SOF
    if(SOF_FLAG)
    {
        usb_sof();
        SOF_FLAG = 0;
    }
    #endif
    
    #ifdef USE_ERROR
    if(ERROR_CONDITION_FLAG)
    {
        usb_error();
        ERROR_CONDITION_FLAG = 0;
    }
    #endif
    
    if(m_usb_state < STATE_DEFAULT) return;
    
    while(TRANSACTION_COMPLETE_FLAG)
    {
        NOP();
        NOP();
        *((uint8_t*)&g_usb_last_USTAT) = USTAT;  // Save a copy of USTAT and clear the Transaction Complete Flag.
        TRANSACTION_COMPLETE_FLAG = 0;           // This is to advance the FIFO fast as possible.
        if(TRANSACTION_EP == EP0)
        {
            if(TRANSACTION_DIR == OUT)
            {
                #if PINGPONG_MODE == PINGPONG_0_OUT || PINGPONG_MODE == PINGPONG_ALL_EP
                EP0_OUT_LAST_PPB = PINGPONG_PARITY;
                if(g_usb_bd_table[PINGPONG_PARITY].STATbits.PID == PID_SETUP_TOKEN)
                {
                #else
                if(g_usb_bd_table[BD0_OUT].STATbits.PID == PID_SETUP_TOKEN)
                {
                #endif
                    process_setup();
                }
                else
                {
                    if(m_control_stage == DATA_OUT_STAGE)
                    {
                        EP0_OUT_DATA_TOGGLE_VAL ^= 1;
                        usb_out_control_transfer();
                    }
                    arm_setup();
                }
            }
            else
            {
                #if PINGPONG_MODE == PINGPONG_ALL_EP
                EP0_IN_LAST_PPB = PINGPONG_PARITY;
                #endif
                if(m_control_stage == DATA_IN_STAGE)
                {
                    EP0_IN_DATA_TOGGLE_VAL ^= 1;
                    usb_in_control_transfer();
                }
                else
                {
                    arm_setup();
                    if(m_update_address)
                    {
                        UADDR = m_saved_address;
                        if(m_usb_state == STATE_DEFAULT && m_saved_address != 0) m_usb_state = STATE_ADDRESS;
                        else if(m_saved_address == 0) RESET_CONDITION_FLAG = 1; // Forced Reset.
                        m_update_address = false;
                    }
                }
            }
        }
        else
        {
            usb_app_tasks();
        }
    }
}
    
void usb_arm_endpoint(bd_t* p_bd, usb_ep_stat_t* p_ep_stat, uint16_t buffer_addr, uint8_t cnt)
{
    if(p_ep_stat->Data_Toggle_Val) p_bd->STAT = _DTSEN | _DTS; // DATA1
    else p_bd->STAT = _DTSEN; // DATA0
    p_bd->CNT   = cnt;
    p_bd->ADR   = buffer_addr;
    p_bd->STAT |= _UOWN;
}

#if PINGPONG_MODE == PINGPONG_ALL_EP
void usb_arm_ep0_in(uint8_t bd_table_index, uint8_t cnt)
{
    if(g_usb_ep_stat[EP0][IN].Data_Toggle_Val) g_usb_bd_table[bd_table_index].STAT = _DTSEN | _DTS; // DATA1
    else g_usb_bd_table[bd_table_index].STAT = _DTSEN; // DATA0
    g_usb_bd_table[bd_table_index].CNT = cnt;
    g_usb_bd_table[bd_table_index].STAT |= _UOWN;
}
#else
void usb_arm_ep0_in(uint8_t cnt)
{
    if(g_usb_ep_stat[EP0][IN].Data_Toggle_Val) g_usb_bd_table[BD0_IN].STAT = _DTSEN | _DTS; // DATA1
    else g_usb_bd_table[BD0_IN].STAT = _DTSEN; // DATA0
    g_usb_bd_table[BD0_IN].CNT = cnt;
    g_usb_bd_table[BD0_IN].STAT |= _UOWN;
}
#endif

void usb_arm_status(bd_t* p_bd)
{
    p_bd->CNT   = 0;
    p_bd->STAT  = _DTSEN | _DTS;
    p_bd->STAT |= _UOWN;
}

void usb_arm_in_status(void)
{
    #if PINGPONG_MODE == PINGPONG_ALL_EP
    if(EP0_IN_LAST_PPB == ODD) usb_arm_status(&g_usb_bd_table[BD0_IN_EVEN]);
    else usb_arm_status(&g_usb_bd_table[BD0_IN_ODD]);
    #else
    usb_arm_status(&g_usb_bd_table[BD0_IN]);
    #endif
}

void usb_stall_ep(bd_t* p_bd)
{
        p_bd->STAT  = _BSTALL;
        p_bd->STAT |= _UOWN;
}

void usb_in_control_transfer(void)
{
    #if PINGPONG_MODE == PINGPONG_ALL_EP
    uint8_t *p_ep;
    uint8_t bd_table_index;
    
    if(EP0_IN_LAST_PPB == ODD)
    {
        p_ep = m_ep0_in_odd;
        bd_table_index = BD0_IN_ODD;
    }
    else
    {
        p_ep = m_ep0_in_even;
        bd_table_index = BD0_IN_EVEN;
    }
    
    if(g_usb_bytes_2_send)
    {
        if(g_usb_bytes_2_send > EP0_SIZE)
        {
            if(g_usb_sending_from == ROM)
            {
                usb_rom_copy((const uint8_t*)((uint16_t)g_usb_rom_ptr), p_ep, EP0_SIZE);
                g_usb_rom_ptr += EP0_SIZE;
            }
            else
            {
                usb_ram_copy((uint8_t*)((uint16_t)g_usb_ram_ptr), p_ep, EP0_SIZE);
                g_usb_ram_ptr += EP0_SIZE;
            }
            usb_arm_ep0_in(bd_table_index, EP0_SIZE);
            g_usb_bytes_2_send -= EP0_SIZE;
        }
        else
        {
            if(g_usb_sending_from == ROM) usb_rom_copy((const uint8_t*)((uint16_t)g_usb_rom_ptr), p_ep, (uint8_t)g_usb_bytes_2_send);
            else usb_ram_copy((uint8_t*)((uint16_t)g_usb_ram_ptr), p_ep, (uint8_t)g_usb_bytes_2_send);
            usb_arm_ep0_in(bd_table_index, (uint8_t)g_usb_bytes_2_send);
            g_usb_bytes_2_send = 0;
        }
    }
    else
    {
        if(g_usb_send_short)
        {
            usb_arm_ep0_in(bd_table_index, 0);
            g_usb_send_short = false;
        }
    }
    #else
    if(g_usb_bytes_2_send)
    {
        if(g_usb_bytes_2_send > EP0_SIZE)
        {
            if(g_usb_sending_from == ROM)
            {
                usb_rom_copy((const uint8_t*)((uint16_t)g_usb_rom_ptr), m_ep0_in, EP0_SIZE);
                g_usb_rom_ptr += EP0_SIZE;
            }
            else
            {
                usb_ram_copy((uint8_t*)((uint16_t)g_usb_ram_ptr), m_ep0_in, EP0_SIZE);
                g_usb_ram_ptr += EP0_SIZE;
            }
            usb_arm_ep0_in(EP0_SIZE);
            g_usb_bytes_2_send -= EP0_SIZE;
        }
        else
        {
            if(g_usb_sending_from == ROM) usb_rom_copy((const uint8_t*)((uint16_t)g_usb_rom_ptr), m_ep0_in, (uint8_t)g_usb_bytes_2_send);
            else usb_ram_copy((uint8_t*)((uint16_t)g_usb_ram_ptr), m_ep0_in, (uint8_t)g_usb_bytes_2_send);
            usb_arm_ep0_in((uint8_t)g_usb_bytes_2_send);
            g_usb_bytes_2_send = 0;
        }
    }
    else
    {
        if(g_usb_send_short)
        {
            usb_arm_ep0_in(0);
            g_usb_send_short = false;
        }
    }
    #endif
}

void usb_out_control_transfer(void)
{
    #if PINGPONG_MODE == PINGPONG_0_OUT || PINGPONG_MODE == PINGPONG_ALL_EP
    if(g_usb_bytes_2_recv > EP0_SIZE)
    {
        if(PINGPONG_PARITY == EVEN) usb_ram_copy(m_ep0_out_even, g_usb_ram_ptr, EP0_SIZE);
        else usb_ram_copy(m_ep0_out_odd, g_usb_ram_ptr, EP0_SIZE);
        g_usb_ram_ptr += EP0_SIZE;
        g_usb_bytes_2_recv -= EP0_SIZE;
    }
    else
    {
        if(PINGPONG_PARITY == EVEN) usb_ram_copy(m_ep0_out_even, g_usb_ram_ptr, (uint8_t)g_usb_bytes_2_recv);
        else usb_ram_copy(m_ep0_out_odd, g_usb_ram_ptr, (uint8_t)g_usb_bytes_2_recv);
        g_usb_ram_ptr += EP0_SIZE;
        g_usb_bytes_2_recv = 0;
    }
    #else
    if(g_usb_bytes_2_recv > EP0_SIZE)
    {
        usb_ram_copy(m_ep0_out, g_usb_ram_ptr, EP0_SIZE);
        g_usb_ram_ptr += EP0_SIZE;
        g_usb_bytes_2_recv -= EP0_SIZE;
    }
    else
    {
        usb_ram_copy(m_ep0_out, g_usb_ram_ptr, (uint8_t)g_usb_bytes_2_recv);
        g_usb_ram_ptr += g_usb_bytes_2_recv;
        g_usb_bytes_2_recv = 0;
    }
    #endif
    if(g_usb_bytes_2_recv == 0)
    {
        #ifdef USE_OUT_CONTROL_FINISHED
        if(usb_out_control_finished()) usb_arm_in_status();
        else usb_request_error();
        #else
        usb_arm_in_status();
        #endif
        m_control_stage = STATUS_IN_STAGE;
    }
}

void usb_rom_copy(const uint8_t *p_rom, uint8_t *p_ep, uint8_t bytes)
{
    for(uint8_t i = 0; i < bytes; i++) p_ep[i] = p_rom[i];
}

void usb_ram_copy(uint8_t *p_ram1, uint8_t *p_ram2, uint8_t bytes)
{
    for(uint8_t i = 0; i < bytes; i++) p_ram2[i] = p_ram1[i];
}

void usb_ram_set(uint8_t val, uint8_t *p_ram, uint16_t bytes)
{
    for(uint16_t i = 0; i < bytes; i++) p_ram[i] = val;
}

/* ************************************************************************** */


/* ************************************************************************** */
/* *********************** LOCAL FUNCTIONS ********************************** */
/* ************************************************************************** */

static void usb_restart(void)
{
    USB_INTERRUPT_ENABLE_REGISTER       = INTERRUPTS_MASK;
    USB_ERROR_INTERRUPT_ENABLE_REGISTER = ERROR_INTERRUPT_MASK;
    
    USB_INTERRUPT_STAT_REGISTER       = 0;   // Interrupt flags cleared
    USB_ERROR_INTERRUPT_STAT_REGISTER = 0;
    
    USB_EP0_CONTROL_REGISTER = 0; // Disable EP0
    
    // Disable EPs > EP0 
    #if NUM_ENDPOINTS > 1 
    USB_EP1_CONTROL_REGISTER = _EPCONDIS;
    #endif
    #if NUM_ENDPOINTS > 2
    USB_EP2_CONTROL_REGISTER = _EPCONDIS;
    #endif
    #if NUM_ENDPOINTS > 3
    USB_EP3_CONTROL_REGISTER = _EPCONDIS;
    #endif
    #if NUM_ENDPOINTS > 4
    USB_EP4_CONTROL_REGISTER = _EPCONDIS;
    #endif
    #if NUM_ENDPOINTS > 5
    USB_EP5_CONTROL_REGISTER = _EPCONDIS;
    #endif
    #if NUM_ENDPOINTS > 6
    USB_EP6_CONTROL_REGISTER = _EPCONDIS;
    #endif
    #if NUM_ENDPOINTS > 7
    USB_EP7_CONTROL_REGISTER = _EPCONDIS;
    #endif
    
    UADDR = 0x00;// Address starts off at 0x00
    
    // USB Settings
    m_dev_settings.Self_Powered  = POWERED_TYPE;
    m_dev_settings.Remote_Wakeup = REMOTE_WAKEUP;
    USB_CONFIGURATION_REGISTER = SPEED_PULLUP | USB_SPEED | PPB;
    
    // Clear BDT Entries
    usb_ram_set(0, (uint8_t*)g_usb_bd_table, BDT_SIZE);
    
    // Setup BDT for EP0 OUT
    #if PINGPONG_MODE == PINGPONG_ALL_EP || PINGPONG_MODE == PINGPONG_0_OUT
    g_usb_bd_table[BD0_OUT_EVEN].ADR = EP0_OUT_EVEN_BUFFER_BASE_ADDR;
    g_usb_bd_table[BD0_OUT_ODD].ADR  = EP0_OUT_ODD_BUFFER_BASE_ADDR;
    #else
    g_usb_bd_table[BD0_OUT].ADR = EP0_OUT_BUFFER_BASE_ADDR;
    #endif
    
    // Setup BDT for EP0 IN
    #if PINGPONG_MODE == PINGPONG_ALL_EP
    g_usb_bd_table[BD0_IN_EVEN].ADR = EP0_IN_EVEN_BUFFER_BASE_ADDR;
    g_usb_bd_table[BD0_IN_ODD].ADR  = EP0_IN_ODD_BUFFER_BASE_ADDR;
    #else
    g_usb_bd_table[BD0_IN].ADR = EP0_IN_BUFFER_BASE_ADDR;
    #endif
    
    // Clear EP statuses
    usb_ram_set(0, (uint8_t*)g_usb_ep_stat, EP_STAT_SIZE);
    
    #if PINGPONG_MODE != PINGPONG_DIS
    // Last_PPB starts on ODD
    for(uint8_t i = 0; i < NUM_ENDPOINTS; i++)
    {
        g_usb_ep_stat[i][0].Last_PPB = ODD;
        g_usb_ep_stat[i][1].Last_PPB = ODD;
    }
    #endif
    
    m_update_address = false;
    g_usb_send_short = false;
    
    m_current_configuration = 0;
    
    while(TRANSACTION_COMPLETE_FLAG) TRANSACTION_COMPLETE_FLAG = 0; // Clear USTAT
    PACKET_TRANSFER_DISABLE = 0;
    
    // EP0 Settings for control
    USB_EP0_CONTROL_REGISTER = _EPHSHK | _EPOUTEN | _EPINEN;
    
    if(m_usb_state == STATE_DETACHED)
    {
        USB_MODULE_ENABLE = 1;
        m_usb_state = STATE_ATTACHED;
        while(SINGLR_ENDED_ZERO){}
        m_usb_state = STATE_POWERED;
    }
    
    m_control_stage = SETUP_STAGE;
    
    #if PINGPONG_MODE != PINGPONG_DIS
    PPB_RESET = 1; // Reset Ping-Pong Buffer Pointers to Even
    NOP();
    NOP();
    NOP();
    NOP();
    PPB_RESET = 0;
    #endif
    
    #if PINGPONG_MODE == PINGPONG_DIS || PINGPONG_MODE == PINGPONG_1_15
    arm_setup();
    #else
    arm_setup();
    g_usb_ep_stat[EP0][OUT].Last_PPB = EVEN;
    arm_setup();
    #endif
}

static void arm_setup(void)
{
    #if PINGPONG_MODE == PINGPONG_0_OUT || PINGPONG_MODE == PINGPONG_ALL_EP
    g_usb_bd_table[EP0_OUT_LAST_PPB].CNT   = 8;
    g_usb_bd_table[EP0_OUT_LAST_PPB].STAT  = 0;
    g_usb_bd_table[EP0_OUT_LAST_PPB].STAT |= _UOWN;
    #else
    g_usb_bd_table[BD0_OUT].CNT   = 8;
    g_usb_bd_table[BD0_OUT].STAT  = 0;
    g_usb_bd_table[BD0_OUT].STAT |= _UOWN;
    #endif
}

static void process_setup(void)
{
    #if PINGPONG_MODE == PINGPONG_ALL_EP
    g_usb_bd_table[BD0_IN_EVEN].STAT = 0; // Stops any pending control IN transfers.
    g_usb_bd_table[BD0_IN_ODD].STAT  = 0; // Stops any pending control IN transfers.
    #else
    g_usb_bd_table[BD0_IN].STAT = 0;      // Stops any pending control IN transfers.
    #endif
    
    #if PINGPONG_MODE == PINGPONG_0_OUT || PINGPONG_MODE == PINGPONG_ALL_EP
    if(PINGPONG_PARITY == ODD)  usb_ram_copy(m_ep0_out_odd, g_usb_setup.array, 8); // Store Setup Packet Data and straight-away re-arm EP0 OUT.
    else usb_ram_copy(m_ep0_out_even, g_usb_setup.array, 8);
    #else
    usb_ram_copy(m_ep0_out, g_usb_setup.array, 8); // Store Setup Packet Data and straight-away re-arm EP0 OUT.
    #endif
    PACKET_TRANSFER_DISABLE = 0; // Must be cleared after every setup transfer!    
    arm_setup(); // USB device should be fast as possible to ACK a new setup packet, that's why we rearm straight-away.
                 // EP0 OUT is also armed to accept OUT Status packets.
    
    EP0_OUT_DATA_TOGGLE_VAL = 1; // First transfer after setup is always DATA1 type.
    EP0_IN_DATA_TOGGLE_VAL  = 1; // First transfer after setup is always DATA1 type.
    
    // Now process the g_usb_setup.
    if(g_usb_setup.bmRequestType_bits.Type == STANDARD)
    {
        switch(g_usb_setup.bRequest)
        {
            // Most common requests are checked first for speed.
            case GET_DESCRIPTOR:
                get_descriptor();
                break;
            case CLEAR_FEATURE:
                set_clear_feature();
                break;
            case SET_ADDRESS:
                set_address();
                break;                
            case SET_CONFIGURATION:
                set_configuration();
                break;
                
                
            case GET_STATUS:
                get_status();
                break;
            case SET_FEATURE:
                set_clear_feature();
                break;
            case SET_DESCRIPTOR:
                set_descriptor();
                break;
            case GET_CONFIGURATION:
                get_configuration();
                break;
            case GET_INTERFACE:
                get_interface();
                break;
            case SET_INTERFACE:
                set_interface();
                break;
            case SYNC_FRAME:
                sync_frame();
                break;
            default:
                usb_request_error();
                break;
        }
    }
    else
    {
        if(usb_service_class_request() == false)
        {
            usb_request_error(); // Class Request wasn't recognized.
        }
    }
}

static void get_status(void)
{
    bool perform_request_error = true;
    union
    {
        struct
        {
            union
            {
                unsigned SelfPowered    :1;
                unsigned Halt           :1;
            };
            unsigned RemoteWakeup       :1;
        };
        uint8_t Byte;
    }get_status_return;
    
    get_status_return.Byte = 0;
    
    if((m_usb_state == STATE_ADDRESS) || (m_usb_state == STATE_CONFIGURED))
    {
        switch(m_get_status.bmRequestType_bits.Recipient)
        {
            case DEVICE:
                get_status_return.SelfPowered  = m_dev_settings.Self_Powered;
                get_status_return.RemoteWakeup = m_dev_settings.Remote_Wakeup;
                perform_request_error = false;
                break;
            case INTERFACE:
                perform_request_error = false;
                break;
            case ENDPOINT:
                if(m_usb_state == STATE_ADDRESS)
                {
                    if(m_get_status.ZeroInterfaceEndpoint_bits.Endpoint_bits.EndpointNumber == EP0) 
                        perform_request_error = false;
                }
                else
                {
                    if(m_get_status.ZeroInterfaceEndpoint_bits.Endpoint_bits.EndpointNumber < NUM_ENDPOINTS) 
                        perform_request_error = false;
                }
                
                if(perform_request_error == false)
                {
                    get_status_return.Halt = 
                            g_usb_ep_stat[m_get_status.ZeroInterfaceEndpoint_bits.Endpoint_bits.EndpointNumber][m_get_status.ZeroInterfaceEndpoint_bits.Endpoint_bits.Direction].Halt;
                }
                break;     
        }
    }
    
    if(perform_request_error)usb_request_error();
    else
    {
        #if PINGPONG_MODE == PINGPONG_ALL_EP
        if(EP0_IN_LAST_PPB == ODD)
        {
            usb_ram_set(0, m_ep0_in_even, 8); // Clear EP ready for data
            m_ep0_in_even[0] = get_status_return.Byte;
            usb_arm_ep0_in(BD0_IN_EVEN, 2);
        }
        else
        {
            usb_ram_set(0, m_ep0_in_odd, 8); // Clear EP ready for data
            m_ep0_in_odd[0] = get_status_return.Byte;
            usb_arm_ep0_in(BD0_IN_ODD, 2);
        }
        #else
        usb_ram_set(0, m_ep0_in, 8); // Clear EP ready for data
        m_ep0_in[0] = get_status_return.Byte;
        usb_arm_ep0_in(2);

        #endif
        m_control_stage = DATA_IN_STAGE;
    }
}

static void set_clear_feature(void)
{
    uint8_t bd_table_index;
    bool perform_request_error = true;
    
    if((m_usb_state == STATE_ADDRESS) || (m_usb_state == STATE_CONFIGURED))
    {
        switch(m_set_clear_feature.bmRequestType_bits.Recipient)
        {
            case DEVICE:
                if(m_set_clear_feature.FeatureSelector == DEVICE_REMOTE_WAKEUP)
                {
                    if(g_usb_setup.bRequest == CLEAR_FEATURE)
                    {
                        m_dev_settings.Remote_Wakeup = REMOTE_WAKEUP_OFF;
                    }
                    else
                    {
                        m_dev_settings.Remote_Wakeup = REMOTE_WAKEUP_ON;
                    }
                    perform_request_error = false;
                }
                break;
            case ENDPOINT:
                if(m_set_clear_feature.FeatureSelector == ENDPOINT_HALT)
                {
                    if(m_usb_state == STATE_CONFIGURED)
                    {
                        if(m_set_clear_feature.ZeroInterfaceEndpoint_bits.Endpoint_bits.EndpointNumber < NUM_ENDPOINTS)
                            perform_request_error = false;
                    }
                    else
                    {
                        if(m_set_clear_feature.ZeroInterfaceEndpoint_bits.Endpoint_bits.EndpointNumber == EP0)
                            perform_request_error = false;
                    }
                    if(perform_request_error == false)
                    {
                        if(m_set_clear_feature.ZeroInterfaceEndpoint_bits.Endpoint_bits.EndpointNumber == EP0)
                        {
                            #if PINGPONG_MODE == PINGPONG_ALL_EP                            
                            if(g_usb_setup.bRequest == CLEAR_FEATURE)
                            {
                                g_usb_ep_stat[EP0][IN].Halt      = 0;
                                g_usb_bd_table[BD0_IN_EVEN].STAT = 0;
                                g_usb_bd_table[BD0_IN_ODD].STAT  = 0;
                            }
                            else
                            {
                                g_usb_ep_stat[EP0][IN].Halt = 1;
                                usb_stall_ep(&g_usb_bd_table[BD0_IN_EVEN]);
                                usb_stall_ep(&g_usb_bd_table[BD0_IN_ODD]);
                            }
                            #else
                            if(g_usb_setup.bRequest == CLEAR_FEATURE)
                            {
                                g_usb_ep_stat[EP0][IN].Halt = 0;
                                g_usb_bd_table[BD0_IN].STAT = 0;
                            }
                            else
                            {
                                g_usb_ep_stat[EP0][IN].Halt = 1;
                                usb_stall_ep(&g_usb_bd_table[BD0_IN]);
                            }
                            #endif
                        }
                        else
                        {
                            #if PINGPONG_MODE == PINGPONG_DIS
                            bd_table_index = (m_set_clear_feature.ZeroInterfaceEndpoint_bits.Endpoint_bits.EndpointNumber * 2u) + 
                                              m_set_clear_feature.ZeroInterfaceEndpoint_bits.Endpoint_bits.Direction;
                            #elif PINGPONG_MODE == PINGPONG_0_OUT
                            bd_table_index = (m_set_clear_feature.ZeroInterfaceEndpoint_bits.Endpoint_bits.EndpointNumber * 2u) + 1u + 
                                              m_set_clear_feature.ZeroInterfaceEndpoint_bits.Endpoint_bits.Direction;
                            #elif PINGPONG_MODE == PINGPONG_1_15
                            bd_table_index = (m_set_clear_feature.ZeroInterfaceEndpoint_bits.Endpoint_bits.EndpointNumber * 2u) + 1u + 
                                              m_set_clear_feature.ZeroInterfaceEndpoint_bits.Endpoint_bits.Direction;
                            #else
                            bd_table_index = (m_set_clear_feature.ZeroInterfaceEndpoint_bits.Endpoint_bits.EndpointNumber * 2u) + 2u + 
                                             (m_set_clear_feature.ZeroInterfaceEndpoint_bits.Endpoint_bits.Direction * 2u);
                            #endif
                            if(g_usb_setup.bRequest == CLEAR_FEATURE)
                            {
                                usb_app_clear_halt(bd_table_index, 
                                                   m_set_clear_feature.ZeroInterfaceEndpoint_bits.Endpoint_bits.EndpointNumber, 
                                                   m_set_clear_feature.ZeroInterfaceEndpoint_bits.Endpoint_bits.Direction);
                            }
                            else
                            {
                                g_usb_ep_stat[m_set_clear_feature.ZeroInterfaceEndpoint_bits.Endpoint_bits.EndpointNumber][m_set_clear_feature.ZeroInterfaceEndpoint_bits.Endpoint_bits.Direction].Halt = 1;
                                usb_stall_ep(&g_usb_bd_table[bd_table_index]);
                                #if PINGPONG_MODE == PINGPONG_1_15 || PINGPONG_MODE == PINGPONG_ALL_EP
                                bd_table_index++;
                                usb_stall_ep(&g_usb_bd_table[bd_table_index]);
                                #endif
                            }
                        }
                    }
                }
                break;
        }
    }
    if(perform_request_error) usb_request_error();
    else
    {
        usb_arm_in_status();
        m_control_stage = STATUS_IN_STAGE;
    }
}

static void set_address(void)
{
    m_saved_address  = (uint8_t)m_set_address.DeviceAddress;
    m_update_address = true;
    usb_arm_in_status();
    m_control_stage  = STATUS_IN_STAGE;
}

static void get_descriptor(void)
{
    bool perform_request_error = true;
    const uint16_t *ptr;
    
    switch(g_usb_get_descriptor.DescriptorType)
    {
        case DEVICE_DESC:
            g_usb_rom_ptr         = (const uint8_t*)&g_device_descriptor;
            g_usb_bytes_available = sizeof(g_device_descriptor);
            perform_request_error = false;
            break;
        case DEVICE_QUALIFIER_DESC:
            break;
        case CONFIGURATION_DESC:
            if(g_usb_get_descriptor.DescriptorIndex < NUM_CONFIGURATIONS)
            {
                g_usb_rom_ptr         = (const uint8_t*) g_config_descriptors[g_usb_get_descriptor.DescriptorIndex];
                ptr                   = (const uint16_t*)g_config_descriptors[g_usb_get_descriptor.DescriptorIndex];
                g_usb_bytes_available = ptr[1];
                perform_request_error = false;
            }
            break;
        case STRING_DESC:
            if(g_usb_get_descriptor.DescriptorIndex < g_size_of_sd)
            {
                g_usb_rom_ptr         = (const uint8_t*)g_string_descriptors[g_usb_get_descriptor.DescriptorIndex];
                g_usb_bytes_available = g_usb_rom_ptr[0];
                perform_request_error = false;
            }
            break;
        default:
            if(usb_get_class_descriptor()) perform_request_error = false;
            break;
    }
    if(perform_request_error) usb_request_error();
    else
    {
        if(g_usb_bytes_available < g_usb_get_descriptor.DescriptorLength)
        {
            g_usb_bytes_2_send = g_usb_bytes_available;
            if(g_usb_bytes_available % EP0_SIZE) g_usb_send_short = false;
            else g_usb_send_short = true;
        }
        else
        {
            g_usb_bytes_2_send = g_usb_get_descriptor.DescriptorLength;
            g_usb_send_short   = false;
        }
        g_usb_sending_from = ROM;
        
        #if PINGPONG_MODE == PINGPONG_ALL_EP
        EP0_IN_LAST_PPB ^= 1;
        usb_in_control_transfer();
        if(g_usb_bytes_2_send != 0)
        {
            EP0_IN_DATA_TOGGLE_VAL ^= 1;
            EP0_IN_LAST_PPB ^= 1;
            usb_in_control_transfer();
        }
        #else
        usb_in_control_transfer();
        #endif
        m_control_stage = DATA_IN_STAGE;
    }
}

static void set_descriptor(void)
{
    usb_request_error();
}

static void get_configuration(void)
{
    uint8_t get_configuration_return;
    if(m_usb_state == STATE_CONFIGURED) get_configuration_return = m_current_configuration;
    else get_configuration_return = 0; // If any state other than configured, return 0
    #if PINGPONG_MODE == PINGPONG_ALL_EP
    if(EP0_IN_LAST_PPB == ODD)
    {
        m_ep0_in_even[0] = get_configuration_return;
        usb_arm_ep0_in(BD0_IN_EVEN, 1);
    }
    else
    {
        m_ep0_in_odd[0] = get_configuration_return;
        usb_arm_ep0_in(BD0_IN_ODD, 1);
    }
    #else
    m_ep0_in[0] = get_configuration_return;
    usb_arm_ep0_in(1);
    #endif
    m_control_stage = DATA_IN_STAGE;
}

static void set_configuration(void)
{
    if((m_usb_state == STATE_ADDRESS || m_usb_state == STATE_CONFIGURED) && (g_usb_set_configuration.ConfigurationValue < NUM_CONFIGURATIONS + 1))
    {
        // Reset Ping-Pong Buffer Pointers to Even
        #if (PINGPONG_MODE != PINGPONG_DIS)
        PPB_RESET = 1;
        NOP();
        NOP();
        NOP();
        NOP();
        PPB_RESET = 0;
        
        // Last_PPB starts on ODD
        for(uint8_t i = 0; i < NUM_ENDPOINTS; i++)
        {
            g_usb_ep_stat[i][0].Last_PPB = ODD;
            g_usb_ep_stat[i][1].Last_PPB = ODD;
        }
        #endif
        usb_arm_in_status();
        m_control_stage = STATUS_IN_STAGE;
        
        m_current_configuration = (uint8_t)g_usb_set_configuration.ConfigurationValue;      

        if(m_current_configuration)
        {
            usb_app_init();
            m_usb_state = STATE_CONFIGURED;
        }
        else m_usb_state = STATE_ADDRESS;
    }
    else
    {
        usb_request_error();
    }
}

static void get_interface(void)
{
    #if NUM_ALT_INTERFACES == 0
    if(m_usb_state == STATE_CONFIGURED && g_get_interface.Interface < NUM_INTERFACES)
    {
        #if PINGPONG_MODE == PINGPONG_ALL_EP
        if(EP0_IN_LAST_PPB == ODD)
        {
            m_ep0_in_even[0] = 0;
            usb_arm_ep0_in(BD0_IN_EVEN, 1);
        }
        else
        {
            m_ep0_in_odd[0] = 0;
            usb_arm_ep0_in(BD0_IN_ODD, 1);
        }
        #else
        m_ep0_in[0] = 0;
        usb_arm_ep0_in(1);
        #endif
    #else
    uint8_t get_interface_return;
    if(m_usb_state == STATE_CONFIGURED && usb_app_get_interface(&get_interface_return, g_get_interface.Interface))
    {
        #if PINGPONG_MODE == PINGPONG_ALL_EP
        if(EP0_IN_LAST_PPB == ODD)
        {
            m_ep0_in_even[0] = get_interface_return;
            usb_arm_ep0_in(BD0_IN_EVEN, 1);
        }
        else
        {
            m_EP0_IN_ODD[0] = get_interface_return;
            usb_arm_ep0_in(BD0_IN_ODD, 1);
        }
        #else
        m_ep0_in[0] = get_interface_return;
        usb_arm_ep0_in(BD0_IN, 1);
        #endif
    #endif
        m_control_stage = DATA_IN_STAGE;
    }
    else usb_request_error();
}
    
static void set_interface(void)
{
    if(usb_app_set_interface((uint8_t)g_usb_set_interface.AlternateSetting, g_usb_set_interface.Interface))
    {
        usb_arm_in_status();
    }
    else
    {
        usb_request_error();
    }

}

static void sync_frame(void)
{
    usb_request_error();
}

/* ************************************************************************** */