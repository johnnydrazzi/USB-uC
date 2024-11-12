/**
 * @file usb_scsi_inq.c
 * @author John Izzard
 * @date 2024-11-12
 * 
 * @brief USB uC - Contains SCSI's Inquiry Command Response.
 */

/**
 * Copyright (C) 2017-2024 John Izzard
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the “Software”), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/**
 * Change Log
 * ----------
 * Version 1.4.1 - 2024-11-12
 * - Changed: MIT License.
 *
 * Version 1.4.0 - 2024-06-30
 * - Changed: Release number v1.13.
 *
 * Version 1.3.0 - 2024-01-26
 * - Changed: Release number v1.12.
 *
 * Version 1.2.0 - 2023-04-07
 * - Added: Support for _18F4450_FAMILY_ and _18F46J50_FAMILY_.
 * - Changed: Release number v1.11.
 *
 * Version 1.1.0 - 2021-05-01
 * - Changed: Release number v1.10.
 *
 * Version 1.0.0 - 2020-06-28
 * - Added: Initial release of the software.
 */

#include "usb_scsi.h"

// String Settings
#define SCSI_VENDER_ID   {'M','i','c','r','o','c','h','p'}
#define SCSI_PRODUCT_REV {'0','1','1','3'}

#if defined(_PIC14E)
#define SCSI_PRODUCT_ID  {'U','S','B',' ','u','C',' ','1','4','5','X',' ',' ',' ',' ',' '}
#elif defined(_18F2450) || defined(_18F4450)
#define SCSI_PRODUCT_ID  {'U','S','B',' ','u','C',' ','X','4','5','0',' ',' ',' ',' ',' '}
#elif defined(_18F2455) || defined(_18F4455)
#define SCSI_PRODUCT_ID  {'U','S','B',' ','u','C',' ','X','4','5','5',' ',' ',' ',' ',' '}
#elif defined(_18F2458) || defined(_18F4458)
#define SCSI_PRODUCT_ID  {'U','S','B',' ','u','C',' ','X','4','5','8',' ',' ',' ',' ',' '}
#elif defined(_18F2550) || defined(_18F4550)
#define SCSI_PRODUCT_ID  {'U','S','B',' ','u','C',' ','X','5','5','0',' ',' ',' ',' ',' '}
#elif defined(_18F2553) || defined(_18F4553)
#define SCSI_PRODUCT_ID  {'U','S','B',' ','u','C',' ','X','5','5','3',' ',' ',' ',' ',' '}
#elif defined(_18F14K50)
#define SCSI_PRODUCT_ID  {'U','S','B',' ','u','C',' ','1','4','K','5','0',' ',' ',' ',' '}
#elif defined(_18F24K50)
#define SCSI_PRODUCT_ID  {'U','S','B',' ','u','C',' ','2','4','K','5','0',' ',' ',' ',' '}
#elif defined(_18F25K50) || defined(_18F45K50)
#define SCSI_PRODUCT_ID  {'U','S','B',' ','u','C',' ','X','5','K','5','0',' ',' ',' ',' '}
#elif defined(_18F24J50) || defined(_18F44J50)
#define SCSI_PRODUCT_ID  {'U','S','B',' ','u','C',' ','X','4','J','5','0',' ',' ',' ',' '}
#elif defined(_18F25J50) || defined(_18F45J50)
#define SCSI_PRODUCT_ID  {'U','S','B',' ','u','C',' ','X','5','J','5','0',' ',' ',' ',' '}
#elif defined(_18F26J50) || defined(_18F46J50)
#define SCSI_PRODUCT_ID  {'U','S','B',' ','u','C',' ','X','6','J','5','0',' ',' ',' ',' '}
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
