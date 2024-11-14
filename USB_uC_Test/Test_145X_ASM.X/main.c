/**
 * @file main.c
 * @author John Izzard
 * @date 2024-11-12
 * 
 * @brief Main C file.
 */

/**
 * Copyright (C) 2017-2024 John Izzard
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the “Software”), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/**
 * Change Log
 * ----------
 * File Version 2.0.0 - 2024-11-12
 * - Changed: MIT License.
 * - Added: Support for build script.
 *
 * File Version 1.0.0 - 2024-06-30
 * - Added: Initial release of the software.
 */

#include <xc.h>
#include <stdint.h>

#define NO_XTAL 0
#define MHz_12  1
#define MHz_16  2

#define DM164127 0 // Compatible with DM164127-2/DV164139-2 https://www.microchip.com/developmenttools/ProductDetails/PartNO/DM164127-2.
#define GENERAL  1 // Compatible with dev boards that have a reset button.
#define XPRESS   2 // Compatible with the programmer (PIC16F1454) on DM164141 (used it for development).
#define CUSTOM   3 // Write your own.
#ifndef BOARD_VERSION
#define BOARD_VERSION XPRESS
#endif

#if BOARD_VERSION == DM164127
#ifndef XTAL_USED
#define XTAL_USED         NO_XTAL // Could also use MHz_12.
#endif
#define BUTTON_PORT_BIT   3
#define BUTTON_PORT       PORTA
#define BUTTON_WPU_BIT    3
#define BUTTON_WPU        WPUA
#define BUTTON_ACTIVE_LOW
#define LED_BIT           0
#define LED_LAT           LATC
#define LED_TRIS          TRISC

#elif BOARD_VERSION == GENERAL
#ifndef XTAL_USED
#define XTAL_USED         NO_XTAL
#endif
#define BUTTON_PORT_BIT   3
#define BUTTON_PORT       PORTA
#define BUTTON_WPU_BIT    3
#define BUTTON_WPU        WPUA
#define BUTTON_ACTIVE_LOW
#define LED_BIT
#define LED_LAT
#define LED_TRIS

#elif BOARD_VERSION == XPRESS
#ifndef XTAL_USED
#define XTAL_USED         NO_XTAL
#endif
#define BUTTON_PORT_BIT   5
#define BUTTON_PORT       PORTA
#define BUTTON_ANSEL_BIT  5
#define BUTTON_ANSEL      ANSELA
#define BUTTON_ACTIVE_LOW
#define LED_BIT           3
#define LED_LAT           LATC
#define LED_TRIS          TRISC

#elif BOARD_VERSION == CUSTOM
#ifndef XTAL_USED
#define XTAL_USED           // Select oscillator option.
#endif
#define BUTTON_PORT_BIT     // Button's bit in the I/O PORT.
#define BUTTON_PORT         // Button's I/O PORT.
//#define BUTTON_ANSEL_BIT  // Uncomment and define the Button's pin to make digital (if needed).
//#define BUTTON_ANSEL      // Uncomment and define the ANSEL register.
//#define BUTTON_WPU_BIT    // Uncomment and define the Button's Weak Pull-Up pin (if needed).
//#define BUTTON_WPU        // Uncomment and define the WPU register.
#define BUTTON_ACTIVE_LOW   // Uncomment to make the Button active low.
#define LED_BIT             // LED's bit in the LAT register.
#define LED_LAT             // LED's LAT register.
#define LED_TRIS            // LED's TRIS register.
//#define LED_ACTIVE_LOW    // Uncomment to make the LED active low.
#endif

#define _XTAL_FREQ          48000000
#define PLL_STARTUP_DELAY() __delay_ms(3)

#ifndef BUTTON_ACTIVE_LOW
    #define BUTTON_PRESSED (BUTTON_PORT & (1 << BUTTON_PORT_BIT))
#else
    #define BUTTON_PRESSED !(BUTTON_PORT & (1 << BUTTON_PORT_BIT))
#endif
        
#ifndef LED_ACTIVE_LOW
    #define LED_ON()  LED_LAT |= (1 << LED_BIT)
    #define LED_OFF() LED_LAT &= ~(1 << LED_BIT)
#else
    #define LED_ON()  LED_LAT &= ~(1 << LED_BIT)
    #define LED_OFF() LED_LAT |= (1 << LED_BIT)
#endif

#define LED_OUPUT() LED_TRIS &= ~(1 << LED_BIT)

static void inline init(void);

void main(void)
{
    init();
    LED_OUPUT();
    LED_OFF();

    while(1)
    {
        if(BUTTON_PRESSED)
        {
            LED_ON();
            __delay_ms(250);
            LED_OFF();
            __delay_ms(250);
        }
        else
        {
            LED_ON();
            __delay_ms(500);
            LED_OFF();
            __delay_ms(500);
        }
    }
    return;
}

static void inline init(void)
{
    #if XTAL_USED == NO_XTAL
    OSCCONbits.IRCF = 0xF;
    #endif
    #if XTAL_USED != MHz_12
    OSCCONbits.SPLLMULT = 1;
    #endif
    OSCCONbits.SPLLEN = 1;
    PLL_STARTUP_DELAY();
    #if XTAL_USED == NO_XTAL
    ACTCONbits.ACTSRC = 1;
    ACTCONbits.ACTEN = 1;
    #endif
    
    // Make boot pin digital.
    #ifdef BUTTON_ANSEL
    BUTTON_ANSEL &= ~(1<<BUTTON_ANSEL_BIT);
    #endif

    // Apply pull-up.
    #ifdef BUTTON_WPU
    WPUA = 0;
    #if defined(_16F1459)
    WPUB = 0;
    #endif
    BUTTON_WPU |= (1 << BUTTON_WPU_BIT);
    OPTION_REGbits.nWPUEN = 0;
    #endif
}