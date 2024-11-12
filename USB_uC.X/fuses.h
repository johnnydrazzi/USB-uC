/**
 * @file fuses.h
 * @author John Izzard
 * @date 2024-11-12
 * 
 * @brief PIC Configuration Bits.
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
 * File Version 2.1.1 - 2024-11-11
 * - Changed: MIT License.
 *
 * File Version 2.1.0 - 2023-04-07
 * - Added: Support for _18F4450_FAMILY_ and _18F46J50_FAMILY_.
 *
 * File Version 2.0.0 - 2021-05-01
 * - Added: Configurable LVP and MCLR options.
 *
 * File Version 1.0.0 - 2020-06-28
 * - Added: Initial release of the software.
 */

#ifndef FUSES_H
#define FUSES_H

#include "config.h"

#if defined(_PIC14E)
// CONFIG1
#if (XTAL_USED == NO_XTAL)
#pragma config FOSC = INTOSC
#else
#pragma config FOSC = HS
#endif
#pragma config WDTE = SWDTEN    // Watchdog Timer Enable (WDT controlled by the SWDTEN bit in the WDTCON register)
#pragma config PWRTE = ON       // Power-up Timer Enable (PWRT enabled)
#ifdef USE_MCLRE
#pragma config MCLRE = ON
#else
#pragma config MCLRE = OFF
#endif
#pragma config CP = OFF         // Flash Program Memory Code Protection (Program memory code protection is disabled)
#pragma config BOREN = SBODEN   // Brown-out Reset Enable (Brown-out Reset controlled by the SBOREN bit in the BORCON register)
#pragma config CLKOUTEN = OFF   // Clock Out Enable (CLKOUT function is disabled. I/O or oscillator function on the CLKOUT pin)
#pragma config IESO = OFF       // Internal/External Switchover Mode (Internal/External Switchover Mode is disabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enable (Fail-Safe Clock Monitor is disabled)

// CONFIG2
#pragma config WRT = OFF        // Flash Memory Self-Write Protection (Write protection off)
#pragma config CPUDIV = NOCLKDIV// CPU System Clock Selection Bit (NO CPU system divide)
#pragma config USBLSCLK = 48MHz // USB Low SPeed Clock Selection bit (System clock expects 48 MHz, FS/LS USB CLKENs divide-by is set to 8.)
#pragma config PLLMULT = 3x     // PLL Multipler Selection Bit (3x Output Frequency Selected)
#pragma config PLLEN = DISABLED // PLL Enable Bit (3x or 4x PLL Disabled)
#pragma config STVREN = ON      // Stack Overflow/Underflow Reset Enable (Stack Overflow or Underflow will cause a Reset)
#pragma config BORV = HI        // Brown-out Reset Voltage Selection (Brown-out Reset Voltage (Vbor), high trip point selected.)
#pragma config LPBOR = OFF      // Low-Power Brown Out Reset (Low-Power BOR is disabled)
#ifdef USE_LVP
#pragma config LVP = ON
#else
#pragma config LVP = OFF
#endif

#elif defined(_18F4450_FAMILY_)
// CONFIG1L
#ifndef __CLANG__
#pragma config PLLDIV = XTAL_USED // Broken using C99
#else
#if XTAL_USED == MHz_4
#pragma config PLLDIV = 1
#elif XTAL_USED == MHz_8
#pragma config PLLDIV = 2
#elif XTAL_USED == MHz_12
#pragma config PLLDIV = 3
#elif XTAL_USED == MHz_16
#pragma config PLLDIV = 4
#elif XTAL_USED == MHz_20
#pragma config PLLDIV = 5
#elif XTAL_USED == MHz_24
#pragma config PLLDIV = 6
#elif XTAL_USED == MHz_40
#pragma config PLLDIV = 10
#elif XTAL_USED == MHz_48
#pragma config PLLDIV = 12
#else
#error XTAL_USED has invalid paramater
#endif
#endif
#pragma config CPUDIV = OSC1_PLL2// System Clock Postscaler Selection bits ([Primary Oscillator Src: /1][96 MHz PLL Src: /2])
#pragma config USBDIV = 2       // USB Clock Selection bit (used in Full-Speed USB mode only; UCFG:FSEN = 1) (USB clock source comes from the 96 MHz PLL divided by 2)

// CONFIG1H
#pragma config FOSC = HSPLL_HS  // Oscillator Selection bits (HS oscillator, PLL enabled (HSPLL))
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enable bit (Fail-Safe Clock Monitor disabled)
#pragma config IESO = OFF       // Internal/External Oscillator Switchover bit (Oscillator Switchover mode disabled)

// CONFIG2L
#pragma config PWRT = ON        // Power-up Timer Enable bit (PWRT enabled)
#pragma config BOR = SOFT       // Brown-out Reset Enable bits (Brown-out Reset enabled and controlled by software (SBOREN is enabled))
#pragma config BORV = 28        // Brown-out Reset Voltage bits (2.8V)
#pragma config VREGEN = ON      // USB Voltage Regulator Enable bit (USB voltage regulator enabled)

// CONFIG2H
#pragma config WDT = OFF        // Watchdog Timer Enable bit (WDT disabled (control is placed on the SWDTEN bit))
#pragma config WDTPS = 256      // Watchdog Timer Postscale Select bits (1:256)

// CONFIG3H
#pragma config PBADEN = OFF     // PORTB A/D Enable bit (PORTB<4:0> pins are configured as digital I/O on Reset)
#pragma config LPT1OSC = OFF    // Low-Power Timer 1 Oscillator Enable bit (Timer1 configured for higher power operation)
#ifdef USE_MCLRE
#pragma config MCLRE = ON
#else
#pragma config MCLRE = OFF
#endif

// CONFIG4L
#pragma config STVREN = ON      // Stack Full/Underflow Reset Enable bit (Stack full/underflow will cause Reset)
#ifdef USE_LVP
#pragma config LVP = ON
#else
#pragma config LVP = OFF
#endif
#pragma config BBSIZ = BB1K     // Boot Block Size Select bit (1KW Boot block size)
#pragma config XINST = OFF      // Extended Instruction Set Enable bit (Instruction set extension and Indexed Addressing mode disabled (Legacy mode))

// CONFIG5L
#pragma config CP0 = OFF        // Code Protection bit (Block 0 (000800-001FFFh) or (001000-001FFFh) is not code-protected)
#pragma config CP1 = OFF        // Code Protection bit (Block 1 (002000-003FFFh) is not code-protected)

// CONFIG5H
#pragma config CPB = OFF        // Boot Block Code Protection bit (Boot block (000000-0007FFh) or (000000-000FFFh) is not code-protected)

// CONFIG6L
#pragma config WRT0 = ON        // Write Protection bit (Block 0 (000800-001FFFh) or (001000-001FFFh) is write-protected)
#pragma config WRT1 = OFF       // Write Protection bit (Block 1 (002000-003FFFh) is not write-protected)

// CONFIG6H
#pragma config WRTC = ON        // Configuration Register Write Protection bit (Configuration registers (300000-3000FFh) are write-protected)
#pragma config WRTB = ON        // Boot Block Write Protection bit (Boot block (000000-0007FFh) or (000000-000FFFh) is write-protected)

// CONFIG7L
#pragma config EBTR0 = OFF      // Table Read Protection bit (Block 0 (000800-001FFFh) or (001000-001FFFh) is not protected from table reads executed in other blocks)
#pragma config EBTR1 = OFF      // Table Read Protection bit (Block 1 (002000-003FFFh) is not protected from table reads executed in other blocks)

// CONFIG7H
#pragma config EBTRB = OFF      // Boot Block Table Read Protection bit (Boot block (000000-0007FFh) or (000000-000FFFh) is not protected from table reads executed in other blocks)

#elif defined(_18F4550_FAMILY_)
// CONFIG1L
#ifndef __CLANG__
#pragma config PLLDIV = XTAL_USED // Broken using C99
#else
#if XTAL_USED == MHz_4
#pragma config PLLDIV = 1
#elif XTAL_USED == MHz_8
#pragma config PLLDIV = 2
#elif XTAL_USED == MHz_12
#pragma config PLLDIV = 3
#elif XTAL_USED == MHz_16
#pragma config PLLDIV = 4
#elif XTAL_USED == MHz_20
#pragma config PLLDIV = 5
#elif XTAL_USED == MHz_24
#pragma config PLLDIV = 6
#elif XTAL_USED == MHz_40
#pragma config PLLDIV = 10
#elif XTAL_USED == MHz_48
#pragma config PLLDIV = 12
#else
#error XTAL_USED has invalid paramater
#endif
#endif
#pragma config CPUDIV = OSC1_PLL2// System Clock Postscaler Selection bits ([Primary Oscillator Src: /1][96 MHz PLL Src: /2])
#pragma config USBDIV = 2       // USB Clock Selection bit (used in Full-Speed USB mode only; UCFG:FSEN = 1) (USB clock source comes from the 96 MHz PLL divided by 2)

// CONFIG1H
#pragma config FOSC = HSPLL_HS  // Oscillator Selection bits (HS oscillator, PLL enabled (HSPLL))
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enable bit (Fail-Safe Clock Monitor disabled)
#pragma config IESO = OFF       // Internal/External Oscillator Switchover bit (Oscillator Switchover mode disabled)

// CONFIG2L
#pragma config PWRT = ON        // Power-up Timer Enable bit (PWRT enabled)
#pragma config BOR = SOFT       // Brown-out Reset Enable bits (Brown-out Reset enabled and controlled by software (SBOREN is enabled))
#pragma config BORV = 2         // Brown-out Reset Voltage bits (Setting 1 2.79V)
#pragma config VREGEN = ON      // USB Voltage Regulator Enable bit (USB voltage regulator enabled)

// CONFIG2H
#pragma config WDT = OFF        // Watchdog Timer Enable bit (WDT disabled (control is placed on the SWDTEN bit))
#pragma config WDTPS = 256      // Watchdog Timer Postscale Select bits (1:256)

// CONFIG3H
#pragma config CCP2MX = ON      // CCP2 MUX bit (CCP2 input/output is multiplexed with RC1)
#pragma config PBADEN = OFF     // PORTB A/D Enable bit (PORTB<4:0> pins are configured as digital I/O on Reset)
#pragma config LPT1OSC = OFF    // Low-Power Timer 1 Oscillator Enable bit (Timer1 configured for higher power operation)
#ifdef USE_MCLRE
#pragma config MCLRE = ON
#else
#pragma config MCLRE = OFF
#endif

// CONFIG4L
#pragma config STVREN = ON      // Stack Full/Underflow Reset Enable bit (Stack full/underflow will cause Reset)
#ifdef USE_LVP
#pragma config LVP = ON
#else
#pragma config LVP = OFF
#endif
#pragma config XINST = OFF      // Extended Instruction Set Enable bit (Instruction set extension and Indexed Addressing mode disabled (Legacy mode))

// CONFIG5L
#pragma config CP0 = OFF        // Code Protection bit (Block 0 (000800-001FFFh) is not code-protected)
#pragma config CP1 = OFF        // Code Protection bit (Block 1 (002000-003FFFh) is not code-protected)
#pragma config CP2 = OFF        // Code Protection bit (Block 2 (004000-005FFFh) is not code-protected)
#if defined(_18F2550) || defined(_18F4550) || defined(_18F2553) || defined(_18F4553)
#pragma config CP3 = OFF        // Code Protection bit (Block 3 (006000-007FFFh) is not code-protected)
#endif

// CONFIG5H
#pragma config CPB = OFF        // Boot Block Code Protection bit (Boot block (000000-0007FFh) is not code-protected)
#pragma config CPD = OFF        // Data EEPROM Code Protection bit (Data EEPROM is not code-protected)

// CONFIG6L
#pragma config WRT0 = ON        // Write Protection bit (Block 0 (000800-001FFFh) is write-protected)
#pragma config WRT1 = OFF       // Write Protection bit (Block 1 (002000-003FFFh) is not write-protected)
#pragma config WRT2 = OFF       // Write Protection bit (Block 2 (004000-005FFFh) is not write-protected)
#if defined(_18F2550) || defined(_18F4550) || defined(_18F2553) || defined(_18F4553)
#pragma config WRT3 = OFF       // Write Protection bit (Block 3 (006000-007FFFh) is not write-protected)
#endif

// CONFIG6H
#pragma config WRTC = ON        // Configuration Register Write Protection bit (Configuration registers (300000-3000FFh) are write-protected)
#pragma config WRTB = ON        // Boot Block Write Protection bit (Boot block (000000-0007FFh) is write-protected)
#pragma config WRTD = OFF       // Data EEPROM Write Protection bit (Data EEPROM is not write-protected)

// CONFIG7L
#pragma config EBTR0 = OFF      // Table Read Protection bit (Block 0 (000800-001FFFh) is not protected from table reads executed in other blocks)
#pragma config EBTR1 = OFF      // Table Read Protection bit (Block 1 (002000-003FFFh) is not protected from table reads executed in other blocks)
#pragma config EBTR2 = OFF      // Table Read Protection bit (Block 2 (004000-005FFFh) is not protected from table reads executed in other blocks)
#if defined(_18F2550) || defined(_18F4550) || defined(_18F2553) || defined(_18F4553)
#pragma config EBTR3 = OFF      // Table Read Protection bit (Block 3 (006000-007FFFh) is not protected from table reads executed in other blocks)
#endif

// CONFIG7H
#pragma config EBTRB = OFF      // Boot Block Table Read Protection bit (Boot block (000000-0007FFh) is not protected from table reads executed in other blocks)

#elif defined(_18F14K50)
// CONFIG1L
#pragma config CPUDIV = NOCLKDIV// CPU System Clock Selection bits (No CPU System Clock divide)
#pragma config USBDIV = OFF     // USB Clock Selection bit (USB clock comes directly from the OSC1/OSC2 oscillator block; no divide)

// CONFIG1H
#pragma config FOSC = HS        // Oscillator Selection bits (HS oscillator)
#pragma config PLLEN = OFF      // 4 X PLL Enable bit (PLL is under software control)
#pragma config PCLKEN = ON      // Primary Clock Enable bit (Primary clock enabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enable (Fail-Safe Clock Monitor disabled)
#pragma config IESO = OFF       // Internal/External Oscillator Switchover bit (Oscillator Switchover mode disabled)

// CONFIG2L
#pragma config PWRTEN = ON      // Power-up Timer Enable bit (PWRT enabled)
#pragma config BOREN = ON       // Brown-out Reset Enable bits (Brown-out Reset enabled and controlled by software (SBOREN is enabled))
#pragma config BORV = 27        // Brown-out Reset Voltage bits (VBOR set to 2.7 V nominal)

// CONFIG2H
#pragma config WDTEN = OFF      // Watchdog Timer Enable bit (WDT is controlled by SWDTEN bit of the WDTCON register)
#pragma config WDTPS = 256      // Watchdog Timer Postscale Select bits (1:256)

// CONFIG3H
#pragma config HFOFST = OFF     // HFINTOSC Fast Start-up bit (The system clock is held off until the HFINTOSC is stable.)
#ifdef USE_MCLRE
#pragma config MCLRE = ON
#else
#pragma config MCLRE = OFF
#endif

// CONFIG4L
#pragma config STVREN = ON      // Stack Full/Underflow Reset Enable bit (Stack full/underflow will cause Reset)
#ifdef USE_LVP
#pragma config LVP = ON
#else
#pragma config LVP = OFF
#endif
#pragma config BBSIZ = OFF      // Boot Block Size Select bit (1kW boot block size)
#pragma config XINST = OFF      // Extended Instruction Set Enable bit (Instruction set extension and Indexed Addressing mode disabled (Legacy mode))

// CONFIG5L
#pragma config CP0 = OFF        // Code Protection bit (Block 0 not code-protected)
#pragma config CP1 = OFF        // Code Protection bit (Block 1 not code-protected)

// CONFIG5H
#pragma config CPB = OFF        // Boot Block Code Protection bit (Boot block not code-protected)
#pragma config CPD = OFF        // Data EEPROM Code Protection bit (Data EEPROM not code-protected)

// CONFIG6L
#pragma config WRT0 = ON        // Table Write Protection bit (Block 0 write-protected)
#pragma config WRT1 = OFF       // Table Write Protection bit (Block 1 not write-protected)

// CONFIG6H
#pragma config WRTC = ON        // Configuration Register Write Protection bit (Configuration registers write-protected)
#pragma config WRTB = ON        // Boot Block Write Protection bit (Boot block write-protected)
#pragma config WRTD = OFF       // Data EEPROM Write Protection bit (Data EEPROM not write-protected)

// CONFIG7L
#pragma config EBTR0 = OFF      // Table Read Protection bit (Block 0 not protected from table reads executed in other blocks)
#pragma config EBTR1 = OFF      // Table Read Protection bit (Block 1 not protected from table reads executed in other blocks)

// CONFIG7H
#pragma config EBTRB = OFF      // Boot Block Table Read Protection bit (Boot block not protected from table reads executed in other blocks)


#elif defined(_18F24K50) || defined(_18F25K50) || defined(_18F45K50)
// CONFIG1L
#pragma config PLLSEL = PLL4X   // PLL Selection (4x clock multiplier)
#pragma config CFGPLLEN = OFF   // PLL Enable Configuration bit (PLL Disabled (firmware controlled))
#pragma config CPUDIV = NOCLKDIV// CPU System Clock Postscaler (CPU uses system clock (no divide))
#pragma config LS48MHZ = SYS48X8// Low Speed USB mode with 48 MHz system clock (System clock at 48 MHz, USB clock divider is set to 8)

// CONFIG1H
#if (XTAL_USED == NO_XTAL)
#pragma config FOSC = INTOSCIO
#else
#pragma config FOSC = HSH
#endif
#pragma config PCLKEN = OFF     // Primary Oscillator Shutdown (Primary oscillator shutdown firmware controlled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor (Fail-Safe Clock Monitor disabled)
#pragma config IESO = OFF       // Internal/External Oscillator Switchover (Oscillator Switchover mode disabled)

// CONFIG2L
#pragma config nPWRTEN = ON     // Power-up Timer Enable (Power up timer enabled)
#pragma config BOREN = ON       // Brown-out Reset Enable (BOR controlled by firmware (SBOREN is enabled))
#pragma config BORV = 285       // Brown-out Reset Voltage (BOR set to 2.85V nominal)
#pragma config nLPBOR = ON      // Low-Power Brown-out Reset (Low-Power Brown-out Reset enabled)

// CONFIG2H
#pragma config WDTEN = SWON     // Watchdog Timer Enable bits (WDT controlled by firmware (SWDTEN enabled))
#pragma config WDTPS = 256      // Watchdog Timer Postscaler (1:256)

// CONFIG3H
#pragma config CCP2MX = RC1     // CCP2 MUX bit (CCP2 input/output is multiplexed with RC1)
#pragma config PBADEN = OFF     // PORTB A/D Enable bit (PORTB<5:0> pins are configured as digital I/O on Reset)
#pragma config T3CMX = RC0      // Timer3 Clock Input MUX bit (T3CKI function is on RC0)
#pragma config SDOMX = RB3      // SDO Output MUX bit (SDO function is on RB3)
#ifdef USE_MCLRE
#pragma config MCLRE = ON
#else
#pragma config MCLRE = OFF
#endif

// CONFIG4L
#pragma config STVREN = ON      // Stack Full/Underflow Reset (Stack full/underflow will cause Reset)
#ifdef USE_LVP
#pragma config LVP = ON
#else
#pragma config LVP = OFF
#endif
#pragma config ICPRT = OFF      // Dedicated In-Circuit Debug/Programming Port Enable (ICPORT disabled)
#pragma config XINST = OFF      // Extended Instruction Set Enable bit (Instruction set extension and Indexed Addressing mode disabled)

// CONFIG5L
#pragma config CP0 = OFF        // Block 0 Code Protect (Block 0 is not code-protected)
#pragma config CP1 = OFF        // Block 1 Code Protect (Block 1 is not code-protected)
#if !defined(_18F24K50)
#pragma config CP2 = OFF        // Block 2 Code Protect (Block 2 is not code-protected)
#pragma config CP3 = OFF        // Block 3 Code Protect (Block 3 is not code-protected)
#endif

// CONFIG5H
#pragma config CPB = OFF        // Boot Block Code Protect (Boot block is not code-protected)
#pragma config CPD = OFF        // Data EEPROM Code Protect (Data EEPROM is not code-protected)

// CONFIG6L
#pragma config WRT0 = ON        // Block 0 Write Protect (Block 0 (0800-1FFFh) is write-protected)
#pragma config WRT1 = OFF       // Block 1 Write Protect (Block 1 (2000-3FFFh) is not write-protected)
#if !defined(_18F24K50)
#pragma config WRT2 = OFF       // Block 2 Write Protect (Block 2 (04000-5FFFh) is not write-protected)
#pragma config WRT3 = OFF       // Block 3 Write Protect (Block 3 (06000-7FFFh) is not write-protected)
#endif

// CONFIG6H
#pragma config WRTC = ON        // Configuration Registers Write Protect (Configuration registers (300000-3000FFh) are write-protected)
#pragma config WRTB = ON        // Boot Block Write Protect (Boot block (0000-7FFh) is write-protected)
#pragma config WRTD = OFF       // Data EEPROM Write Protect (Data EEPROM is not write-protected)

// CONFIG7L
#pragma config EBTR0 = OFF      // Block 0 Table Read Protect (Block 0 is not protected from table reads executed in other blocks)
#pragma config EBTR1 = OFF      // Block 1 Table Read Protect (Block 1 is not protected from table reads executed in other blocks)
#if !defined(_18F24K50)
#pragma config EBTR2 = OFF      // Block 2 Table Read Protect (Block 2 is not protected from table reads executed in other blocks)
#pragma config EBTR3 = OFF      // Block 3 Table Read Protect (Block 3 is not protected from table reads executed in other blocks)
#endif

// CONFIG7H
#pragma config EBTRB = OFF      // Boot Block Table Read Protect (Boot block is not protected from table reads executed in other blocks)


#elif defined(_18F46J50_FAMILY_)
// CONFIG1L
#pragma config WDTEN = OFF      // Watchdog Timer (Disabled - Controlled by SWDTEN bit)
#ifndef __CLANG__
#pragma config PLLDIV = XTAL_USED // Broken using C99
#else
#if XTAL_USED == MHz_4
#pragma config PLLDIV = 1
#elif XTAL_USED == MHz_8
#pragma config PLLDIV = 2
#elif XTAL_USED == MHz_12
#pragma config PLLDIV = 3
#elif XTAL_USED == MHz_16
#pragma config PLLDIV = 4
#elif XTAL_USED == MHz_20
#pragma config PLLDIV = 5
#elif XTAL_USED == MHz_24
#pragma config PLLDIV = 6
#elif XTAL_USED == MHz_40
#pragma config PLLDIV = 10
#elif XTAL_USED == MHz_48
#pragma config PLLDIV = 12
#else
#error XTAL_USED has invalid paramater
#endif
#endif
#pragma config STVREN = ON      // Stack Overflow/Underflow Reset  (Enabled)
#pragma config XINST = OFF      // Extended Instruction Set (Disabled)

// CONFIG1H
#pragma config CPUDIV = OSC1    // CPU System Clock Postscaler (No CPU system clock divide)
#pragma config CP0 = OFF        // Code Protect (Program memory is not code-protected)

// CONFIG2L
#pragma config OSC = HSPLL      // Oscillator (HS+PLL, USB-HS+PLL)
#pragma config T1DIG = ON       // T1OSCEN Enforcement (Secondary Oscillator clock source may be selected)
#pragma config LPT1OSC = OFF    // Low-Power Timer1 Oscillator (High-power operation)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor (Disabled)
#pragma config IESO = OFF       // Internal External Oscillator Switch Over Mode (Disabled)

// CONFIG2H
#pragma config WDTPS = 256      // Watchdog Postscaler (1:256)

// CONFIG3L
#pragma config DSWDTOSC = INTOSCREF// DSWDT Clock Select (DSWDT uses INTRC)
#pragma config RTCOSC = T1OSCREF// RTCC Clock Select (RTCC uses T1OSC/T1CKI)
#pragma config DSBOREN = OFF    // Deep Sleep BOR (Disabled)
#pragma config DSWDTEN = OFF    // Deep Sleep Watchdog Timer (Disabled)
#pragma config DSWDTPS = G2     // Deep Sleep Watchdog Postscaler (1:2,147,483,648 (25.7 days))

// CONFIG3H
#pragma config IOL1WAY = OFF    // IOLOCK One-Way Set Enable bit (The IOLOCK bit (PPSCON<0>) can be set and cleared as needed)
#pragma config MSSP7B_EN = MSK7 // MSSP address masking (7 Bit address masking mode)

// CONFIG4L
#pragma config WPFP = PAGE_7    // Write/Erase Protect Page Start/End Location (Write Protect Program Flash Page 7)
#pragma config WPEND = PAGE_0   // Write/Erase Protect Region Select (valid when WPDIS = 0) (Page 0 through WPFP<5:0> erase/write protected)
#pragma config WPCFG = ON       // Write/Erase Protect Configuration Region (Configuration Words page erase/write-protected)

// CONFIG4H
#pragma config WPDIS = ON       // Write Protect Disable bit (WPFP<5:0>/WPEND region erase/write protected)

#elif defined(_18F26J53) || defined(_18F46J53) || defined(_18F27J53) || defined(_18F47J53)
//#elif defined(_18F47J53_FAMILY_) // Doesn't work!
// CONFIG1L
#pragma config WDTEN = OFF      // Watchdog Timer (Disabled - Controlled by SWDTEN bit)
#ifndef __CLANG__
#pragma config PLLDIV = XTAL_USED // Broken using C99
#else
#if XTAL_USED == MHz_4
#pragma config PLLDIV = 1
#elif XTAL_USED == MHz_8
#pragma config PLLDIV = 2
#elif XTAL_USED == MHz_12
#pragma config PLLDIV = 3
#elif XTAL_USED == MHz_16
#pragma config PLLDIV = 4
#elif XTAL_USED == MHz_20
#pragma config PLLDIV = 5
#elif XTAL_USED == MHz_24
#pragma config PLLDIV = 6
#elif XTAL_USED == MHz_40
#pragma config PLLDIV = 10
#elif XTAL_USED == MHz_48
#pragma config PLLDIV = 12
#else
#error XTAL_USED has invalid paramater
#endif
#endif
#pragma config CFGPLLEN = OFF   // PLL Enable Configuration Bit (PLL Disabled)
#pragma config STVREN = ON      // Stack Overflow/Underflow Reset (Enabled)
#pragma config XINST = OFF      // Extended Instruction Set (Disabled)

// CONFIG1H
#pragma config CPUDIV = OSC1    // CPU System Clock Postscaler (No CPU system clock divide)
#pragma config CP0 = OFF        // Code Protect (Program memory is not code-protected)

// CONFIG2L
#pragma config OSC = HSPLL      // Oscillator (HS+PLL, USB-HS+PLL)
#pragma config SOSCSEL = HIGH   // T1OSC/SOSC Power Selection Bits (High Power T1OSC/SOSC circuit selected)
#pragma config CLKOEC = OFF     // EC Clock Out Enable Bit  (CLKO output disabled on the RA6 pin)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor (Disabled)
#pragma config IESO = OFF       // Internal External Oscillator Switch Over Mode (Disabled)

// CONFIG2H
#pragma config WDTPS = 256      // Watchdog Postscaler (1:256)

// CONFIG3L
#pragma config DSWDTOSC = INTOSCREF// DSWDT Clock Select (DSWDT uses INTRC)
#pragma config RTCOSC = T1OSCREF// RTCC Clock Select (RTCC uses T1OSC/T1CKI)
#pragma config DSBOREN = OFF    // Deep Sleep BOR (Disabled)
#pragma config DSWDTEN = OFF    // Deep Sleep Watchdog Timer (Disabled)
#pragma config DSWDTPS = G2     // Deep Sleep Watchdog Postscaler (1:2,147,483,648 (25.7 days))

// CONFIG3H
#pragma config IOL1WAY = OFF    // IOLOCK One-Way Set Enable bit (The IOLOCK bit (PPSCON<0>) can be set and cleared as needed)
#pragma config ADCSEL = BIT12   // ADC 10 or 12 Bit Select (12 - Bit ADC Enabled)
#pragma config MSSP7B_EN = MSK7 // MSSP address masking (7 Bit address masking mode)

// CONFIG4L
#pragma config WPFP = PAGE_7    // Write/Erase Protect Page Start/End Location (Write Protect Program Flash Page 7)
#pragma config WPCFG = ON       // Write/Erase Protect Configuration Region  (Configuration Words page erase/write-protected)

// CONFIG4H
#pragma config WPDIS = ON       // Write Protect Disable bit (WPFP<6:0>/WPEND region erase/write protected)
#pragma config WPEND = PAGE_0   // Write/Erase Protect Region Select bit (valid when WPDIS = 0) (Pages 0 through WPFP<6:0> erase/write protected)
#pragma config LS48MHZ = SYS48X8// Low Speed USB mode with 48 MHz system clock bit (System clock at 48 MHz USB CLKEN divide-by is set to 8)
#endif

#include <xc.h>

#endif /* FUSES_H */