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
;    Filename: config.inc                                                      *
;    Date: 2024-11-12                                                          *
;    File Version: 1.1.0                                                       *
;    Author: John Izzard                                                       *
;    Company: N/A                                                              *
;    Description: Configure the bootloder here. Enable features, change the    *
;                 user firmware location, select bootloader and LED pins,      *
;                 select oscillator type, etc.                                 *
;                                                                              *
;*******************************************************************************
;                                                                              *
;    Revision History:                                                         *
;    File Version 1.1.0 - 2024-11-12                                           *
;    - Changed: MIT license.                                                   *
;    - Added: Guards for BOARD_VERSION and XTAL_USED, so that build script     *
;             can add these preprocessor defines. Hex is unchanged.            *
;    - Changed: Default hex was PIC16F1454, all are now exported.              *
;    - Changed: Device used is defined in configuration now.                   *
;                                                                              *
;    File Version 1.0.0 - 2021-05-01                                           *
;    - Added: Initial release.                                                 *
;                                                                              *
;*******************************************************************************

    ifndef CONFIG_INC
    #define CONFIG_INC
    
    radix dec
    
;**** SETTINGS *****************************************************************
; Bootloader Features:
; Set to '1' to enable the feature, otherwite set to '0'. Play with enabling/disabling
; features to save on space.
#define USE_ABOUT_FILE      1 ; Include a ABOUT.HTM file in the drive.
#define USE_PROG_MEM_FILE   1 ; Include a PROG_MEM.BIN file in the drive.
#define USE_PROG_MEM_DELETE 1 ; Delete PROGMEM.BIN to erase user firmware.
#define USE_PROG_MEM_READ   1 ; PROG_MEM.BIN file contains user firmware data.
#define BUTTON_LEAVE        1 ; Leave the bootloader by pressing the boot button.
#define BUTTON_LEAVE_PLUS   1 ; Won't check for the button if user firmware is erased (more elegant).
    
; Flash Regions:
; USER_PROGRAM must be a multiple of 32 (0x20). Don't go over 0x800!
USER_PROGRAM        equ 0x800 
USER_INT_VECT       equ USER_PROGRAM+4
END_OF_FLASH        equ 0x2000 ;(Don't modify).
CONFIG_REGION_START equ 0x8000 ;(Don't modify).

; Processor:
; Defined in the configuration used.
; Uncomment the processor used if using old MPLAB (not MPLAB X).
;#define PIC16F1454
;#define PIC16F1455
;#define PIC16F1459
 
; Oscillator Options:
; Define XTAL_USED in board options as one of the following.
#define NO_XTAL 0 ; Use internal oscillator.
#define MHz_12  1 ; Use external 12 MHz crystal oscillator.
#define MHz_16  2 ; Use external 16 MHz crystal oscillator.
 
; Board Options:
; Define BOARD_VERSION as one of the following dev boards.
#define DM164127 0 ; Compatible with DM164127-2/DV164139-2 https://www.microchip.com/developmenttools/ProductDetails/PartNO/DM164127-2.
#define GENERAL  1 ; Compatible with dev boards that have a reset button.
#define XPRESS   2 ; Compatible with the programmer (PIC16F1454) on DM164141 (used it for development).
#define CUSTOM   3 ; Write your own.
#ifndef BOARD_VERSION
#define BOARD_VERSION DM164127
#endif

    if BOARD_VERSION == DM164127
#ifndef XTAL_USED
#define XTAL_USED         NO_XTAL ; Could also use MHz_12.
#endif
#define BUTTON_PORT_BIT   3
#define BUTTON_PORT       PORTA
#define BUTTON_WPU_BIT    3
#define BUTTON_WPU        WPUA
#define BUTTON_ACTIVE_LOW
#define USE_BOOT_LED
#define LED_BIT           0
#define LED_LAT           LATC
#define LED_TRIS          TRISC
    endif

    if BOARD_VERSION == GENERAL
#ifndef XTAL_USED
#define XTAL_USED         NO_XTAL
#endif
#define BUTTON_PORT_BIT   3
#define BUTTON_PORT       PORTA
#define BUTTON_WPU_BIT    3
#define BUTTON_WPU        WPUA
#define BUTTON_ACTIVE_LOW
    endif

    if BOARD_VERSION == XPRESS
#ifndef XTAL_USED
#define XTAL_USED         NO_XTAL
#endif
#define USE_MCLRE
#define USE_LVP
#define BUTTON_PORT_BIT   5
#define BUTTON_PORT       PORTA
#define BUTTON_ACTIVE_LOW
#define USE_BOOT_LED
#define LED_BIT           3
#define LED_LAT           LATC
#define LED_TRIS          TRISC
    endif

    if BOARD_VERSION == CUSTOM
#ifndef XTAL_USED
#define XTAL_USED           ; Select oscillator option.
#endif
#define USE_MCLRE           ; Uncomment to enable MCLRE (reset pin).
#define USE_LVP             ; Uncomment if LVP (Low Voltage Programming) is needed.
#define BUTTON_PORT_BIT     ; Bootloader Button's bit in the I/O PORT.
#define BUTTON_PORT         ; Bootloader Button's I/O PORT.
;#define BUTTON_ANSEL_BIT   ; Uncomment and define the Bootloader Button's pin to make digital (if needed).
;#define BUTTON_ANSEL       ; Uncomment and define the ANSEL register.
;#define BUTTON_WPU_BIT     ; Uncomment and define the Bootloader Button's Weak Pull-Up pin (if needed).
;#define BUTTON_WPU         ; Uncomment and define the WPU register.
#define BUTTON_ACTIVE_LOW   ; Uncomment to make the Bootloader Button active low.
#define USE_BOOT_LED        ; Uncomment if you wish to have a Bootloader LED.
#define LED_BIT             ; Bootloader LED's bit in the LAT register.
#define LED_LAT             ; Bootloader LED's LAT register.
#define LED_TRIS            ; Bootloader LED's TRIS register.
;#define LED_ACTIVE_LOW     ; Uncomment to make the Bootloader LED active low.
    endif
 
;*******************************************************************************
 
 
;**** HELPERS ******************************************************************
    ifdef USE_MCLRE
#define _MCLRE _MCLRE_ON
    else
#define _MCLRE _MCLRE_OFF
    endif
    
    ifdef USE_LVP
#define _LVP _LVP_ON
    else
#define _LVP _LVP_OFF
    endif
 
    if XTAL_USED == NO_XTAL
#define _FOSC _FOSC_INTOSC
    else
#define _FOSC _FOSC_HS
    endif
 
    ifndef BUTTON_ACTIVE_LOW
BUTTON_pressed_skip macro
    btfss        BUTTON_PORT,BUTTON_PORT_BIT
    endm
BUTTON_released_skip macro
    btfsc        BUTTON_PORT,BUTTON_PORT_BIT
    endm
    else
BUTTON_pressed_skip macro
    btfsc        BUTTON_PORT,BUTTON_PORT_BIT
    endm
BUTTON_released_skip macro
    btfss        BUTTON_PORT,BUTTON_PORT_BIT
    endm
    endif
    
    ifndef LED_ACTIVE_LOW
LED_on macro
    bsf          LED_LAT,LED_BIT
    endm
LED_off macro
    bcf          LED_LAT,LED_BIT
    endm
    else
LED_on macro
    bcf          LED_LAT,LED_BIT
    endm
LED_off macro
    bsf          LED_LAT,LED_BIT
    endm
    endif
    
;*******************************************************************************
    
    endif