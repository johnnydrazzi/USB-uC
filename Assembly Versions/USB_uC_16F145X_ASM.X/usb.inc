;*******************************************************************************
;                                                                              *
;    Copyright (c) 2024 John Izzard                                            *
;                                                                              *
;    Permission is hereby granted, free of charge, to any person obtaining     *
;    a copy of this software and associated documentation files (the           *
;    "Software"), to deal in the Software without restriction, including       *
;    without limitation the rights to use, copy, modify, merge, publish,       *
;    distribute, sublicense, and/or sell copies of the Software, and to        *
;    permit persons to whom the Software is furnished to do so, subject to     *
;    the following conditions:                                                 *
;                                                                              *
;    The above copyright notice and this permission notice shall be            *
;    included in all copies or substantial portions of the Software.           *
;                                                                              *
;    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,           *
;    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF        *
;    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.    *
;    IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY      *
;    CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,      *
;    TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE         *
;    SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                    *
;                                                                              *
;*******************************************************************************
;                                                                              *
;    Filename: usb.inc                                                         *
;    Date: 2024-11-12                                                          *
;    File Version: 1.0.1                                                       *
;    Author: John Izzard                                                       *
;    Company: N/A                                                              *
;    Description: USB related constants and defines.                           *
;                                                                              *
;*******************************************************************************
;                                                                              *
;    Revision History:                                                         *
;    File Version 1.0.1 - 2024-11-12                                           *
;    - Changed: MIT license.                                                   *
;                                                                              *
;    File Version 1.0.0 - 2021-05-01                                           *
;    - Added: Initial release.                                                 *
;                                                                              *
;*******************************************************************************
    
    ifndef USB_INC
    #define USB_INC
    
    radix dec
    
; **** USB STATES **************************************************************
STATE_DETACHED   equ 0
STATE_ATTACHED   equ 1
STATE_POWERED    equ 2
STATE_DEFAULT    equ 3
STATE_ADDRESS    equ 4
STATE_SUSPENDED  equ 5
STATE_CONFIGURED equ 6

; ******************************************************************************


; **** CONTROL TRANSFER STAGES *************************************************
SETUP_STAGE      equ 0
DATA_IN_STAGE    equ 1
DATA_OUT_STAGE   equ 2
STATUS_IN_STAGE  equ 3
STATUS_OUT_STAGE equ 4

; ******************************************************************************


; **** PIDS ********************************************************************
; Token PIDs
PID_OUT_TOKEN   equ b'0001'
PID_IN_TOKEN    equ b'1001'
PID_SOF_TOKEN   equ b'0101'
PID_SETUP_TOKEN equ b'1101'

; Data PIDs
PID_DATA0 equ b'0011'
PID_DATA1 equ b'1011'
PID_DATA2 equ b'0111'
PID_MDATA equ b'1111'

; Handshake PIDs
PID_ACK_HANDSHAKE   equ b'0010'
PID_NAK_HANDSHAKE   equ b'1010'
PID_STALL_HANDSHAKE equ b'1110'
PID_NYET_HANDSHAKE  equ b'0110'

; Special PIDs
PID_PREamble equ b'1100'
PID_ERR      equ b'1100'
PID_Split    equ b'1000'
PID_Ping     equ b'0100'

; ******************************************************************************


; **** USTAT *******************************************************************
; Mask for isolating Endpoint out of USTAT.
USTAT_ENDP_MASK equ b'01111000'

; ******************************************************************************


; **** BDT *********************************************************************
; Buffer Descriptor bits.
BDn_STAT_BC8      equ 0
BDn_STAT_BC9      equ 1	    
BDn_STAT_PID_MASK equ b'00111100'
BDn_STAT_PID0     equ 2
BDn_STAT_PID1     equ 3
BDn_STAT_PID2     equ 4
BDn_STAT_PID3     equ 5
BDn_STAT_BSTALL   equ 2
BDn_STAT_DTSEN    equ 3
BDn_STAT_DTS      equ 6
BDn_STAT_UOWN     equ 7

; ******************************************************************************


; **** DESCRIPTORS *************************************************************
; Size of Descriptors.
DEVICE_DESC_SIZE      equ 18
CONFIG_DESC_SIZE      equ 9
INT_DESC_SIZE         equ 9
EP_DESC_SIZE          equ 7
TOTAL_DESC_SIZE       equ CONFIG_DESC_SIZE+INT_DESC_SIZE+EP_DESC_SIZE+EP_DESC_SIZE
STR_Z_DESC_SIZE       equ 4
STR_VENDOR_DESC_SIZE  equ 14
STR_PRODUCT_DESC_SIZE equ 24
STR_SERIAL_DESC_SIZE  equ 26

; Number of string descriptors (currently not used).
NUM_STR_DESCRIPTORS equ 2
 
; ******************************************************************************


; **** USB FLAGS ***************************************************************
; USB Flags  
NEED_RESET     equ 0
UPDATE_ADDRESS equ 1
SEND_SHORT     equ 2
     
; ******************************************************************************
     
     
; **** ENDPOINT STATUS FLAGS ***************************************************
; EPx Status Flags.
EP_STAT_DATA_TOGGLE equ 0
EP_STAT_HALT        equ 1
EP_STAT_LAST_PPB    equ 2
    
; ******************************************************************************


; **** USB VARIABLES ***********************************************************
; Sizes of variable blocks.
BDT_SIZE          equ 16
EP0_SIZE          equ 8
EP1_SIZE          equ 64
USB_VARS_SIZE     equ 12
SHR_USB_VARS_SIZE equ 4
    
; USB Variable Adresses.
BDT_ADDRESS          equ 0x0020
EP0_OUT_ADDRESS      equ BDT_ADDRESS+BDT_SIZE     ;0x30
EP0_IN_ADDRESS       equ EP0_OUT_ADDRESS+EP0_SIZE ;0x38
SETUP_ADDRESS        equ EP0_IN_ADDRESS+EP0_SIZE  ;0x40
USB_VARS_ADDRESS     equ SETUP_ADDRESS+EP0_SIZE   ;0x48
SHR_USB_VARS_ADDRESS equ 0x70
EP1_OUT_ADDRESS      equ 0x0120
EP1_IN_ADDRESS       equ 0x01A0

; ******************************************************************************
       
    endif