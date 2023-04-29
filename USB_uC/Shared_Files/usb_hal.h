/**
 * @file usb_hal.h
 * @brief <i>Hardware Abstraction Layer</i> for Endpoints.
 * @author John Izzard
 * @date 10/03/2023
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
#ifndef USB_HAL_H
#define USB_HAL_H

#include <xc.h>
#include <stdint.h>
#include "usb_config.h"


/* ************************************************************************** */
/* ************************ UEPn REGISTER BITS ****************************** */
/* ************************************************************************** */

#define _EPHSHK    0x10
#define _EPCONDIS  0x08
#define _EPOUTEN   0x04
#define _EPINEN    0x02
#define _EPSTALL   0x01

/* ************************************************************************** */


/* ************************************************************************** */
/* ************************ BD STAT REGISTER BITS *************************** */
/* ************************************************************************** */

#define _UOWN   (uint8_t)0x80
#define _DTS    (uint8_t)0x40
#define _DTSEN  (uint8_t)0x08
#define _BSTALL (uint8_t)0x04

/* ************************************************************************** */


/* ************************************************************************** */
/* ************************* UCFG REGISTER BITS ***************************** */
/* ************************************************************************** */

#define _UTEYE 0x80
#define _UPUEN 0x10
#define _FSEN  0x04
#define _PPB1  0x02
#define _PPB0  0x01

#define PPB PINGPONG_MODE

/* ************************************************************************** */


/* ************************************************************************** */
/* ************************* UIE REGISTER BITS ****************************** */
/* ************************************************************************** */

#define _SOFIE   0x40
#define _STALLIE 0x20
#define _IDLEIE  0x10
#define _TRNIE   0x08
#define _ACTVIE  0x04
#define _UERIE   0x02
#define _URSTIE  0x01

/* ************************************************************************** */


/* ************************************************************************** */
/* *************************** UEIE REGISTER BITS *************************** */
/* ************************************************************************** */

#define _BTSEE   0x80
#define _BTOEE   0x10
#define _DFN8EE  0x08
#define _CRC16EE 0x04
#define _CRC5EE  0x02
#define _PIDEE   0x01

/* ************************************************************************** */


/* ************************************************************************** */
/* **************************** USTAT DEFINES ******************************* */
/* ************************************************************************** */

#define IN   1
#define OUT  0

#define EP0  0
#define EP1  1
#define EP2  2
#define EP3  3
#define EP4  4
#define EP5  5
#define EP6  6
#define EP7  7
#define EP8  8
#define EP9  9
#define EP10 10
#define EP11 11
#define EP12 12
#define EP13 13
#define EP14 14
#define EP15 15

/* ************************************************************************** */


/* ************************************************************************** */
/* ********* DEFINES FOR EXPLICIT ACCESS TO BUFFER DESCRIPTORS ************** */
/* ************************************************************************** */

#define EVEN 0
#define ODD  1
#if (PINGPONG_MODE == PINGPONG_DIS)
#define BD0_OUT   0u
#define BD0_IN    1u
#define BD1_OUT   2u
#define BD1_IN    3u
#if NUM_ENDPOINTS > 2
#define BD2_OUT   4u
#define BD2_IN    5u
#endif
#if NUM_ENDPOINTS > 3
#define BD3_OUT   6u
#define BD3_IN    7u
#endif
#elif (PINGPONG_MODE == PINGPONG_0_OUT)
#define BD0_OUT_EVEN  0u
#define BD0_OUT_ODD   1u
#define BD0_IN        2u
#define BD1_OUT       3u
#define BD1_IN        4u
#if NUM_ENDPOINTS > 2
#define BD2_OUT       5u
#define BD2_IN        6u
#endif
#if NUM_ENDPOINTS > 3
#define BD3_OUT       7u
#define BD3_IN        8u
#endif
#elif (PINGPONG_MODE == PINGPONG_1_15)
#define BD0_OUT      0u
#define BD0_IN       1u
#define BD1_OUT_EVEN 2u
#define BD1_OUT_ODD  3u
#define BD1_IN_EVEN  4u
#define BD1_IN_ODD   5u
#if NUM_ENDPOINTS > 2
#define BD2_OUT_EVEN 6u
#define BD2_OUT_ODD  7u
#define BD2_IN_EVEN  8u
#define BD2_IN_ODD   9u
#endif
#if NUM_ENDPOINTS > 3
#define BD3_OUT_EVEN 10u
#define BD3_OUT_ODD  11u
#define BD3_IN_EVEN  12u
#define BD3_IN_ODD   13u
#endif
#elif (PINGPONG_MODE == PINGPONG_ALL_EP)
#define BD0_OUT_EVEN  0u
#define BD0_OUT_ODD   1u
#define BD0_IN_EVEN   2u
#define BD0_IN_ODD    3u
#define BD1_OUT_EVEN  4u
#define BD1_OUT_ODD   5u
#define BD1_IN_EVEN   6u
#define BD1_IN_ODD    7u
#if NUM_ENDPOINTS > 2
#define BD2_OUT_EVEN  8u
#define BD2_OUT_ODD   9u
#define BD2_IN_EVEN   10u
#define BD2_IN_ODD    11u
#endif
#if NUM_ENDPOINTS > 3
#define BD3_OUT_EVEN  12u
#define BD3_OUT_ODD   13u
#define BD3_IN_EVEN   14u
#define BD3_IN_ODD    15u
#endif
#endif

/* ************************************************************************** */


/* ************************************************************************** */
/* ******************* BUFFER DESCRIPTOR SIZE CALCULATION  ****************** */
/* ************************************************************************** */

#if (PINGPONG_MODE == PINGPONG_DIS)
#define NUM_BD (NUM_ENDPOINTS*2)
#elif (PINGPONG_MODE == PINGPONG_0_OUT)
#define NUM_BD ((NUM_ENDPOINTS*2)+1)
#elif (PINGPONG_MODE == PINGPONG_1_15)
#define NUM_BD ((NUM_ENDPOINTS*4)-2)
#else
#define NUM_BD (NUM_ENDPOINTS*4)
#endif
#define BDT_SIZE (NUM_BD*4)

/* ************************************************************************** */


/* ************************************************************************** */
/* ********************* PROCESSOR SPECIFIC DEFINES  ************************ */
/* ************************************************************************** */

#if defined(_PIC14E)
#define BDT_BASE_ADDR   0x2000
#define SETUP_DATA_ADDR 0x70
#elif defined(_18F13K50) || defined(_18F14K50)
#define BDT_BASE_ADDR   0x200
#define SETUP_DATA_ADDR 0x60
#define EP_BUFFERS_STARTING_ADDR (BDT_BASE_ADDR + BDT_SIZE)
#elif defined(_18F26J53) || defined(_18F46J53) || defined(_18F27J53) || defined(_18F47J53)
#define BDT_BASE_ADDR   0xD00
#define SETUP_DATA_ADDR 0x60
#define EP_BUFFERS_STARTING_ADDR (BDT_BASE_ADDR + BDT_SIZE)
#else
#define BDT_BASE_ADDR   0x400
#define SETUP_DATA_ADDR 0x60
#define EP_BUFFERS_STARTING_ADDR (BDT_BASE_ADDR + BDT_SIZE)
#endif

#if defined(_18F24K50)||defined(_18F25K50)||defined(_18F45K50)
#define USB_INTERRUPT_ENABLE PIE3bits.USBIE
#define USB_INTERRUPT_FLAG   PIR3bits.USBIF
#else
#define USB_INTERRUPT_ENABLE PIE2bits.USBIE
#define USB_INTERRUPT_FLAG   PIR2bits.USBIF
#endif

#define TRANSACTION_EP  g_usb_last_USTAT.ENDP
#define TRANSACTION_DIR g_usb_last_USTAT.DIR
#define PINGPONG_PARITY g_usb_last_USTAT.PPBI

/* ************************************************************************** */


/* ************************************************************************** */
/* ******************************** TYPES *********************************** */
/* ************************************************************************** */

/** 
 * @var BDT
 * @brief Buffer Descriptor Table Structure. 
 */
typedef struct
{
    union
    {
        struct
        {   
            union
            {
                struct
                {
                    unsigned BC8:1;
                    unsigned BC9:1;
                    unsigned BSTALL:1;
                    unsigned DTSEN:1;
                    unsigned :1;
                    unsigned :1;
                    unsigned DTS:1;
                    unsigned UOWN:1;
                };
                struct
                {
                    unsigned :2;
                    unsigned PID0:1;
                    unsigned PID1:1;
                    unsigned PID2:1;
                    unsigned PID3:1;
                    unsigned :2;
                };
                struct
                {
                    unsigned :2;
                    unsigned PID:4;
                    unsigned :2;
                };
            }STATbits;
            uint8_t  CNT;
            uint8_t  ADRL;
            uint8_t  ADRH;
        };
        struct
        {
            uint8_t  STAT;
            unsigned :8;
            uint16_t ADR;
        };
    };
}bd_t;

/* ************************************************************************** */

#endif /* USB_HAL_H */