
#include <xc.h>
#include <stdint.h>

#define PIM          0 // Compatible with Microchip's MA180029 dev board https://www.microchip.com/DevelopmentTools/ProductDetails/PartNO/MA180029.
#define PIC_CLICKER  1 // Compatible with MikroElektronika's PIC clicker dev board https://www.mikroe.com/clicker-pic18fj.
#define DEV_BOARD    2 // A custom dev board I use.
#define CUSTOM       3 // Write your own.
#define BOARD_VERSION DEV_BOARD

#if BOARD_VERSION == PIM
#define BUTTON_PORT_BIT   2
#define BUTTON_PORT       PORTB
#define BUTTON_ANCON_BIT  0
#define BUTTON_ANCON      ANCON1
#define BUTTON_ACTIVE_LOW
#define LED_BIT           1
#define LED_LAT           LATE
#define LED_TRIS          TRISE

#elif BOARD_VERSION == PIC_CLICKER
#define BUTTON_PORT_BIT   3
#define BUTTON_PORT       PORTD
#define BUTTON_ACTIVE_LOW
#define LED_BIT           0
#define LED_LAT           LATA
#define LED_TRIS          TRISA

#elif BOARD_VERSION == DEV_BOARD
#define BUTTON_PORT_BIT   6
#define BUTTON_PORT       PORTB
#define LED_BIT           7
#define LED_LAT           LATB
#define LED_TRIS          TRISB

#elif BOARD_VERSION == CUSTOM
#define BUTTON_PORT_BIT     // Button's bit in the I/O PORT.
#define BUTTON_PORT         // Button's I/O PORT.
//#define BUTTON_ANCON_BIT  // Uncomment and define the Button's pin to make digital (if needed).
//#define BUTTON_ANCON      // Uncomment and define the ANCON register.
//#define BUTTON_WPU_BIT    // Uncomment and define the Button's Weak Pull-Up pin (if needed).
//#define BUTTON_WPU        // Uncomment and define the WPU register.
//#define BUTTON_RXPU_BIT   // Uncomment and define the bit that enables the Weak Pull-Up pin.
//#define BUTTON_RXPU_REG   // Uncomment and define which register has the RXPU bit to enable the Weak Pull-Up pin.
#define BUTTON_ACTIVE_LOW   // Uncomment to make the Button active low.
#define LED_BIT             // LED's bit in the LAT register.
#define LED_LAT             // LED's LAT register.
#define LED_TRIS            // LED's TRIS register.
//#define LED_ACTIVE_LOW    // Uncomment to make the LED active low.
#endif

#define _XTAL_FREQ 48000000
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

#if !defined(TRISE)
#define TRISE TRISE
uint8_t TRISE __at(0xF96); // This shouldn't be missing!!
#endif

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
    OSCTUNEbits.PLLEN = 1;
    PLL_STARTUP_DELAY();
    
    // Make boot pin digital.
    #ifdef BUTTON_ANCON
    BUTTON_ANCON |= (1<<BUTTON_ANCON_BIT);
    #endif

    // Apply pull-up.
    #ifdef BUTTON_WPU
    #if defined(_18F24J50) || defined(_18F25J50) || defined(_18F26J50) || defined(_18F26J53) || defined(_18F27J53)
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
