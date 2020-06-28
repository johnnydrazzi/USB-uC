/**
 * @file flash.c
 * @author John Izzard
 * @date 13/02/2019
 * 
 * USB uC - USB MSD Bootloader.
 * Copyright (C) 2017-2019  John Izzard
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
#include <stdint.h>
#include <xc.h>
#include "flash.h"

#define LOOPS _FLASH_ERASE_SIZE/_FLASH_WRITE_SIZE

#if defined(_PIC14E)
#define _EECON1 PMCON1
#define _EECON1bits PMCON1bits
#define _EECON2 PMCON2
#define _EEADR PMADRL
#define _EEADRH PMADRH
#define _EEDATA PMDATL
#define _EEDATH PMDATH
#else
#define _EECON1 EECON1
#define _EECON1bits EECON1bits
#define _EECON2 EECON2
#define _EEADR EEADR
#define _EEADRH EEADRH
#define _EEDATA EEDATA
#define _EEDATH EEDATH
#endif

#if defined(_PIC14)||defined(_PIC14E) 
void Flash_ReadBytes(uint16_t start_addr, uint16_t bytes, uint8_t *flash_array){
    _EECON1 = 0x80;
    while(bytes){
        _EEADRH = (uint8_t)(start_addr>>8);
        _EEADR = (uint8_t)(start_addr);
        
        _EECON1bits.RD = 1;
        NOP();
        NOP();
        
        *flash_array++ = _EEDATA;
        bytes--;
        if(bytes == 0) break;
        
        *flash_array++ = _EEDATH;
        bytes--;
        if(bytes == 0) break;
        
        start_addr++;
    }
}
void Flash_Erase(uint16_t start_addr, uint16_t end_addr){
    _EECON1 = 0x84;
    while(start_addr<end_addr){
        _EEADRH = (uint8_t)(start_addr>>8);
        _EEADR = (uint8_t)(start_addr);
        _EECON1bits.FREE = 1;
        _EECON2 = 0x55;
        _EECON2 = 0xAA;
        _EECON1bits.WR = 1;
        NOP();
        NOP();
        start_addr += _FLASH_ERASE_SIZE;
    }
    _EECON1bits.WREN = 0;
}
void Flash_EraseWriteBlock(uint16_t start_addr, uint8_t *flash_array){
#if _FLASH_ERASE_SIZE>_FLASH_WRITE_SIZE
    uint8_t i;
    
    Flash_Erase(start_addr, start_addr + _FLASH_ERASE_SIZE);
    
    for(i=0;i<LOOPS;i++){
        Flash_WriteBlock(start_addr, flash_array);
        flash_array+= (_FLASH_WRITE_SIZE*2);
        start_addr += _FLASH_WRITE_SIZE;
    }
#else
    Flash_Erase(start_addr,start_addr + _FLASH_ERASE_SIZE);
    Flash_WriteBlock(start_addr, flash_array);
#endif
}
void Flash_WriteBlock(uint16_t start_addr, uint8_t *flash_array){
#ifdef _PIC14
    uint8_t i;
    
    _EECON1 = 0x84; // EEPGD = 1, CFGS = 0, WREN = 1
    _EEADRH = (uint8_t)(start_addr>>8);
    _EEADR = (uint8_t)(start_addr);
    for(i=0;i<_FLASH_WRITE_SIZE;i++){
        _EEADRH = (uint8_t)(start_addr>>8);
        _EEADR = (uint8_t)(start_addr);
        _EEDATA = *flash_array++;
        _EEDATH = *flash_array++;
        _EECON2 = 0x55;
        _EECON2 = 0xAA;
        _EECON1bits.WR = 1;
        NOP();
        NOP();
        _EEADR++;
    }
    _EECON1bits.WREN = 0;
#else
    uint8_t word_cnt = _FLASH_WRITE_SIZE;
    
    _EECON1 = 0xA4; // EEPGD = 1, CFGS = 0, FREE = 0, LWLO = 1, WREN = 1
    _EEADRH = (uint8_t)(start_addr>>8);
    _EEADR = (uint8_t)(start_addr);
    while(1){
        _EEDATA = *flash_array++;
        _EEDATH = *flash_array++;
        word_cnt--;
        if(word_cnt == 0)break;
        _EECON2 = 0x55;
        _EECON2 = 0xAA;
        _EECON1bits.WR = 1;
        NOP();
        NOP();
        _EEADR++;
    }
    _EECON1bits.LWLO = 0;
    _EECON2 = 0x55;
    _EECON2 = 0xAA;
    _EECON1bits.WR = 1;
    NOP();
    NOP();
    _EECON1bits.WREN = 0;
#endif
}
#elif defined(_PIC18)
void Flash_ReadBytes(uint24_t start_addr, uint24_t bytes, uint8_t *flash_array){
    EECON1 = 0x80; // EEPGD = 1 and CFGS = 0
    while(bytes){
        TBLPTRU = (uint8_t)(start_addr>>16);
        TBLPTRH = (uint8_t)(start_addr>>8);
        TBLPTRL = (uint8_t)(start_addr);
        
        asm("TBLRDPOSTINC");
        *flash_array++ = TABLAT;
        bytes--;
        if(bytes == 0) break;
        
        asm("TBLRDPOSTINC");
        *flash_array++ = TABLAT;
        bytes--;
        if(bytes == 0) break;
        
        start_addr+=2;
    }
}
void Flash_Erase(uint24_t start_addr, uint24_t end_addr){
    EECON1 = 0x84; // EEPGD = 1, CFGS = 0, WREN = 1
    while(start_addr<end_addr){
        TBLPTRU = (uint8_t)(start_addr>>16);
        TBLPTRH = (uint8_t)(start_addr>>8);
        TBLPTRL = (uint8_t)(start_addr);
        EECON1bits.FREE = 1;
        EECON2 = 0x55;
        EECON2 = 0xAA;
        EECON1bits.WR = 1;
        start_addr += _FLASH_ERASE_SIZE;
    }
    EECON1bits.WREN = 0;
}
void Flash_EraseWriteBlock(uint24_t start_addr, uint8_t *flash_array){
#if _FLASH_ERASE_SIZE>_FLASH_WRITE_SIZE
    uint8_t i;
    
    Flash_Erase(start_addr, start_addr + _FLASH_ERASE_SIZE);
    
    for(i=0;i<LOOPS;i++){
        Flash_WriteBlock(start_addr, flash_array);
        flash_array+= _FLASH_WRITE_SIZE;
        start_addr += _FLASH_WRITE_SIZE;
    }
#else
    Flash_Erase(start_addr,start_addr + _FLASH_ERASE_SIZE);
    Flash_WriteBlock(start_addr, flash_array);
#endif
}
void Flash_WriteBlock(uint24_t start_addr, uint8_t *flash_array){
    uint8_t i;
    
    EECON1 = 0x84; // EEPGD = 1, CFGS = 0, WREN = 1
    TBLPTR = 0;
    for(i=0;i<_FLASH_WRITE_SIZE;i++){
        TABLAT = *flash_array++;
        asm("TBLWTPOSTINC");
    }
    TBLPTRU = (uint8_t)(start_addr>>16);
    TBLPTRH = (uint8_t)(start_addr>>8);
    TBLPTRL = (uint8_t)(start_addr);
    EECON2 = 0x55;
    EECON2 = 0xAA;
    EECON1bits.WR = 1;
    EECON1bits.WREN = 0;
}
#else
#error FLASH - DEVICE NOT YET SUPPORTED
#endif