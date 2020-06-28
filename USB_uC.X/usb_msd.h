/**
 * @file usb_msd.h
 * @brief <i>Mass Storage Class</i> core header.
 * @author John Izzard
 * @date 05/06/2020
 * 
 * USB uC - MSD Library.
 * Copyright (C) 2017-2020  John Izzard
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
#ifndef USB_MSD_H
#define USB_MSD_H

#include <stdint.h>
#include <stdbool.h>
#include "usb_config.h"
#include "usb_hal.h"
#include "usb_msd_config.h"
#include "usb_scsi.h"

/* ************************************************************************** */
/* ************************** PIC14 WARNING ********************************* */
/* ************************************************************************** */

#ifdef _PIC14E
#warning "MSD EP Buffer addresses need to be manually set for PIC16 devices."
#endif

/* ************************************************************************** */


/* ************************************************************************** */
/* **************************** MSD EP ADDRESSES **************************** */
/* ************************************************************************** */

#if PINGPONG_MODE == PINGPONG_DIS || PINGPONG_MODE == PINGPONG_1_15
#define MSD_EP_BUFFERS_STARTING_ADDR (EP_BUFFERS_STARTING_ADDR + (EP0_SIZE*2))
#elif PINGPONG_MODE == PINGPONG_0_OUT
#define MSD_EP_BUFFERS_STARTING_ADDR  (EP_BUFFERS_STARTING_ADDR + (EP0_SIZE*3))
#else
#define MSD_EP_BUFFERS_STARTING_ADDR (EP_BUFFERS_STARTING_ADDR + (EP0_SIZE*4))
#endif

#if PINGPONG_MODE == PINGPONG_DIS || PINGPONG_MODE == PINGPONG_0_OUT
#ifdef _PIC14E
#define MSD_EP_OUT_BUFFER_BASE_ADDR 0x2050
#define MSD_EP_IN_BUFFER_BASE_ADDR  0x20A0
#else
#define MSD_EP_OUT_BUFFER_BASE_ADDR  MSD_EP_BUFFERS_STARTING_ADDR
#define MSD_EP_IN_BUFFER_BASE_ADDR   (MSD_EP_BUFFERS_STARTING_ADDR + MSD_EP_SIZE)
#endif

#else // PINGPONG_MODE == PINGPONG_1_15 || PINGPONG_MODE == PINGPONG_ALL_EP
#ifdef _PIC14E
#define MSD_EP_OUT_EVEN_BUFFER_BASE_ADDR 0x2050
#define MSD_EP_OUT_ODD_BUFFER_BASE_ADDR  0x20A0
#define MSD_EP_IN_EVEN_BUFFER_BASE_ADDR  0x20F0
#define MSD_EP_IN_ODD_BUFFER_BASE_ADDR   0x2140
#else
#define MSD_EP_OUT_EVEN_BUFFER_BASE_ADDR   MSD_EP_BUFFERS_STARTING_ADDR
#define MSD_EP_OUT_ODD_BUFFER_BASE_ADDR   (MSD_EP_BUFFERS_STARTING_ADDR +  MSD_EP_SIZE)
#define MSD_EP_IN_EVEN_BUFFER_BASE_ADDR   (MSD_EP_BUFFERS_STARTING_ADDR + (MSD_EP_SIZE * 2))
#define MSD_EP_IN_ODD_BUFFER_BASE_ADDR    (MSD_EP_BUFFERS_STARTING_ADDR + (MSD_EP_SIZE * 3))
#endif
extern uint8_t MSD_EP_OUT_EVEN[MSD_EP_SIZE]    __at(MSD_EP_OUT_EVEN_BUFFER_BASE_ADDR);
extern uint8_t MSD_EP_OUT_ODD[MSD_EP_SIZE]     __at(MSD_EP_OUT_ODD_BUFFER_BASE_ADDR);
extern uint8_t MSD_EP_IN_EVEN[MSD_EP_SIZE]     __at(MSD_EP_IN_EVEN_BUFFER_BASE_ADDR);
extern uint8_t MSD_EP_IN_ODD[MSD_EP_SIZE]      __at(MSD_EP_IN_ODD_BUFFER_BASE_ADDR);
#endif

/* ************************************************************************** */


/* ************************************************************************** */
/* *************************** MSD 13 ERROR CASES *************************** */
/* ************************************************************************** */

// The Thirteen Error Cases
#define CASE_1  0x0001
#define CASE_2  0x0002
#define CASE_3  0x0004
#define CASE_4  0x0008
#define CASE_5  0x0010
#define CASE_6  0x0020
#define CASE_7  0x0040
#define CASE_8  0x0080
#define CASE_9  0x0100
#define CASE_10 0x0200
#define CASE_11 0x0400
#define CASE_12 0x0800
#define CASE_13 0x1000

/* ************************************************************************** */


/* ************************************************************************** */
/* ******************************* MSD CODES ******************************** */
/* ************************************************************************** */

// MSD Subclass Codes
#define SCSI_CMD_N_SUPPORT 0x00
#define RBC                0x01
#define MMC_5              0x02
#define QIC_157            0x03
#define UFI                0x04
#define SFF_8070i          0x05
#define SCSI_TRANSPARENT   0x06
#define LSD_FS             0x07
#define IEEE_1667          0x08

// MSD Protocol Codes
#define CBI_W_COMPL_INT    0x00
#define CBI_N_COMPL_INT    0x01
#define BBB                0x50
#define UAS                0x62

// Class Specific Descriptor Codes
#define PIPE_USAGE_DESC    0x24

/* ************************************************************************** */


/* ************************************************************************** */
/* **************************** MSD SIGNATURES ****************************** */
/* ************************************************************************** */

// dCBWSignature/dCSWSignature values
#define CBW_SIG 0x43425355
#define CSW_SIG 0x53425355

/* ************************************************************************** */


/* ************************************************************************** */
/* ******************************** MSD STATES ****************************** */
/* ************************************************************************** */

// MSD STATES
#define MSD_CBW           0
#define MSD_NO_DATA_STAGE 1
#define MSD_DATA_SENT     2
#define MSD_CSW           3
#define MSD_READ_DATA     4
#define MSD_WRITE_DATA    5
#define MSD_WAIT_ILLEGAL  6
#define MSD_WAIT_IVALID   7
#define MSD_READ_FINISHED 8

/* ************************************************************************** */


/* ************************************************************************** */
/* **************************** MSD REQUEST CODES *************************** */
/* ************************************************************************** */

// MSD Request Codes
#define ADSC 0
#define GET_REQUESTS 0xFC
#define PUT_REQUESTS 0xFD
#define GET_MAX_LUN  0xFE
#define BOMSR        0xFF

/* ************************************************************************** */


/* ************************************************************************** */
/* ******************************** MSD HAL ********************************* */
/* ************************************************************************** */

#define MSD_TRANSACTION_DIR m_tasks_buff.task_stat[m_task_get_index].DIR
#define MSD_PINGPONG_PARITY m_tasks_buff.task_stat[m_task_get_index].PPBI

#define MSD_EP_OUT_LAST_PPB        g_usb_ep_stat[MSD_EP][OUT].Last_PPB
#define MSD_EP_IN_LAST_PPB         g_usb_ep_stat[MSD_EP][IN].Last_PPB
#define MSD_EP_OUT_DATA_TOGGLE_VAL g_usb_ep_stat[MSD_EP][OUT].Data_Toggle_Val
#define MSD_EP_IN_DATA_TOGGLE_VAL  g_usb_ep_stat[MSD_EP][IN].Data_Toggle_Val

#if defined(_PIC14E)
#define CBW_DATA_ADDR SETUP_DATA_ADDR - 31
#else
#define CBW_DATA_ADDR SETUP_DATA_ADDR + 8
#endif

/* ************************************************************************** */


/* ************************************************************************** */
/* ****************************** MSD CSW STATUS **************************** */
/* ************************************************************************** */

/// Status Values
#define COMMAND_PASSED 0
#define COMMAND_FAILED 1
#define PHASE_ERROR    2

/* ************************************************************************** */


/* ************************************************************************** */
/* ******************************** MSD TYPES ******************************* */
/* ************************************************************************** */

/** Command Block Wrapper Structure */
typedef union
{
    uint8_t BYTES[31];
    struct
    {
        uint32_t dCBWSignature;
        uint32_t dCBWTag;
        uint32_t dCBWDataTransferLength;
        union
        {
            uint8_t  bmCBWFlags;
            struct
            {
                unsigned :7;
                unsigned Direction:1;
            };
        };
        uint8_t  bCBWLUN;
        uint8_t  bCBWCBLength;
        uint8_t  CBWCB0[16];
    }; 
}msd_cbw_t;

/** Command Status Wrapper Structure */
typedef union
{
    uint8_t BYTES[13];
    struct
    {
        uint32_t dCSWSignature;
        uint32_t dCSWTag;
        uint32_t dCSWDataResidue;
        uint8_t  bCSWStatus;
    };
}msd_csw_t;

/** READ_10 Variables Structure */
typedef struct
{
    union
    {
        uint8_t START_LBA_BYTES[4];
        uint32_t START_LBA;
    };
    uint32_t LBA;
    union
    {
        uint8_t TF_LEN_BYTES[2];
        uint16_t TF_LEN;
    };
    uint32_t TF_LEN_IN_BYTES;
    uint32_t CBW_TF_LEN;
}msd_rw_10_vars_t;

/** Stores the amount of bytes to be return in responses. */
typedef union
{
    uint16_t val;
    struct
    {
        uint8_t LB;
        uint8_t HB;
    };
}msd_bytes_to_transfer_t;

/* ************************************************************************** */


/* ************************************************************************** */
/* ****************** MSD GLOBAL VARS FROM: usb_msd_acm.c ******************* */
/* ************************************************************************** */

#if PINGPONG_MODE == PINGPONG_DIS || PINGPONG_MODE == PINGPONG_0_OUT
extern uint8_t g_msd_ep_out[MSD_EP_SIZE] __at(MSD_EP_OUT_BUFFER_BASE_ADDR);
extern uint8_t g_msd_ep_in[MSD_EP_SIZE]  __at(MSD_EP_IN_BUFFER_BASE_ADDR);
#else // PINGPONG_MODE == PINGPONG_1_15 || PINGPONG_MODE == PINGPONG_ALL_EP
extern uint8_t g_msd_ep_out_even[MSD_EP_SIZE] __at(MSD_EP_OUT_EVEN_BUFFER_BASE_ADDR);
extern uint8_t g_msd_ep_out_odd[MSD_EP_SIZE]  __at(MSD_EP_OUT_ODD_BUFFER_BASE_ADDR);
extern uint8_t g_msd_ep_in_even[MSD_EP_SIZE]  __at(MSD_EP_IN_EVEN_BUFFER_BASE_ADDR);
extern uint8_t g_msd_ep_in_odd[MSD_EP_SIZE]   __at(MSD_EP_IN_ODD_BUFFER_BASE_ADDR);
#endif

extern uint16_t g_msd_byte_of_sect;
#ifndef MSD_LIMITED_RAM
extern uint8_t g_msd_sect_data[512];
#endif
extern msd_cbw_t                 g_msd_cbw __at(CBW_DATA_ADDR);
extern msd_csw_t                 g_msd_csw;
extern msd_rw_10_vars_t          g_msd_rw_10_vars;
extern msd_bytes_to_transfer_t   g_msd_bytes_to_transfer;
extern scsi_fixed_format_sense_t g_msd_fixed_format_sense;

/* ************************************************************************** */


/* ************************************************************************** */
/* ***************************** MSD FUNCTIONS ****************************** */
/* ************************************************************************** */

/**
 * @fn bool msd_class_request(void)
 * 
 * @brief Used to service Class Requests for MSD.
 * 
 * @return Returns success (true) or failure (false) to execute the Request.
 */
bool msd_class_request(void);

/**
 * @fn void msd_init(void)
 * 
 * @brief Used to initialize EPs used by the MSD library.
 * 
 * EP buffers are cleared, EP settings are configured, and the Buffer Descriptor Table values are setup.
 */
void msd_init(void);

/**
 * @fn void msd_add_task(void)
 * 
 * @brief Adds a MSD Task to the queue.
 * 
 * Adds a MSD Task to the queue so that msd_tasks() can service it.
 */
void msd_add_task(void);

/**
 * @fn void msd_tasks(void)
 * 
 * @brief Services MSD tasks.
 * 
 * msd_tasks() must be run frequently in your main program loop. msd_tasks() 
 * doesn't run inside AppTasks() because msd_tasks() can delay the speed of
 * servicing the Control EP when the USTAT buffer has values for MSD_EPs. This
 * can prevent the arming of EP0_OUT in time for a SETUP_PACKET. The host sees
 * this as an error. Place msd_add_task() instead.
 */
void msd_tasks(void);

/**
 * @fn void msd_clear_halt(uint8_t bdt_index, uint8_t EP, uint8_t dir)
 * 
 * @brief Clear Endpoints used by MSD.
 * 
 * Used to inform the MSD library to clear a halt/stall condition on
 * it's endpoint.
 */
void msd_clear_halt(uint8_t bdt_index, uint8_t ep, uint8_t dir);

/**
 * @fn void msd_clear_ep_toggle(void)
 * 
 * @brief Used to clear MSD EP's Data Toggle values.
 * 
 */
void msd_clear_ep_toggle(void);

#if PINGPONG_MODE == PINGPONG_1_15 || PINGPONG_MODE == PINGPONG_ALL_EP
/**
 * @fn msd_arm_ep_out(uint8_t bdt_index)
 * 
 * @brief Arms MSD EP OUT Endpoint for a transaction.
 * 
 * The function is used to arm MSD EP OUT for a transaction.
 * 
 * @param[in] bdt_index Buffer Descriptor Index.
 * 
 * <b>Code Example:</b>
 * <ul style="list-style-type:none"><li>
 * @code
 * msd_arm_ep_out(MSD_BD_OUT_EVEN);
 * @endcode
 * </li></ul>
 */
void msd_arm_ep_out(uint8_t bdt_index);

/**
 * @fn void msd_arm_ep_in(uint8_t bdt_index, uint16_t cnt)
 * 
 * @brief Arms MSD EP IN Endpoint for a transaction.
 * 
 * The function is used to arm MSD EP IN for a transaction.
 * 
 * @param[in] bdt_index Buffer Descriptor Index.
 * @param[in] cnt Amount of bytes being transfered.
 * 
 * <b>Code Example:</b>
 * <ul style="list-style-type:none"><li>
 * @code
 * msd_arm_ep_in(MSD_BD_IN_EVEN,MSD_EP_SIZE);
 * @endcode
 * </li></ul>
 */
void msd_arm_ep_in(uint8_t bdt_index, uint16_t cnt);

#else
/**
 * @fn msd_arm_ep_out(void)
 * 
 * @brief Arms MSD EP OUT Endpoint for a transaction.
 * 
 * The function is used to arm MSD EP OUT for a transaction.
 * 
 * 
 * <b>Code Example:</b>
 * <ul style="list-style-type:none"><li>
 * @code
 * msd_arm_ep_out();
 * @endcode
 * </li></ul>
 */
void msd_arm_ep_out(void);

/**
 * @fn void msd_arm_ep_in(uint16_t cnt)
 * 
 * @brief Arms MSD EP IN Endpoint for a transaction.
 * 
 * The function is used to arm MSD EP IN for a transaction.
 * 
 * @param[in] cnt Amount of bytes being transfered.
 * 
 * <b>Code Example:</b>
 * <ul style="list-style-type:none"><li>
 * @code
 * msd_arm_ep_in(MSD_EP_SIZE);
 * @endcode
 * </li></ul>
 */
void msd_arm_ep_in(uint16_t cnt);
#endif

// TODO: descriptions for these
// USER FUNCTIONS TO PLACE IN MAIN
bool    msd_media_present(void);
uint8_t msd_test_unit_ready(void);
uint8_t msd_start_stop_unit(void);
void    msd_read_capacity(void);
void    msd_rx_sector(void);
void    msd_tx_sector(void);
bool    msd_wr_protect(void);

/* ************************************************************************** */

#endif /* USB_MSD_H */