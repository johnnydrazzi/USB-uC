/**
 * @file main.c
 * @brief Main C file.
 * @author John Izzard
 * @date 17/06/2020
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

#include <xc.h>
#include <stdint.h>
#include "fuses.h"
#include "config.h"
#include "usb.h"
#include "bootloader.h"
#include "flash.h"

#if defined(_PIC14E)
void __interrupt() isr(void)
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
static void inline boot_end(void);
static void flash_led(void);

static uint16_t m_delay_cnt = 0;

void main(void)
{
    boot_init();

    if(BUTTON || (*g_user_first_inst == 0xFFFF))
    {
        while(BUTTON){}
        LED_TRIS = 0;
        flash_led();
        g_boot_reset = false;
        usb_init();
        while(1)
        {
            usb_tasks();
            msd_tasks();
            if(g_boot_reset)
            {
                m_delay_cnt++;
                __delay_ms(1);
                if(m_delay_cnt == 20) // Delay used to close USB after some time.
                {
                    goto RESTART;
                }
            }
            if(BUTTON)
            {
                __delay_ms(100);
                while(BUTTON){}
                __delay_ms(100); // Incase button bounce, stops bootloader starting again.
                goto RESTART;
            }
        }
    }
    
    goto START_USER;
    
    RESTART:
    usb_close();
    __asm("RESET");
    
    START_USER:
    boot_end();
    #if defined(_PIC14E)
    __asm("LJMP "___mkstr(PROG_REGION_START / 2));
    #else
    __asm("goto " ___mkstr(PROG_REGION_START));
    #endif
}

static void inline boot_init(void)
{
    #if defined(_PIC14E)
    #if (XTAL_USED == NO_XTAL)
    OSCCONbits.IRCF = 0xF;
    #endif
    #if (XTAL_USED != MHz_12)
    OSCCONbits.SPLLMULT = 1;
    #endif
    OSCCONbits.SPLLEN = 1;
    PLL_STARTUP_DELAY();
    #if (XTAL_USED == NO_XTAL)
    ACTCONbits.ACTSRC = 1;
    ACTCONbits.ACTEN = 1;
    #endif

    #elif defined(_18F14K50) || defined(_18F13K50)
    OSCTUNEbits.SPLLEN = 1;
    PLL_STARTUP_DELAY();
    ANSEL = 0;
    
    #elif defined(_18F24K50) || defined(_18F25K50) || defined(_18F45K50)
    #if XTAL_USED == NO_XTAL
    OSCCONbits.IRCF = 7;
    #endif
    #if (XTAL_USED == MHz_16) || (XTAL_USED == NO_XTAL)
    OSCTUNEbits.SPLLMULT = 1;
    #endif
    OSCCON2bits.PLLEN = 1;
    PLL_STARTUP_DELAY();
    #if XTAL_USED == NO_XTAL
    ACTCONbits.ACTSRC = 1;
    ACTCONbits.ACTEN = 1;
    ANSELA = 0;
    #endif
    
    #elif defined(_18F26J53) || defined(_18F46J53) || defined(_18F27J53) || defined(_18F47J53)
    OSCTUNEbits.PLLEN = 1;
    PLL_STARTUP_DELAY();
    #endif
}

static void inline boot_end(void)
{
    #if defined(_PIC14E)
    #if (XTAL_USED == NO_XTAL)
    ACTCONbits.ACTEN = 0;
    ACTCONbits.ACTSRC = 0;
    #endif
    OSCCONbits.SPLLEN = 0;
    #if (XTAL_USED != MHz_12)
    OSCCONbits.SPLLMULT = 0;
    #endif
    #if (XTAL_USED == NO_XTAL)
    OSCCONbits.IRCF = 7;
    #endif
    

    #elif defined(_18F14K50)
    OSCTUNEbits.SPLLEN = 0;
    ANSEL = 0xFF;
    
    #elif defined(_18F24K50) || defined(_18F25K50) || defined(_18F45K50)
    #if XTAL_USED == NO_XTAL
    ANSELA = 0xFF;
    ACTCONbits.ACTSRC = 0;
    ACTCONbits.ACTEN = 0;
    #endif
    OSCCON2bits.PLLEN = 0;
    #if (XTAL_USED == MHz_16)||(XTAL_USED == NO_XTAL)
    OSCTUNEbits.SPLLMULT = 0;
    #endif
    #if XTAL_USED == NO_XTAL
    OSCCONbits.IRCF = 3;
    #endif

    #else
    OSCTUNEbits.PLLEN = 0;
    #endif
}

static void flash_led(void)
{
    for(uint8_t i = 0; i < 3; i++)
    {
        LED = 1;
        __delay_ms(100);
        LED = 0;
        __delay_ms(50);
    }
}

void msd_rx_sector(void)
{
    boot_process_read();
}

void msd_tx_sector(void)
{
    boot_process_write();
}