;*******************************************************************************
;                                                                              *
;    This program is free software: you can redistribute it and/or modify      *
;    it under the terms of the GNU General Public License as published by      *
;    the Free Software Foundation, either version 3 of the License, or         *
;    (at your option) any later version.                                       *
;                                                                              *
;    This program is distributed in the hope that it will be useful,           *
;    but WITHOUT ANY WARRANTY; without even the implied warranty of            *
;    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
;    GNU General Public License for more details.                              *
;                                                                              *
;    You should have received a copy of the GNU General Public License         *
;    along with this program.  If not, see <https://www.gnu.org/licenses/>.    *
;                                                                              *
;*******************************************************************************
;                                                                              *
;    Filename: usb_descriptors.asm                                             *
;    Date: 18/04/2021                                                          *
;    File Version: 1.00                                                        *
;    Author: John Izzard                                                       *
;    Company: N/A                                                              *
;    Description: Contains all the USB descriptors used.                       *
;                                                                              *
;*******************************************************************************
;                                                                              *
;    Revision History:                                                         *
;                                                                              *
;*******************************************************************************

    radix dec
    include "usb.inc"
    include "config.inc"
    include "usb_ch9.inc"
    include "usb_msd.inc"

; Descriptor Settings
DEV_DESC_VID equ 0x04D8 ; Device Vendor ID
DEV_DESC_PID equ 0xEB78 ; Device Product ID
REL_NUM      equ 0x0200 ; Release Number

; Global Descriptor Symbols
    global device_descriptor, configuration_descriptor, interface_descriptor
    global ep1_out_descriptor, ep1_in_descriptor
    global string_zero_descriptor, serial_string_descriptor

;*******************************************************************************
; Descriptors
;*******************************************************************************
descriptors code ;DESC_ADDR
device_descriptor:
    dt DEVICE_DESC_SIZE
    dt DEVICE_DESC_TYPE
    dt 0x00, 0x02
    dt 0x00
    dt 0x00
    dt 0x00
    dt EP0_SIZE
    dt low DEV_DESC_VID, high DEV_DESC_VID
    dt low DEV_DESC_PID, high DEV_DESC_PID
    dt low REL_NUM, high REL_NUM
    dt 0x00
    dt 0x00
    dt 0x01
    dt 0x01
configuration_descriptor:
    dt CONFIG_DESC_SIZE
    dt CONFIGURATION_DESC_TYPE
    dt low TOTAL_DESC_SIZE, high TOTAL_DESC_SIZE
    dt 0x01
    dt 0x01
    dt 0x00
    dt 0xC0
    dt 50
interface_descriptor:
    dt INT_DESC_SIZE
    dt INTERFACE_DESC_TYPE
    dt 0x00
    dt 0x00
    dt 0x02
    dt MSC_CLASS
    dt 0x06
    dt 0x50
    dt 0x00
ep1_out_descriptor:
    dt EP_DESC_SIZE
    dt ENDPOINT_DESC_TYPE
    dt 0x81
    dt 0x02
    dt EP1_SIZE, 0x00
    dt 0x01
ep1_in_descriptor:
    dt EP_DESC_SIZE
    dt ENDPOINT_DESC_TYPE
    dt 0x01
    dt 0x02
    dt EP1_SIZE, 0x00
    dt 0x01
string_zero_descriptor:
    dt STR_Z_DESC_SIZE
    dt STRING_DESC_TYPE
    dt 0x09
    dt 0x04
;vendor_string_descriptor:
;    dt STR_VENDOR_DESC_SIZE
;    dt STRING_DESC_TYPE
;    dt 'J',0
;    dt 'o',0
;    dt 'h',0
;    dt 'n',0
;    dt 'n',0
;    dt 'y',0
;product_string_descriptor:
;    dt STR_PRODUCT_DESC_SIZE
;    dt STRING_DESC_TYPE
;    dt 'U',0
;    dt 'S',0
;    dt 'B',0
;    dt ' ',0
;    dt 'u',0
;    dt 'C',0
;    dt ' ',0
;    dt '1',0
;    dt '4',0
;    dt '5',0
;    dt 'x',0
serial_string_descriptor:
    dt STR_SERIAL_DESC_SIZE
    dt STRING_DESC_TYPE
    dt '1',0
    dt '2',0
    dt '3',0
    dt '4',0
    dt '5',0
    dt '6',0
    dt '7',0
    dt '8',0
    dt '9',0
    dt '0',0
    dt '9',0
    dt '9',0

    end
