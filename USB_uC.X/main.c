/**
 * @file main.c
 * @brief Main C file.
 * @author John Izzard
 * @date 25/04/2023
 * 
 * USB uC - USB MSD Bootloader.
 * Copyright (C) 2017-2023  John Izzard
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
 * 
 * Project Version: v1.11
 * MPLABX: v6.05
 * XC8: v2.41
 * Device Packages Used:
 *      - PIC12-16F1xxx_DFP v1.3.90
 *      - PIC18Fxxxx_DFP v1.3.36
 *      - PIC18F-K_DFP v1.8.249
 *      - PIC18F-J_DFP v1.5.44
 */

#include <xc.h>
#include <stdint.h>
#include <stdbool.h>
#include "fuses.h"
#include "config.h"
#include "usb.h"
#include "bootloader.h"
#include "flash.h"

#if defined(_PIC14E)
__asm("GLOBAL _isr");

void isr(void) __at(0x4)
{
    __asm("LJMP "___mkstr(((PROG_REGION_START / 2) + 0x4)));
}
#else
__asm("PSECT intcode");
__asm("goto    "___mkstr(PROG_REGION_START + 0x08));
__asm("PSECT intcodelo");
__asm("goto    "___mkstr(PROG_REGION_START + 0x18));
#endif


static void inline boot_init(void);
static void inline boot_uninit(void);
static void check_user_first_inst(void);

bool user_firmware = false;

static uint8_t m_delay_cnt = 0;

void main(void)
{
    boot_init();
    __delay_ms(50); // Incase of capacitance on boot pin.
    check_user_first_inst();
    
    if(BUTTON_PRESSED || (user_firmware == false))
    {
        while(BUTTON_PRESSED){}
        __delay_ms(20); // De-bounce.
        #ifdef USE_BOOT_LED
        LED_OUPUT();
        LED_ON();
        #endif
        g_boot_reset = false;
        usb_init();
        while(1)
        {
            usb_tasks();
            msd_tasks();
            if(g_boot_reset)   goto delayed_reset;
            if(BUTTON_PRESSED && user_firmware) goto button_reset;
        }
    }
    
    // User firmware detected.
    boot_uninit();
    #if defined(_PIC14E)
    __asm("LJMP "___mkstr(PROG_REGION_START / 2));
    #else
    __asm("GOTO " ___mkstr(PROG_REGION_START));
    #endif
    
    // Button was pushed while in bootloader.
    button_reset:
    while(BUTTON_PRESSED)
    {
        usb_tasks();
        msd_tasks();
    }
    
    // Ready to leave the bootloader.
    // "Gracefully" disconnect from USB (reset). Just gives some time to finish
    // SCSI WRITE_10 or any other USB activity before disconnecting the from the
    // USB. Prevents OS's reporting an error occurred.
    delayed_reset:
    while(1)
    {
        usb_tasks();
        msd_tasks();
        m_delay_cnt++;
        __delay_us(500);
        if(m_delay_cnt == 200) break;
    }
    __asm("RESET");
}

static void inline boot_init(void)
{
    // Oscillator Settings.
    // PIC16F145X.
    #if defined(_PIC14E)
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

    // PIC18FX450, PIC18FX550, and PIC18FX455.
    #elif defined(_18F4450_FAMILY_) || defined(_18F4550_FAMILY_)
    PLL_STARTUP_DELAY();
    
    // PIC18F14K50.
    #elif defined(_18F14K50)
    OSCTUNEbits.SPLLEN = 1;
    PLL_STARTUP_DELAY();
    
    // PIC18F2XK50.
    #elif defined(_18F24K50) || defined(_18F25K50) || defined(_18F45K50)
    #if XTAL_USED == NO_XTAL
    OSCCONbits.IRCF = 7;
    #endif
    #if (XTAL_USED != MHz_12)
    OSCTUNEbits.SPLLMULT = 1;
    #endif
    OSCCON2bits.PLLEN = 1;
    PLL_STARTUP_DELAY();
    #if XTAL_USED == NO_XTAL
    ACTCONbits.ACTSRC = 1;
    ACTCONbits.ACTEN = 1;
    #endif

    // PIC18F2XJ53 and PIC18F4XJ53.
    #elif defined(__J_PART)
    OSCTUNEbits.PLLEN = 1;
    PLL_STARTUP_DELAY();
    #endif

    
    // Make boot pin digital.
    #if defined(BUTTON_ANSEL) 
    BUTTON_ANSEL &= ~(1<<BUTTON_ANSEL_BIT);
    #elif defined(BUTTON_ANCON)
    BUTTON_ANCON |= (1<<BUTTON_ANCON_BIT);
    #endif


    // Apply pull-up.
    #ifdef BUTTON_WPU
    #if defined(_PIC14E)
    WPUA = 0;
    #if defined(_16F1459)
    WPUB = 0;
    #endif
    BUTTON_WPU |= (1 << BUTTON_WPU_BIT);
    OPTION_REGbits.nWPUEN = 0;
    
    #elif defined(_18F4450_FAMILY_) || defined(_18F4550_FAMILY_)
    LATB = 0;
    LATD = 0;
    BUTTON_WPU |= (1 << BUTTON_WPU_BIT);
    #if BUTTON_RXPU_REG == INTCON2
    INTCON2 &= 7F;
    #else
    PORTE |= 80;
    #endif
    
    #elif defined(_18F14K50)
    WPUA = 0;
    WPUB = 0;
    BUTTON_WPU |= (1 << BUTTON_WPU_BIT);
    INTCON2bits.nRABPU = 0;
    
    #elif defined(_18F24K50) || defined(_18F25K50) || defined(_18F45K50)
    WPUB = 0;
    TRISE &= 0x7F;
    BUTTON_WPU |= (1 << BUTTON_WPU_BIT);
    INTCON2bits.nRBPU = 0;
    
    #elif defined(_18F24J50) || defined(_18F25J50) || defined(_18F26J50) || defined(_18F26J53) || defined(_18F27J53)
    LATB = 0;
    BUTTON_WPU |= (1 << BUTTON_WPU_BIT);
    BUTTON_RXPU_REG &= ~(1 << BUTTON_RXPU_BIT);
    
    #elif defined(_18F44J50) || defined(_18F45J50) || defined(_18F46J50) || defined(_18F46J53) || defined(_18F47J53)
    LATB = 0;
    LATD = 0;
    LATE = 0;
    BUTTON_WPU |= (1 << BUTTON_WPU_BIT);
    BUTTON_RXPU_REG &= ~(1 << BUTTON_RXPU_BIT);
    #endif
    #endif
}

static void inline boot_uninit(void)
{
    // Disable pull-up.
    #ifdef BUTTON_WPU
    #if defined(_PIC14E)
    OPTION_REG = 0xFF;
    WPUA = 0xFF;
    #if defined(_16F1459)
    WPUB = 0xFF;
    #endif

    #elif defined(_18F4450_FAMILY_) || defined(_18F4550_FAMILY_)
    #if BUTTON_RXPU_REG == INTCON2
    INTCON2 |= 80;
    #else
    PORTE &= 7F;
    #endif
    LATB = 0xFF;
    LATD = 0xFF;

    #elif defined(_18F14K50)
    INTCON2 = 0xFF;
    WPUA = 0xFF;
    WPUB = 0xFF;
    
    #elif defined(_18F24K50) || defined(_18F25K50) || defined(_18F45K50)
    INTCON2 = 0xFF;
    WPUB = 0xFF;
    TRISE = 0xFF;
    
    #elif defined(_18F24J50) || defined(_18F25J50) || defined(_18F26J50) || defined(_18F26J53) || defined(_18F27J53)
    BUTTON_RXPU_REG |= (1 << BUTTON_RXPU_BIT);
    LATB = 0xFF;
    
    #elif defined(_18F44J50) || defined(_18F45J50) || defined(_18F46J50) || defined(_18F46J53) || defined(_18F47J53)
    BUTTON_RXPU_REG |= (1 << BUTTON_RXPU_BIT);
    LATB = 0xFF;
    LATD = 0xFF;
    LATE = 0xFF;
    #endif
    #endif

    
    // Make boot pin analog.
    #if defined(BUTTON_ANSEL) 
    BUTTON_ANSEL = 0xFF;
    #elif defined(BUTTON_ANCON)
    BUTTON_ANCON = 0;
    #endif
    
    
    // Oscillator Settings.
    // PIC16F145X.
    #if defined(_PIC14E)
    #if (XTAL_USED == NO_XTAL)
    ACTCON = 0;
    #endif
    OSCCON = 0x1C;

    // PIC18F14K50.
    #elif defined(_18F14K50)
    OSCTUNEbits.SPLLEN = 0;
    
    // PIC18F2XK50.
    #elif defined(_18F24K50) || defined(_18F25K50) || defined(_18F45K50)
    #if XTAL_USED == NO_XTAL
    ACTCON = 0;
    #endif
    OSCCON2bits.PLLEN = 0;
    #if (XTAL_USED != MHz_12)
    OSCTUNEbits.SPLLMULT = 0;
    #endif
    #if XTAL_USED == NO_XTAL
    OSCCON = 0x30;
    #endif

    // PIC18F2XJ53.
    #elif defined(__J_PART)
    OSCTUNEbits.PLLEN = 0;
    #endif

    
}

static void check_user_first_inst(void)
{
#if defined(_PIC14E)
    PMCON1 = 0;
    PMADR = (PROG_REGION_START / 2);
    PMCON1bits.RD = 1;
    __asm("NOP");
    __asm("NOP");
    if(PMDAT == 0x3FFF) user_firmware = false;
    else user_firmware = true;
#else
    uint8_t inst[2];
    EECON1 = 0x80;
    TBLPTR = PROG_REGION_START;
    __asm("TBLRDPOSTINC");
    inst[0] = TABLAT;
    __asm("TBLRDPOSTINC");
    inst[1] = TABLAT;
    
    if(*((uint16_t*)inst) == 0xFFFF) user_firmware = false;
    else user_firmware = true;
#endif
}

void msd_rx_sector(void)
{
    boot_process_read();
}

void msd_tx_sector(void)
{
    boot_process_write();
}