/**
 * @file bootloader.h
 * @author John Izzard
 * @date 18/06/2020
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
#ifndef BOOTLOADER_H
#define BOOTLOADER_H

/* Emulated FAT16 File System
             ______________
    0x00000 |              |
            |  BOOT SECT   | 0x200 (512B)
    0x001FF |______________|
    0x00200 |              |
            |   FAT SECT   | 0x2200 (8.5KB)
    0x023FF |______________|
    0x02400 |              |
            |  ROOT SECT   | 0x200 (512B)
    0x025FF |______________|
    0x02600 |              |
            |  DATA SECT   | 0x200000 (2MB)
   0x2025FF |______________|

 */

/* PIC16F145X ROM Space
             ______________
    0x00000 |____RESET_____|
    0x00008 |____INT_HP____|
    0x00018 |____INT_LP____| 0x2000 (8KB)
            |              |
            | BOOT LOADER  |
    0x01FFF |______________|
    0x02000 |              |
            |  PROG MEM    | 0x2000 (8KB)
    0x03FFF |______________|
    0x10000 |              |
            | CONFIG WORDS | 0x0E
    0x1000D |______________|
 */

/* PIC18F14K50 ROM Space
             ______________
    0x00000 |____RESET_____|
    0x00008 |____INT_HP____|
    0x00018 |____INT_LP____| 0x2000 (8KB)
            |              |
            | BOOT LOADER  |
    0x01FFF |______________|
    0x02000 |              |
            |  PROG MEM    | 0x2000 (8KB)
    0x03FFF |______________|
   0x300000 |              |
            | CONFIG WORDS | 0x0E
   0x30000D |______________|
 */

/* PIC18F24K50 ROM Space
             ______________
    0x00000 |____RESET_____|
    0x00008 |____INT_HP____|
    0x00018 |____INT_LP____| 0x2000 (8KB)
            |              |
            | BOOT LOADER  |
    0x01FFF |______________|
    0x02000 |              |
            |  PROG MEM    | 0x2000 (8KB)
    0x03FFF |______________|
   0x300000 |              |
            | CONFIG WORDS | 0x0E
   0x30000D |______________|
 */

/* PIC18FX5K50 ROM Space
             ______________
    0x00000 |____RESET_____|
    0x00008 |____INT_HP____|
    0x00018 |____INT_LP____| 0x2000 (8KB)
            |              |
            | BOOT LOADER  |
    0x01FFF |______________|
    0x02000 |              |
            |  PROG MEM    | 0x6000 (24KB)
    0x07FFF |______________|
   0x300000 |              |
            | CONFIG WORDS | 0x0E
   0x30000D |______________|
 */

/* PIC18FX6J53 ROM Space
             ______________
    0x00000 |____RESET_____|
    0x00008 |____INT_HP____|
    0x00018 |____INT_LP____| 0x2000 (8KB)
            |              |
            | BOOT LOADER  |
    0x01FFF |______________|
    0x02000 |              |
            |  PROG MEM    | 0x0DC00 (55KB)
    0x0FBFF |______________|
    0x0FC00 |              |
            | CONFIG WORDS | 0x400 (1KB)
    0x0FFFF |______________|
 */

/* PIC18FX7J53 ROM Space
             ______________
    0x00000 |____RESET_____|
    0x00008 |____INT_HP____|
    0x00018 |____INT_LP____| 0x2000 (8KB)
            |              |
            | BOOT LOADER  |
    0x01FFF |______________|
    0x02000 |              |
            |  PROG MEM    | 0x1DC00 (119KB)
    0x1FBFF |______________|
    0x1FC00 |              |
            | CONFIG WORDS | 0x400 (1KB)
    0x1FFFF |______________|
 */

#include "config.h"

// Memory Regions.
#if defined(_PIC14E)
#define BOOT_REGION_START     0x00000
#define PROG_REGION_START     0x02000
#define END_OF_FLASH          0x04000
#define CONFIG_REGION_START   0x10000
#define CONFIG_BLOCK_REGION   CONFIG_REGION_START
#define CONFIG_PAGE_START     CONFIG_REGION_START
#define DEV_ID_START          0x1000C

// FLASH USER SPACE
#define FILE_SIZE 0x2000 // In bytes

#elif defined(_18F14K50) || defined(_18F24K50)
#define BOOT_REGION_START     0x00000
#define PROG_REGION_START     0x02000
#define END_OF_FLASH          0x04000
#define ID_REGION_START       0x200000
#define CONFIG_REGION_START   0x300000
#define CONFIG_BLOCK_REGION   CONFIG_REGION_START
#define CONFIG_PAGE_START     CONFIG_REGION_START
#define DEV_ID_START          0x3FFFFE
#define EEPROM_REGION_START   0xF00000
#define END_OF_EEPROM         0xF00100
#define EEPROM_SIZE           0x100

// FLASH USER SPACE
#define FILE_SIZE 0x2000 // In bytes

#elif defined(_18F25K50) || defined(_18F45K50)
#define BOOT_REGION_START     0x00000
#define PROG_REGION_START     0x02000
#define END_OF_FLASH          0x08000
#define ID_REGION_START       0x200000
#define CONFIG_REGION_START   0x300000
#define CONFIG_BLOCK_REGION   CONFIG_REGION_START
#define CONFIG_PAGE_START     CONFIG_REGION_START
#define DEV_ID_START          0x3FFFFE
#define EEPROM_REGION_START   0xF00000
#define END_OF_EEPROM         0xF00100
#define EEPROM_SIZE           0x100

// FLASH USER SPACE
#define FILE_SIZE 0x6000 // In bytes

#elif defined(_18F26J53) || defined(_18F46J53)
#define BOOT_REGION_START     0x00000
#define PROG_REGION_START     0x02000
#define CONFIG_BLOCK_REGION   0x0FFC0
#define CONFIG_REGION_START   0x0FFF8
#define CONFIG_PAGE_START     0x0FC00
#define END_OF_FLASH          0x10000
#define DEV_ID_START          0x3FFFFE

// FLASH USER SPACE
#define FILE_SIZE 0x0E000 // In bytes

#elif defined(_18F27J53) || defined(_18F47J53)
#define BOOT_REGION_START     0x00000
#define PROG_REGION_START     0x02000
#define CONFIG_BLOCK_REGION   0x1FFC0
#define CONFIG_REGION_START   0x1FFF8
#define CONFIG_PAGE_START     0x1FC00
#define END_OF_FLASH          0x20000
#define DEV_ID_START          0x3FFFFE

// FLASH USER SPACE
#define FILE_SIZE 0x1E000 // In bytes
#endif

// Make changes based on if has EEPROM or not.
#ifdef EEPROM_REGION_START
#define HAS_EEPROM
#endif

// FAT16 File system constants.
#define ROOT_ENTRY_COUNT 16
#define FAT_SIZE 17

#define BOOT_SECT_ADDR     0
#define FAT_SECT_ADDR      1
#define ROOT_SECT_ADDR     18
#define DATA_SECT_ADDR     19
#ifndef HAS_EEPROM
#define ABOUT_SECT_ADDR    DATA_SECT_ADDR
#define PROG_MEM_SECT_ADDR 20
#else
#define ABOUT_SECT_ADDR    DATA_SECT_ADDR
#define EEPROM_SECT_ADDR   20
#define PROG_MEM_SECT_ADDR 21
#endif

#define FILE_CLUSTERS (FILE_SIZE / 512)

#ifdef HAS_EEPROM
#define PROG_MEM_CLUST 4
#else
#define PROG_MEM_CLUST 3
#endif

// Bootloader State.
#define BOOT_DUMMY    0
#define BOOT_LOAD_HEX 1
#define BOOT_FINISHED 2

// Hex Parser State.
#define HEX_START       0
#define HEX_REC_LEN     1
#define HEX_LOAD_OFFSET 2
#define HEX_RECTYPE     3
#define HEX_DATA        4
#define HEX_ELA         5
#define HEX_ESA         6
#define HEX_SLA         7
#define HEX_SSA         8
#define HEX_EOF         9
#define HEX_CHKSUM     10
#define HEX_PARSING    11
#define HEX_FINISHED   12
#define HEX_FAULT      13

// Hex File Record Types.
#define  DATA_REC 0 // Data Record
#define  EOF_REC  1 // End of File Record
#define  ESA_REC  2 // Extended Segment Address Record
#define  SSA_REC  3 // Start Segment Address Record
#define  ELA_REC  4 // Extended Linear Address Record
#define  SLA_REC  5 // Start Linear Address Record

// Volume Labels based on proccessor.
#if defined(_PIC14E)
#define VOLUME_LABEL {'P','I','C','1','6','F','1','4','5','X',' '}
#elif defined(_18F14K50)
#define VOLUME_LABEL {'P','I','C','1','8','F','1','4','K','5','0'}
#elif defined(_18F24K50)
#define VOLUME_LABEL {'P','I','C','1','8','F','2','4','K','5','0'}
#elif defined(_18F25K50) || defined(_18F45K50)
#define VOLUME_LABEL {'P','I','C','1','8','F','X','5','K','5','0'}
#elif defined(_18F26J53) || defined(_18F46J53)
#define VOLUME_LABEL {'P','I','C','1','8','F','X','6','J','5','3'}
#elif defined(_18F27J53) || defined(_18F47J53)
#define VOLUME_LABEL {'P','I','C','1','8','F','X','7','J','5','3'}
#endif

#define ROOT_NAME VOLUME_LABEL

#include <stdint.h>
#include "usb_msd.h"

/** Boot Sector */
typedef struct
{
    uint8_t  jmpBoot[3];
    uint8_t  OEMName[8];
    uint16_t BytesPerSec;
    uint8_t  SecPerClus;
    uint16_t RsvdSecCnt;
    uint8_t  NumFATs;
    uint16_t RootEntCnt;
    uint16_t TotSec16;
    uint8_t  Media;
    uint16_t FATSz16;
    uint16_t SecPerTrk;
    uint16_t NumHeads;
    uint32_t HiddSec;
    uint32_t TotSec32;
    uint8_t  DrvNum;
    uint8_t  Reserved1;
    uint8_t  BootSig;
    uint8_t  VolID[4];
    uint8_t  VolLab[11];
    uint8_t  FilSysType[8];
}BOOT16_t;

const BOOT16_t BOOT16 = {
    {0xEB,0x3C,0x90},
    {'M','S','D','O','S','5','.','0'},
    BYTES_PER_BLOCK_LE,
    1,
    1,
    1,
    ROOT_ENTRY_COUNT,
    VOL_CAPACITY_IN_BLOCKS,
    0xF8,
    FAT_SIZE,
    0,
    0,
    0,
    0,
    0,
    0,
    0x29,
    {0x86,0xE8,0xA3,0x56},
    VOLUME_LABEL,
    {'F','A','T','1','6',' ',' ',' '}
};

///** Directory Entry Structure */
//typedef struct
//{
//    uint8_t  Name[11];
//    uint8_t  Attr;
//    uint8_t  NTRes;
//    uint8_t  CrtTimeTenth;
//    uint16_t CrtTime;
//    uint16_t CrtDate;
//    uint16_t LstAccDate;
//    uint16_t FstClusHI;
//    uint16_t WrtTime;
//    uint16_t WrtDate;
//    uint16_t FstClusLO;
//    uint32_t FileSize;
//}DIR_ENTRY_t;
typedef uint8_t DIR_ENTRY_t[11];

/** Root Directory Type */
typedef struct
{
    DIR_ENTRY_t VOL;
    #if !defined(SIMPLE_BOOTLOADER)
    DIR_ENTRY_t FILE1;
    DIR_ENTRY_t FILE2;
    #if defined(HAS_EEPROM)
    DIR_ENTRY_t FILE3;
    #endif
    #endif
}ROOT_DIR_t;

const uint8_t aboutFile[] = "<html><script>window.location=\"https://github.com/johnnydrazzi/USB-uC\";</script></html>";

/** Volume Root Entry */
const ROOT_DIR_t ROOT =
{
    ROOT_NAME,
    #if !defined(SIMPLE_BOOTLOADER)
    {'A','B','O','U','T',' ',' ',' ','H','T','M'},
    #if defined(HAS_EEPROM)
    {'E','E','P','R','O','M',' ',' ','B','I','N'},
    #endif
    {'P','R','O','G','_','M','E','M','B','I','N'}
    #endif
};

extern bool g_boot_reset;

void boot_process_read(void);
void boot_process_write(void);

#endif /* BOOTLOADER_H */
