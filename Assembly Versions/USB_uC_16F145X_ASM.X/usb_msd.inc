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
;    Filename: usb_msd.inc                                                     *
;    Date: 18/04/2021                                                          *
;    File Version: 1.00                                                        *
;    Author: John Izzard                                                       *
;    Company: N/A                                                              *
;    Description: MSD related constants and defines.                           *
;                                                                              *
;*******************************************************************************
;                                                                              *
;    Revision History:                                                         *
;                                                                              *
;*******************************************************************************
    
    ifndef USB_MSD_INC
    #define USB_MSD_INC
    
    radix dec
    
; **** MSD FLAGS ***************************************************************
END_DATA_SHORT equ 0
WAIT_FOR_BOMSR equ 1
UNIT_ATTENTION equ 2
 
; ******************************************************************************

 
; **** MSD OFFSETS *************************************************************
; CBW Offsets.
#define dCBWSignature_0          csw
#define dCBWSignature_1          csw+1
#define dCBWSignature_2          csw+2
#define dCBWSignature_3          csw+3
#define dCBWTag_0                csw+4
#define dCBWTag_1                csw+5
#define dCBWTag_2                csw+6
#define dCBWTag_3                csw+7
#define dCBWDataTransferLength_0 csw+8
#define dCBWDataTransferLength_1 csw+9
#define dCBWDataTransferLength_2 csw+10
#define dCBWDataTransferLength_3 csw+11
#define bCBWFlags                csw+12    

; CSW Offsets.
#define dCSWSignature_0   csw
#define dCSWSignature_1   csw+1
#define dCSWSignature_2   csw+2
#define dCSWSignature_3   csw+3
#define dCSWTag_0         csw+4
#define dCSWTag_1         csw+5
#define dCSWTag_2         csw+6
#define dCSWTag_3         csw+7
#define dCSWDataResidue_0 csw+8
#define dCSWDataResidue_1 csw+9
#define dCSWDataResidue_2 csw+10
#define dCSWDataResidue_3 csw+11
#define bCSWStatus        csw+12

; Command Block Offsets.
#define CB_0 EP1_out+15
#define CB_1 EP1_out+16
#define CB_2 EP1_out+17
#define CB_3 EP1_out+18
#define CB_4 EP1_out+19
#define CB_5 EP1_out+20
#define CB_6 EP1_out+21
#define CB_7 EP1_out+22
#define CB_8 EP1_out+23
#define CB_9 EP1_out+24
 
; ******************************************************************************
 
 
; **** SCSI OFFSETS ************************************************************
; SCSI Generic Command Offsets.
#define SCSI_OP_CODE  CB_0 ;EP1_out+15
#define SCSI_LBA_3    CB_2 ;EP1_out+17
#define SCSI_LBA_2    CB_3 ;EP1_out+18
#define SCSI_LBA_1    CB_4 ;EP1_out+19
#define SCSI_LBA_0    CB_5 ;EP1_out+20
#define SCSI_TF_LEN_1 CB_7 ;EP1_out+22
#define SCSI_TF_LEN_0 CB_8 ;EP1_out+23

; SCSI REQUEST_SENSE Command ALLOCATION_LENGTH Offset.
#define SCSI_REQ_SEN_AL_LEN CB_4 ; EP1_out+19

; SCSI INQUIRY Command ALLOCATION_LENGTH Offset.
#define SCSI_INQ_AL_LEN_1 CB_3 ;EP1_out+18
#define SCSI_INQ_AL_LEN_0 CB_4 ;EP1_out+19
    
; SCSI MODE_SENSE Command ALLOCATION_LENGTH Offset.
#define SCSI_MODE_SEN_AL_LEN CB_4 ; EP1_out+19
   
; SCSI READ_CAPACITY Command PMI Offset.
#define SCSI_READ_CAP_PMI CB_8 ; EP1_out+23

; ******************************************************************************
 
 
; **** VOLUME SETTINGS *********************************************************
BYTES_PER_BLOCK_LE     equ 0x200
VOL_CAPACITY_IN_BLOCKS equ 0x1013
LAST_BLOCK_LE          equ 0x1012 ; 4114 (VOL_CAPACITY_IN_BLOCKS - 1)
LAST_BLOCK_BE          equ 0x1210
 
; ******************************************************************************
 
 
; ******************************************************************************
DEV_Dn equ 0
DEV_Di equ 1
DEV_Do equ 2

; ******************************************************************************
 

; ******************************************************************************
CASE_1  equ 0
CASE_2  equ 1
CASE_3  equ 2
CASE_4  equ 3
CASE_5  equ 4
CASE_6  equ 5
CASE_7  equ 6
CASE_8  equ 7
CASE_9  equ 0
CASE_10 equ 1
CASE_11 equ 2
CASE_12 equ 3
CASE_13 equ 4
 
; ******************************************************************************

 
; **** COMMAND BLOCK STATUS VALUES *********************************************
COMMAND_PASSED equ 0
COMMAND_FAILED equ 1
PHASE_ERROR    equ 2
    
; ******************************************************************************
    
    
; **** MSD VARIABLES ***********************************************************
; Sizes of variable blocks.
MSD_VARS_SIZE equ 22

; File Register Adresses.
MSD_VARS_ADDRESS     equ USB_VARS_ADDRESS+USB_VARS_SIZE         ; 0x54
SHR_MSD_VARS_ADDRESS equ SHR_USB_VARS_ADDRESS+SHR_USB_VARS_SIZE ; 0x74
 
; ******************************************************************************
 
    endif