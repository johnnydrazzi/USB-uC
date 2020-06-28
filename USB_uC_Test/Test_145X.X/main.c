/*
 * File:   main.c
 * Author: John
 *
 * Created on 24 March 2018, 6:45 PM
 */

#include <xc.h>
#include <stdint.h>

#define NO_XTAL 0
#define MHz_12  1
#define MHz_16  2

#define XTAL_USED NO_XTAL

#if (XTAL_USED == MHz_12)
#define _XTAL_FREQ 12000000
#else
#define _XTAL_FREQ 16000000
#endif

//#define XPRESS

#ifdef XPRESS
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

void main(void)
{
    #if (XTAL_USED == NO_XTAL)
    OSCCONbits.IRCF = 0xF;
    #endif
    LED = 0;
    LED_TRIS = 0;
    while(1)
    {
        if(BUTTON)
        {
            LED = 1;
            __delay_ms(250);
            LED = 0;
            __delay_ms(250);
        }
        else
        {
            LED = 1;
            __delay_ms(500);
            LED = 0;
            __delay_ms(500);
        }
    }
    return;
}
