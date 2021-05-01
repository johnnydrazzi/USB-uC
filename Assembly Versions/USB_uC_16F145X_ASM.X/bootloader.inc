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
;    Filename: bootloader.inc                                                  *
;    Date: 18/04/2021                                                          *
;    File Version: 1.00                                                        *
;    Author: John Izzard                                                       *
;    Company: N/A                                                              *
;    Description: Bootloader related constants and defines.                    *
;                                                                              *
;*******************************************************************************
;                                                                              *
;    Revision History:                                                         *
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