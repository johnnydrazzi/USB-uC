/**
 * @file usb_msd.c
 * @brief Contains <i>Mass Storage Class</i> functions.
 * @author John Izzard
 * @date 20/04/2021
 * 
 * USB uC - MSD Library.
 * Copyright (C) 2017-2021  John Izzard
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
#include <stdbool.h>
#include <stdint.h>
#include "usb.h"
#include "usb_msd.h"
#include "usb_ch9.h"
#include "usb_app.h"
#include "usb_scsi.h"

/******************************************************************************/
/************************* DEVICE DIRECTION DEFINES ***************************/
/******************************************************************************/

#define Dn 0 ///< Device expecting NO data transfer.
#define Di 1 ///< Device expecting IN data transfer.
#define Do 2 ///< Device expecting OUT data transfer.

/******************************************************************************/


/******************************************************************************/
/****************************** MSD ENDPOINTS *********************************/
/******************************************************************************/

#if PINGPONG_MODE == PINGPONG_DIS || PINGPONG_MODE == PINGPONG_0_OUT
uint8_t g_msd_ep_out[MSD_EP_SIZE] __at(MSD_EP_OUT_BUFFER_BASE_ADDR);
uint8_t g_msd_ep_in[MSD_EP_SIZE]  __at(MSD_EP_IN_BUFFER_BASE_ADDR);
#else // PINGPONG_MODE == PINGPONG_1_15 || PINGPONG_MODE == PINGPONG_ALL_EP
uint8_t g_msd_ep_out_even[MSD_EP_SIZE] __at(MSD_EP_OUT_EVEN_BUFFER_BASE_ADDR);
uint8_t g_msd_ep_out_odd[MSD_EP_SIZE]  __at(MSD_EP_OUT_ODD_BUFFER_BASE_ADDR);
uint8_t g_msd_ep_in_even[MSD_EP_SIZE]  __at(MSD_EP_IN_EVEN_BUFFER_BASE_ADDR);
uint8_t g_msd_ep_in_odd[MSD_EP_SIZE]   __at(MSD_EP_IN_ODD_BUFFER_BASE_ADDR);
#endif

/******************************************************************************/


/******************************************************************************/
/****************************** SECTOR VARS ***********************************/
/******************************************************************************/

uint16_t g_msd_byte_of_sect;
#ifndef MSD_LIMITED_RAM
uint8_t g_msd_sect_data[512];
#endif

/******************************************************************************/


/******************************************************************************/
/******************************* MSD GLOBAL VARS ******************************/
/******************************************************************************/

msd_cbw_t                 g_msd_cbw __at(CBW_DATA_ADDR);
msd_csw_t                 g_msd_csw __at(CBW_DATA_ADDR);
msd_rw_10_vars_t          g_msd_rw_10_vars;
msd_bytes_to_transfer_t   g_msd_bytes_to_transfer;
uint8_t                   g_msd_sense_key;
uint8_t                   g_msd_additional_sense_code;
uint8_t                   g_msd_additional_sense_code_qualifier;


#if PINGPONG_MODE == PINGPONG_1_15 || PINGPONG_MODE == PINGPONG_ALL_EP
scsi_read_capacity_10_t g_msd_read_capacity_10;
scsi_mode_sense_t       g_msd_mode_sense;
#else
scsi_read_capacity_10_t g_msd_read_capacity_10 __at(MSD_EP_IN_BUFFER_BASE_ADDR);
scsi_mode_sense_t       g_msd_mode_sense       __at(MSD_EP_IN_BUFFER_BASE_ADDR);
#endif

/******************************************************************************/


/******************************************************************************/
/******************************* LOCAL VARS ***********************************/
/******************************************************************************/

static scsi_request_sense_cmd_t    m_request_sense_cmd    __at(CBW_DATA_ADDR + 15);
static scsi_inquiry_cmd_t          m_inquiry_cmd          __at(CBW_DATA_ADDR + 15);
static scsi_mode_sense_6_cmd_t     m_mode_sense_6_cmd     __at(CBW_DATA_ADDR + 15);
static scsi_read_capacity_10_cmd_t m_read_capacity_10_cmd __at(CBW_DATA_ADDR + 15);
static scsi_read_10_cmd_t          m_read_10_cmd          __at(CBW_DATA_ADDR + 15);
static scsi_write_10_cmd_t         m_write_10_cmd         __at(CBW_DATA_ADDR + 15);
static scsi_mode_select_6_cmd_t    m_mode_select_6_cmd    __at(CBW_DATA_ADDR + 15);
static scsi_pamr_cmd_t             m_pamr_cmd             __at(CBW_DATA_ADDR + 15);

volatile static uint8_t m_msd_state;
volatile static bool    m_end_data_short;
volatile static bool    m_wait_for_bomsr;
volatile static bool    m_clear_halt_event;

volatile static bool    m_media_present;
volatile static bool    m_unit_attention;

volatile static uint8_t m_task_cnt;
volatile static uint8_t m_task_put_index;
volatile static uint8_t m_task_get_index;

volatile static union
{
    uint8_t     task[8];
    usb_ustat_t task_stat[8];
}m_tasks_buff;

/******************************************************************************/


/******************************************************************************/
/****************** INQUIRY RESPONSE FROM: usb_scsi_inq.c *********************/
/******************************************************************************/

extern const scsi_inquiry_t g_scsi_inquiry;

/******************************************************************************/


/******************************************************************************/
/*********************** LOCAL FUNCTION DELARATIONS ***************************/
/******************************************************************************/

/**
 * @fn void service_cbw(void)
 * 
 * @brief Used to service Command Block Wrapper on MSD's Endpoint.
 * 
 * The function parses the Command Block, and responds to SCSI Commands.
 */
static void service_cbw(void);

/**
 * @fn void setup_cbw(void)
 * 
 * @brief Used to arm MSD's OUT Endpoint ready for CBW.
 * 
 * The function arms MSD's OUT Endpoint ready for CBW.
 */
static void setup_cbw(void);

/**
 * @fn void setup_csw(void)
 * 
 * @brief Used to service Command Status Wrapper on MSD's IN Endpoint.
 * 
 * The function loads the Status data and arms MSD's IN Endpoint for the CSW.
 */
static void setup_csw(void);

/**
 * @fn void service_read10(void)
 * 
 * @brief Services the READ_10 SCSI Command.
 */
static void service_read10(void);

/**
 * @fn void service_write10(void)
 * 
 * @brief Services the WRITE_10 SCSI Command.
 */
static void service_write10(void);

/**
 * @fn bool check_13_cases(uint32_t device_bytes, uint8_t dev_expect)
 * 
 * @brief Checks 13 possible error/non-error cases.
 * 
 * @param device_bytes The amount of bytes the device wishes to send.
 * @param dev_expect The direction the device expects data to transfer.
 * @return pass_fail True if COMMAND_PASSED, false if PHASE_ERROR.
 * 
 * <b>Code Example:</b>
 * <ul style="list-style-type:none"><li>
 * @code
 * check_13_cases(g_msd_rw_10_vars.TF_LEN_IN_BYTES, Di);
 * @endcode
 * </li></ul>
 */
static bool check_13_cases(uint32_t device_bytes, uint8_t dev_expect);

/**
 * @fn bool cbw_valid(void)
 * 
 * @brief Checks if CBW is valid.
 * 
 * @return valid Returns true if valid.
 */
static bool cbw_valid(void);

/**
 * @fn void fail_command(void)
 * 
 * @brief Response to a failed SCSI command.
 * 
 * If the host is expecting no data transfer (Hn), ready the CSW.
 * The function will stall either MSD's IN Endpoint or OUT depending on data 
 * direction. A COMMAND_FAILED status is returned in the CSW stage.
 * 
 * <b>Code Example:</b>
 * <ul style="list-style-type:none"><li>
 * @code
 * fail_command();
 * @endcode
 * </li></ul>
 */
static void fail_command(void);

/**
 * @fn void send_data_response(uint8_t device_bytes)
 * 
 * @brief Handles SCSI commands where data in stage is expected and less than
 * MSD_EP_SIZE in size. Basically all commands expecting data except READ_10
 * and WRITE_10.
 * 
 * @param device_bytes The amount of bytes the device wishes to send.
 */
static void send_data_response(uint8_t device_bytes);

/**
 * @fn void cause_bomsr(void)
 * 
 * @brief Causes a Bulk Only Mass Storage Reset.
 * 
 * The function stalls all MSD's Endpoints. The function is used when a serious 
 * error has occurred and the devices wishes to re-sync with the computer.
 */
static void cause_bomsr(void);

/**
 * @fn void invalid_command_sense(void)
 * 
 * @brief Sets the sense values for common Illegal Request.
 */
static void invalid_command_sense(void);

/**
 * @fn void media_not_present_sense(void)
 * 
 * @brief Sets the sense values for condition when media isn't present.
 */
static void media_not_present_sense(void);

/**
 * @fn void unit_attention_sense(void)
 * 
 * @brief Sets the sense values for Unit Attention.
 */
static void unit_attention_sense(void);

/**
 * @fn bool check_for_media(void)
 * 
 * @brief Checks to see if media is present/available. Also sets 
 * m_unit_attention value to true when the media availability changes.
 * 
 * @return Returns true when media is present.
 */
static bool check_for_media(void);

/******************************************************************************/


/******************************************************************************/
/****************************** MSD FUNCTIONS *********************************/
/******************************************************************************/

#if PINGPONG_MODE == PINGPONG_1_15 || PINGPONG_MODE == PINGPONG_ALL_EP
void msd_arm_ep_out(uint8_t bdt_index)
{
    if(g_usb_ep_stat[MSD_EP][OUT].Data_Toggle_Val) g_usb_bd_table[bdt_index].STAT = _DTSEN | _DTS; // DATA1
    else g_usb_bd_table[bdt_index].STAT = _DTSEN; // DATA0
    g_usb_bd_table[bdt_index].CNT       = MSD_EP_SIZE;
    g_usb_bd_table[bdt_index].STAT     |= _UOWN;
}

void msd_arm_ep_in(uint8_t bdt_index, uint16_t cnt)
{
    if(g_usb_ep_stat[MSD_EP][IN].Data_Toggle_Val) g_usb_bd_table[bdt_index].STAT = _DTSEN | _DTS; // DATA1
    else g_usb_bd_table[bdt_index].STAT = _DTSEN; // DATA0
    g_usb_bd_table[bdt_index].CNT       = cnt;
    g_usb_bd_table[bdt_index].STAT     |= _UOWN;
}

#else
void msd_arm_ep_out(void)
{
    if(g_usb_ep_stat[MSD_EP][OUT].Data_Toggle_Val) g_usb_bd_table[MSD_BD_OUT].STAT = _DTSEN | _DTS; // DATA1
    else g_usb_bd_table[MSD_BD_OUT].STAT = _DTSEN; // DATA0
    g_usb_bd_table[MSD_BD_OUT].CNT       = MSD_EP_SIZE;
    g_usb_bd_table[MSD_BD_OUT].STAT     |= _UOWN;
}

void msd_arm_ep_in(uint16_t cnt)
{
    if(g_usb_ep_stat[MSD_EP][IN].Data_Toggle_Val) g_usb_bd_table[MSD_BD_IN].STAT = _DTSEN | _DTS; // DATA1
    else g_usb_bd_table[MSD_BD_IN].STAT = _DTSEN; // DATA0
    g_usb_bd_table[MSD_BD_IN].CNT       = cnt;
    g_usb_bd_table[MSD_BD_IN].STAT     |= _UOWN;
}
#endif

void msd_stall_ep_out(void)
{
    #if PINGPONG_MODE == PINGPONG_1_15 || PINGPONG_MODE == PINGPONG_ALL_EP
    g_usb_ep_stat[MSD_EP][OUT].Halt  = 1;
    usb_stall_ep(&g_usb_bd_table[MSD_BD_OUT_EVEN]);
    usb_stall_ep(&g_usb_bd_table[MSD_BD_OUT_ODD]);
    #else
    g_usb_ep_stat[MSD_EP][OUT].Halt  = 1;
    usb_stall_ep(&g_usb_bd_table[MSD_BD_OUT]);
    #endif
}

void msd_stall_ep_in(void)
{
    #if PINGPONG_MODE == PINGPONG_1_15 || PINGPONG_MODE == PINGPONG_ALL_EP
    g_usb_ep_stat[MSD_EP][IN].Halt  = 1;
    usb_stall_ep(&g_usb_bd_table[MSD_BD_IN_EVEN]);
    usb_stall_ep(&g_usb_bd_table[MSD_BD_IN_ODD]);
    #else
    g_usb_ep_stat[MSD_EP][IN].Halt  = 1;
    usb_stall_ep(&g_usb_bd_table[MSD_BD_IN]);
    #endif
}


bool msd_class_request(void)
{   
    if(g_usb_setup.bRequest == BOMSR) // Bulk Only Mass Storage Reset
    {
        if(g_usb_setup.wValue != 0 || g_usb_setup.wIndex != 0 || g_usb_setup.wLength != 0) return false;
        #if PINGPONG_MODE == PINGPONG_1_15 || PINGPONG_MODE == PINGPONG_ALL_EP
        if(MSD_EP_OUT_LAST_PPB == ODD && g_usb_bd_table[MSD_BD_OUT_EVEN].STATbits.UOWN == 0) setup_cbw();
        else if (MSD_EP_OUT_LAST_PPB == EVEN && g_usb_bd_table[MSD_BD_OUT_ODD].STATbits.UOWN == 0) setup_cbw();
        #else
        if(!g_usb_bd_table[MSD_BD_OUT].STATbits.UOWN) setup_cbw();
        #endif
        m_task_cnt       = 0;
        m_task_put_index = 0;
        m_task_get_index = 0;
        
        m_wait_for_bomsr = false;
        m_unit_attention = false;
        usb_arm_in_status();
        usb_set_control_stage(STATUS_IN_STAGE);
    }
    else
    {
        // Only supported class request is BOMSR, GET_MAX_LUN is not supported
        return false;
    }
    return true;
}


void msd_init(void)
{
    #if PINGPONG_MODE == PINGPONG_1_15 || PINGPONG_MODE == PINGPONG_ALL_EP
    // BD settings
    g_usb_bd_table[MSD_BD_OUT_EVEN].STAT = 0;
    g_usb_bd_table[MSD_BD_OUT_EVEN].ADR  = (uint16_t)g_msd_ep_out_even;
    g_usb_bd_table[MSD_BD_OUT_ODD].STAT  = 0;
    g_usb_bd_table[MSD_BD_OUT_ODD].ADR   = (uint16_t)g_msd_ep_out_odd;
    g_usb_bd_table[MSD_BD_IN_EVEN].STAT  = 0;
    g_usb_bd_table[MSD_BD_IN_EVEN].ADR   = (uint16_t)g_msd_ep_in_even;
    g_usb_bd_table[MSD_BD_IN_ODD].STAT   = 0;
    g_usb_bd_table[MSD_BD_IN_ODD].ADR    = (uint16_t)g_msd_ep_in_odd;
    #else
    // BD settings
    g_usb_bd_table[MSD_BD_OUT].STAT = 0;
    g_usb_bd_table[MSD_BD_OUT].ADR  = (uint16_t)g_msd_ep_out;
    g_usb_bd_table[MSD_BD_IN].STAT  = 0;
    g_usb_bd_table[MSD_BD_IN].ADR   = (uint16_t)g_msd_ep_in;
    #endif

    // EP Settings
    MSD_UEPbits.EPHSHK   = 1; // Handshaking enabled 
    MSD_UEPbits.EPCONDIS = 1; // Don't allow SETUP
    MSD_UEPbits.EPOUTEN  = 1; // EP output enabled
    MSD_UEPbits.EPINEN   = 1; // EP input enabled
    
    g_usb_ep_stat[MSD_EP][OUT].Halt = 0;
    g_usb_ep_stat[MSD_EP][IN].Halt  = 0;
    msd_clear_ep_toggle();
    
    m_wait_for_bomsr    = false;
    m_unit_attention    = false;
    m_end_data_short    = false;
    m_clear_halt_event  = false;
    
    m_task_cnt       = 0;
    m_task_put_index = 0;
    m_task_get_index = 0;
    
    setup_cbw();
}


void msd_add_task(void)
{
    if(m_task_cnt < 4)
    {
        m_tasks_buff.task[m_task_put_index++] = *((uint8_t*)&g_usb_last_USTAT);
        if(m_task_put_index == 4) m_task_put_index = 0;
        m_task_cnt++;
    }
}


void msd_tasks(void)
{
    USB_INTERRUPT_ENABLE = 0;
    if(m_task_cnt)
    {
        if(MSD_TRANSACTION_DIR == OUT)
        {
            MSD_EP_OUT_DATA_TOGGLE_VAL ^= 1;
            #if PINGPONG_MODE == PINGPONG_1_15 || PINGPONG_MODE == PINGPONG_ALL_EP
            MSD_EP_OUT_LAST_PPB = MSD_PINGPONG_PARITY;
            #endif
            switch(m_msd_state)
            {
                #ifdef USE_WRITE_10
                case MSD_WRITE_DATA:
                    service_write10();
                    break;
                #endif
                case MSD_CBW:
                    service_cbw();
                    break;
            }
        }
        else
        {
            MSD_EP_IN_DATA_TOGGLE_VAL ^= 1;
            #if PINGPONG_MODE == PINGPONG_1_15 || PINGPONG_MODE == PINGPONG_ALL_EP
            MSD_EP_IN_LAST_PPB = MSD_PINGPONG_PARITY;
            #endif
            switch(m_msd_state)
            {
                case MSD_READ_DATA:
                    service_read10();
                    break;
                #if PINGPONG_MODE == PINGPONG_1_15 || PINGPONG_MODE == PINGPONG_ALL_EP
                case MSD_READ_FINISHED:
                    m_msd_state = MSD_DATA_SENT;
                    break;
                #endif
                case MSD_DATA_SENT:
                    if(m_end_data_short)
                    {
                        msd_stall_ep_in();
                        m_end_data_short = false;
                        m_msd_state = MSD_WAIT_CLEAR;
                        break;
                    }
                    setup_csw();
                    break;
                case MSD_CSW:
                    setup_cbw();
                    break;
            }
        }
        m_task_get_index++;
        if(m_task_get_index == 4) m_task_get_index = 0;
        m_task_cnt--;
    }
    else if(m_clear_halt_event)
    {
        if(m_msd_state == MSD_WAIT_BOMSR) setup_cbw();
        else if(m_msd_state == MSD_WAIT_CLEAR) setup_csw();
        m_clear_halt_event = false;
    }
    USB_INTERRUPT_ENABLE = 1;
}


void msd_clear_halt(uint8_t bdt_index, uint8_t ep, uint8_t dir)
{
    if(!m_wait_for_bomsr)
    {
        g_usb_ep_stat[ep][dir].Data_Toggle_Val = 0;
        if(g_usb_ep_stat[ep][dir].Halt)
        {
            g_usb_ep_stat[ep][dir].Halt      = 0;
            g_usb_bd_table[bdt_index].STAT   = 0;
            #if PINGPONG_MODE == PINGPONG_1_15 || PINGPONG_MODE == PINGPONG_ALL_EP
            g_usb_bd_table[++bdt_index].STAT = 0;
            #endif
        }
        m_clear_halt_event = true;
    }
}


void msd_clear_ep_toggle(void)
{
    MSD_EP_OUT_DATA_TOGGLE_VAL = 0;
    MSD_EP_IN_DATA_TOGGLE_VAL  = 0;
}


static void service_cbw(void)
{
    uint8_t  dev_expect;

    #if PINGPONG_MODE == PINGPONG_1_15 || PINGPONG_MODE == PINGPONG_ALL_EP
    uint8_t *in_ep_addr;
    
    if(MSD_EP_IN_LAST_PPB == ODD)
    {
        in_ep_addr = g_msd_ep_in_even;
    }
    else
    {
        in_ep_addr = g_msd_ep_in_odd;
    }
    
    if(MSD_EP_OUT_LAST_PPB == ODD) usb_ram_copy(g_msd_ep_out_odd, g_msd_cbw.BYTES, 31);
    else usb_ram_copy(g_msd_ep_out_even, g_msd_cbw.BYTES, 31);
    #else
    usb_ram_copy(g_msd_ep_out, g_msd_cbw.BYTES, 31);
    #endif
    
    if(!cbw_valid()) return;
    
    switch(g_msd_cbw.CBWCB0[0])
    {
        case WRITE_10:
            #if defined(USE_WRITE_10) && defined(USE_WR_PROTECT)
            if(msd_wr_protect())
            {
            #endif
                #if !defined(USE_WRITE_10) || defined(USE_WR_PROTECT)
                g_msd_sense_key                       = DATA_PROTECT;
                g_msd_additional_sense_code           = ASC_WRITE_PROTECTED;
                g_msd_additional_sense_code_qualifier = ASCQ_WRITE_PROTECTED;
                fail_command();
                return;
                #endif
            #if defined(USE_WRITE_10) && defined(USE_WR_PROTECT)
            }
            #endif
        case READ_10:
            #ifdef USE_WRITE_10
            if(g_msd_cbw.CBWCB0[0] == READ_10) dev_expect = Di;	
            else dev_expect = Do;	
            #else	
            dev_expect = Di;	
            #endif
            
            #ifdef USE_EXTERNAL_MEDIA
            if(!check_for_media())
            {
                media_not_present_sense();
                fail_command();
                return;
            }
            #endif
            g_msd_rw_10_vars.TF_LEN_BYTES[0] = m_read_10_cmd.TF_LEN_BYTES[1];
            g_msd_rw_10_vars.TF_LEN_BYTES[1] = m_read_10_cmd.TF_LEN_BYTES[0];
            
            if(g_msd_rw_10_vars.TF_LEN == 0)
            {
                check_13_cases(0, Dn);
                return;
            }
            
            g_msd_rw_10_vars.START_LBA_BYTES[0] = m_read_10_cmd.LBA_BYTES[3];
            g_msd_rw_10_vars.START_LBA_BYTES[1] = m_read_10_cmd.LBA_BYTES[2];
            g_msd_rw_10_vars.START_LBA_BYTES[2] = m_read_10_cmd.LBA_BYTES[1];
            g_msd_rw_10_vars.START_LBA_BYTES[3] = m_read_10_cmd.LBA_BYTES[0];
            g_msd_rw_10_vars.LBA = g_msd_rw_10_vars.START_LBA;
            
            if((g_msd_rw_10_vars.LBA + g_msd_rw_10_vars.TF_LEN) > VOL_CAPACITY_IN_BLOCKS)
            {
                g_msd_sense_key                       = ILLEGAL_REQUEST;
                g_msd_additional_sense_code           = ASC_LOGICAL_BLOCK_ADDRESS_OUT_OF_RANGE;
                g_msd_additional_sense_code_qualifier = ASCQ_LOGICAL_BLOCK_ADDRESS_OUT_OF_RANGE;
                fail_command();
                return;
            }
            
            g_msd_rw_10_vars.TF_LEN_IN_BYTES = ((uint32_t)g_msd_rw_10_vars.TF_LEN)*BYTES_PER_BLOCK_LE;
            
            if(!check_13_cases(g_msd_rw_10_vars.TF_LEN_IN_BYTES, dev_expect)) return;
            
            g_msd_byte_of_sect = 0;
            
            #ifdef USE_WRITE_10
            if(dev_expect == Do)
            {
                #if PINGPONG_MODE == PINGPONG_1_15 || PINGPONG_MODE == PINGPONG_ALL_EP
                msd_arm_ep_out((uint8_t)MSD_BD_OUT_EVEN + (MSD_EP_OUT_LAST_PPB ^ 1));
                MSD_EP_OUT_DATA_TOGGLE_VAL ^= 1;
                msd_arm_ep_out((uint8_t)MSD_BD_OUT_EVEN + MSD_EP_OUT_LAST_PPB);
                #else
                msd_arm_ep_out();
                #endif
                m_msd_state = MSD_WRITE_DATA;
                return;
            }
            #endif
            #ifdef MSD_LIMITED_RAM
            #if PINGPONG_MODE == PINGPONG_1_15 || PINGPONG_MODE == PINGPONG_ALL_EP
            MSD_EP_IN_LAST_PPB ^= 1;
            service_read10();

            MSD_EP_IN_DATA_TOGGLE_VAL ^= 1;
            MSD_EP_IN_LAST_PPB ^= 1;
            service_read10();
            #else
            service_read10();
            #endif
            #else
            #if PINGPONG_MODE == PINGPONG_1_15 || PINGPONG_MODE == PINGPONG_ALL_EP
            MSD_EP_IN_LAST_PPB ^= 1;
            msd_rx_sector();
            service_read10();

            MSD_EP_IN_DATA_TOGGLE_VAL ^= 1;
            MSD_EP_IN_LAST_PPB ^= 1;
            service_read10();
            #else
            msd_rx_sector();
            service_read10();
            #endif
            #endif
            m_msd_state = MSD_READ_DATA;
            break;
            
        case TEST_UNIT_READY:
            #ifdef USE_EXTERNAL_MEDIA
            m_media_present = check_for_media();
            if(m_unit_attention)
            {
                m_unit_attention = false;
                unit_attention_sense();
                fail_command();
                return;
            }
            
            if(!m_media_present)
            {
                media_not_present_sense();
                fail_command();
                return;
            }
            #else
            if(m_unit_attention)
            {
                m_unit_attention = false;
                unit_attention_sense();
                fail_command();
                return;
            }    
            #endif
            
            #ifdef USE_TEST_UNIT_READY
            if(check_13_cases(0, Dn) && msd_test_unit_ready()) fail_command();
            #else
            check_13_cases(0, Dn);
            #endif
            break;
            
            
        #ifdef USE_PREVENT_ALLOW_MEDIUM_REMOVAL
        case PREVENT_ALLOW_MEDIUM_REMOVAL:
            #ifdef USE_EXTERNAL_MEDIA
            if(!check_for_media())
            {
                media_not_present_sense();
                fail_command();
                return;
            }
            #endif
            invalid_command_sense();
            fail_command();
            break;
        #endif
            
            
        case REQUEST_SENSE:
            g_msd_bytes_to_transfer.LB = m_request_sense_cmd.ALLOCATION_LENGTH;
            g_msd_bytes_to_transfer.HB = 0;
            if(g_msd_bytes_to_transfer.val)
            {
                if(g_msd_bytes_to_transfer.val > 18) g_msd_bytes_to_transfer.val = 18;
                
                #if PINGPONG_MODE == PINGPONG_1_15 || PINGPONG_MODE == PINGPONG_ALL_EP
                usb_ram_set(0, in_ep_addr, g_msd_bytes_to_transfer.val);
                
                in_ep_addr[0] = CURRENT_FIXED; // RESPONSE_CODE
                in_ep_addr[2] = g_msd_sense_key;
                in_ep_addr[7] = 10; // ADDITIONAL_SENSE_LENGTH
                in_ep_addr[12] = g_msd_additional_sense_code;
                in_ep_addr[13] = g_msd_additional_sense_code_qualifier;
                #else
                usb_ram_set(0, g_msd_ep_in, g_msd_bytes_to_transfer.val);
                
                g_msd_ep_in[0] = CURRENT_FIXED; // RESPONSE_CODE
                g_msd_ep_in[2] = g_msd_sense_key;
                g_msd_ep_in[7] = 10; // ADDITIONAL_SENSE_LENGTH
                g_msd_ep_in[12] = g_msd_additional_sense_code;
                g_msd_ep_in[13] = g_msd_additional_sense_code_qualifier;
                #endif
                send_data_response(g_msd_bytes_to_transfer.val);
                return;
            }
            check_13_cases(0, Dn);
            break;
            
            
        case INQUIRY:
            g_msd_bytes_to_transfer.LB = m_inquiry_cmd.ALLOCATION_LENGTH_BYTES[1];
            g_msd_bytes_to_transfer.HB = m_inquiry_cmd.ALLOCATION_LENGTH_BYTES[0];
            
            if(g_msd_bytes_to_transfer.val)
            {
                if(g_msd_bytes_to_transfer.val > 36) g_msd_bytes_to_transfer.val = 36;
                #if PINGPONG_MODE == PINGPONG_1_15 || PINGPONG_MODE == PINGPONG_ALL_EP
                usb_rom_copy((const uint8_t*)&g_scsi_inquiry, in_ep_addr, g_msd_bytes_to_transfer.val);
                #else
                usb_rom_copy((const uint8_t*)&g_scsi_inquiry, g_msd_ep_in, g_msd_bytes_to_transfer.val);
                #endif
                send_data_response(g_msd_bytes_to_transfer.val);
                return;
            } 
            check_13_cases(0, Dn);
            break;
            
            
        case MODE_SENSE_6:
            g_msd_bytes_to_transfer.LB = m_mode_sense_6_cmd.ALLOCATION_LENGTH;
            g_msd_bytes_to_transfer.HB = 0;
            
            #ifdef USE_EXTERNAL_MEDIA
            if(!check_for_media())
            {
                media_not_present_sense();
                fail_command();
                return;
            }
            #endif
            
            if(g_msd_bytes_to_transfer.val)
            {
                if(g_msd_bytes_to_transfer.val > 4) g_msd_bytes_to_transfer.val = 4;
                g_msd_mode_sense.MODE_DATA_LENGTH          = 0x03;
                g_msd_mode_sense.MEDIUM_TYPE               = 0x00;
                g_msd_mode_sense.DEVICE_SPECIFIC_PARAMETER = 0x00; // 0x00 for R/W, 0x80 for R-only
                g_msd_mode_sense.BLOCK_DESCRIPTOR_LENGTH   = 0x00;
                #if PINGPONG_MODE == PINGPONG_1_15 || PINGPONG_MODE == PINGPONG_ALL_EP
                usb_ram_copy((uint8_t*)&g_msd_mode_sense, in_ep_addr, g_msd_bytes_to_transfer.val);
                #endif
                send_data_response(g_msd_bytes_to_transfer.val);
                return;
            }
            check_13_cases(0, Dn);
            break;
            
        #ifdef USE_START_STOP_UNIT
        case START_STOP_UNIT:
            #ifdef USE_EXTERNAL_MEDIA
            if(!check_for_media())
            {
                media_not_present_sense();
                fail_command();
                return;
            }
            #endif
            if(check_13_cases(0, Dn) && msd_start_stop_unit())
            {
                fail_command();
                return;
            }
            break;
        #endif
            
            
        case READ_CAPACITY:
            #ifdef USE_EXTERNAL_MEDIA
            if(!check_for_media())
            {
                media_not_present_sense();
                fail_command();
                return;
            }
            #endif
            if((m_read_capacity_10_cmd.LOGICAL_BLOCK_ADDRESS != 0)&&(m_read_capacity_10_cmd.PMI == 0))
            {
                g_msd_sense_key                       = ILLEGAL_REQUEST;
                g_msd_additional_sense_code           = ASC_INVALID_FIELD_IN_CBD;
                g_msd_additional_sense_code_qualifier = ASCQ_INVALID_FIELD_IN_CBD;
                fail_command();
                return;
            }
            g_msd_rw_10_vars.START_LBA_BYTES[0] = m_read_capacity_10_cmd.LOGICAL_BLOCK_ADDRESS_BYTES[3];// Big-endian to little-endian
            g_msd_rw_10_vars.START_LBA_BYTES[1] = m_read_capacity_10_cmd.LOGICAL_BLOCK_ADDRESS_BYTES[2];
            g_msd_rw_10_vars.START_LBA_BYTES[2] = m_read_capacity_10_cmd.LOGICAL_BLOCK_ADDRESS_BYTES[1];
            g_msd_rw_10_vars.START_LBA_BYTES[3] = m_read_capacity_10_cmd.LOGICAL_BLOCK_ADDRESS_BYTES[0];

            g_msd_rw_10_vars.LBA = g_msd_rw_10_vars.START_LBA;

            #ifdef USE_READ_CAPACITY
            msd_read_capacity();
            #else
            if(g_msd_rw_10_vars.START_LBA > LAST_BLOCK_LE)
            {
                g_msd_read_capacity_10.RETURNED_LOGICAL_BLOCK_ADDRESS = 0xFFFFFFFFUL;
            }
            else
            {
                g_msd_read_capacity_10.RETURNED_LOGICAL_BLOCK_ADDRESS = LAST_BLOCK_BE;
            }
            g_msd_read_capacity_10.BLOCK_LENGTH_IN_BYTES = BYTES_PER_BLOCK_BE;
            #endif
            #if PINGPONG_MODE == PINGPONG_1_15 || PINGPONG_MODE == PINGPONG_ALL_EP
            usb_ram_copy((uint8_t*)&g_msd_read_capacity_10, in_ep_addr, 8);
            #endif
            send_data_response(8);
            break;
            
        #ifdef USE_VERIFY_10
        case VERIFY_10:
            #ifdef USE_EXTERNAL_MEDIA
            if(!check_for_media())
            {
                media_not_present_sense();
                fail_command();
                return;
            }
            #endif
            check_13_cases(0, Dn);
            break;
        #endif
        default:
            invalid_command_sense();
            fail_command();
            break;
    }
}


static void setup_cbw(void)
{
    #if PINGPONG_MODE == PINGPONG_1_15 || PINGPONG_MODE == PINGPONG_ALL_EP
    msd_arm_ep_out((uint8_t)MSD_BD_OUT_EVEN + (MSD_EP_OUT_LAST_PPB ^ 1));
    #else
    msd_arm_ep_out();
    #endif
    m_msd_state = MSD_CBW;
}


static void setup_csw(void)
{
    g_msd_csw.BYTES[3] = 'S';
    #if PINGPONG_MODE == PINGPONG_1_15 || PINGPONG_MODE == PINGPONG_ALL_EP
    if(MSD_EP_IN_LAST_PPB == ODD)
    {
        usb_ram_copy(g_msd_csw.BYTES, g_msd_ep_in_even, 13);
        msd_arm_ep_in(MSD_BD_IN_EVEN, 13);
    }
    else
    {
        usb_ram_copy(g_msd_csw.BYTES, g_msd_ep_in_odd, 13);
        msd_arm_ep_in(MSD_BD_IN_ODD, 13);
    }
    #else
    usb_ram_copy(g_msd_csw.BYTES, g_msd_ep_in, 13);
    msd_arm_ep_in(13);
    #endif
    m_msd_state = MSD_CSW;
}


static bool check_13_cases(uint32_t device_bytes, uint8_t dev_expect)
{
// Chapter 6.7 of MSC BOT Spec 1.0
// ------ CASE ACTIONS ------
//CASE_1 (Hn = Dn):
//    - bCSWStatus = COMMAND_PASSED.
//    - dCSWDataResidue = 0.
//    - setup_csw.
//
//CASE_2 (Hn < Di) | CASE_3 (Hn < Do):
//    - bCSWStatus = PHASE_ERROR.
//    - dCSWDataResidue is ignored by host.
//    - stall_ep_in, once EP is cleared setup_csw.
//
//CASE_4 (Hi > Dn) | CASE_5 (Hi > Di):
//    - bCSWStatus = COMMAND_PASSED.
//    - Send available data.
//    - dCSWDataResidue = dCBWDataTransferLength - data_sent.
//    - stall_ep_in, once EP is cleared setup_csw.
//
//CASE_6 (Hi = Di):
//    - bCSWStatus = COMMAND_PASSED.
//    - Send all data.
//    - dCSWDataResidue = 0.
//    - setup_csw.
//    
//CASE_7 (Hi < Di) | CASE_8 (Hi <> Do):
//    - bCSWStatus = PHASE_ERROR.
//    - dCSWDataResidue is ignored by host.
//    - stall_ep_in, once EP is cleared setup_csw.
//    
//CASE_9 (Ho > Dn) | CASE_11 (Ho > Do) | CASE_12 (Ho = Do):
//    - bCSWStatus = COMMAND_PASSED.
//    - Send/receive available data.
//    - dCSWDataResidue = dCBWDataTransferLength - data_sent.
//    - if CASE_9 or CASE_11 stall_ep_out.
//    - setup_csw (if stalled, once cleared).
//	
//CASE_10 (Ho <> Di) | CASE_13 (Ho < Do):
//    - bCSWStatus = PHASE_ERROR.
//    - dCSWDataResidue is ignored by host.
//    - stall_ep_out, once EP is cleared setup_csw.
    
    if(dev_expect == Dn)
    {
        if(g_msd_cbw.dCBWDataTransferLength == 0)
        {
            setup_csw();
            goto command_passed;
        }
        else if(g_msd_cbw.Direction == IN) msd_stall_ep_in();
        else                               msd_stall_ep_out();
        m_msd_state = MSD_WAIT_CLEAR;
        goto command_passed;
    }
    if(g_msd_cbw.Direction == IN && dev_expect == Do)   goto phase_error;
    if(g_msd_cbw.Direction == OUT && dev_expect == Di)  goto phase_error;
    if(device_bytes > g_msd_cbw.dCBWDataTransferLength) goto phase_error;
    if(device_bytes < g_msd_cbw.dCBWDataTransferLength) m_end_data_short = true;
    
    command_passed:
    g_msd_csw.bCSWStatus = COMMAND_PASSED;
    return true;
    
    phase_error:
    cause_bomsr();
    g_msd_csw.bCSWStatus = PHASE_ERROR;
    return false;
}


static bool cbw_valid(void)
{
    #if PINGPONG_MODE == PINGPONG_1_15 || PINGPONG_MODE == PINGPONG_ALL_EP
    if(MSD_EP_OUT_LAST_PPB == ODD)
    {
        if(g_usb_bd_table[MSD_BD_OUT_ODD].CNT != 31) goto cbw_not_valid;
    }
    else
    {
        if(g_usb_bd_table[MSD_BD_OUT_EVEN].CNT != 31) goto cbw_not_valid;
    }
    #else
    if(g_usb_bd_table[MSD_BD_OUT].CNT != 31) goto cbw_not_valid;
    #endif
    if(g_msd_cbw.dCBWSignature != CBW_SIG) goto cbw_not_valid;
    return true;
    
    cbw_not_valid:
    m_wait_for_bomsr = true;
    cause_bomsr();
    m_msd_state = MSD_WAIT_BOMSR;
    return false;
}


static void cause_bomsr(void)
{
    msd_stall_ep_out();
    msd_stall_ep_in();
    m_msd_state = MSD_WAIT_CLEAR;
}


static void fail_command(void)
{
    if(g_msd_cbw.dCBWDataTransferLength == 0) // Hn
    {
        g_msd_csw.bCSWStatus = COMMAND_FAILED;
        setup_csw();
    }
    else 
    {
        if(g_msd_cbw.Direction) msd_stall_ep_in();  // Hi
        else                    msd_stall_ep_out(); // Ho
        g_msd_csw.bCSWStatus = COMMAND_FAILED;
        m_msd_state = MSD_WAIT_CLEAR;
    }
}


static void send_data_response(uint8_t device_bytes)
{
    if(!check_13_cases((uint32_t)device_bytes, Di)) return;
    
    g_msd_csw.dCSWDataResidue -= device_bytes;
    
    #if PINGPONG_MODE == PINGPONG_1_15 || PINGPONG_MODE == PINGPONG_ALL_EP
    msd_arm_ep_in((uint8_t)MSD_BD_IN_EVEN + (MSD_EP_IN_LAST_PPB ^ 1), device_bytes);
    #else
    msd_arm_ep_in(device_bytes);
    #endif
    
    m_msd_state = MSD_DATA_SENT;
}


static void service_read10(void)
{
    #if PINGPONG_MODE == PINGPONG_1_15 || PINGPONG_MODE == PINGPONG_ALL_EP
    #ifndef MSD_LIMITED_RAM
    uint8_t *ep_address;
    #endif
    uint8_t bdt_index;
    
    if(MSD_EP_IN_LAST_PPB == ODD)
    {
        #ifndef MSD_LIMITED_RAM
        ep_address = g_msd_ep_in_odd;
        #endif
        bdt_index = MSD_BD_IN_ODD;
    }
    else
    {
        #ifndef MSD_LIMITED_RAM
        ep_address = g_msd_ep_in_even;
        #endif
        bdt_index = MSD_BD_IN_EVEN;
    }
    
    #ifdef MSD_LIMITED_RAM
    msd_rx_sector();
    #else
    usb_ram_copy(g_msd_sect_data + g_msd_byte_of_sect, ep_address, MSD_EP_SIZE); // Load EP size worth of data from the g_msd_sect_data buffer.
    #endif
    
    g_msd_byte_of_sect += MSD_EP_SIZE;
    if(g_msd_byte_of_sect == BYTES_PER_BLOCK_LE) // More than one sector is required. Last bytes of sector were sent, increment the address, and load new sector.
    {
        g_msd_rw_10_vars.LBA++;
        #ifndef MSD_LIMITED_RAM
        msd_rx_sector();
        #endif
        g_msd_byte_of_sect = 0;
    }
    
    msd_arm_ep_in(bdt_index, MSD_EP_SIZE);
    
    g_msd_rw_10_vars.TF_LEN_IN_BYTES -= MSD_EP_SIZE;
    g_msd_csw.dCSWDataResidue -= MSD_EP_SIZE;
    
    if(g_msd_rw_10_vars.TF_LEN_IN_BYTES == 0)
    {
        MSD_EP_IN_DATA_TOGGLE_VAL ^= 1;
        m_msd_state = MSD_READ_FINISHED;
    }
    return;
    
    #else
    #ifdef MSD_LIMITED_RAM
    msd_rx_sector();
    #else
    usb_ram_copy(g_msd_sect_data + g_msd_byte_of_sect, g_msd_ep_in, MSD_EP_SIZE); // Load EP size worth of data from the g_msd_sect_data buffer.
    #endif

    g_msd_byte_of_sect += MSD_EP_SIZE;
    if(g_msd_byte_of_sect == BYTES_PER_BLOCK_LE) // More than one sector is required. Last bytes of sector were sent, increment the address, and load new sector.
    {
        g_msd_rw_10_vars.LBA++;
        #ifndef MSD_LIMITED_RAM
        msd_rx_sector();
        #endif
        g_msd_byte_of_sect = 0;
    }

    msd_arm_ep_in(MSD_EP_SIZE);

    g_msd_rw_10_vars.TF_LEN_IN_BYTES -= MSD_EP_SIZE;
    g_msd_csw.dCSWDataResidue -= MSD_EP_SIZE;
    
    if(g_msd_rw_10_vars.TF_LEN_IN_BYTES == 0) m_msd_state = MSD_DATA_SENT;
    return;
    #endif
}


static void service_write10(void)
{
    #if PINGPONG_MODE == PINGPONG_1_15 || PINGPONG_MODE == PINGPONG_ALL_EP
    #ifndef MSD_LIMITED_RAM
    uint8_t *ep_address;
    
    if(MSD_EP_OUT_LAST_PPB == ODD) ep_address = g_msd_ep_out_odd;
    else ep_address = g_msd_ep_out_even;
    #endif

    #ifdef MSD_LIMITED_RAM
    msd_tx_sector();
    #else
    usb_ram_copy(ep_address, g_msd_sect_data + g_msd_byte_of_sect, MSD_EP_SIZE); // Load EP size worth of data from EP to g_msd_sect_data buffer.
    #endif
    g_msd_byte_of_sect += MSD_EP_SIZE;
    if(g_msd_byte_of_sect == BYTES_PER_BLOCK_LE){
        #ifndef MSD_LIMITED_RAM
        msd_tx_sector();
        #endif
        g_msd_rw_10_vars.LBA++;
        g_msd_byte_of_sect = 0;
    }

    g_msd_rw_10_vars.TF_LEN_IN_BYTES -= MSD_EP_SIZE;
    g_msd_csw.dCSWDataResidue -= MSD_EP_SIZE;
    
    if(g_msd_rw_10_vars.TF_LEN_IN_BYTES == 0)
    {
        MSD_EP_OUT_DATA_TOGGLE_VAL ^= 1;
        if(m_end_data_short)
        {
            msd_stall_ep_out();
            m_end_data_short = false;
            m_msd_state = MSD_WAIT_CLEAR;
        }
        else setup_csw();
    }
    else
    {
        msd_arm_ep_out((uint8_t)MSD_BD_OUT_EVEN + MSD_EP_OUT_LAST_PPB);
    }
    
    #else
    #ifdef MSD_LIMITED_RAM
    msd_tx_sector();
    #else
    usb_ram_copy(g_msd_ep_out, g_msd_sect_data + g_msd_byte_of_sect, MSD_EP_SIZE); // Load EP size worth of data from EP to g_msd_sect_data buffer.
    #endif
    g_msd_byte_of_sect += MSD_EP_SIZE;
    if(g_msd_byte_of_sect == BYTES_PER_BLOCK_LE){
        #ifndef MSD_LIMITED_RAM
        msd_tx_sector();
        #endif
        g_msd_rw_10_vars.LBA++;
        g_msd_byte_of_sect = 0;
    }
    g_msd_rw_10_vars.TF_LEN_IN_BYTES -= MSD_EP_SIZE;
    g_msd_csw.dCSWDataResidue -= MSD_EP_SIZE;
    
    if(g_msd_rw_10_vars.TF_LEN_IN_BYTES == 0)
    {
        if(m_end_data_short)
        {
            msd_stall_ep_out();
            m_end_data_short = false;
            m_msd_state = MSD_WAIT_CLEAR;
        }
        else setup_csw();
    }
    else msd_arm_ep_out();
    #endif
}


static void invalid_command_sense(void)
{
    g_msd_sense_key                       = ILLEGAL_REQUEST;
    g_msd_additional_sense_code           = ASC_INVALID_COMMAND_OPCODE;
    g_msd_additional_sense_code_qualifier = ASCQ_INVALID_COMMAND_OPCODE;
}


static void media_not_present_sense(void)
{
    g_msd_sense_key                       = NOT_READY;
    g_msd_additional_sense_code           = ASC_MEDIUM_NOT_PRESENT;
    g_msd_additional_sense_code_qualifier = ASCQ_MEDIUM_NOT_PRESENT;
}


static void unit_attention_sense(void)
{
    g_msd_sense_key                       = UNIT_ATTENTION;
    g_msd_additional_sense_code           = ASC_NOT_READY_TO_READY_CHANGE;
    g_msd_additional_sense_code_qualifier = ASCQ_MEDIUM_MAY_HAVE_CHANGED;
}

#ifdef USE_EXTERNAL_MEDIA
static bool check_for_media(void)
{
    static bool prev_val = false;
    bool return_val;
    
    return_val = msd_media_present();
    
    if(return_val != prev_val) m_unit_attention = true;
    
    prev_val = return_val;
    
    return return_val;
}
#endif

/******************************************************************************/