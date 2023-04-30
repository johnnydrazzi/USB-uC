/**
 * @file usb.h
 * @brief Contains definitions used by the core USB stack and declaration of global functions and variables.
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

#ifndef USB_H
#define USB_H

#include "usb_hal.h"
#include "usb_ch9.h"
#include "usb_config.h"
#include <stdint.h>
#include <stdbool.h>

/* ************************************************************************** */
/* ***************************** USB STATES ********************************* */
/* ************************************************************************** */

#define STATE_DETACHED   0
#define STATE_ATTACHED   1
#define STATE_POWERED    2
#define STATE_DEFAULT    3
#define STATE_ADDRESS    4
#define STATE_SUSPENDED  5
#define STATE_CONFIGURED 6

/* ************************************************************************** */


/* ************************************************************************** */
/* ********************* CONTROL TRANSFER STAGES **************************** */
/* ************************************************************************** */

#define SETUP_STAGE      0
#define DATA_IN_STAGE    1
#define DATA_OUT_STAGE   2
#define STATUS_IN_STAGE  3
#define STATUS_OUT_STAGE 4

/* ************************************************************************** */


/* ************************************************************************** */
/* ****************************** PIDS ************************************** */
/* ************************************************************************** */

// Token PIDs
#define PID_OUT_TOKEN    0b0001
#define PID_IN_TOKEN     0b1001
#define PID_SOF_TOKEN    0b0101
#define PID_SETUP_TOKEN  0b1101

// Data PIDs
#define PID_DATA0        0b0011
#define PID_DATA1        0b1011
#define PID_DATA2        0b0111
#define PID_MDATA        0b1111

// Handshake PIDs
#define PID_ACK_HANDSHAKE   0b0010
#define PID_NAK_HANDSHAKE   0b1010
#define PID_STALL_HANDSHAKE 0b1110
#define PID_NYET_HANDSHAKE  0b0110

// Special PIDs
#define PID_PREamble 0b1100
#define PID_ERR      0b1100
#define PID_Split    0b1000
#define PID_Ping     0b0100

/* ************************************************************************** */


/* ************************************************************************** */
/* ************************* EP0 BUFFER BASE ADDRESSES ********************** */
/* ************************************************************************** */

#ifdef _PIC14E
#warning "Control EP Buffer addresses have been manually set for PIC16 devices."
#if (PINGPONG_MODE == PINGPONG_DIS) || (PINGPONG_MODE == PINGPONG_1_15)
#if EP0_SIZE == 8
#define EP0_OUT_BUFFER_BASE_ADDR 0x21E0
#define EP0_IN_BUFFER_BASE_ADDR  0x21E8
#elif EP0_SIZE == 16
#define EP0_OUT_BUFFER_BASE_ADDR 0x21D0
#define EP0_IN_BUFFER_BASE_ADDR  0x21E0
#elif EP0_SIZE == 32
#define EP0_OUT_BUFFER_BASE_ADDR 0x21A0
#define EP0_IN_BUFFER_BASE_ADDR  0x21C0
#else //EP0_SIZE == 64
#define EP0_OUT_BUFFER_BASE_ADDR 0x2150
#define EP0_IN_BUFFER_BASE_ADDR  0x2190
#endif
#elif (PINGPONG_MODE == PINGPONG_0_OUT)
#if EP0_SIZE == 8
#define EP0_OUT_EVEN_BUFFER_BASE_ADDR 0x21D8
#define EP0_OUT_ODD_BUFFER_BASE_ADDR  0x21E0
#define EP0_IN_BUFFER_BASE_ADDR       0x21E8
#elif EP0_SIZE == 16
#define EP0_OUT_EVEN_BUFFER_BASE_ADDR 0x21C0
#define EP0_OUT_ODD_BUFFER_BASE_ADDR  0x21D0
#define EP0_IN_BUFFER_BASE_ADDR       0x21E0
#elif EP0_SIZE == 32
#define EP0_OUT_EVEN_BUFFER_BASE_ADDR 0x2170
#define EP0_OUT_ODD_BUFFER_BASE_ADDR  0x2190
#define EP0_IN_BUFFER_BASE_ADDR       0x21B0
#else //EP0_SIZE == 64
#define EP0_OUT_EVEN_BUFFER_BASE_ADDR 0x20F0
#define EP0_OUT_ODD_BUFFER_BASE_ADDR  0x2140
#define EP0_IN_BUFFER_BASE_ADDR       0x2190
#endif
#else
#if EP0_SIZE == 8
#define EP0_OUT_EVEN_BUFFER_BASE_ADDR 0x21D0
#define EP0_OUT_ODD_BUFFER_BASE_ADDR  0x21D8
#define EP0_IN_EVEN_BUFFER_BASE_ADDR  0x21E0
#define EP0_IN_ODD_BUFFER_BASE_ADDR   0x21E8
#elif EP0_SIZE == 16
#define EP0_OUT_EVEN_BUFFER_BASE_ADDR 0x21B0
#define EP0_OUT_ODD_BUFFER_BASE_ADDR  0x21C0
#define EP0_IN_EVEN_BUFFER_BASE_ADDR  0x21D0
#define EP0_IN_ODD_BUFFER_BASE_ADDR   0x21E0
#elif EP0_SIZE == 32
#define EP0_OUT_EVEN_BUFFER_BASE_ADDR 0x2150
#define EP0_OUT_ODD_BUFFER_BASE_ADDR  0x2170
#define EP0_IN_EVEN_BUFFER_BASE_ADDR  0x2190
#define EP0_IN_ODD_BUFFER_BASE_ADDR   0x21B0
#else //EP0_SIZE == 64
#define EP0_OUT_EVEN_BUFFER_BASE_ADDR 0x20A0
#define EP0_OUT_ODD_BUFFER_BASE_ADDR  0x20F0
#define EP0_IN_EVEN_BUFFER_BASE_ADDR  0x2140
#define EP0_IN_ODD_BUFFER_BASE_ADDR   0x2190
#endif
#endif
#else // PIC18 devices
#if (PINGPONG_MODE == PINGPONG_DIS)
#define EP0_BUFFER_BASE_ADDR     EP_BUFFERS_STARTING_ADDR
#define EP0_OUT_BUFFER_BASE_ADDR EP0_BUFFER_BASE_ADDR
#define EP0_IN_BUFFER_BASE_ADDR (EP0_BUFFER_BASE_ADDR + EP0_SIZE)

#elif (PINGPONG_MODE == PINGPONG_0_OUT)
#define EP0_BUFFER_BASE_ADDR          EP_BUFFERS_STARTING_ADDR
#define EP0_OUT_EVEN_BUFFER_BASE_ADDR EP0_BUFFER_BASE_ADDR
#define EP0_OUT_ODD_BUFFER_BASE_ADDR (EP0_BUFFER_BASE_ADDR + EP0_SIZE)
#define EP0_IN_BUFFER_BASE_ADDR      (EP0_BUFFER_BASE_ADDR + (EP0_SIZE * 2))

#elif (PINGPONG_MODE == PINGPONG_1_15)
#define EP0_BUFFER_BASE_ADDR     EP_BUFFERS_STARTING_ADDR
#define EP0_OUT_BUFFER_BASE_ADDR EP0_BUFFER_BASE_ADDR
#define EP0_IN_BUFFER_BASE_ADDR (EP0_BUFFER_BASE_ADDR + EP0_SIZE)

#else
#define EP0_BUFFER_BASE_ADDR EP_BUFFERS_STARTING_ADDR
#define EP0_OUT_EVEN_BUFFER_BASE_ADDR EP0_BUFFER_BASE_ADDR
#define EP0_OUT_ODD_BUFFER_BASE_ADDR (EP0_BUFFER_BASE_ADDR + EP0_SIZE)
#define EP0_IN_EVEN_BUFFER_BASE_ADDR (EP0_BUFFER_BASE_ADDR + (EP0_SIZE * 2))
#define EP0_IN_ODD_BUFFER_BASE_ADDR  (EP0_BUFFER_BASE_ADDR + (EP0_SIZE * 3))
#endif
#endif

/* ************************************************************************** */


/* ************************************************************************** */
/* ************************ EP0 STATUS HAL ********************************** */
/* ************************************************************************** */

#define EP0_OUT_LAST_PPB        g_usb_ep_stat[EP0][OUT].Last_PPB
#define EP0_IN_LAST_PPB         g_usb_ep_stat[EP0][IN].Last_PPB
#define EP0_OUT_DATA_TOGGLE_VAL g_usb_ep_stat[EP0][OUT].Data_Toggle_Val
#define EP0_IN_DATA_TOGGLE_VAL  g_usb_ep_stat[EP0][IN].Data_Toggle_Val

/* ************************************************************************** */


/* ************************************************************************** */
/* **************************** PIC14E EPCONDIS ***************************** */
/* ************************************************************************** */

#ifdef _PIC14E
#define EP1CONDIS EPCONDIS
#define EP0CONDIS EPCONDIS
#endif

/* ************************************************************************** */


/* ************************************************************************** */
/* *************************** REQUEST ERROR ******************************** */
/* ************************************************************************** */

#if (PINGPONG_MODE != PINGPONG_ALL_EP)
#define usb_request_error() usb_stall_ep(&g_usb_bd_table[BD0_IN])
#else
#define usb_request_error() do{usb_stall_ep(&g_usb_bd_table[BD0_IN_EVEN]); \
                               usb_stall_ep(&g_usb_bd_table[BD0_IN_ODD]);}while(0)
#endif

/* ************************************************************************** */


/* ************************************************************************** */
/* **************************** EP STATUS SIZE ****************************** */
/* ************************************************************************** */

#define EP_STAT_SIZE (NUM_ENDPOINTS * 2)

/* ************************************************************************** */


/* ************************************************************************** */
/* **************************** ROM / RAM DEFINES *************************** */
/* ************************************************************************** */

#define ROM 0
#define RAM 1

/* ************************************************************************** */


/* ************************************************************************** */
/* ********************************* TYPES ********************************** */
/* ************************************************************************** */

/** Device Settings Type */
typedef struct
{
    unsigned Self_Powered  :1;
    unsigned Remote_Wakeup :1;
    unsigned               :6;
}usb_dev_settings_t;

/** EP Status Type */
typedef struct
{
    unsigned Data_Toggle_Val :1;
    unsigned Halt            :1;
    unsigned Last_PPB        :1;
    unsigned                 :5;
}usb_ep_stat_t;

/** USTAT Type */
typedef struct
{
    unsigned      :1;
    unsigned PPBI :1;
    unsigned DIR  :1;
    unsigned ENDP :4;
    unsigned      :1;
}usb_ustat_t;

/* ************************************************************************** */


/* ************************************************************************** */
/* ******************** GLOBAL VARIABLES FROM: usb.c ************************ */
/* ************************************************************************** */

extern usb_ustat_t             g_usb_last_USTAT;
extern usb_ep_stat_t           g_usb_ep_stat[NUM_ENDPOINTS][2];

extern ch9_setup_t             g_usb_setup             __at(SETUP_DATA_ADDR);
extern ch9_get_descriptor_t    g_usb_get_descriptor    __at(SETUP_DATA_ADDR);
extern ch9_set_configuration_t g_usb_set_configuration __at(SETUP_DATA_ADDR);
extern ch9_set_interface_t     g_usb_set_interface     __at(SETUP_DATA_ADDR);

extern uint16_t                g_usb_bytes_available;
extern uint16_t                g_usb_bytes_2_send;
extern uint16_t                g_usb_bytes_2_recv;
extern bool                    g_usb_send_short;
extern uint8_t                 g_usb_sending_from;
extern const uint8_t          *g_usb_rom_ptr;
extern uint8_t                *g_usb_ram_ptr;
extern bd_t                    g_usb_bd_table[NUM_BD] __at(BDT_BASE_ADDR);

/* ************************************************************************** */


/* ************************************************************************** */
/* *********************** USB FUNCTIONS ************************************ */
/* ************************************************************************** */

/**
 * @fn void usb_tasks(void)
 * 
 * @brief The core USB state machine for handling USB device state and tasks.
 * 
 * The <i>usb_tasks()</i> function is designed to run frequently in the main 
 * program loop to handle all USB tasks when polling method is used and run in 
 * ISR when interrupts are enabled. The function contains the USB Device state 
 * machine as seen in <i>Section 9.1.1</i> of the <i>USB Specification 2.0</i>.
 */
void usb_tasks(void);

/** 
 * @fn void usb_init(void)
 * 
 * @brief Sets up USB for operation.
 * 
 * Firstly forces the USB reset flag high and then the usb_restart function is 
 * run.
 */
void usb_init(void);

/** 
 * @fn void usb_close(void)
 * 
 * @brief Detaches from USB bus and clears USB related registers.
 */
void usb_close(void);

/** 
 * @fn uint8_t usb_get_state(void)
 * 
 * @brief Returns the USB state stored in m_usb_state in usb.c.
 * 
 * @return state The USB state e.g STATE_CONFIGURED
 * 
 * <b>Code Example:</b>
 * <ul style="list-style-type:none"><li>
 * @code
 * while(usb_get_state() < STATE_CONFIGURED){};
 * @endcode
 * </li></ul>
 */
uint8_t usb_get_state(void);

/** 
 * @fn uint8_t usb_get_configuration(void)
 * 
 * @brief Returns the current configuration stored in m_current_configuration in usb.c.
 * 
 * @return configuration The current configuration.
 */
uint8_t usb_get_configurationn(void);

/** 
 * @fn void usb_set_control_stage(uint8_t control_stage)
 * 
 * @brief Sets the next control stage on EP0 e.g. DATA_IN_STAGE.
 * 
 * @param[in] control_stage The control stage.
 * 
 * <b>Code Example:</b>
 * <ul style="list-style-type:none"><li>
 * @code
 * usb_set_control_stage(DATA_IN_STAGE);
 * @endcode
 * </li></ul>
 */
void usb_set_control_stage(uint8_t control_stage);

/**
 * @fn void usb_arm_endpoint(bd_t* p_bd, usb_ep_stat_t* p_ep_stat, uint16_t buffer_addr, uint8_t cnt)
 * 
 * @brief Arms any OUT/IN Endpoint for a transaction.
 * 
 * The function is used to arm any endpoint (OUT/IN) for a transaction.
 * 
 * @param[in] p_bd Buffer Descriptor pointer.
 * @param[in] p_ep_stat Endpoint status pointer.
 * @param[in] buffer_addr Address of data in RAM to transfer in data stage.
 * @param[in] cnt Amount of data being transfered.
 * 
 * <b>Code Example:</b>
 * <ul style="list-style-type:none"><li>
 * @code
 * usb_arm_endpoint(&g_usb_bd_table[BD1_OUT], &g_usb_ep_stat[EP1][OUT], EP1_OUT_BUFFER_BASE_ADDR, 31);
 * @endcode
 * </li></ul>
 */
void usb_arm_endpoint(bd_t* p_bd, usb_ep_stat_t* p_ep_stat, uint16_t buffer_addr, uint8_t cnt);


#if PINGPONG_MODE == PINGPONG_ALL_EP
/**
 * @fn void usb_arm_ep0_in(uint8_t bd_table_index, uint8_t cnt)
 * 
 * @brief Arms EP0 IN Endpoint for a transaction.
 * 
 * The function is used to arm EP0 IN for a transaction.
 * 
 * @param[in] bd_table_index Buffer Descriptor Index.
 * @param[in] cnt Amount of bytes being transfered.
 * 
 * <b>Code Example:</b>
 * <ul style="list-style-type:none"><li>
 * @code
 * usb_arm_ep0_in(BD0_IN_EVEN,8);
 * @endcode
 * </li></ul>
 */
void usb_arm_ep0_in(uint8_t bd_table_index, uint8_t cnt);
#else
/**
 * @fn void usb_arm_ep0_in(uint8_t cnt)
 * 
 * @brief Arms EP0 IN Endpoint for a transaction.
 * 
 * The function is used to arm EP0 IN for a transaction.
 * 
 * @param[in] cnt Amount of bytes being transfered.
 * 
 * <b>Code Example:</b>
 * <ul style="list-style-type:none"><li>
 * @code
 * usb_arm_ep0_in(8);
 * @endcode
 * </li></ul>
 */
void usb_arm_ep0_in(uint8_t cnt);
#endif

/**
 * @fn void usb_arm_status(bd_t* p_bd)
 * 
 * @brief Arms any OUT/IN Endpoint for status stage (Zero-length data packet). 
 * 
 * @param[in] p_bd Buffer Descriptor pointer.
 * 
 * <b>Code Example:</b>
 * <ul style="list-style-type:none"><li>
 * @code
 * usb_arm_status(&g_usb_bd_table[BD0_IN]);
 * @endcode
 * </li></ul>
 */
void usb_arm_status(bd_t* p_bd);

/**
 * @fn void usb_arm_in_status(void)
 * 
 * @brief Arms any EP0 IN Endpoint for status stage (Zero-length data packet). 
 * 
 * <b>Code Example:</b>
 * <ul style="list-style-type:none"><li>
 * @code
 * usb_arm_in_status();
 * @endcode
 * </li></ul>
 */
void usb_arm_in_status(void);

/**
 * @fn void usb_stall_ep(bd_t* p_bd)
 * 
 * @brief Stalls any OUT/IN Endpoint. 
 * 
 * @param[in] p_bd Buffer Descriptor pointer.
 * 
 * <b>Code Example:</b>
 * <ul style="list-style-type:none"><li>
 * @code
 * g_usb_ep_stat[EP1][IN].Halt = 1;
 * usb_stall_ep(&g_usb_bd_table[BD1_IN]);
 * @endcode
 * </li></ul>
 */
void usb_stall_ep(bd_t* p_bd);

/**
 * @fn void usb_out_control_transfer(void)
 * 
 * @brief Processes a OUT Control (EP0 OUT) transaction. <b>So far not used</b>
 * 
 */
void usb_out_control_transfer(void);

// TODO: Is this needed?
void usb_out_control_status(void);

/**
 * @fn void usb_in_control_transfer(void)
 * 
 * @brief Processes a IN Control (EP0 IN) transaction.
 * 
 */
void usb_in_control_transfer(void);

/**
 * @fn void usb_rom_copy(const uint8_t *p_rom, uint8_t *p_ep, uint8_t bytes)
 * 
 * @brief Copy values from ROM and place in endpoint.
 * 
 * @param[in] *p_rom Pointer to the address in ROM to copy from.
 * @param[in] *p_ep Pointer to the endpoint address in RAM to copy to.
 * @param[in] bytes Amount of bytes to copy.
 * 
 */
void usb_rom_copy(const uint8_t *p_rom, uint8_t *p_ep, uint8_t bytes);

/**
 * @fn void usb_ram_copy(uint8_t *p_ram1, uint8_t *p_ram2, uint8_t bytes)
 * 
 * @brief Copy values from RAM and place in endpoint.
 * 
 * @param[in] *p_ram1 Pointer to the address in RAM to copy from.
 * @param[in] *p_ram2 Pointer to the endpoint address in RAM to copy to.
 * @param[in] bytes Amount of bytes to copy.
 * 
 */
void usb_ram_copy(uint8_t *p_ram1, uint8_t *p_ram2, uint8_t bytes);

/**
 * @fn void usb_ram_set(uint8_t val, uint8_t *p_ram, uint16_t bytes)
 * 
 * @brief Set an area of RAM to a particular value. Useful for clearing an endpoint.
 * 
 * @param[in] val Value to set to.
 * @param[in] *p_ram Pointer to the starting RAM address.
 * @param[in] bytes Amount of bytes to set.
 * 
 */
void usb_ram_set(uint8_t val, uint8_t *p_ram, uint16_t bytes);

// TODO: descriptions for these
// USER FUNCTIONS TO PLACE IN MAIN
void usb_reset(void);
void usb_error(void);
void usb_idle(void);
void usb_activity(void);
void usb_sof(void);

/**
 * @fn bool usb_out_control_finished(void)
 * 
 * @brief Is called at the end of a OUT Control Transfer.
 * 
 * @return Returns true if successful and false if unsuccessful.
 */
bool usb_out_control_finished(void);

/* ************************************************************************** */

#endif /* USB_H */