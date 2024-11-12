/**
 * @file bootloader.c
 * @author John Izzard
 * @date 2024-11-12
 * 
 * @brief USB uC - USB MSD Bootloader.
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
 * File Version 4.0.1 - 2024-11-12
 * - Changed: MIT License.
 *
 * File Version 4.0.0 - 2024-06-30
 * - Changed: Large refactoring / size reduction.
 *
 * File Version 3.0.0 - 2024-01-28
 * - Changed: Large refactoring.
 *
 * File Version 2.1.0 - 2023-04-07
 * - Added: Support for _18F4450_FAMILY_ and _18F46J50_FAMILY_.
 *
 * File Version 2.0.0 - 2021-05-01
 * - Changed: Refactoring and API change for user_first_instruction -> user_firmware.
 * - Added: Full support for PIC16F145X.
 *
 * File Version 1.0.0 - 2020-06-28
 * - Added: Initial release of the software.
 */

#include <xc.h>
#include <stdint.h>
#include <stdbool.h>
#include "usb.h"
#include "bootloader.h"
#include "config.h"
#include "flash.h"
#include "eeprom.h"

#if defined(_PIC14E)
// _FLASH_WRITE_SIZE is in words (14bits), double to be in bytes.
#define FLASH_WRITE_SIZE (_FLASH_WRITE_SIZE * 2)
#else
#define FLASH_WRITE_SIZE  _FLASH_WRITE_SIZE
#endif

#define INDEX_MASK (((uint24_t)FLASH_WRITE_SIZE) - 1)
#define FLASH_ADDR_MASK ~INDEX_MASK

/* ************************************************************************** */
/* ************************** GLOBAL VARIABLES ****************************** */
/* ************************************************************************** */

bool g_boot_reset;

/* ************************************************************************** */
/* ************************* EXTERNAL VARIABLES ***************************** */
/* ************************************************************************** */

extern bool user_firmware;

/* ************************************************************************** */
/* ************************* STATIC PROTOTYPES ****************************** */
/* ************************************************************************** */

static void     generate_boot(void);
static void     generate_FAT(void);
static void     generate_root(void);

static uint8_t  hex_parse(uint8_t chr);
static bool     hex_char_to_char(uint8_t* chr);

static bool     update_erase_block(uint24_t address, uint8_t* data, uint8_t cnt);
static uint32_t LBA_to_flash_addr (uint32_t LBA);
static void     delete_file(void);
static bool     safely_write_block(uint24_t start_addr);

static uint8_t  get_device(void);

/* ************************************************************************** */
/* ************************** STATIC VARIABLES ****************************** */
/* ************************************************************************** */

static uint8_t  m_flash_block[FLASH_WRITE_SIZE];
static uint24_t m_prev_flash_addr = PROG_REGION_START;
static uint8_t  m_prev_block_index = 0;

/* ************************************************************************** */
/* ************************** GLOBAL FUNCTIONS ****************************** */
/* ************************************************************************** */

void boot_process_read(void)
{
    usb_ram_set(0, g_msd_ep_in, MSD_EP_SIZE); // Blank Regions of memory are read as zero.
    
    if(g_msd_rw_10_vars.LBA == BOOT_SECT_ADDR)      generate_boot(); // If PC is reading the Boot Sector.
    else if(g_msd_rw_10_vars.LBA == FAT_SECT_ADDR)  generate_FAT();  // If PC is reading the first FAT Sector.
    else if(g_msd_rw_10_vars.LBA == ROOT_SECT_ADDR) generate_root(); // If PC is reading the Root Sector.
    #ifndef SIMPLE_BOOTLOADER
    else if(g_msd_rw_10_vars.LBA >= DATA_SECT_ADDR) // If PC is reading the Data Sector.
    {
        if(g_msd_rw_10_vars.LBA == ABOUT_SECT_ADDR) // If PC is reading ABOUT file data.
        {
            if(g_msd_byte_of_sect == 0) usb_rom_copy(aboutFile, g_msd_ep_in, 64);
            else if(g_msd_byte_of_sect == 64) usb_rom_copy((aboutFile + 64), g_msd_ep_in, sizeof(aboutFile)-64);
        }
        #if defined(HAS_EEPROM)
        else if(g_msd_rw_10_vars.LBA == EEPROM_SECT_ADDR)
        {
            for(uint8_t i = 0; i < MSD_EP_SIZE; i++) g_msd_ep_in[i] = EEPROM_Read((uint8_t)g_msd_byte_of_sect + i);
        }
        #endif
        else
        {
            // Convert from LBA address space to flash address space.
            uint24_t addr = (uint24_t)LBA_to_flash_addr(g_msd_rw_10_vars.LBA);
            if(addr < END_OF_FLASH) // If address is in flash space.
            {
                // Read flash into g_msd_ep_in buffer.
                #if defined(_PIC14E)
                Flash_ReadBytes(addr / 2, MSD_EP_SIZE, g_msd_ep_in);
                #else
                Flash_ReadBytes(addr, MSD_EP_SIZE, g_msd_ep_in);
                #endif
            }
        }
    }
    #endif
}

void boot_process_write(void)
{
    static uint8_t boot_state = BOOT_DUMMY;
    uint16_t i;
    
    if(boot_state == BOOT_DUMMY)
    {
        // If this is the first block, and it's in the DATA sector.
        if(g_msd_rw_10_vars.LBA == g_msd_rw_10_vars.START_LBA && g_msd_rw_10_vars.LBA >= DATA_SECT_ADDR)
        {
            #if defined(SIMPLE_BOOTLOADER) || !defined(HAS_EEPROM)
            if(g_msd_byte_of_sect == 0 && g_msd_ep_out[0] == ':') // First byte of HEX file is ':'.
            { 
                if(user_firmware) delete_file();
                usb_ram_set(0xFF, m_flash_block, sizeof(m_flash_block));
                boot_state = BOOT_LOAD_HEX;
            }
            #else
            if(g_msd_rw_10_vars.LBA == EEPROM_SECT_ADDR && g_msd_byte_of_sect < EEPROM_SIZE)
            {
                for(i = 0; i < MSD_EP_SIZE; i++) EEPROM_Write((uint8_t)(g_msd_byte_of_sect + i), g_msd_ep_out[i]);
            }
            else if(g_msd_byte_of_sect == 0 && g_msd_ep_out[0] == ':') // First byte of HEX file is ':'.
            { 
                if(user_firmware) delete_file();
                usb_ram_set(0xFF, m_flash_block, sizeof(m_flash_block));
                boot_state = BOOT_LOAD_HEX;
            }
            #endif
        }
        #ifndef SIMPLE_BOOTLOADER
        if(g_msd_rw_10_vars.LBA == ROOT_SECT_ADDR && g_msd_byte_of_sect == 64)
        {
            #ifdef HAS_EEPROM
            if(user_firmware && (g_msd_ep_out[32] == 0x00 || g_msd_ep_out[32] == 0xE5))
            #else
            if(user_firmware && (g_msd_ep_out[0] == 0x00 || g_msd_ep_out[0] == 0xE5))
            #endif
            {
                delete_file();  
                g_boot_reset = true;
            }

            #ifdef HAS_EEPROM
            if(g_msd_ep_out[0] == 0x00 || g_msd_ep_out[0] == 0xE5)
            {
                for(i = 0; i < EEPROM_SIZE; i++) EEPROM_Write((uint8_t)i, 0xFF);
                g_boot_reset = true;
            }
            #endif
        }
        #endif
    }
    
    if(boot_state == BOOT_LOAD_HEX && g_msd_rw_10_vars.LBA >= DATA_SECT_ADDR)
    {
        uint8_t hex_result;
        
        for(i = 0; i < MSD_EP_SIZE; i++)
        {
            hex_result = hex_parse(g_msd_ep_out[i]);
            if(hex_result != HEX_PARSING)
            {
                if(hex_result == HEX_FAULT) delete_file();
                boot_state = BOOT_FINISHED;
                g_boot_reset = true;
                break;
            }
        }
    }
}

/* ************************************************************************** */
/* ************************** STATIC FUNCTIONS ****************************** */
/* ************************************************************************** */

static void generate_boot(void)
{
    if(g_msd_byte_of_sect == 0) usb_rom_copy((const uint8_t*)(&BOOT16), g_msd_ep_in, sizeof(BOOT16));
    else if(g_msd_byte_of_sect == 448)
    {
        g_msd_ep_in[62] = 0x55;
        g_msd_ep_in[63] = 0xAA;
    }
}

static void generate_FAT(void)
{
    // The following code assumes FAT16 is used and all files can fit inside
    // the first 512 bytes of FAT.
    
    #ifdef SIMPLE_BOOTLOADER // Simple bootloader only contains reserved FAT entries.
    if(g_msd_byte_of_sect == 0)
    {
        g_msd_ep_in[0] = 0xF8;
        g_msd_ep_in[1] = 0xFF;
        g_msd_ep_in[2] = 0xFF;
        g_msd_ep_in[3] = 0xFF;
    }
    
    #else // Non-simple bootloader contains files such as ABOUT, EEPROM and PROG_MEM. FAT needs to be generated (more compact).
    uint16_t FAT_cluster;
    uint16_t *p_FAT_entry = (uint16_t*)g_msd_ep_in;
    
    #if ((PROG_MEM_CLUST + FILE_CLUSTERS) * 2) <= MSD_EP_SIZE // If FAT fits into MSD_EP_SIZE, use this code, it's more compact.
    if(g_msd_byte_of_sect == 0)
    {
        p_FAT_entry[0] = 0xFFF8;
        p_FAT_entry[1] = 0xFFFF;
        p_FAT_entry[2] = 0xFFFF;
        #ifdef HAS_EEPROM
        p_FAT_entry[3] = 0xFFFF;
        #endif
        
        if(user_firmware)
        {
            for(FAT_cluster = PROG_MEM_CLUST; FAT_cluster < (PROG_MEM_CLUST + FILE_CLUSTERS - 1); FAT_cluster++)
            {
                p_FAT_entry[FAT_cluster] = FAT_cluster + 1;
            }
            p_FAT_entry[FAT_cluster] = 0xFFFF;
        }
    }
    
    #else // FAT is larger than MSD_EP_SIZE
    if(g_msd_byte_of_sect == 0)
    {
        p_FAT_entry[0] = 0xFFF8;
        p_FAT_entry[1] = 0xFFFF;
        p_FAT_entry[2] = 0xFFFF;
        #ifdef HAS_EEPROM
        p_FAT_entry[3] = 0xFFFF;
        #endif
        FAT_cluster = PROG_MEM_CLUST;
    }
    else
    {
        // Convert g_msd_byte_of_sect to FAT_cluster location by dividing by 2.
        FAT_cluster = (g_msd_byte_of_sect >> 1);
    }
    
    // If chip is erased, don't generate FAT entries for PROG_MEM.
    if(!user_firmware) return; 
    
    // Detect past the end of PROG_MEM file, don't generate any more FAT entries.
    if(FAT_cluster > (PROG_MEM_CLUST + FILE_CLUSTERS)) return; 
    
    for(uint16_t i = FAT_cluster % (MSD_EP_SIZE / 2); i < (MSD_EP_SIZE / 2); i++)
    {
        // If last cluster, finish with EOF entry.
        if(FAT_cluster == (PROG_MEM_CLUST + FILE_CLUSTERS - 1))
        {
            p_FAT_entry[i] = 0xFFFF;
            break;
        }
        p_FAT_entry[i] = FAT_cluster + 1;
        FAT_cluster++;
    }
    #endif
    #endif
}

static void generate_root(void)
{
    if(g_msd_byte_of_sect == 0)
    {
        usb_rom_copy(ROOT.VOL, &g_msd_ep_in[0], 11);
        g_msd_ep_in[11] = 0x08;
        #if defined(_PIC14E)
        g_msd_ep_in[9] = get_device();
        #elif !defined(_18F14K50) && !defined(_18F24K50)
        g_msd_ep_in[6] = get_device();
        #endif
        #ifndef SIMPLE_BOOTLOADER
        usb_rom_copy(ROOT.FILE1, &g_msd_ep_in[32], 11);
        g_msd_ep_in[43] = 0x21; // ATTR_READ_ONLY (0x01) | ATTR_ARCHIVE (0x20).
        g_msd_ep_in[58] = 2;
        g_msd_ep_in[60] = (uint8_t)sizeof(aboutFile);
        g_msd_ep_in[61] = (uint8_t)(sizeof(aboutFile) >> 8);
        #endif
    }
    #ifndef SIMPLE_BOOTLOADER
    else if(g_msd_byte_of_sect == 64)
    {
        #ifdef HAS_EEPROM
        usb_rom_copy(ROOT.FILE2, &g_msd_ep_in[0], 11);
        g_msd_ep_in[11] = 0x20; // ATTR_ARCHIVE.
        g_msd_ep_in[26] = 3;
        g_msd_ep_in[28] = (uint8_t)EEPROM_SIZE;
        g_msd_ep_in[29] = (uint8_t)(EEPROM_SIZE >> 8);
        if(user_firmware)
        {
            usb_rom_copy(ROOT.FILE3, &g_msd_ep_in[32], 11);
            g_msd_ep_in[43] = 0x21; // ATTR_READ_ONLY | ATTR_ARCHIVE.
            g_msd_ep_in[58] = (uint8_t)PROG_MEM_CLUST;
            g_msd_ep_in[60] = (uint8_t)FILE_SIZE;
            g_msd_ep_in[61] = (uint8_t)(FILE_SIZE >> 8);
            #if FILE_SIZE >= 0x10000
            g_msd_ep_in[62] = (uint8_t)(FILE_SIZE >> 16);
            #endif
        }
        #else
        if(user_firmware)
        {
            usb_rom_copy(ROOT.FILE2, &g_msd_ep_in[0], 11);
            g_msd_ep_in[11] = 0x21; // ATTR_READ_ONLY (0x01) | ATTR_ARCHIVE (0x20).
            g_msd_ep_in[26] = (uint8_t)PROG_MEM_CLUST;
            g_msd_ep_in[28] = (uint8_t)FILE_SIZE;
            g_msd_ep_in[29] = (uint8_t)(FILE_SIZE >> 8);
            #if FILE_SIZE >= 0x10000
            g_msd_ep_in[30] = (uint8_t)(FILE_SIZE >> 16);
            #endif
        }
        #endif
    }
    #endif
}


static bool update_erase_block(uint24_t address, uint8_t* data, uint8_t cnt)
{
    uint16_t i;
    uint8_t  block_index = address & INDEX_MASK;
    uint24_t flash_addr  = address & FLASH_ADDR_MASK;

    if((flash_addr != m_prev_flash_addr) && (m_prev_block_index != 0)) // If new block
    {
        if(!safely_write_block(m_prev_flash_addr)) return false; // Write remaining data in m_flash_block to flash for previous flash address
        usb_ram_set(0xFF, m_flash_block, sizeof(m_flash_block)); // Fill new block with 0xFF            
    }
    
    for(i = 0; i < cnt; i++) // Write DATA_REC data into m_flash_block 
    {
        m_flash_block[block_index++] = *data;
        data++;
        if(block_index == sizeof(m_flash_block)) break; // Break if m_flash_block is full, it's possible that data still remains
    }

    if(block_index == sizeof(m_flash_block))
    {
        if(!safely_write_block(flash_addr)) return false;        // Write completed m_flash_block to flash
        usb_ram_set(0xFF, m_flash_block, sizeof(m_flash_block)); // Fill new block with 0xFF        
        block_index = 0;                                         // Reset m_flash_block index
        i++;
        if(i < cnt)                                              // If data still remains
        {
            for(i = i; i < cnt; i++)                             // Write remaining data to new m_flash_block
            {
                m_flash_block[block_index++] = *data;
                data++;
            }
        }
        flash_addr += sizeof(m_flash_block);
    }
    m_prev_block_index = block_index;
    m_prev_flash_addr  = flash_addr;
    
    return true;
}

static uint8_t hex_parse(uint8_t chr)
{
    static uint8_t  ret_code, hex_state = HEX_START;
    static uint8_t  rec_len, chksum_calc, rectype, data[16], data_index;
    static uint16_t load_offset, char_cnt = 0, parse_data = 0;
    static uint24_t ULBA_calc = 0;
    static bool     is_hex_data, new_state = false;
    
    is_hex_data = hex_char_to_char(&chr);

    // If this is not the start of the HEX record, and chr is not 0-9 or A-F in ASCII, this is an error.
    if(hex_state != HEX_START && is_hex_data == false) return HEX_FAULT;
    
    ret_code  = HEX_PARSING;
    parse_data = (parse_data << 4) | chr;
    char_cnt++;
    
    if(hex_state == HEX_START)
    {   // Wait for start of file ':'
        if(chr == '\r' || chr == '\n') {}
        else if(chr == ':')
        {
            hex_state = HEX_REC_LEN;
            new_state = true;
        }
        else ret_code = HEX_FAULT;
    }
    else if(hex_state == HEX_REC_LEN && char_cnt == 2)
    {   // Get Record length
        rec_len = (uint8_t)parse_data;
        if(rec_len <= 0x10) // Support only up to 16 byte rows.
        {
            chksum_calc = rec_len;
            hex_state   = HEX_LOAD_OFFSET;
            new_state   = true;
        }
        else ret_code = HEX_FAULT;
    }
    else if(hex_state == HEX_LOAD_OFFSET && char_cnt == 4)
    {   // Get Load Offset (offset address from current base address)
        load_offset  = parse_data;
        chksum_calc += (uint8_t)load_offset;
        chksum_calc += (uint8_t)(load_offset >> 8);
        hex_state    = HEX_RECTYPE;
        new_state    = true;
    }
    else if(hex_state == HEX_RECTYPE && char_cnt == 2)
    {   // Get Record Type
        rectype      = (uint8_t)parse_data;
        chksum_calc += rectype;
        if(rectype == DATA_REC) // Data Record
        {
            data_index = 0;
            hex_state  = HEX_DATA;
        }
        else if(rectype == EOF_REC) hex_state = HEX_CHKSUM; // End of File Record
        else if(rectype == ELA_REC) hex_state = HEX_ELA;    // Extended Linear Address Record
        else ret_code  = HEX_FAULT;
        new_state = true;
    }
    else if(hex_state == HEX_DATA && char_cnt == 2)
    {
        data[data_index] = (uint8_t)parse_data;
        chksum_calc += data[data_index++];
        if(data_index >= rec_len) hex_state = HEX_CHKSUM;
        new_state = true;
    }
    else if(hex_state == HEX_ELA && char_cnt == 4)
    {
        chksum_calc += (uint8_t)parse_data;
        chksum_calc += (uint8_t)(parse_data >> 8);
        ULBA_calc = ((uint24_t)parse_data) << 16;
        hex_state = HEX_CHKSUM;
        new_state = true;
    }
    else if(hex_state == HEX_CHKSUM && char_cnt == 2)
    {
        chksum_calc += (uint8_t)parse_data;
        if(chksum_calc) ret_code = HEX_FAULT;
        else
        {
            if(rectype == DATA_REC) // Data Record
            {
                if(!update_erase_block(ULBA_calc + (uint24_t)load_offset, data, rec_len)) ret_code = HEX_FAULT;
            }
            else if(rectype == EOF_REC) // End of File Record
            {
                ret_code = HEX_FINISHED;
                if(m_prev_block_index != 0) // Data left in m_flash_block not yet written.
                {
                    if(!safely_write_block(m_prev_flash_addr)) ret_code = HEX_FAULT;
                }
            }
            hex_state = HEX_START;
            new_state = true;
        }
    }
    
    if(new_state)
    {
        char_cnt  = 0;
        parse_data = 0;
        new_state = false;
    }
    
    return ret_code;
}

static bool hex_char_to_char(uint8_t* chr)
{
    if((*chr > '/') && (*chr < ':'))      *chr -= 48; // It's a number
    else if((*chr > '@') && (*chr < 'G')) *chr -= 55; // It's a letter
    else return false; // Not ASCII HEX
    
    return true;
}

static void delete_file(void)
{
#if defined(_PIC14E)
    Flash_Erase(PROG_REGION_START / 2, END_OF_FLASH / 2);
#elif defined(__J_PART)
    Flash_Erase(PROG_REGION_START, CONFIG_PAGE_START);
#else
    Flash_Erase(PROG_REGION_START, END_OF_FLASH);
#endif
}

static bool safely_write_block(uint24_t start_addr)
{
#ifdef __J_PART
    if(start_addr < CONFIG_PAGE_START && start_addr >= PROG_REGION_START) Flash_WriteBlock(start_addr, m_flash_block);
    else if(start_addr < END_OF_FLASH){}      
    else return false;
    return true;
#else
    if((start_addr < END_OF_FLASH) && (start_addr >= PROG_REGION_START))
    {
        #ifdef _PIC14E
        Flash_WriteBlock(start_addr / 2, m_flash_block);
        #else
        Flash_WriteBlock(start_addr, m_flash_block);
        #endif
    }
    #ifndef _PIC14E
    else if(start_addr == ID_REGION_START){}
    #endif
    else if(start_addr == CONFIG_REGION_START){}
    #ifdef HAS_EEPROM
    else if((start_addr < END_OF_EEPROM) && (start_addr >= EEPROM_REGION_START))
    {
        start_addr &= 0xFF;
        for(uint8_t i = 0; i < _FLASH_WRITE_SIZE; i++) EEPROM_Write((uint8_t)start_addr + i, m_flash_block[i]);
    }
    #endif
    else if(start_addr < PROG_REGION_START){}
    else return false;
    return true;
#endif
}

#ifndef SIMPLE_BOOTLOADER
static uint32_t LBA_to_flash_addr(uint32_t LBA)
{
    return ((LBA - PROG_MEM_SECT_ADDR) << 9) + PROG_REGION_START + g_msd_byte_of_sect;
}
#endif

static uint8_t get_device(void)
{
    #if defined(_PIC14E)
    PMCON1 = 0xC0;
    PMADR = (DEV_ID_START / 2);
    PMCON1bits.RD = 1;
    __asm("NOP");
    __asm("NOP");
    #else
    EECON1 = 0xC0;
    TBLPTR = DEV_ID_START;
    __asm("TBLRDPOSTINC");
    #endif 
    
    #if defined(_PIC14E)
    switch(PMDATL & 0x03)
    {
        case 0:
            return '4';
        case 1:
            return '5';
        case 3:
            return '9';
        default:
            return 'X';
    }
    
    #elif defined(_18F4550_FAMILY_)
    if(TABLAT & 0x40) return '2';
    else return '4';
    #elif defined(_18F4450_FAMILY_) || defined(_18F25K50) || defined(_18F45K50) 
    if(TABLAT & 0x20) return '2';
    else return '4';
    #elif defined(_18F46J50_FAMILY_)
    if((TABLAT & 0xE0) > 0x40) return '4';
    else return '2';
    #elif defined(_18F26J53) || defined(_18F46J53) || defined(_18F27J53) || defined(_18F47J53)
    if(TABLAT & 0x80) return '4';
    else return '2';
    #endif
}