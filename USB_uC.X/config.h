/**
 * @file config.h
 * @brief PIC Microcontroller Settings.
 * @author John Izzard
 * @date 18/06/2020
 * 
 * USB uC - USB MSD Bootloader.
 * Copyright (C) 2017-2020  John Izzard
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef CONFIG_H
#define CONFIG_H

// Oscillator Options:
// Define XTAL_USED as one of the following. NO_XTAL means internal oscillator 
// is used (supported parts with ACT only), otherwise select the crystal value
// used MHz_XX. PIC18F14K50 will ignore XTAL_USED, as it can only use a 12 MHz
// crystal for USB full speed.
#if defined(_PIC14E) || defined(_18F24K50) || defined(_18F25K50) || defined(_18F45K50)
#define NO_XTAL 0
#define MHz_12  1
#define MHz_16  2

#elif defined(__J_PART)
#define MHz_4  1
#define MHz_8  2
#define MHz_12 3
#define MHz_16 4
#define MHz_20 5
#define MHz_24 6
#define MHz_40 10
#define MHz_48 12
#endif

#define _XTAL_FREQ          48000000      // Don't modify.
#define PLL_STARTUP_DELAY() __delay_ms(3) // Don't modify.

// PIC16F145X Settings:
#if defined(_PIC14E)
#define DM164127 0 // Compatible with DM164127-2/DV164139-2 https://www.microchip.com/developmenttools/ProductDetails/PartNO/DM164127-2.
#define GENERAL  1 // Compatible with dev boards that have a reset button.
#define XPRESS   2 // Compatible with the programmer (PIC16F1454) on DM164141 (used it for development).
#define CUSTOM   3 // Write your own.
#define BOARD_VERSION DM164127

#if BOARD_VERSION == DM164127
#define XTAL_USED         NO_XTAL // Could also use MHz_12.
#define BUTTON_PORT_BIT   3
#define BUTTON_PORT       PORTA
#define BUTTON_WPU_BIT    3
#define BUTTON_WPU        WPUA
#define BUTTON_ACTIVE_LOW
#define USE_BOOT_LED
#define LED_BIT           0
#define LED_LAT           LATC
#define LED_TRIS          TRISC

#elif BOARD_VERSION == GENERAL
#define XTAL_USED         NO_XTAL
#define BUTTON_PORT_BIT   3
#define BUTTON_PORT       PORTA
#define BUTTON_WPU_BIT    3
#define BUTTON_WPU        WPUA
#define BUTTON_ACTIVE_LOW

#elif BOARD_VERSION == XPRESS
#define XTAL_USED         NO_XTAL
#define USE_MCLRE
#define USE_LVP
#define BUTTON_PORT_BIT   5
#define BUTTON_PORT       PORTA
#define BUTTON_ACTIVE_LOW
#define USE_BOOT_LED
#define LED_BIT           3
#define LED_LAT           LATC
#define LED_TRIS          TRISC

#elif BOARD_VERSION == CUSTOM
#define XTAL_USED           // Select oscillator option.
#define USE_MCLRE           // Uncomment to enable MCLRE (reset pin).
#define USE_LVP             // Uncomment if LVP (Low Voltage Programming) is needed.
#define BUTTON_PORT_BIT     // Bootloader Button's bit in the I/O PORT.
#define BUTTON_PORT         // Bootloader Button's I/O PORT.
//#define BUTTON_ANSEL_BIT  // Uncomment and define the Bootloader Button's pin to make digital (if needed).
//#define BUTTON_ANSEL      // Uncomment and define the ANSEL register.
//#define BUTTON_WPU_BIT    // Uncomment and define the Bootloader Button's Weak Pull-Up pin (if needed).
//#define BUTTON_WPU        // Uncomment and define the WPU register.
#define BUTTON_ACTIVE_LOW   // Uncomment to make the Bootloader Button active low.
#define USE_BOOT_LED        // Uncomment if you wish to have a Bootloader LED.
#define LED_BIT             // Bootloader LED's bit in the LAT register.
#define LED_LAT             // Bootloader LED's LAT register.
#define LED_TRIS            // Bootloader LED's TRIS register.
//#define LED_ACTIVE_LOW    // Uncomment to make the Bootloader LED active low.
#endif

// PIC18F14K50 Settings:
#elif defined(_18F14K50)
#define DM164127 0 // Compatible with DM164127-2/DV164139-2 https://www.microchip.com/developmenttools/ProductDetails/PartNO/DM164127-2.
#define GENERAL  1 // Compatible with dev boards that have a reset button.
#define DEV_BRD  2 // A custom dev board I use.
#define CUSTOM   3 // Write your own.
#define BOARD_VERSION DM164127

#if BOARD_VERSION == DM164127 // Compatible with DM164127-2/DV164139-2 https://www.microchip.com/developmenttools/ProductDetails/PartNO/DM164127-2
                              // and MonkeyBUS - PIC18F14K50.
#define BUTTON_PORT_BIT   3
#define BUTTON_PORT       PORTA
#define BUTTON_WPU_BIT    3
#define BUTTON_WPU        WPUA
#define BUTTON_ACTIVE_LOW
#define USE_BOOT_LED
#define LED_BIT           0
#define LED_LAT           LATC
#define LED_TRIS          TRISC

#elif BOARD_VERSION == GENERAL
#define BUTTON_PORT_BIT   3
#define BUTTON_PORT       PORTA
#define BUTTON_WPU_BIT    3
#define BUTTON_WPU        WPUA
#define BUTTON_ACTIVE_LOW

#elif BOARD_VERSION == DEV_BRD
#define USE_MCLRE
#define BUTTON_PORT_BIT   0
#define BUTTON_PORT       PORTC
#define BUTTON_ANSEL_BIT  4
#define BUTTON_ANSEL      ANSEL
#define BUTTON_ACTIVE_LOW
#define USE_BOOT_LED
#define LED_BIT           1
#define LED_LAT           LATC
#define LED_TRIS          TRISC

#elif BOARD_VERSION == CUSTOM
#define USE_MCLRE           // Uncomment to enable MCLRE (reset pin).
//#define USE_LVP           // Uncomment if LVP (Low Voltage Programming) is needed.
#define BUTTON_PORT_BIT     // Bootloader Button's bit in the I/O PORT.
#define BUTTON_PORT         // Bootloader Button's I/O PORT.
//#define BUTTON_ANSEL_BIT  // Uncomment and define the Bootloader Button's pin to make digital (if needed).
//#define BUTTON_ANSEL      // Uncomment and define the ANSEL register.
//#define BUTTON_WPU_BIT    // Uncomment and define the Bootloader Button's Weak Pull-Up pin (if needed).
//#define BUTTON_WPU        // Uncomment and define the WPU register.
#define BUTTON_ACTIVE_LOW   // Uncomment to make the Bootloader Button active low.
#define USE_BOOT_LED        // Uncomment if you wish to have a Bootloader LED.
#define LED_BIT             // Bootloader LED's bit in the LAT register.
#define LED_LAT             // Bootloader LED's LAT register.
#define LED_TRIS            // Bootloader LED's TRIS register.
//#define LED_ACTIVE_LOW    // Uncomment to make the Bootloader LED active low.
#endif

// PIC18F24K50 and PIC18FX5K50 Settings.
#elif defined(_18F24K50) || defined(_18F25K50) || defined(_18F45K50)
#define PICDEM  0 // Compatible with DM163025-1 https://www.microchip.com/DevelopmentTools/ProductDetails/DM163025-1.
#define P_STAR  1 // Compatible with Pololu's P-Star dev boards https://www.pololu.com/category/217/p-star-programmable-controllers.
#define GENERAL 2 // Compatible with dev boards that have a reset button.
#define CUSTOM  3 // Write your own.
#define BOARD_VERSION PICDEM

#if BOARD_VERSION == PICDEM
#define XTAL_USED         NO_XTAL
#define USE_MCLRE
#define USE_LVP
#define BUTTON_PORT_BIT   4
#define BUTTON_PORT       PORTB
#define BUTTON_ANSEL_BIT  4
#define BUTTON_ANSEL      ANSELB
#define BUTTON_ACTIVE_LOW
#define USE_BOOT_LED
#define LED_BIT           0
#define LED_LAT           LATD
#define LED_TRIS          TRISD

#elif BOARD_VERSION == P_STAR
#define XTAL_USED         NO_XTAL
#define USE_MCLRE
#define USE_LVP
#define BUTTON_PORT_BIT   6
#define BUTTON_PORT       PORTB
#define BUTTON_ANSEL_BIT  6
#define BUTTON_ANSEL      ANSELB
#define USE_BOOT_LED
#define LED_BIT           7
#define LED_LAT           LATB
#define LED_TRIS          TRISB

#elif BOARD_VERSION == GENERAL
#define XTAL_USED         NO_XTAL
#define BUTTON_PORT_BIT   3
#define BUTTON_PORT       PORTE
#define BUTTON_WPU_BIT    7
#define BUTTON_WPU        TRISE
#define BUTTON_ACTIVE_LOW

#elif BOARD_VERSION == CUSTOM
#define XTAL_USED           // Select oscillator option.
#define USE_MCLRE           // Uncomment to enable MCLRE (reset pin).
#define USE_LVP             // Uncomment if LVP (Low Voltage Programming) is needed.
#define BUTTON_PORT_BIT     // Bootloader Button's bit in the I/O PORT.
#define BUTTON_PORT         // Bootloader Button's I/O PORT.
//#define BUTTON_ANSEL_BIT  // Uncomment and define the Bootloader Button's pin to make digital (if needed).
//#define BUTTON_ANSEL      // Uncomment and define the ANSEL register.
//#define BUTTON_WPU_BIT    // Uncomment and define the Bootloader Button's Weak Pull-Up pin (if needed).
//#define BUTTON_WPU        // Uncomment and define the WPU register.
#define BUTTON_ACTIVE_LOW   // Uncomment to make the Bootloader Button active low.
#define USE_BOOT_LED        // Uncomment if you wish to have a Bootloader LED.
#define LED_BIT             // Bootloader LED's bit in the LAT register.
#define LED_LAT             // Bootloader LED's LAT register.
#define LED_TRIS            // Bootloader LED's TRIS register.
//#define LED_ACTIVE_LOW    // Uncomment to make the Bootloader LED active low.
#endif

// PIC18FX6J53 and PIC18FX7J53 Settings.
#elif defined(__J_PART)
#define PIM          0 // Compatible with Microchip's MA180029 dev board https://www.microchip.com/DevelopmentTools/ProductDetails/PartNO/MA180029.
#define PIC_CLICKER  1 // Compatible with MikroElektronika's PIC clicker dev board https://www.mikroe.com/clicker-pic18fj.
#define DEV_BRD      2 // A custom dev board I use.
#define CUSTOM       3 // Write your own.
#define BOARD_VERSION PIM

#if BOARD_VERSION == PIM
#define XTAL_USED         MHz_12
#define BUTTON_PORT_BIT   2
#define BUTTON_PORT       PORTB
#define BUTTON_ANCON_BIT  0
#define BUTTON_ANCON      ANCON1
#define BUTTON_ACTIVE_LOW
#define USE_BOOT_LED
#define LED_BIT           1
#define LED_LAT           LATE
#define LED_TRIS          TRISE

#elif BOARD_VERSION == PIC_CLICKER
#define XTAL_USED         MHz_16
#define BUTTON_PORT_BIT   3
#define BUTTON_PORT       PORTD
#define BUTTON_ACTIVE_LOW
#define USE_BOOT_LED
#define LED_BIT           0
#define LED_LAT           LATA
#define LED_TRIS          TRISA

#elif BOARD_VERSION == DEV_BRD
#define XTAL_USED         MHz_16
#define BUTTON_PORT_BIT   6
#define BUTTON_PORT       PORTB
#define USE_BOOT_LED
#define LED_BIT           7
#define LED_LAT           LATB
#define LED_TRIS          TRISB

#elif BOARD_VERSION == CUSTOM
#define XTAL_USED           // Select oscillator option.
#define BUTTON_PORT_BIT     // Bootloader Button's bit in the I/O PORT.
#define BUTTON_PORT         // Bootloader Button's I/O PORT.
//#define BUTTON_ANCON_BIT  // Uncomment and define the Bootloader Button's pin to make digital (if needed).
//#define BUTTON_ANCON      // Uncomment and define the ANCON register.
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
#endif

#ifndef BUTTON_ACTIVE_LOW
    #define BUTTON_PRESSED  (BUTTON_PORT & (1 << BUTTON_PORT_BIT))
    #define BUTTON_RELEASED !(BUTTON_PORT & (1 << BUTTON_PORT_BIT))
#else
    #define BUTTON_PRESSED  !(BUTTON_PORT & (1 << BUTTON_PORT_BIT))
    #define BUTTON_RELEASED (BUTTON_PORT & (1 << BUTTON_PORT_BIT))
#endif
        
#ifndef LED_ACTIVE_LOW
    #define LED_ON()  LED_LAT |= (1 << LED_BIT)
    #define LED_OFF() LED_LAT &= ~(1 << LED_BIT)
#else
    #define LED_ON()  LED_LAT &= ~(1 << LED_BIT)
    #define LED_OFF() LED_LAT |= (1 << LED_BIT)
#endif

#define LED_OUPUT() LED_TRIS &= ~(1 << LED_BIT)

// WPUE3 (pull-up for RE3/MCLRE) needs to be accessible with PIC18F2XK50 (28 pin devices).
#if !defined(TRISE)
#define TRISE TRISE
uint8_t TRISE __at(0xF96); // This shouldn't be missing!!
#endif

#endif /* CONFIG_H */