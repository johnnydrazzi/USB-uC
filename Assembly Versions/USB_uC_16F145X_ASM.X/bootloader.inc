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
;    Filename: bootloader.inc                                                  *
;    Date: 2024-11-12                                                          *
;    File Version: 1.0.1                                                       *
;    Author: John Izzard                                                       *
;    Company: N/A                                                              *
;    Description: Bootloader related constants and defines.                    *
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

    ifndef BOOTLOADER_INC
    #define BOOTLOADER_INC
	
    include "config.inc"
    
    radix dec
    
;**** BOOT STATES **************************************************************
BOOT_DUMMY    equ 0
BOOT_LOAD_HEX equ 1
BOOT_FINISHED equ 2
 
;*******************************************************************************

 
;**** HEX STATES ***************************************************************
HEX_START       equ 0
HEX_REC_LEN     equ 1
HEX_LOAD_OFFSET equ 2
HEX_RECTYPE     equ 3
HEX_DATA        equ 4
HEX_ELA         equ 5
HEX_CHKSUM      equ 6
      
;*******************************************************************************
      
      
;**** HEX FILE RECORD TYPES ****************************************************
DATA_REC equ 0
EOF_REC  equ 1
ELA_REC  equ 4
  
;*******************************************************************************

  
;**** SECTOR ADDRESSES *********************************************************
BOOT_SECT_ADDR     equ 0
FAT_SECT_ADDR      equ 1
ROOT_SECT_ADDR     equ 18
DATA_SECT_ADDR     equ 19
ABOUT_SECT_ADDR    equ 19
    if USE_ABOUT_FILE == 1
PROG_MEM_SECT_ADDR equ 20
    else
PROG_MEM_SECT_ADDR equ 19
    endif

;*******************************************************************************
  
    
;**** FAT **********************************************************************
ROOT_ENTRY_COUNT equ 16
FAT_SIZE         equ 17
FILE_SIZE        equ 0x4000-(USER_PROGRAM*2)
    if FILE_SIZE % 512
FILE_CLUSTERS    equ (FILE_SIZE/512)+1
    else
FILE_CLUSTERS    equ (FILE_SIZE/512)
    endif
    if USE_ABOUT_FILE
PROG_MEM_CLUST   equ 3
    else
PROG_MEM_CLUST   equ 2
    endif

;*******************************************************************************
	 
  
;**** ABOUT FILE ***************************************************************
#define ABOUT_FILE_SIZE 87 ; Can't be greater than 128.
; ABOUT_STR_1 needs to be exactly 64 characters, when ABOUT_FILE_SIZE is > 64.
#define ABOUT_STR_1 "<html><script>window.location=\"https://github.com/johnnydrazzi/U"
#define ABOUT_STR_2 "SB-uC\";</script></html>"
 
;*******************************************************************************
    
    
;**** BOOT VARIABLES ***********************************************************
BOOT_VARS_ADDRESS      equ 0x00A0
FLASH_VARS_ADDRESS     equ 0x01E0
FLASH_ROW_VARS_ADDRESS equ 0x0220
 
;*******************************************************************************
 
    endif