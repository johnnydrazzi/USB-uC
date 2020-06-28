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

// Settings
#define NORMAL    0
#define DEV_BOARD 1
#define XPRESS    2

#define XTAL_USED   NO_XTAL
#define PIN_VERSION NORMAL

#define _XTAL_FREQ 48000000
#define PLL_STARTUP_DELAY() __delay_ms(3)


#if defined(_PIC14E)
#if PIN_VERSION == XPRESS
#define BUTTON      !PORTAbits.RA5
#define LED         LATCbits.LATC3

#define BUTTON_TRIS TRISAbits.TRISA5
#define LED_TRIS    TRISCbits.TRISC3
#else
#define BUTTON      PORTCbits.RC4
#define LED         LATCbits.LATC5

#define BUTTON_TRIS TRISCbits.TRISC4
#define LED_TRIS    TRISCbits.TRISC5
#endif

#elif defined(_18F14K50)
#define BUTTON      PORTCbits.RC0
#define LED         LATCbits.LATC1

#define BUTTON_TRIS TRISCbits.TRISC0
#define LED_TRIS    TRISCbits.TRISC1

#elif defined(_18F24K50) || defined(_18F25K50) || defined(_18F45K50)
#if XTAL_USED == NO_XTAL && PIN_VERSION == NORMAL
#define BUTTON      PORTAbits.RA6
#define LED         LATAbits.LA7
#define BUTTON_TRIS TRISAbits.TRISA6
#define LED_TRIS    TRISAbits.TRISA7
#else // DEV VERSION
#define BUTTON      PORTBbits.RB6
#define LED         LATBbits.LB7
#define BUTTON_TRIS TRISBbits.TRISB6
#define LED_TRIS    TRISBbits.TRISB7
#endif

#elif defined(__J_PART)
#define BUTTON      PORTBbits.RB6
#define LED         LATBbits.LB7

#define BUTTON_TRIS TRISBbits.TRISB6
#define LED_TRIS    TRISBbits.TRISB7
#endif

#endif /* CONFIG_H */