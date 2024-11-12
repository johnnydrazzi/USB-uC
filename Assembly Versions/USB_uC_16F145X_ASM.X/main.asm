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
;    Filename: main.asm                                                        *
;    Date: 2024-11-12                                                          *
;    File Version: 1.0.1                                                       *
;    Author: John Izzard                                                       *
;    Company: N/A                                                              *
;    Description: Contains the top most level code of the bootloader.          *
;                 Compiled using MPLAB X v5.35 & MPLINK 5.09.                  *
;                 Compiled using MPLAB IDE v8.92 & MPLINK 4.49.                *
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
    radix dec
    
    include "p16f1459.inc" ; Include this to cover all processors in the family.
    include "fuses.inc"
    include "config.inc"
    include "usb.inc"
    
    ; Stop the linker placing outside the bootloader area.
    ; Using this to avoid using a linker file.
    __maxrom 0x1FFF
    __badrom USER_PROGRAM - 0x1FFF
 
    global user_firmware
; Variables used for delays.
MAIN_VARS udata 0x6A
dly_cnt       res 1
dly_cnt1      res 1
dly_cnt2      res 1
dly_cnt3      res 1
user_firmware res 1
 
; External Function Symbols.
    extern usb_init, usb_tasks
    
; External Variable Symbols.
    extern usb_state, usb_flags
      
;*******************************************************************************
; Reset Vector
;*******************************************************************************
resetVec code 0x0000
_reset_vec:
    ; Saving some space by initializing some variables here.
    clrf         user_firmware
    clrf         usb_state
    bsf          usb_state,STATE_DETACHED ; USB state should start as Detached.
    ; Now initialize.
    goto         boot_init

;*******************************************************************************
; Interrupt Vector
;*******************************************************************************
; Redirect interrupts to the user interrupt vector.
intVec code 0x0004
    if USER_PROGRAM < 0x800
    goto         USER_INT_VECT
    else
    lgoto        USER_INT_VECT
    endif

;*******************************************************************************
; Main Code
;*******************************************************************************
mainCode code 0x0006
; Waits for the boot button to be released, turns on the boot LED, sets up the
; oscillator and or PLL to operate at 48MHz for USB, initializes USB and enters
; the main bootloader loop.
_start_bootloader:
    ; Wait for bootloader button's release.
    banksel      BUTTON_PORT
    BUTTON_released_skip
    goto         $-1
    ifdef USE_BOOT_LED
    ; Bootloader LED on.
    banksel      LED_LAT
    LED_on
    ; Bootloader LED pin at output.
    banksel      LED_TRIS
    bcf          LED_TRIS,LED_BIT
    endif
    ; Setup Oscillator to run at 48MHz.
    banksel      OSCCON
    if XTAL_USED == NO_XTAL
    movlw        (1<<SPLLEN)|(1<<SPLLMULT)|(1<<IRCF3)|(1<<IRCF2)|(1<<IRCF1)|(1<<IRCF0)
    movwf        OSCCON
    endif
    if XTAL_USED == MHz_12
    bsf          OSCCON,SPLLEN
    endif
    if XTAL_USED == MHz_16
    movlw        (1<<SPLLEN)|(1<<SPLLMULT)
    movwf        OSCCON
    endif
    ; Wait for PLL and OSC to stabalize, also debounce button.
    ; A little 50ms deboucing delay.
    banksel      0
    movlw        10
    movwf        dly_cnt
    call         delay_500us
    decfsz       dly_cnt,F
    goto         $-2
    ; If using interal oscillator, enable Active Clock Tuning, source is USB.
    if XTAL_USED == NO_XTAL
    banksel      ACTCON
    movlw        (1<<ACTEN)|(1<<ACTSRC)
    movwf        ACTCON
    endif
    ; Init USB.
    call         usb_init ; NEED_RESET = false, cleared in usb_init.
; Periodically checks for USB related tasks, and whether to exit the
; bootloader (reset the device).
_main_loop:
    call         usb_tasks
    btfsc        usb_flags,NEED_RESET
    goto         _delayed_reset
    ifdef BUTTON_LEAVE
    ifdef BUTTON_LEAVE_PLUS
    banksel      0
    btfss        user_firmware,0 ; Check the button if user firmware is there.
    goto         _main_loop
    endif
    banksel      BUTTON_PORT
    BUTTON_pressed_skip ; Attempt to leave bootloader when button pressed.
    endif
    goto         _main_loop
; The boot button was pressed and now waiting the the button to be released
; before attempting to leave the bootloader. While waiting for the button to be 
; released, USB tasks can still be serviced.
_button_reset:
    call         usb_tasks  ; Still service USB tasks while button is held.
    banksel      BUTTON_PORT
    BUTTON_released_skip   ; Wait for button release.
    goto         _button_reset
; "Exit gracefully". This allows the last of the USB tasks to be done before
; disconnecting from the USB (reset). Prevents OS's from reporting errors.
_delayed_reset:
    ; Delay 200 x ~500us = ~100ms before resetting (ignoring USB tasks).
    movlw        200
    movwf        dly_cnt
    call         usb_tasks
    call         delay_500us
    decfsz       dly_cnt,F
    goto         $-3
    reset
; Checks if user firmware is present, returns Z = 1 when user firmware is 
; detected.
check_user_first_inst:           
    banksel      PMADR
    movlw        (low  USER_PROGRAM)
    movwf        PMADRL
    movlw        (high USER_PROGRAM)
    movwf        PMADRH
    bsf          PMCON1,0
    nop
    nop
    movfw        PMDATL    ; Check if LSB is equal to 0xFF.
    xorlw        0xFF
    btfss        STATUS,Z
    return
    movfw        PMDATH    ; Check if MSB is equal to 0x3F.
    xorlw        0x3F
    return
    
; Delays for about 500uS when running at 48MHz.
delay_500us:
    ; 6,000 cycles = 500us. (10x598)+(3(10-1)+7) = 6,014 cycles = ~501uS.
    banksel      0
    movlw        10  
    movwf        dly_cnt2
    movlw        199 ; 600 cycles = 50uS. 3(200-1)+4 = 598 cycles = ~49.83uS.
    movwf        dly_cnt1
    decfsz       dly_cnt1,F 
    goto         $-1
    decfsz       dly_cnt2,F
    goto         $-5
    return
    
; Enables pullups if needed (option in config.inc), it will delay for 50mS 
; before checking the state of the boot button pin (just in case of capacitance
; on the pin). Will either start the bootloader if boot button is pressed or no
; no user firmware is found, otherwise start the user firmware.
boot_init:
    ; If pullup is enabled in config.inc, turn on pullup.
    ifdef BUTTON_WPU
    banksel      BUTTON_WPU
    clrf         WPUA
    ifdef PIC16F1459
    clrf         WPUB
    endif
    bsf          BUTTON_WPU,BUTTON_WPU_BIT
    banksel      OPTION_REG
    bcf          OPTION_REG,NOT_WPUEN
    endif
    ; Make boot button pin digital.
    ifdef BUTTON_ANSEL
    banksel      ANSELA
    bcf          BUTTON_ANSEL,BUTTON_ANSEL_BIT
    endif
    ; Delay for ~50mS in case of capacitance on boot button pin.
    if XTAL_USED == NO_XTAL
    ; 6,014 cycles = ~48mS.
    call         delay_500us
    endif
    if XTAL_USED == MHz_12
    ; 6,014 cycles = ~2mS. ~25 x 2uS. = ~50mS.
    banksel      0
    movlw        25
    movwf        dly_cnt
    call         delay_500us
    decfsz       dly_cnt,F
    goto         $-2
    endif
    if XTAL_USED == MHz_16
    ; 6,014 cycles = ~1.5mS. 33 x ~1.5mS = ~50mS.
    banksel      0
    movlw        33
    movwf        dly_cnt
    call         delay_500us
    decfsz       dly_cnt,F
    goto         $-2
    endif
    ; Check for user firmware.
    call         check_user_first_inst
    btfsc        STATUS,Z
    goto         _start_bootloader
    banksel      0
    bsf          user_firmware,0
    ; Check Bootloader Button.
    banksel      BUTTON_PORT
    BUTTON_released_skip
    goto         _start_bootloader
    if USER_PROGRAM < 0x800
    goto         USER_PROGRAM      ; User area has code.
    else
    lgoto        USER_PROGRAM
    endif
    
    end