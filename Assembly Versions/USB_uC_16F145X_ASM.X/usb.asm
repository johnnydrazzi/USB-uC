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
;    Filename: usb.asm                                                         *
;    Date: 29/04/2021                                                          *
;    File Version: 1.00                                                        *
;    Author: John Izzard                                                       *
;    Company: N/A                                                              *
;    Description: USB Stack is here. Anything USB layer is dealt with here.    *
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
    
    global BD1_OUT_STAT, BD1_OUT_CNT, BD1_OUT_ADRL, BD1_OUT_ADRH
    global BD1_IN_STAT, BD1_IN_CNT, BD1_IN_ADRL, BD1_IN_ADRH
; Buffer Descriptor Table.
BDT udata BDT_ADDRESS
BD0_OUT_STAT res 1
BD0_OUT_CNT  res 1
BD0_OUT_ADRL res 1
BD0_OUT_ADRH res 1
BD0_IN_STAT  res 1
BD0_IN_CNT   res 1
BD0_IN_ADRL  res 1
BD0_IN_ADRH  res 1
BD1_OUT_STAT res 1
BD1_OUT_CNT  res 1
BD1_OUT_ADRL res 1
BD1_OUT_ADRH res 1
BD1_IN_STAT  res 1
BD1_IN_CNT   res 1
BD1_IN_ADRL  res 1
BD1_IN_ADRH  res 1

    global SETUP_bmRequestType, SETUP_bRequest, SETUP_wValueL, SETUP_wValueH
    global SETUP_wIndexL, SETUP_wIndexH, SETUP_wLengthL, SETUP_wLengthH
; Setup Packet Variable.
SETUP udata SETUP_ADDRESS
SETUP_bmRequestType res 1
SETUP_bRequest      res 1
SETUP_wValueL       res 1
SETUP_wValueH       res 1
SETUP_wIndexL       res 1
SETUP_wIndexH       res 1
SETUP_wLengthL      res 1
SETUP_wLengthH      res 1

    global USTAT_copy, EP1_out_stat, EP1_in_stat, temp
; USB Variables.
USB_VARS udata USB_VARS_ADDRESS
USTAT_copy            res  1
EP0_out_stat          res  1
EP0_in_stat           res  1
EP1_out_stat          res  1
EP1_in_stat           res  1
saved_address         res  1
bytes_available       res  1
bytes_2_send          res  1
temp                  res  4

    global usb_state, usb_flags, control_stage, cnt
; Shared USB Variables.
SHARED_USB_VARS udata_shr SHR_USB_VARS_ADDRESS
usb_state     res 1
usb_flags     res 1
control_stage res 1
cnt           res 1

    global EP1_out, EP1_in
; Endpoint Buffers.
EP0_OUT udata EP0_OUT_ADDRESS
EP0_out res EP0_SIZE
EP0_IN udata EP0_IN_ADDRESS
EP0_in res EP0_SIZE
EP1_OUT udata EP1_OUT_ADDRESS
EP1_out res EP1_SIZE
EP1_IN udata EP1_IN_ADDRESS
EP1_in res EP1_SIZE

; External Function Symbols.
    extern msd_tasks, msd_clear_halt, app_init, msd_clear_ep_toggle, class_request
    
; External Descriptor Symbols.
    extern device_descriptor, configuration_descriptor, interface_descriptor
    extern ep1_out_descriptor, ep1_in_descriptor
    extern string_zero_descriptor, serial_string_descriptor

; Global USB Function Symbols.
    global usb_init, usb_tasks, arm_in_status, usb_request_error, mem_copy, mem_set
    
usbCode code
;*******************************************************************************
; Initialize USB
;*******************************************************************************
; Initializes USB and stack. Called during initialization and when reset flag
; is detected in usb_tasks.
usb_init:
    banksel     UIR
    clrf        UEP0   ; Clear USB EP0 Control Register.
    clrf        UEP1   ; Clear USB EP1 Control Register.
    clrf        UIR    ; Clear USB Status Interrupts, TRNIF and USTAT FIFO.
    btfsc       UIR,TRNIF
    goto        $-2
    clrf        UADDR  ; Clear USB Address.
    movlw       (1<<UPUEN)|(1<<FSEN); Full speed, pullups, and disable pingpong buffering.
    movwf       UCFG
    banksel     0
    clrf        BD0_OUT_STAT ; Clear BDT entries.
    clrf        BD0_IN_STAT
    clrf        BD1_OUT_STAT
    clrf        BD1_IN_STAT
    movlw       low EP0_out  ; Assign address in BDT for EP0_OUT.
    movwf       BD0_OUT_ADRL
    movlw       high EP0_out
    movwf       BD0_OUT_ADRH
    movlw       low EP0_in   ; Assign address in BDT for EP0_IN.
    movwf       BD0_IN_ADRL
    movlw       high EP0_in
    movwf       BD0_IN_ADRH
    movlw       low EP1_out  ; Assign address in BDT for EP1_OUT.
    movwf       BD1_OUT_ADRL
    movlw       high EP1_out
    movwf       BD1_OUT_ADRH
    movlw       low EP1_in   ; Assign address in BDT for EP1_IN.
    movwf       BD1_IN_ADRL
    movlw       high EP1_in
    movwf       BD1_IN_ADRH
    clrf        EP0_out_stat ; Clear USB EP statuses.
    clrf        EP0_in_stat
    clrf        EP1_out_stat
    clrf        EP1_in_stat
    clrf        usb_flags ; UPDATE_ADDRESS = false. SEND_SHORT = false.
    banksel     UCON
    bcf         UCON,PKTDIS ; UCON.PKTDIS = 0.
    movlw       (1<<EPINEN)|(1<<EPOUTEN)|(1<<EPHSHK)
    movwf       UEP0
    ; Check if USB state is Detached, and USB module needs enabling.
    btfss       usb_state,STATE_DETACHED
    goto        $+4
    ; usb_state == STATE_DETACHED.
    bsf         UCON,USBEN ; Turn on USB module  .
    btfsc       UCON,SE0   ; Wait for SE0 condition.
    goto        $-1
    clrf        control_stage
    bsf         control_stage,SETUP_STAGE ; control_stage = SETUP_STAGE.
    call        arm_setup
    clrf        usb_state
    bsf         usb_state,STATE_DEFAULT ; usb_state = STATE_DEFAULT.
    banksel     UIR
    bcf         UIR,URSTIF
    return

;*******************************************************************************
; USB State Machine
;*******************************************************************************
; This function is designed to be called regularly inside the main loop. It will 
; check for USB reset flag or transaction complete flag and either return or 
; goto another function for further processing.
usb_tasks:
    ; Check USB reset flag.
    banksel     UIR
    btfsc       UIR,URSTIF   
    call        usb_init
_check_transaction:
    btfss       UIR,TRNIF
    return
_copy_USTAT:
    nop         ; nop's needed, see section 26.2.3 in datasheet.
    nop
    ; Make a copy of USTAT.
    movfw       USTAT
    banksel     0
    movwf       USTAT_copy
    ; Clear transaction flag / progress FIFO.
    banksel     UIR
    bcf         UIR,TRNIF   
_check_EP0:
    banksel     0
    movlw       USTAT_ENDP_MASK
    andwf       USTAT_copy,W      
    bnz         _check_EPn
    ; Check direction.
    btfsc       USTAT_copy,DIR
    goto        _EP0_in_trans
_EP0_out_trans:
    ; Check if PID Token is SETUP or OUT.
    movlw       BDn_STAT_PID_MASK
    andwf       BD0_OUT_STAT,W
    xorlw       (PID_SETUP_TOKEN<<BDn_STAT_PID0)
    bz          process_setup ; PID == SETUP.
    ; Check if the control stage is DATA_OUT.
    btfss       control_stage,DATA_OUT_STAGE
    goto        arm_setup
    ; control stage == DATA_OUT_STAGE.
    ; Toggle EP0_out_stat DATA TOGGLE bit.
    movlw       (1<<EP_STAT_DATA_TOGGLE)
    xorwf       EP0_out_stat,F
    return      ; out_control_transfer should never be needed.
    goto        arm_setup
_EP0_in_trans:
    ; Check if the control stage is DATA_IN.
    btfss       control_stage,DATA_IN_STAGE 
    goto        _status_in_trans
    ; control stage == DATA_IN_STAGE.
    ; Toggle EP0_in_stat DATA TOGGLE bit.
    movlw       (1<<EP_STAT_DATA_TOGGLE)
    xorwf       EP0_in_stat,F
    goto        in_control_transfer
_status_in_trans:
    ; control_stage must be STATUS_IN_STAGE.
    call        arm_setup
    ; Check if address needs updating following a set_address request.
    btfss       usb_flags,UPDATE_ADDRESS
    return
    ; UPDATE_ADDRESS == true.
    ; UADDR = saved_address.
    movfw       saved_address
    banksel     UADDR
    movwf       UADDR
    ; Check if updating the address was appropriate.
    ; Is saved_address == 0.
    banksel     0
    movfw       saved_address
    bz          _saved_address_is_0
    ; Is usb_state == STATE_DEFAULT.
    btfss       usb_state,STATE_DEFAULT
    ; usb_state != STATE_DEFAULT.
    goto        _clear_update_address
    ; usb_state == STATE_DEFAULT, usb_state = STATE_ADDRESS.
    clrf        usb_state
    bsf         usb_state,STATE_ADDRESS
    goto        _clear_update_address
_saved_address_is_0:
    ; Force USB reset to bring USB state to Default.
    banksel     UIR
    bsf         UIR,URSTIF ; Force USB reset/re-init.
    banksel     0
_clear_update_address:
    bcf         usb_flags,UPDATE_ADDRESS
    return
_check_EPn:
    movlw       USTAT_ENDP_MASK
    andwf       USTAT_copy,W
    xorlw       0x08 ; ENDP == 1.
    bz          msd_tasks
    return      ; Should never happen.

    
;*******************************************************************************
; Arm USB Endpoint
;*******************************************************************************
; Arms EP0_OUT ready for SETUP, and will always return.
arm_setup:
    banksel     0
    movlw       8
    movwf       BD0_OUT_CNT                ; BD0_out_CNT = 8
    clrf        BD0_OUT_STAT               ; BD0_out_STAT = 0
    bsf         BD0_OUT_STAT,BDn_STAT_UOWN ; BD0_out_STAT.UOWN = 1
    return

; Arms EP0_IN for DATA_IN transaction.
; The transaction length should be passed to WREG before calling.
arm_ep0_in:
    movwf       BD0_IN_CNT
    clrf        BD0_IN_STAT
    btfsc       EP0_in_stat,EP_STAT_DATA_TOGGLE
    bsf         BD0_IN_STAT,BDn_STAT_DTS
    bsf         BD0_IN_STAT,BDn_STAT_DTSEN
    bsf         BD0_IN_STAT,BDn_STAT_UOWN
    return

; Arms for STATUS_IN transaction.
arm_in_status:
    clrf        BD0_IN_CNT
    movlw       (1<<BDn_STAT_DTSEN)|(1<<BDn_STAT_DTS)
    movwf       BD0_IN_STAT
    bsf         BD0_IN_STAT,BDn_STAT_UOWN
    return

;*******************************************************************************
; In Control Transfer
;*******************************************************************************
; Deals with control transfers with direction being IN.
in_control_transfer:
    ; Check if bytes_2_send == 0.
    movfw       bytes_2_send
    bz          _no_bytes_2_send
    ; bytes_2_send != 0, copying data to EP0_IN.
    movlw       low EP0_in
    movwf       FSR1L
    clrf        FSR1H ; EP0_in is < 0x100.
    ; Check if bytes_2_send > EP0_SIZE.
    movfw       bytes_2_send
    sublw       EP0_SIZE
    bnc         _bytes_2_send_greater_EP0_SIZE
    ; bytes_2_send <= EP0_SIZE.
    movfw       bytes_2_send
    movwf       cnt
    call        mem_copy
    movfw       bytes_2_send
    call        arm_ep0_in
    clrf        bytes_2_send
    return
_bytes_2_send_greater_EP0_SIZE:
    movlw       EP0_SIZE
    movwf       cnt
    call        mem_copy
    movlw       EP0_SIZE
    call        arm_ep0_in
    movlw       EP0_SIZE
    subwf       bytes_2_send,F ; bytes_2_send -= EP0_SIZE.
    return
_no_bytes_2_send:
    ; Check if SEND_SHORT.
    btfss       usb_flags,SEND_SHORT  
    return
    ; SEND_SHORT == true.
    bcf         usb_flags,SEND_SHORT  ; SEND_SHORT = false.
    clrw
    goto        arm_ep0_in

;*******************************************************************************
; Process Setup Packet
;*******************************************************************************
; Will parse and process a setup packet. It does not return itself, but will
; go to a function to deal with a specific request, or flow on to
; usb_request_error when a non-supported request is found.
process_setup:
    ; Stop any pending control IN transfers.
    clrf        BD0_IN_STAT 
    ; Make a copy of the SETUP packet.
    clrf        FSR0H ; EP0_out's address is < 0x100.
    clrf        FSR1H ; SETUP_ADDRESS is < 0x100.
    movlw       low EP0_out
    movwf       FSR0L
    movlw       low SETUP_ADDRESS
    movwf       FSR1L
    movlw       8
    movwf	cnt
    call        mem_copy
    ; Clear packet transfer disable, must be cleared after every setup transfer!
    banksel     UCON
    bcf         UCON,PKTDIS
    banksel     0
    call        arm_setup
    ; First data packet is always DATA1 type.
    bsf         EP0_out_stat,EP_STAT_DATA_TOGGLE
    bsf         EP0_in_stat,EP_STAT_DATA_TOGGLE
    ; Check if bmRequestType is STANDARD or CLASS.
    movlw       TYPE_MASK
    andwf       SETUP_bmRequestType,W
    bnz         class_request ; STANDARD == 0.
    ; bmRequestType is STANDARD, check which request.
    movlw       GET_DESCRIPTOR
    xorwf       SETUP_bRequest,W
    bz          get_descriptor
    movlw       CLEAR_FEATURE
    xorwf       SETUP_bRequest,W
    bz          set_clear_feature
    movlw       SET_ADDRESS
    xorwf       SETUP_bRequest,W
    bz          set_address
    movlw       SET_CONFIGURATION
    xorwf       SETUP_bRequest,W
    bz          set_configuration
    movlw       GET_STATUS
    xorwf       SETUP_bRequest,W
    bz          get_status
    movlw       SET_FEATURE
    xorwf       SETUP_bRequest,W
    bz          set_clear_feature
    movlw       GET_CONFIGURATION
    xorwf       SETUP_bRequest,W
    bz          get_configuration
    movlw       GET_INTERFACE
    xorwf       SETUP_bRequest,W
    bz          get_interface
    movlw       SET_INTERFACE
    xorwf       SETUP_bRequest,W
    bz          set_interface
    ; Flow on to usb_request_error.

;*******************************************************************************
; USB Request Error
;*******************************************************************************
; Stalls the in control endpoint (EP0_in), aka REQUEST_ERROR.
usb_request_error:
    movlw       (1<<BDn_STAT_BSTALL)      ; BD0_in_STAT = BSTALL.
    movwf       BD0_IN_STAT
    bsf         BD0_IN_STAT,BDn_STAT_UOWN ; BD0_in_STAT |= UOWN.
    return

;*******************************************************************************
; Memory Functions
;*******************************************************************************
; Uses indirect registers to copy memory from either flash or RAM pointed
; to by FSR0, to another location in RAM pointed to by FSR1. 
; It expects that you have already assigned addresses to FSR0 and FSR1 and
; written the amount you want to copy to the cnt, befor calling the 
; function.
mem_copy:
    moviw       FSR0++
    movwi       FSR1++
    decfsz      cnt,F
    goto        mem_copy
    return

; Will set a region of RAM to a value. Mostly used for clearing IN Endpoints.
; It expects that you have assigned a starting address to FSR0, the amount
; of RAM in cnt and placed the value you wish to set in WREG, before calling
; the fuction.
mem_set:
    movwi       FSR0++
    decfsz      cnt,F
    goto        mem_set
    return

;*******************************************************************************
; Valid USB State
;*******************************************************************************
; Checks if USB state is STATE_ADRRESS or STATE_CONFIGURED (valid), goes to
; usb_request_error if not valid.
valid_usb_state:
    movlw       (1<<STATE_ADDRESS)|(1<<STATE_CONFIGURED)
    andwf       usb_state,W
    bz          usb_request_error
    return
    
;*******************************************************************************
; USB Requests
;*******************************************************************************
; Services a get_descriptor request.
; Returns to main loop after.
get_descriptor:
    movfw	SETUP_wValueH
    xorlw	DEVICE_DESC_TYPE
    bz		_device_desc_req
    movfw	SETUP_wValueH
    xorlw	CONFIGURATION_DESC_TYPE
    bz		_config_desc_req
    movfw	SETUP_wValueH
    xorlw	STRING_DESC_TYPE
    bz		_string_desc_req
    goto	usb_request_error
_device_desc_req:
    movlw	low device_descriptor
    movwf	FSR0L
    movlw	high (device_descriptor + 0x8000)
    movwf	FSR0H
    movlw	DEVICE_DESC_SIZE
    goto	_get_descriptor_exit
_config_desc_req:
    movfw	SETUP_wValueL
    bnz		usb_request_error
    movlw	low configuration_descriptor
    movwf	FSR0L
    movlw	high (configuration_descriptor + 0x8000)
    movwf	FSR0H
    movlw	TOTAL_DESC_SIZE
    goto	_get_descriptor_exit
_string_desc_req:
;    movlw       NUM_STR_DESCRIPTORS
;    subwf       SETUP_wValueL,W
;    bc          usb_request_error
;    movfw       SETUP_wValueL
;    bz          _str_zero
;    xorlw       1
;    bz          _str_vendor
;    movfw       SETUP_wValueL
;    xorlw       2
;    bz          _str_product
    movfw       SETUP_wValueL
    bz          _str_zero
    xorlw       1
    bnz         usb_request_error
_str_serial:
    movlw	low serial_string_descriptor
    movwf	FSR0L
    movlw	high (serial_string_descriptor + 0x8000)
    movwf	FSR0H
    movlw	STR_SERIAL_DESC_SIZE
    goto	_get_descriptor_exit
;_str_vendor:
;    movlw	low vendor_string_descriptor
;    movwf	FSR0L
;    movlw	high (vendor_string_descriptor + 0x8000)
;    movwf	FSR0H
;    movlw	STR_VENDOR_DESC_SIZE
;    goto	_get_descriptor_exit
;_str_product:
;    movlw	low product_string_descriptor
;    movwf	FSR0L
;    movlw	high (product_string_descriptor + 0x8000)
;    movwf	FSR0H
;    movlw	STR_PRODUCT_DESC_SIZE
;    goto	_get_descriptor_exit
_str_zero:
    movlw	low string_zero_descriptor
    movwf	FSR0L
    movlw	high (string_zero_descriptor + 0x8000)
    movwf	FSR0H
    movlw	STR_Z_DESC_SIZE
_get_descriptor_exit:
    movwf	bytes_available
    ; Upper byte of SETUP_wLength should be zero, we don't have descriptors 
    movfw       SETUP_wLengthH ; larger than 255 bytes.
    bnz         $+5 
    movfw       SETUP_wLengthL
    subwf       bytes_available,W     ; bytes_available < SETUP_wLength.
    bc          _get_desc_wLength_ok
    movfw       bytes_available
    movwf       bytes_2_send
    movlw       b'00000111'
    andwf       bytes_available,W
    bz          $+3
    bcf         usb_flags,SEND_SHORT ; bytes_available % EP0_SIZE != 0.
    goto        $+2
    bsf         usb_flags,SEND_SHORT ; bytes_available % EP0_SIZE = 0.
    call        in_control_transfer
    clrf        control_stage
    bsf         control_stage,DATA_IN_STAGE
    return
_get_desc_wLength_ok:
    movfw       SETUP_wLengthL
    movwf       bytes_2_send
    bcf         usb_flags,SEND_SHORT
    goto        $-7
    
; Services a set_feature or clear_feature request.
; SET_FEATURE or CLEAR_FEATURE is only valid in ADDRESS or CONFIGURED states.
; If the recipient is DEVICE, response is request_error because REMOTE_WAKEUP
; is not supported.
; If recipient is ENDPOINT, and requested endpoint is 0, do nothing.
; If recipient is ENDPOINT and is greater that 1, reponse is request_error.
; If recipient is ENDPOINT and equals 1, but usb_state is ADDRESS_STATE,
; reponse is request_error.
; If recipient is ENDPOINT and equals 1, the response is either stall or clear
; stall on the endpoint (EP1_OUT or EP1_IN).
set_clear_feature:
    ; Check if usb_state is STATE_ADDRESS or STATE_CONFIGURED.
    call        valid_usb_state
    ; Check if wValueH is non-zero (wouldn't be valid FeatureSelector).
    movfw       SETUP_wValueH
    bnz         usb_request_error
    ; Mask and copy bmRequestType.Recipient to temp.
    movlw       RECIPIENT_MASK
    andwf       SETUP_bmRequestType,W
    ; Is Recipient == ENDPOINT.
    xorlw       ENDPOINT
    bnz         usb_request_error ; Recipient is not a ENDPOINT.
_endpoint_set_clear:
    ; Check if wValueL is not ENDPOINT_HALT (wouldn't be valid).
    movfw       SETUP_wValueL 
    bnz         usb_request_error ; ENDPOINT_HALT == 0.
    ; Mask EndpointNumber and store in temp.
    movfw       SETUP_wIndexL
    andlw       b'00001111'
    movwf       temp
    ; Check if host is asking to halt or clear halt on EP0.
    ; USB 2.0 spec says it's valid, and in the 9.4.5 get_status section, it
    ; says "It is neither required nor recommended that the Halt feature be 
    ; implemented for the Default Control Pipe". Many devices don't support 
    ; this, i.e. EP0_IN's stall is always cleared by setup packet. 
    ; I agree to this method, any set/clear feature for EP0 will be accepted
    ; as valid, but do nothing. Another option would be request_error response.
    bz          _set_clear_feature_exit
    ; Check if usb_state is STATE_ADDRESS, spec says to perform request_error 
    ; when EP > 0 and usb_state is STATE_ADDRESS.
    btfsc       usb_state,STATE_ADDRESS
    goto        usb_request_error
    ; usb_state == STATE_CONFIGURED, check that EndpointNumber < NUM_ENDPOINTS.
    movlw       2
    subwf       temp,W
    bc          usb_request_error
    ; All checks out, perform set / clear halt on EP1.
    clrf        FSR0H ; Going to indirectly address BDT and EP_stats,
    clrf        FSR1H ; BDT and EP_stats addresses are < 0x100.
    ; Check the direction
    btfsc       SETUP_wIndexL,7 ;
    goto        _EP1_IN_set_clear
_EP1_OUT_set_clear:
    movlw       EP1_out_stat
    movwf       FSR0L
    movlw       BD1_OUT_STAT
    movwf       FSR1L
    goto        $+5
_EP1_IN_set_clear:
    movlw       EP1_in_stat
    movwf       FSR0L
    movlw       BD1_IN_STAT
    movwf       FSR1L
    ; Set / Clear.
    movfw       SETUP_bRequest
    xorlw       CLEAR_FEATURE
    bnz         $+3
    ; Clear.
    call        msd_clear_halt
    goto        _set_clear_feature_exit
    ; Set.
    bsf         INDF0,EP_STAT_HALT
    movlw       (1<<BDn_STAT_BSTALL)
    movwf       INDF1
    bsf         INDF1,BDn_STAT_UOWN
_set_clear_feature_exit:
    call        arm_in_status
    clrf        control_stage
    bsf         control_stage,STATUS_IN_STAGE
    return

; Services a set_address request. The new address is saved and UPDATE_ADDRESS
; flag is set. Address will be apdated in the status in stage.
; Returns to main loop after.
set_address:
    movfw       SETUP_wValueL
    movwf       saved_address
    bsf         usb_flags,UPDATE_ADDRESS
    call        arm_in_status
    clrf        control_stage
    bsf         control_stage,STATUS_IN_STAGE
    return

; Services a set_configuration request.
set_configuration:
    ; Check if usb_state is STATE_ADDRESS or STATE_CONFIGURED.
    call        valid_usb_state
    ; Check for wValueH !=0.
    movfw       SETUP_wValueH
    bnz         usb_request_error
    ; check that number of configurations is either 0 or 1.
    movlw       2 ; Number of configurations+1.
    subwf       SETUP_wValueL,W
    bc          usb_request_error
    ; All checks out, select new configuration.
    call        arm_in_status
    clrf        control_stage
    bsf         control_stage,STATUS_IN_STAGE
    clrf        usb_state
    btfss       SETUP_wValueL,0
    goto        _set_configuration_0
_set_configuration_1:
    call        app_init
    bsf         usb_state,STATE_CONFIGURED
    return
_set_configuration_0:
    bsf         usb_state,STATE_ADDRESS
    return

; Services a get_status request.
; If recipient is DEVICE, REMOTE_WAKEUP is not supported, and device is always 
; bus powered (SELF_POWERED = 0), so response is 0.
; If recipient is INTERFACE, reponse is always 0.
; If recipient is ENDPOINT, and requested endpoint is 0, response is 0.
; If recipient is ENDPOINT and is greater that 1, reponse is request_error.
; If recipient is ENDPOINT and equals 1, but usb_state is ADDRESS_STATE,
; reponse is request_error.
; If recipient is ENDPOINT and equals 1, the response is whether the endpoint is 
; halted or not (EP1_OUT or EP1_IN).
get_status:
    ; Check if usb_state is STATE_ADDRESS or STATE_CONFIGURED.
    call        valid_usb_state
    ; Clear first two bytes of EP0_IN for response.
    clrf        EP0_in
    clrf        EP0_in+1
    ; Mask and copy bmRequestType.Recipient to temp.
    movlw       RECIPIENT_MASK
    andwf       SETUP_bmRequestType,W
    movwf       temp
    ; Is Recipient == DEVICE.
    bz          _get_status_exit ; DEVICE = 0.
    ; Is Recipient == INTERFACE.
    xorlw       INTERFACE
    bz          _get_status_exit
    ; Is Recipient == ENDPOINT.
    movfw       temp
    xorlw       ENDPOINT
    bz          _endpoint_get_status
    ; Recipient is not a DEVICE, INTERFACE or ENDPOINT.
    goto        usb_request_error 
_endpoint_get_status:
    ; Mask EndpointNumber and store in temp.
    movfw       SETUP_wIndexL
    andlw       b'00001111'
    movwf       temp
    ; Check if host is asking to get halt status on EP0.
    ; USB 2.00 says this is valid. See above set_clear_feature for why always 
    ; respond with halt cleared.
    bz          _get_status_exit
    ; Check if usb_state is STATE_ADDRESS, spec says to perform request_error 
    ; when EP > 0 and usb_state is STATE_ADDRESS.
    btfsc       usb_state,STATE_ADDRESS
    goto        usb_request_error
    ; usb_state == STATE_CONFIGURED, check that EndpointNumber < NUM_ENDPOINTS.
    movlw       2 ; Number of endpoints.
    subwf       temp,W
    bc          usb_request_error
    ; All checks out, perform get status on EP1.
    btfsc       SETUP_wIndexL,7 ;
    goto        _EP1_IN_get_status
_EP1_OUT_get_status:
    btfsc       EP1_out_stat,EP_STAT_HALT
    bsf         EP0_in,0
    goto        _get_status_exit
_EP1_IN_get_status:
    btfsc       EP1_in_stat,EP_STAT_HALT
    bsf         EP0_in,0
_get_status_exit:
    clrf        control_stage
    bsf         control_stage,DATA_IN_STAGE
    movlw       2
    goto        arm_ep0_in

; Services a get_configuration request. If usb_state is STATE_CONFIGURED, a 1
; is returned, otherwise 0.
get_configuration:
    clrf        EP0_in
    btfsc       usb_state,STATE_CONFIGURED
    bsf         EP0_in,0
    clrf        control_stage
    bsf         control_stage,DATA_IN_STAGE
    movlw       1
    goto        arm_ep0_in

; Services a get_interface request. Respond with request_error if host is 
; refering to an interface other than interface 0.
; usb_state should be STATE_CONFIGURED, otherwise respond with request_error.
; No alternate settings for our interface, always respond with 0.
; *Possible space saving, don't check for non zero values, or usb_state,
; always accept request.
get_interface:
    movfw       SETUP_wIndexL
    iorwf       SETUP_wIndexH,W ; 
    bnz         usb_request_error
    btfss       usb_state,STATE_CONFIGURED
    goto        usb_request_error
    clrf        EP0_in
    movlw       1
    goto        arm_ep0_in

; Services a set_interface request. Respond with request_error if host is 
; refering to an interface other than interface 0.
; No alternate settings for our interface, should be 0, otherwise repond with
; request_error.
; *Possible space saving, don't check for non zero values, always accept request.
set_interface:
    movfw       SETUP_wValueH     ; alternate_setting should be zero.
    iorwf       SETUP_wValueL,W
    iorwf       SETUP_wIndexL,W   ; interface should be zero.
    bnz         usb_request_error
    call        msd_clear_ep_toggle
    goto        arm_in_status

    end