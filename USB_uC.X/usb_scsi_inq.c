/**
 * @file usb_scsi_inq.c
 * @brief Contains SCSI's Inquiry Command Response.
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
#include "usb_scsi.h"

// String Settings
#define SCSI_VENDER_ID   {'M','i','c','r','o','c','h','p'}
#define SCSI_PRODUCT_REV {'0','1','1','0'}

#if defined(_PIC14E)
#define SCSI_PRODUCT_ID  {'U','S','B',' ','u','C',' ','1','4','5','X',' ',' ',' ',' ',' '}
#elif defined(_18F14K50)
#define SCSI_PRODUCT_ID  {'U','S','B',' ','u','C',' ','1','4','K','5','0',' ',' ',' ',' '}
#elif defined(_18F24K50)
#define SCSI_PRODUCT_ID  {'U','S','B',' ','u','C',' ','2','4','K','5','0',' ',' ',' ',' '}
#elif defined(_18F25K50) || defined(_18F45K50)
#define SCSI_PRODUCT_ID  {'U','S','B',' ','u','C',' ','X','5','K','5','0',' ',' ',' ',' '}
#elif defined(_18F26J53) || defined(_18F46J53)
#define SCSI_PRODUCT_ID  {'U','S','B',' ','u','C',' ','X','6','J','5','3',' ',' ',' ',' '}
#elif defined(_18F27J53) || defined(_18F47J53)
#define SCSI_PRODUCT_ID  {'U','S','B',' ','u','C',' ','X','7','J','5','3',' ',' ',' ',' '}
#else
#error "SCSI Inquiry Error: Part not supported"
#endif

const scsi_inquiry_t g_scsi_inquiry =
{
   {0x00, // (7-5 Peripheral Qualifier) 0b000, (4-0 Peripheral Device Type) 0b00000: connected, SBC-4 direct block access
    0x80, // (7 RMB) 1, (6-0 Reserved): Removable
    0x04, // (7-0) 0x04: Complies to ANSI INCITS 351-2001 (SPC-2)
    0x02, // (7-6 Obsolete) 0, (5 NORMACA) 0, (4 HISUP) 0, (3-0 Response Data Format) 0b0010: not supported, not supported, always two
    0x20, // (7-0 Additional Length N-4) 0x20: Bytes left in response (36-4=32)
    0x00, // (7 SCCS) 0, (6 ACC) 0, (5-4 TPGS) 0, (3 3PC) 0, (2-1 Reserved) 0, (0 Protect) 0: all unsupported
    0x00, // (7 Obsolete) 0, (6 ENCSERV) 0, (5 VS) 0, (4 MULTIP) 0, (3-0 Obsolete) 0b0000: all unsupported
    0x00},// (7-2 Obsolete) 0, (1 CMDQUE) 0, (0 VS) 0: all unsupported
    SCSI_VENDER_ID,  // T10 Vender ID
    SCSI_PRODUCT_ID, // Product ID
    SCSI_PRODUCT_REV // Product Revision Number
};
