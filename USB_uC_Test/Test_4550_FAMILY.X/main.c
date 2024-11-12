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
 * File Version 3.0.0 - 2024-11-12
 * - Changed: MIT License.
 * - Added: Support for build script.
 *
 * File Version 2.0.0 - 2023-04-11
 * - Removed: Irrelevant XTAL options.
 * - Changed: Default board is DEV_BOARD.
 *
 * File Version 1.0.0 - 2023-04-07
 * - Added: Initial release of the software.
 */

#include <xc.h>
#include <stdint.h>

#define MIKROE_647 0 // Compatible with MIKROE-647
#define GENERAL    1 // Compatible with dev boards that have a reset button.
#define DEV_BOARD  2 // A custom dev board I use.
#define CUSTOM     3 // Write your own.
#ifndef BOARD_VERSION
#define BOARD_VERSION DEV_BOARD
#endif

#if BOARD_VERSION == MIKROE_647
#define BUTTON_PORT_BIT   3
#define BUTTON_PORT       PORTE
#define BUTTON_ACTIVE_LOW
#define USE_BOOT_LED
#define LED_BIT           1
#define LED_LAT           LATA
#define LED_TRIS          TRISA

#elif BOARD_VERSION == GENERAL
#define BUTTON_PORT_BIT   3
#define BUTTON_PORT       PORTE
#define BUTTON_ACTIVE_LOW

#elif BOARD_VERSION == DEV_BOARD
#define BUTTON_PORT_BIT   6
#define BUTTON_PORT       PORTB
#define USE_BOOT_LED
#define LED_BIT           7
#define LED_LAT           LATB
#define LED_TRIS          TRISB

#elif BOARD_VERSION == CUSTOM
#define BUTTON_PORT_BIT     // Bootloader Button's bit in the I/O PORT.
#define BUTTON_PORT         // Bootloader Button's I/O PORT.
//#define BUTTON_WPU_BIT    // Uncomment and define the Bootloader Button's Weak Pull-Up pin (if needed).
//#define BUTTON_WPU        // Uncomment and define the WPU register.
//#define BUTTON_RXPU_BIT   // Uncomment and define the bit that enables the Weak Pull-Up pin.
//#define BUTTON_RXPU_REG   // Uncomment and define which register has the RXPU bit to enable the Weak Pull-Up pin.
#define BUTTON_ACTIVE_LOW   // Uncomment to make the Bootloader Button active low.
#define USE_BOOT_LED        // Uncomment if you wish to have a Bootloader LED.
#define LED_BIT             // Bootloader LED's bit in the LAT register.
#define LED_LAT             // Bootloader LED's LAT register.
#define LED_TRIS            // Bootloader LED's TRIS register.
//#define LED_ACTIVE_LOW    // Uncomment to make the Bootloader LED active low.
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

//__EEPROM_DATA(0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07);
//__EEPROM_DATA(0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F);
//__EEPROM_DATA(0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17);
//__EEPROM_DATA(0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F);
//__EEPROM_DATA(0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27);
//__EEPROM_DATA(0x28,0x29,0x2A,0x2B,0x2C,0x2D,0x2E,0x2F);
//__EEPROM_DATA(0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37);
//__EEPROM_DATA(0x38,0x39,0x3A,0x3B,0x3C,0x3D,0x3E,0x3F);
//__EEPROM_DATA(0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47);
//__EEPROM_DATA(0x48,0x49,0x4A,0x4B,0x4C,0x4D,0x4E,0x4F);
//__EEPROM_DATA(0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57);
//__EEPROM_DATA(0x58,0x59,0x5A,0x5B,0x5C,0x5D,0x5E,0x5F);
//__EEPROM_DATA(0x60,0x61,0x62,0x63,0x64,0x65,0x66,0x67);
//__EEPROM_DATA(0x68,0x69,0x6A,0x6B,0x6C,0x6D,0x6E,0x6F);
//__EEPROM_DATA(0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77);
//__EEPROM_DATA(0x78,0x79,0x7A,0x7B,0x7C,0x7D,0x7E,0x7F);
//__EEPROM_DATA(0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87);
//__EEPROM_DATA(0x88,0x89,0x8A,0x8B,0x8C,0x8D,0x8E,0x8F);
//__EEPROM_DATA(0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97);
//__EEPROM_DATA(0x98,0x99,0x9A,0x9B,0x9C,0x9D,0x9E,0x9F);
//__EEPROM_DATA(0xA0,0xA1,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7);
//__EEPROM_DATA(0xA8,0xA9,0xAA,0xAB,0xAC,0xAD,0xAE,0xAF);
//__EEPROM_DATA(0xB0,0xB1,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7);
//__EEPROM_DATA(0xB8,0xB9,0xBA,0xBB,0xBC,0xBD,0xBE,0xBF);
//__EEPROM_DATA(0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7);
//__EEPROM_DATA(0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF);
//__EEPROM_DATA(0xD0,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7);
//__EEPROM_DATA(0xD8,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,0xDF);
//__EEPROM_DATA(0xE0,0xE1,0xE2,0xE3,0xE4,0xE5,0xE6,0xE7);
//__EEPROM_DATA(0xE8,0xE9,0xEA,0xEB,0xEC,0xED,0xEE,0xEF);
//__EEPROM_DATA(0xF0,0xF1,0xF2,0xF3,0xF4,0xF5,0xF6,0xF7);
//__EEPROM_DATA(0xF8,0xF9,0xFA,0xFB,0xFC,0xFD,0xFE,0xFF);

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
    PLL_STARTUP_DELAY();
}
