/**
 * @file flash.h
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
#ifndef FLASH_H
#define FLASH_H

#include <stdint.h>

#ifndef _PIC18 // Non-PIC18
void Flash_ReadBytes(uint16_t start_addr, uint16_t bytes, uint8_t *flash_array);
void Flash_Erase(uint16_t start_addr, uint16_t end_addr);
void Flash_EraseWriteBlock(uint16_t start_addr, uint8_t *flash_array);
void Flash_WriteBlock(uint16_t start_addr, uint8_t *flash_array);
#else
void Flash_ReadBytes(uint24_t start_addr, uint24_t bytes, uint8_t *flash_array);
void Flash_Erase(uint24_t start_addr, uint24_t end_addr);
void Flash_EraseWriteBlock(uint24_t start_addr, uint8_t *flash_array);
void Flash_WriteBlock(uint24_t start_addr, uint8_t *flash_array);
void Flash_WriteConfigBlock(uint8_t *flash_array);
#endif /* _PIC18 */

#endif /* FLASH_H */