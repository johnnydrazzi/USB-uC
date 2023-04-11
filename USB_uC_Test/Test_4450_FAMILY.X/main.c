
#include <xc.h>
#include <stdint.h>

#define MHz_4  1
#define MHz_8  2
#define MHz_12 3
#define MHz_16 4
#define MHz_20 5
#define MHz_24 6
#define MHz_40 10
#define MHz_48 12

#define GENERAL    0 // Compatible with dev boards that have a reset button.
#define DEV_BOARD  1 // A custom dev board I use.
#define CUSTOM     2 // Write your own.
#define BOARD_VERSION DEV_BOARD

#if BOARD_VERSION == GENERAL
#define XTAL_USED         MHz_8
#define BUTTON_PORT_BIT   3
#define BUTTON_PORT       PORTE
#define BUTTON_ACTIVE_LOW

#elif BOARD_VERSION == DEV_BOARD
#define XTAL_USED         MHz_8
#define USE_MCLRE
#define USE_LVP
#define BUTTON_PORT_BIT   6
#define BUTTON_PORT       PORTB
#define USE_BOOT_LED
#define LED_BIT           7
#define LED_LAT           LATB
#define LED_TRIS          TRISB

#elif BOARD_VERSION == CUSTOM
#define XTAL_USED           // Select oscillator option.
#define USE_MCLRE           // Uncomment to enable MCLRE (reset pin).
#define USE_LVP             // Uncomment if LVP (Low Voltage Programming) is needed.
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
