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
;    Filename: usb_msd.asm                                                     *
;    Date: 18/04/2021                                                          *
;    File Version: 1.00                                                        *
;    Author: John Izzard                                                       *
;    Company: N/A                                                              *
;    Description: MSD Stack is here. Anything MSD layer or SCSI is dealt with  *
;                 here.                                                        *
;                                                                              *
;*******************************************************************************
;                                                                              *
;    Revision History:                                                         *
;                                                                              *
;*******************************************************************************

    radix dec
    include "p16f1454.inc"
    include "config.inc"
    include "usb.inc"
    include "usb_ch9.inc"
    include "usb_msd.inc"

; USB class request
BOMSR equ 0xFF

; MSD State machine states
MSD_CBW           equ 0
MSD_DATA_SENT     equ 1
MSD_CSW           equ 2
MSD_READ_DATA     equ 3
MSD_WRITE_DATA    equ 4
MSD_WAIT_CLEAR    equ 5
MSD_WAIT_BOMSR    equ 6

; MSD Variables.
    global msd_byte_of_sect
MSD_VARS udata MSD_VARS_ADDRESS
sense_key             res  1
additional_sense_code res  1
csw                   res 13
dev_expect            res  1
TF_LEN_BYTES          res  4
msd_byte_of_sect      res  2

    global START_LBA, LBA
; Shared MSD Variables.
SHARED_MSD_VARS udata_shr SHR_MSD_VARS_ADDRESS
msd_state     res 1
msd_flags     res 1
TF_LEN        res 2
START_LBA     res 4
LBA           res 4

; External Variable Symbols
    extern BD1_OUT_STAT, BD1_OUT_CNT, BD1_OUT_ADRL, BD1_OUT_ADRH
    extern BD1_IN_STAT, BD1_IN_CNT, BD1_IN_ADRL, BD1_IN_ADRH
    extern SETUP_bmRequestType, SETUP_bRequest, SETUP_wValueL, SETUP_wValueH
    extern SETUP_wIndexL, SETUP_wIndexH, SETUP_wLengthL, SETUP_wLengthH
    extern USTAT_copy, EP1_out_stat, EP1_in_stat, temp
    extern control_stage, cnt
    extern EP1_out, EP1_in
    extern boot_state

; External Function Symbols
    extern arm_in_status, usb_request_error, mem_copy, mem_set
    extern msd_tx_sector, msd_rx_sector

; Global MSD Function Symbols
    global msd_tasks, msd_clear_halt, app_init, msd_clear_ep_toggle, class_request

msdCode code
;*******************************************************************************
; Init MSD
;*******************************************************************************
; Initialize EP1, EPn_xxx_stats and reset variables related to MSD.
; BDT for EP1 has been setup in usb_init.
app_init:
    banksel     UEP1
    movlw       (1<<EPHSHK)|(1<<EPCONDIS)|(1<<EPOUTEN)|(1<<EPINEN)
    movwf       UEP1
    banksel     0
    bcf         EP1_out_stat,EP_STAT_HALT
    bcf         EP1_in_stat,EP_STAT_HALT
    call        msd_clear_ep_toggle
    clrf        msd_flags
    clrf        sense_key
    clrf        additional_sense_code
    banksel     boot_state
    movlw       1
    movwf       boot_state ; boot_state = BOOT_DUMMY
    goto        setup_cbw

;*******************************************************************************
; Class Request
;*******************************************************************************
; Services class requests, only BOMSR is supported.
; *Possible space saving, remove valid check.
class_request:
    ; Check that bRequest is BOMSR, otherwise respond with request_error.
    movlw       BOMSR
    xorwf       SETUP_bRequest,W
    bnz         usb_request_error
    ; Check for valid BOMSR, wValue, wIndex and wLength should be zero.
    movfw       SETUP_wValueH
    iorwf       SETUP_wValueL,W
    iorwf       SETUP_wIndexH,W
    iorwf       SETUP_wIndexL,W
    iorwf       SETUP_wLengthH,W
    iorwf       SETUP_wLengthL,W
    bnz         usb_request_error
    ; If EP1_OUT isn't already armed, arm it.
    btfss       BD1_OUT_STAT,BDn_STAT_UOWN
    call        setup_cbw
    ; Clear flags
    clrf        msd_flags
    ; Set the control_stage to STATUS_IN_STAGE and arm EP0_IN for status.
    clrf        control_stage
    bsf         control_stage,STATUS_IN_STAGE
    goto        arm_in_status

;*******************************************************************************
; Clear MSD Endpoint Toggle
;*******************************************************************************
; Clears the DATA_TOGGLE values for EP1.
msd_clear_ep_toggle:
    bcf         EP1_out_stat,EP_STAT_DATA_TOGGLE
    bcf         EP1_in_stat,EP_STAT_DATA_TOGGLE
    return

;*******************************************************************************
; Setup Command Block Wrapper
;*******************************************************************************
; Sets msd_state to MSD_CBW and arms EP1_OUT ready to receive CBW.
setup_cbw:
    movlw       (1<<MSD_CBW)
    movwf       msd_state
    goto        msd_arm_ep_out

;*******************************************************************************
; Setup Command Status Wrapper
;*******************************************************************************
; Changes last character of signature to make USBS, copies csw variable to
; EP1_in, sets msd_state to MSD_CSW, and arms EP1_IN ready to send CSW.
setup_csw:
    banksel     0
    movlw       'S'
    movwf       dCSWSignature_3
    clrf        FSR0H ; csw is < 0x100.
    movlw       low csw
    movwf       FSR0L
    movlw       high EP1_in
    movwf       FSR1H
    movlw       low EP1_in
    movwf       FSR1L
    movlw       13
    movwf       cnt
    call        mem_copy
    movlw       (1<<MSD_CSW)
    movwf       msd_state
    movlw       13
    goto        msd_arm_ep_in

;*******************************************************************************
; Arm MSD Endpoint
;*******************************************************************************
; Arms EP1_out.
msd_arm_ep_out:
    banksel     0
    movlw       EP1_SIZE
    movwf       BD1_OUT_CNT
    clrf        BD1_OUT_STAT
    btfsc       EP1_out_stat,EP_STAT_DATA_TOGGLE
    bsf         BD1_OUT_STAT,BDn_STAT_DTS
    bsf         BD1_OUT_STAT,BDn_STAT_DTSEN
    bsf         BD1_OUT_STAT,BDn_STAT_UOWN
    return

; Arms EP1_in, requires WREG being pre-loaded with CNT.
msd_arm_ep_in:
    banksel     0
    movwf       BD1_IN_CNT
    clrf        BD1_IN_STAT
    btfsc       EP1_in_stat,EP_STAT_DATA_TOGGLE
    bsf         BD1_IN_STAT,BDn_STAT_DTS
    bsf         BD1_IN_STAT,BDn_STAT_DTSEN
    bsf         BD1_IN_STAT,BDn_STAT_UOWN
    return

;*******************************************************************************
; MSD State Machine
;*******************************************************************************
; Called whenever a transaction complete flag is refering to EP1 in USTAT.
msd_tasks:
    banksel     0
    ; Check the direction of transaction.
    btfsc       USTAT_copy,DIR
    goto        _msd_trans_dir_in
_msd_trans_dir_out:
    ; Toggle DATA toggle bit.
    movlw       (1<<EP_STAT_DATA_TOGGLE)
    xorwf       EP1_out_stat,F
    ; Check if msd_state == MSD_WRITE_DATA.
    btfsc       msd_state,MSD_WRITE_DATA
    goto        service_write10
    ; Check if msd_state == MSD_CBW.
    btfsc       msd_state,MSD_CBW
    goto        service_cbw
    return
_msd_trans_dir_in:
    ; Toggle DATA toggle bit.
    movlw       (1<<EP_STAT_DATA_TOGGLE)
    xorwf       EP1_in_stat,F
    ; Check if msd_state == MSD_READ_DATA.
    btfsc       msd_state,MSD_READ_DATA
    goto        service_read10
    ; Check if msd_state == MSD_DATA_SENT.
    btfsc       msd_state,MSD_DATA_SENT
    goto        _msd_data_sent
    ; Check if msd_state == MSD_CSW.
    btfsc       msd_state,MSD_CSW
    goto        setup_cbw
    return
_msd_data_sent:
    btfss       msd_flags,END_DATA_SHORT
    goto        setup_csw
    call        msd_stall_EP_IN
    bcf         msd_flags,END_DATA_SHORT
    movlw       (1<<MSD_WAIT_CLEAR)
    movwf       msd_state
    return

;*******************************************************************************
; Clear Halt
;*******************************************************************************
; Will allow the clearing of halt condition from within clear_feature request,
; but only if not pending a BOMSR, after a invalid CBW has occured.
; See 6.6.1 of MSC BOT spec.
msd_clear_halt:
    btfsc       msd_flags,WAIT_FOR_BOMSR
    return
    bcf         INDF0,EP_STAT_DATA_TOGGLE
    btfss       INDF0,EP_STAT_HALT
    goto        $+3
    bcf         INDF0,EP_STAT_HALT
    clrf        INDF1
    ; Update state machine, check if msd_state is MSD_WAIT_BOMSR or
    ; MSD_WAIT_CLEAR.
    btfsc       msd_state,MSD_WAIT_BOMSR
    goto        setup_cbw ; An invalid CBW must have occured.
    btfsc       msd_state,MSD_WAIT_CLEAR
    goto        setup_csw ; An EP was stalled due to ending data short,
                          ; COMMAND_FAILED, or PHASE_ERROR.
    return

;*******************************************************************************
; Service CBW
;*******************************************************************************
; A transaction completed on EP1_OUT and msd_state was equal to MSD_CBW.
; Will service the CBW and perform the SCSI command.
; *Possbile space saving remove CBW valid check. See if can remove some
; supported SCSI commands.
service_cbw:
    banksel     0
    ; CBW valid?
    ; CBW received needs to be 31 bytes (check BD1_OUT_CNT).
    movfw       BD1_OUT_CNT
    xorlw       31
    bnz         _cbw_not_valid
    ; Check that CBW's dCBWSignature = "USBC".
    banksel     EP1_out
    movfw       EP1_out
    xorlw       'U'
    bnz         _cbw_not_valid
    movfw       EP1_out+1
    xorlw       'S'
    bnz         _cbw_not_valid
    movfw       EP1_out+2
    xorlw       'B'
    bnz         _cbw_not_valid
    movfw       EP1_out+3
    xorlw       'C'
    bz          _cbw_is_valid
_cbw_not_valid:
    banksel     0
    bsf         msd_flags,WAIT_FOR_BOMSR ; See msd_clear_halt for use.
    call        cause_bomsr
    movlw       (1<<MSD_WAIT_BOMSR)
    movwf       msd_state
    return
_cbw_is_valid:
    banksel     0
    ; Copy first 13 bytes from CBW to temporary csw variable.
    movlw       high EP1_out
    movwf       FSR0H
    movlw       low EP1_out
    movwf       FSR0L
    clrf        FSR1H ; csw is < 0x100.
    movlw       low csw
    movwf       FSR1L
    movlw       13
    movwf       cnt
    call        mem_copy
    ; Check CBWCB[0] for the SCSI command.
    banksel     EP1_out
    movfw       SCSI_OP_CODE
    xorlw       0x2A ; WRITE_10.
    bz          _write_10
    movfw       SCSI_OP_CODE
    xorlw       0x28 ; READ_10.
    bz          _read_10
    movfw       SCSI_OP_CODE
    xorlw       0x00 ; TEST_UNIT_READY.
    bz          no_data_response
    movfw       SCSI_OP_CODE
    xorlw       0x03 ; REQUEST_SENSE.
    bz          _request_sense
    movfw       SCSI_OP_CODE
    xorlw       0x12 ; INQUIRY.
    bz          _inquiry
    movfw       SCSI_OP_CODE
    xorlw       0x1A ; MODE_SENSE_6.
    bz          _mode_sense_6
    movfw       SCSI_OP_CODE
    xorlw       0x25 ; READ_CAPACITY.
    bz          _read_capacity
    ; Command is invalid, either not recognised or unsupported.
    call        invalid_command_sense
    ; Check if dCBWDataTransferLength != 0, would mean Hn.
    movfw       dCBWDataTransferLength_0
    iorwf       dCBWDataTransferLength_1,W
    iorwf       dCBWDataTransferLength_2,W
    iorwf       dCBWDataTransferLength_3,W
    bnz         _fail_command_check_dir
_fail_command_Hn:
    movlw       1 ; COMMAND_FAILED.
    movwf       bCSWStatus
    call        setup_csw
    return
_fail_command_check_dir:
    btfss       bCBWFlags,7
    goto        _fail_command_Ho
_fail_command_Hi:
    call        msd_stall_EP_IN
    goto        $+2
_fail_command_Ho:
    call        msd_stall_EP_OUT
    movlw       (1<<MSD_WAIT_CLEAR)
    movwf       msd_state
_command_fail_exit:
    movlw       1 ; COMMAND_FAILED.
    movwf       bCSWStatus
    return

;*******************************************************************************
; SCSI Commands
;*******************************************************************************
; *Possible space saving, remove all the validity checks.
_write_10:
    banksel     0
    clrf        dev_expect
    bsf         dev_expect,DEV_Do
    goto        _read_write_10
_read_10:
    banksel     0
    clrf        dev_expect
    bsf         dev_expect,DEV_Di
_read_write_10:
    ; Check if SCSI_TF_LEN == 0.
    banksel     EP1_out
    movfw       SCSI_TF_LEN_0   ; CBWCB[8].
    iorwf       SCSI_TF_LEN_1,W ; CBWCB[7].
    bz          no_data_response
    ; Copy SCSI_TF_LEN (also SCSI_TF_LEN converted to little endian) to TF_LEN.
    ; May seem redundant, but stops lots of banksel's as TF_LEN is in shared
    ; memory.
    movfw       SCSI_TF_LEN_1 ; CBWCB[7].
    movwf       TF_LEN+1
    movfw       SCSI_TF_LEN_0 ; CBWCB[8].
    movwf       TF_LEN
    ; Copy LBA to LBA and LBA_START (also SCSI_LBA converted to little endian).
    movfw       SCSI_LBA_3  ; CBWCB[2].
    movwf       START_LBA+3
    movwf       LBA+3
    movfw       SCSI_LBA_2  ; CBWCB[3].
    movwf       START_LBA+2
    movwf       LBA+2
    movfw       SCSI_LBA_1  ; CBWCB[4].
    movwf       START_LBA+1
    movwf       LBA+1
    movfw       SCSI_LBA_0  ; CBWCB[5].
    movwf       START_LBA
    movwf       LBA
    ; Check if requesting data out of bounds.
    movfw       LBA+3
    iorwf       LBA+2,W
    banksel     0
    bnz         _read_write_10_fail ; LBA[3..2] != 0.
    ; Add TF_LEN and LBA and store in temp.
    movfw       TF_LEN
    movwf       temp
    movfw       TF_LEN+1
    movwf       temp+1
    movfw       LBA      ; LBA[0].
    addwf       temp,F   ; TF_LEN[0].
    movfw       LBA+1    ; LBA[1].
    addwfc      temp+1,F ; TF_LEN[1].
    bc          _read_write_10_fail ; Overflow.
    ; Check for TF_LEN + START_LBA > VOL_CAPACITY_IN_BLOCKS (0x1013).
    movfw       temp+1
    sublw       high VOL_CAPACITY_IN_BLOCKS ; 0x10.
    bnc         _read_write_10_fail ; temp[1] > VOL_CAPACITY_IN_BLOCKS[1].
    bnz         $+5 ; temp[1] < VOL_CAPACITY_IN_BLOCKS[1].
    ; temp[1] == VOL_CAPACITY_IN_BLOCKS[1].
    movfw       temp
    sublw       low VOL_CAPACITY_IN_BLOCKS ; 0x13.
    bnc         _read_write_10_fail ; temp[0] > VOL_CAPACITY_IN_BLOCKS[0].
    ; TF_LEN + START_LBA <= VOL_CAPACITY_IN_BLOCKS.
    ; TF_LEN_BYTES = TF_LEN*512.
    clrf        TF_LEN_BYTES
    clrf        TF_LEN_BYTES+2
    clrf        TF_LEN_BYTES+3
    lslf        TF_LEN,W
    movwf       TF_LEN_BYTES+1
    btfsc       STATUS,C
    bsf         TF_LEN_BYTES+2,0
    lslf        TF_LEN+1,W
    iorwf       TF_LEN_BYTES+1,F
    btfsc       STATUS,C
    bsf         TF_LEN_BYTES+3,0
    call        check_13_cases
    btfss       STATUS,Z
    return      ; PHASE_ERROR occured, both EPs are stalled in check_13_cases.
    clrf        msd_byte_of_sect
    clrf        msd_byte_of_sect+1
    btfss       dev_expect,DEV_Di
    goto        $+4
    call        service_read10
    movlw       (1<<MSD_READ_DATA)
    goto        $+3
    call        msd_arm_ep_out
    movlw       (1<<MSD_WRITE_DATA)
    movwf       msd_state
    return
_read_write_10_fail:
    movlw       0x05 ; ILLEGAL_REQUEST.
    movwf       sense_key
    movlw       0x21 ; ASC_LOGICAL_BLOCK_ADDRESS_OUT_OF_RANGE.
    movwf       additional_sense_code
    goto        _fail_command_check_dir
; Service REQUEST_SENSE SCSI command.
_request_sense:
    movfw       SCSI_REQ_SEN_AL_LEN
    ; Check ALLOCATION_LENGTH == 0.
    bz          no_data_response
    ; ALLOCATION_LENGTH != 0, clear first 18 bytes of EP1_IN.
    movlw       18
    movwf       cnt
    movlw       high EP1_in
    movwf       FSR0H
    movlw       low EP1_in
    movwf       FSR0L
    clrw        ; WREG holds value to set.
    call        mem_set
    ; Fill in sense
    banksel     EP1_in
    movlw       0x70 ; RESPONSE_CODE = CURRENT_FIXED.
    movwf       EP1_in
    movlw       0x0A ; ADDITIONAL_SENSE_LENGTH = 0x0A.
    movwf       EP1_in+7
    banksel     0
    movfw       sense_key
    banksel     EP1_in
    movwf       EP1_in+2
    banksel     0
    movfw       additional_sense_code
    banksel     EP1_in
    movwf       EP1_in+12
    ; check ALLOCATION_LENGTH > 18.
    banksel     EP1_out
    movfw       SCSI_REQ_SEN_AL_LEN
    sublw       18
    bc          $+3
    ; ALLOCATION_LENGTH > 18.
    movlw       18
    goto        send_data_response
    ; ALLOCATION_LENGTH <= 18.
    movfw       SCSI_REQ_SEN_AL_LEN
    goto        send_data_response
; Service INQUIRY SCSI command.
_inquiry:
    movfw       SCSI_INQ_AL_LEN_0
    iorwf       SCSI_INQ_AL_LEN_1,W
    bz          no_data_response ; ALLOCATION_LENGTH must be 0.
    ; ALLOCATION_LENGTH != 0, preload EP1_in with enquiry data.
    movlw       low scsi_inquiry
    movwf       FSR0L
    movlw       high (scsi_inquiry + 0x8000)
    movwf       FSR0H
    movlw       low EP1_in
    movwf       FSR1L
    movlw       high EP1_in
    movwf       FSR1H
    movlw       36
    movwf	cnt
    call        mem_copy
    ; high byte != 0, > 36.
    movfw       SCSI_INQ_AL_LEN_1
    bnz         _inquiry_exit_grt_36
    ; Check if lower byte greater than 36.
    movfw       SCSI_INQ_AL_LEN_0
    sublw       36
    bnc         _inquiry_exit_grt_36
    ; lower byte <= 36.
    movfw       SCSI_INQ_AL_LEN_0
    goto        send_data_response
    ; ALLOCATION_LENGTH_BYTES > 36.
_inquiry_exit_grt_36:
    movlw       36
    goto        send_data_response
; Service MODE_SENSE_6 SCSI command.
_mode_sense_6:
    movfw       SCSI_MODE_SEN_AL_LEN
    bz          no_data_response
    movlw       0x03
    banksel     EP1_in
    movwf       EP1_in   ; mode_sense.MODE_DATA_LENGTH          = 0x03.
    clrf        EP1_in+1 ; mode_sense.MEDIUM_TYPE               = 0x00.
    clrf        EP1_in+2 ; mode_sense.DEVICE_SPECIFIC_PARAMETER = 0x00.
    clrf        EP1_in+3 ; mode_sense.BLOCK_DESCRIPTOR_LENGTH   = 0x00.
    banksel     EP1_out
    movfw       SCSI_MODE_SEN_AL_LEN
    sublw       4
    bc          $+3
    ; ALLOCATION_LENGTH > 4.
    movlw       4
    goto        send_data_response
    ; ALLOCATION_LENGTH <= 4.
    movfw       SCSI_MODE_SEN_AL_LEN
    goto        send_data_response
; Service READ_CAPACITY SCSI command.
_read_capacity:
    movfw       SCSI_READ_CAP_PMI
    andlw       0x01
    bnz         _read_capacity_valid
    movfw       SCSI_LBA_3
    iorwf       SCSI_LBA_2,W
    iorwf       SCSI_LBA_1,W
    iorwf       SCSI_LBA_0,W
    bz          _read_capacity_valid
    call        cause_bomsr
    banksel     0
    movlw       0x05 ; SENSE_KEY = ILLEGAL_REQUEST.
    movwf       sense_key
    movlw       0x24
    movwf       additional_sense_code
    return
_read_capacity_valid:
    ; Check LOGICAL_BLOCK_ADDRESS > LAST_BLOCK_LE.
    movfw       SCSI_LBA_3   ; LOGICAL_BLOCK_ADDRESS > LAST_BLOCK_LE.
    iorwf       SCSI_LBA_2,W
    bnz         _LBA_greater_LB
    movfw       SCSI_LBA_1
    sublw       high LAST_BLOCK_LE
    bnc         _LBA_greater_LB  ; LOGICAL_BLOCK_ADDRESS_H > LAST_BLOCK_LE_H.
    bnz         _LBA_less_eq_LB  ; LOGICAL_BLOCK_ADDRESS_H < LAST_BLOCK_LE_H.
    ; LOGICAL_BLOCK_ADDRESS_H == LAST_BLOCK_LE_H.
    movfw       SCSI_LBA_0
    sublw       low LAST_BLOCK_LE
    bc          _LBA_less_eq_LB
_LBA_greater_LB:
    ; LOGICAL_BLOCK_ADDRESS > LAST_BLOCK_LE.
    banksel     EP1_in
    movlw       0xFF
    movwf       EP1_in
    movwf       EP1_in+1
    movwf       EP1_in+2
    movwf       EP1_in+3
    goto        _read_capacity_exit
_LBA_less_eq_LB:
    ; LOGICAL_BLOCK_ADDRESS <= LAST_BLOCK_LE.
    banksel     EP1_in
    clrf        EP1_in
    clrf        EP1_in+1
    movlw       high LAST_BLOCK_LE
    movwf       EP1_in+2
    movlw       low LAST_BLOCK_LE
    movwf       EP1_in+3
_read_capacity_exit:
    ; BLOCK_LENGTH_IN_BYTES = BYTES_PER_BLOCK_BE (512).
    clrf        EP1_in+4
    clrf        EP1_in+5
    movlw       0x02
    movwf       EP1_in+6
    clrf        EP1_in+7
    ; Send response.
    banksel     0
    movlw       8
    goto        send_data_response


;*******************************************************************************
; Check 13 Cases
;*******************************************************************************
; Chapter 6.7 of MSC BOT Spec 1.0
; ------ CASE ACTIONS ------
;CASE_1 (Hn = Dn):
;	- bCSWStatus = COMMAND_PASSED.
;	- dCSWDataResidue = 0.
;       - setup_csw.
;
;CASE_2 (Hn < Di) | CASE_3 (Hn < Do):
;	- bCSWStatus = PHASE_ERROR.
;       - dCSWDataResidue is ignored by host.
;	- stall_ep_in, once EP is cleared setup_csw.
;
;CASE_4 (Hi > Dn) | CASE_5 (Hi > Di):
;	- bCSWStatus = COMMAND_PASSED.
;	- Send available data.
;	- dCSWDataResidue = dCBWDataTransferLength - data_sent.
;	- stall_ep_in, once EP is cleared setup_csw.
;
;CASE_6 (Hi = Di):
;	- bCSWStatus = COMMAND_PASSED.
;	- Send all data.
;	- dCSWDataResidue = 0.
;	- setup_csw.
;
;CASE_7 (Hi < Di) | CASE_8 (Hi <> Do):
;	- bCSWStatus = PHASE_ERROR.
;       - dCSWDataResidue is ignored by host.
;	- stall_ep_in, once EP is cleared setup_csw.
;
;CASE_9 (Ho > Dn) | CASE_11 (Ho > Do) | CASE_12 (Ho = Do):
;	- bCSWStatus = COMMAND_PASSED.
;	- Send/receive available data.
;	- dCSWDataResidue = dCBWDataTransferLength - data_sent.
;	- if CASE_9 or CASE_11 stall_ep_out.
;	- setup_csw (if stalled, once cleared).
;
;CASE_10 (Ho <> Di) | CASE_13 (Ho < Do):
;	- bCSWStatus = PHASE_ERROR.
;       - dCSWDataResidue is ignored by host.
;	- stall_ep_out, once EP is cleared setup_csw.

; Preload in temp the next MSD state machine state to be applied when
; bCSWStatus = COMMAND_SUCCESS.
check_13_cases:
    banksel     0
_test_Dn:
    btfss       dev_expect,DEV_Dn
    goto        _test_Hi_Do
    movfw       dCBWDataTransferLength_3
    iorwf       dCBWDataTransferLength_2,W
    iorwf       dCBWDataTransferLength_1,W
    iorwf       dCBWDataTransferLength_0,W
    bnz         $+5
    clrf        bCSWStatus ; COMMAND_SUCCESS Hn = Dn.
    call        setup_csw
    movfw       bCSWStatus
    return
    btfss       bCBWFlags,7
    goto        $+3
    call        msd_stall_EP_IN ; Hi > Dn.
    goto        $+2
    call        msd_stall_EP_OUT ; Ho > Dn.
    movlw       (1<<MSD_WAIT_CLEAR)
    movwf       msd_state
    clrf        bCSWStatus ; COMMAND_SUCCESS.
    return
_test_Hi_Do:
    btfss       bCBWFlags,7
    goto        _test_Ho_Di
    btfss       dev_expect,DEV_Di
    goto        _command_phase_error_exit
    goto        _test_H_less_D
_test_Ho_Di:
    btfss       dev_expect,DEV_Do
    goto        _command_phase_error_exit
_test_H_less_D:
    movfw       TF_LEN_BYTES+3
    subwf       dCBWDataTransferLength_3,W
    bnz         $+11
    movfw       TF_LEN_BYTES+2
    subwf       dCBWDataTransferLength_2,W
    bnz         $+7
    movfw       TF_LEN_BYTES+1
    subwf       dCBWDataTransferLength_1,W
    bnz         $+3
    movfw       TF_LEN_BYTES
    subwf       dCBWDataTransferLength_0,W
    ; Blocks CASE_2, CASE_3, CASE_7 and CASE_13.
    bnc         _command_phase_error_exit
    btfss       STATUS,Z
    bsf         msd_flags,END_DATA_SHORT
_command_success_exit:
    movfw       temp
    movwf       msd_state
    clrf        bCSWStatus ; COMMAND_SUCCESS.
    return
_command_phase_error_exit:
    movlw       PHASE_ERROR
    movwf       bCSWStatus
    goto        cause_bomsr

;*******************************************************************************
; Bulk-Only Mass Storage Reset
;*******************************************************************************
; Called to cause a Bulk Only Mass Storage Reset.
cause_bomsr:
    banksel     0
    call        msd_stall_EP_OUT
    call        msd_stall_EP_IN
    movlw       (1<<MSD_WAIT_CLEAR)
    movwf       msd_state
    return

;*******************************************************************************
; No Data Response
;*******************************************************************************
; Called when no response to a SCSI command is expected by the device.
no_data_response:
    banksel     0
    clrf        dev_expect
    bsf         dev_expect,DEV_Dn
    goto        check_13_cases

;*******************************************************************************
; Data Response
;*******************************************************************************
; Called when a response to a SCSI command is expected by the device.
; Preload the amount of data to be sent in the WREG. No response from the device
; will be greater than SIZE_EP1 (64 bytes).
send_data_response:
    banksel     0
    movwf       TF_LEN_BYTES
    clrf        TF_LEN_BYTES+1
    clrf        TF_LEN_BYTES+2
    clrf        TF_LEN_BYTES+3
    clrf        dev_expect
    bsf         dev_expect,DEV_Di
    movlw       (1<<MSD_DATA_SENT)
    movwf       temp
    call        check_13_cases
    btfss       STATUS,Z
    return      ; PHASE_ERROR occured, both EPs are stalled in check_13_cases.
_send_data_response_exit:
    ; Residue: dCSWDataResidue = dCBWDataTransferLength - device_bytes.
    movfw       TF_LEN_BYTES
    subwf       dCSWDataResidue_0,F
    ; Arm EP1_in
    movfw       TF_LEN_BYTES
    goto        msd_arm_ep_in

;*******************************************************************************
; READ_10 and WRIGHT_10 SCSI Command Data
;*******************************************************************************
; Services the exchange of data following a SCSI READ_10 or WRITE_10 command.
service_write10:
    call        msd_tx_sector
    goto        _service_rw10
service_read10:
    call        msd_rx_sector
_service_rw10:
    banksel     0
    ; msd_byte_of_sect += MSD_EP_SIZE.
    movlw       EP1_SIZE
    addwf       msd_byte_of_sect,F
    btfsc       STATUS,C
    incf        msd_byte_of_sect+1,F
    ; check msd_byte_of_sect == BYTES_PER_BLOCK_LE.
    btfss       msd_byte_of_sect+1,1
    goto        _service_wr10_exit ; BYTES_PER_BLOCK_LE MSB != 0x20.
    ; LBA++.
    incf        LBA,F
    btfsc       STATUS,Z
    incf        LBA+1,F
    ; msd_byte_of_sect = 0.
    clrf        msd_byte_of_sect
    clrf        msd_byte_of_sect+1
_service_wr10_exit:
    ; if READ10 call msd_arm_ep_in.
    movlw       EP1_SIZE
    btfsc       dev_expect,DEV_Di
    call        msd_arm_ep_in
    ; dCSWDataResidue -= EP1_SIZE.
    subwf       dCSWDataResidue_0,F
    clrw
    subwfb      dCSWDataResidue_1,F
    subwfb      dCSWDataResidue_2,F
    subwfb      dCSWDataResidue_3,F
    ; TF_LEN_BYTES -= EP1_SIZE.
    movlw       EP1_SIZE
    subwf       TF_LEN_BYTES,F
    clrw
    subwfb      TF_LEN_BYTES+1,F
    subwfb      TF_LEN_BYTES+2,F
    subwfb      TF_LEN_BYTES+3,F
    ; check if TF_LEN_BYTES == 0.
    movfw       TF_LEN_BYTES
    iorwf       TF_LEN_BYTES+1,W
    iorwf       TF_LEN_BYTES+2,W
    iorwf       TF_LEN_BYTES+3,W
    bnz         _service_wr10_not_finished
_service_wr10_finished:
    btfss       dev_expect,DEV_Do
    goto        $+8
    ; Write finished.
    btfss       msd_flags,END_DATA_SHORT
    goto        setup_csw
    call        msd_stall_EP_OUT
    bcf         msd_flags,END_DATA_SHORT
    movlw       (1<<MSD_WAIT_CLEAR)
    movwf       msd_state
    return
    ; Read finished.
    movlw       (1<<MSD_DATA_SENT)
    movwf       msd_state
    return
_service_wr10_not_finished:
    btfsc       dev_expect,DEV_Do
    call        msd_arm_ep_out
    return

;*******************************************************************************
; Invalid Command Sense
;*******************************************************************************
; An invalid command occured, setup the sense to inform the host.
invalid_command_sense:
    banksel     0
    movlw       0x05 ; SENSE_KEY = ILLEGAL_REQUEST.
    movwf       sense_key
    movlw       0x20 ; ADDITIONAL_SENSE_CODE = ASC_INVALID_COMMAND_OPCODE.
    movwf       additional_sense_code
    ; ADDITIONAL_SENSE_CODE_QUALIFIER = ASCQ_INVALID_COMMAND_OPCODE, cleared.
    return

;*******************************************************************************
; MSD EP Stall Functions
;*******************************************************************************
; Stalls EP1_out.
msd_stall_EP_OUT:
    banksel     0
    bsf         EP1_out_stat,EP_STAT_HALT
    movlw       (1<<BDn_STAT_BSTALL)
    movwf       BD1_OUT_STAT
    bsf         BD1_OUT_STAT,BDn_STAT_UOWN
    return

; Stalls EP1_in.
msd_stall_EP_IN:
    banksel     0
    bsf         EP1_in_stat,EP_STAT_HALT
    movlw       (1<<BDn_STAT_BSTALL)
    movwf       BD1_IN_STAT
    bsf         BD1_IN_STAT,BDn_STAT_UOWN
    return

;*******************************************************************************
; SCSI INQUIRY Command Response Data
;*******************************************************************************
scsi_inquiry:
    dt 0x00
    dt 0x80
    dt 0x04
    dt 0x02
    dt 0x20
    dt 0x00
    dt 0x00
    dt 0x00
    dt "Microchp"
    dt "USB uC 145X     "
    dt "0200"

    end
