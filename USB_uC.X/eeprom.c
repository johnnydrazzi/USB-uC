/**
 * @file eeprom.c
 * @author John Izzard
 * @date 2024-11-12
 * 
 * @brief EEPROM Library.
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
 * File Version 2.0.1 - 2024-11-12
 * - Changed: MIT License.
 *
 * File Version 2.0.0 - 2024-06-30
 * - Changed: File name to lower case.
 *
 * File Version 1.1.0 - 2023-04-07
 * - Added: Support for _18F4450_FAMILY_.
 *
 * File Version 1.0.0 - 2020-06-28
 * - Added: Initial release of the software.
 */

#include <xc.h>
#include <stdint.h>

#ifdef _18F46K80
void EEPROM_Write(uint16_t address,uint8_t data){
    EEADRH = address>>8;
    EEADR = address;
    EEDATA = data;
    
    EECON1bits.EEPGD = 0; // 0 means EEPROM is selected, not Program Memory
    EECON1bits.CFGS = 0;  // 0 means configuration register is not selected, Program/EEPROM is
    EECON1bits.WREN = 1;  // 1 means writing is enabled
    
    //INTCONbits.GIE = 0;   // Turn off interrupts
    EECON2 = 0x55;
    EECON2 = 0x0AA;
    EECON1bits.WR = 1;
    while(EECON1bits.WR){}
    //INTCONbits.GIE = 1;
    
    EECON1bits.WREN = 0;
}
uint8_t EEPROM_Read(uint16_t address){
    EEADRH = address>>8;
    EEADR = address;
    
    EECON1bits.EEPGD = 0;
    EECON1bits.CFGS = 0;
    EECON1bits.RD = 1;
    asm("NOP");
    
    return EEDATA;
}
#endif

#if defined(_18F4550_FAMILY_)||defined(_18F24K50)||defined(_18F25K50)||defined(_18F45K50)
void EEPROM_Write(uint8_t address,uint8_t data){
    EEADR = address;
    EEDATA = data;
    
    EECON1bits.EEPGD = 0; // 0 means EEPROM is selected, not Program Memory
    EECON1bits.CFGS = 0;  // 0 means configuration register is not selected, Program/EEPROM is
    EECON1bits.WREN = 1;  // 1 means writing is enabled
    
    EECON2 = 0x55;
    EECON2 = 0x0AA;
    EECON1bits.WR = 1;
    while(EECON1bits.WR){}
    
    EECON1bits.WREN = 0;
}
uint8_t EEPROM_Read(uint8_t address){
    EEADR = address;
    
    EECON1bits.EEPGD = 0;
    EECON1bits.CFGS = 0;
    EECON1bits.RD = 1;
    asm("NOP");
    
    return EEDATA;
}
#endif

#if defined(_18F14K50)
void EEPROM_Write(uint8_t address,uint8_t data){
    EEADR = address;
    EEDATA = data;
    
    EECON1bits.EEPGD = 0; // 0 means EEPROM is selected, not Program Memory
    EECON1bits.CFGS = 0;  // 0 means configuration register is not selected, Program/EEPROM is
    EECON1bits.WREN = 1;  // 1 means writing is enabled
    
    EECON2 = 0x55;
    EECON2 = 0x0AA;
    EECON1bits.WR = 1;
    while(EECON1bits.WR){}
    
    EECON1bits.WREN = 0;
}
uint8_t EEPROM_Read(uint8_t address){
    EEADR = address;
    
    EECON1bits.EEPGD = 0;
    EECON1bits.CFGS = 0;
    EECON1bits.RD = 1;
    asm("NOP");
    
    return EEDATA;
}
#endif