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
;    Filename: usb_ch9.inc                                                     *
;    Date: 2024-11-12                                                          *
;    File Version: 1.0.1                                                       *
;    Author: John Izzard                                                       *
;    Company: N/A                                                              *
;    Description: USB Spec 2.0 Chapter 9 related masks and constants.          *
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
    
    ifndef USB_CH9_INC
    #define USB_CH9_INC
    
    radix dec
    
; **** SETUP PACKET ************************************************************
; bmRequestType D7: Data transfer direction
DIRECTION_MASK equ b'10000000'
HOST_TO_DEVICE equ 0x00
DEVICE_TO_HOST equ 0x01

; bmRequestType D6...5: Type
TYPE_MASK equ b'01100000'
STANDARD  equ 0x00
CLASS     equ 0x01
VENDOR    equ 0x02

; bmRequestType D4...0: Recipient
RECIPIENT_MASK equ b'00011111'
DEVICE         equ 0x00
INTERFACE      equ 0x01
ENDPOINT       equ 0x02
OTHER          equ 0x03

;  Standard Request Codes (bRequest)
GET_STATUS        equ 0x00
CLEAR_FEATURE     equ 0x01
SET_FEATURE       equ 0x03
SET_ADDRESS       equ 0x05
GET_DESCRIPTOR    equ 0x06
SET_DESCRIPTOR    equ 0x07
GET_CONFIGURATION equ 0x08
SET_CONFIGURATION equ 0x09
GET_INTERFACE     equ 0x0A
SET_INTERFACE     equ 0x0B
SYNC_FRAME        equ 0x0C

; ******************************************************************************


; **** SET / CLEAR FEATURE *****************************************************
; Standard Feature Selectors
ENDPOINT_HALT        equ 0
DEVICE_REMOTE_WAKEUP equ 1
TEST_MODE            equ 2

; Test Mode Selectors
Test_J            equ 1
Test_K            equ 2
Test_SE0_NAK      equ 3
Test_Packet       equ 4
Test_Force_Enable equ 5

; ******************************************************************************


; **** DESCRIPTOR TYPES ********************************************************
DEVICE_DESC_TYPE             equ 1
CONFIGURATION_DESC_TYPE      equ 2
STRING_DESC_TYPE             equ 3
INTERFACE_DESC_TYPE          equ 4
ENDPOINT_DESC_TYPE           equ 5
DEVICE_QUALIFIER_DESC_TYPE   equ 6
OTHER_SPEED_CONFIG_DESC_TYPE equ 7
INTERFACE_POWER_DESC_TYPE    equ 8

; ******************************************************************************


; **** CLASS CODES *************************************************************
NA        equ 0
CDC_CLASS equ 2
HID_CLASS equ 3
MSC_CLASS equ 8

; ******************************************************************************
 
 
; **** SETUP PACKET DATA *******************************************************
; Setup Packet Data
bmRequestType equ 0
bRequest      equ 1
wValueL       equ 2
wValueH	      equ 3
wIndexL	      equ 4
wIndexH	      equ 5
wLengthL      equ 6
wLengthH      equ 7
      
; Set/Clear Feature Data
FeatureSelectorL       equ 2
FeatureSelectorH       equ 3
ZeroInterfaceEndpointL equ 4
ZeroInterfaceEndpointH equ 5
 
; Set/Get Descriptor Data
DescriptorIndex   equ 2
DescriptorType    equ 3
LanguageID_ZERO_L equ 4
LanguageID_ZERO_H equ 5
DescriptorLengthL equ 6
DescriptorLengthH equ 7
 
; ******************************************************************************
 
    endif