/**
 * @file EEPROM.c
 * @brief EEPROM library.
 * @author John Izzard
 * @date 13/08/2017
 * 
 * This file works with all versions
 * 
 * USB uC - USB MSD Bootloader.
 * Copyright (C) 2018  John Izzard
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

#if defined(_18F24K50)||defined(_18F25K50)||defined(_18F45K50)||defined(_18F25K50)
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