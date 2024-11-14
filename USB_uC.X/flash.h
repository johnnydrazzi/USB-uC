/**
 * @file flash.h
 * @author John Izzard
 * @date 2024-11-12
 * 
 * @brief Flash Library.
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
 * File Version 1.0.1 - 2024-11-12
 * - Changed: MIT License.
 *
 * File Version 1.0.0 - 2020-06-28
 * - Added: Initial release of the software.
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