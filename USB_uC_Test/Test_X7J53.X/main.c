/*
 * File:   main.c
 * Author: Johnny
 *
 * Created on 24 February 2018, 3:25 PM
 */

#include <xc.h>

#define _XTAL_FREQ 48000000

#define BUTTON PORTBbits.RB6
#define LED    LATBbits.LB7

#define BUTTON_TRIS TRISBbits.TRISB6
#define LED_TRIS    TRISBbits.TRISB7

#define OUTPUT 0
#define INPUT  1

void main(void)
{
    OSCTUNEbits.PLLEN = 1;
    BUTTON_TRIS = INPUT;
    LED = 0;
    LED_TRIS = OUTPUT;
    
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
