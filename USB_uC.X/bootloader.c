/**
 * @file bootloader.c
 * @author John Izzard
 * @date 10/03/2023
 * 
 * USB uC - USB MSD Bootloader.
 * Copyright (C) 2017-2023  John Izzard
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
#include <stdbool.h>
#include "usb.h"
#include "bootloader.h"
#include "config.h"
#include "flash.h"
#include "EEPROM.h"

#define INDEX_MASK (((uint24_t)FLASH_WRITE_SIZE) - 1)
#define FLASH_ADDR_MASK ~INDEX_MASK

bool g_boot_reset;

static uint8_t  m_flash_block[FLASH_WRITE_SIZE];
static uint24_t m_prev_flash_addr = PROG_REGION_START;
static uint8_t  m_prev_block_index = 0;

extern bool     user_firmware;

static uint8_t  hex_parse(uint8_t chr);
static bool     update_erase_block(uint24_t address, uint8_t* data, uint8_t cnt);
static void     generate_FAT(void);
static bool     hex_char_to_char(uint8_t* chr);
static uint32_t LBA_to_flash_addr (uint32_t LBA);
static void     delete_file(void);
static bool     safely_write_block(uint24_t start_addr);

static uint8_t get_device(void);

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

void boot_process_read(void)
{
    usb_ram_set(0, g_msd_ep_in, MSD_EP_SIZE); // Blank Regions of memory are read as zero.
    
    if(g_msd_rw_10_vars.LBA == BOOT_SECT_ADDR) // If PC is reading the Boot Sector.
    {
        if(g_msd_byte_of_sect == 0) usb_rom_copy((const uint8_t*)(&BOOT16), g_msd_ep_in, sizeof(BOOT16));
        else if(g_msd_byte_of_sect == 448)
        {
            g_msd_ep_in[62] = 0x55;
            g_msd_ep_in[63] = 0xAA;
        }
    }
    else if(g_msd_rw_10_vars.LBA == FAT_SECT_ADDR) // If PC is reading the first FAT Sector.
    {
        generate_FAT();
    }
    else if(g_msd_rw_10_vars.LBA == ROOT_SECT_ADDR) // If PC is reading the Root Sector.
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
            for(uint8_t i = 0; i < MSD_EP_SIZE; i++) g_msd_ep_in[i] = EEPROM_Read(g_msd_byte_of_sect + i);
        }
        #endif
        else
        {
            // Convert from LBA address space to flash address space.
            uint24_t addr = LBA_to_flash_addr(g_msd_rw_10_vars.LBA);
            if(addr < PROG_REGION_END) // If address is in flash space.
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
    uint8_t  hex_result;
    uint16_t i;
    
    if(boot_state == BOOT_DUMMY)
    {
        // If this is the first block, and it's in the DATA sector.
        if(g_msd_rw_10_vars.LBA == g_msd_rw_10_vars.START_LBA && g_msd_rw_10_vars.LBA >= DATA_SECT_ADDR)
        {
            #if defined(SIMPLE_BOOTLOADER) || !defined(HAS_EEPROM)
            if(g_msd_byte_of_sect == 0)
            { 
                if(g_msd_ep_out[0] == ':') // First byte of HEX file is ':'.
                {
                    if(user_firmware) delete_file();
                    usb_ram_set(0xFF, m_flash_block, sizeof(m_flash_block));
                    boot_state = BOOT_LOAD_HEX;
                }
            }
            #else
            if(g_msd_rw_10_vars.LBA == EEPROM_SECT_ADDR)
            {
                if(g_msd_byte_of_sect < EEPROM_SIZE)
                {
                    for(i = 0; i < MSD_EP_SIZE; i++) EEPROM_Write(g_msd_byte_of_sect + i, g_msd_ep_out[i]);
                }
            }
            else if(g_msd_byte_of_sect == 0)
            { 
                if(g_msd_ep_out[0] == ':') // First byte of HEX file is ':'.
                {
                    if(user_firmware) delete_file();
                    usb_ram_set(0xFF, m_flash_block, sizeof(m_flash_block));
                    boot_state = BOOT_LOAD_HEX;
                }
            }
            #endif
        }
        #ifndef SIMPLE_BOOTLOADER
        if(g_msd_rw_10_vars.LBA == ROOT_SECT_ADDR)
        {
            if(g_msd_byte_of_sect == 64)
            {
                #if defined(HAS_EEPROM)
                if(user_firmware)
                {
                    if(g_msd_ep_out[32] == 0x00 || g_msd_ep_out[32] == 0xE5)
                    {
                        delete_file();  
                        g_boot_reset = true;
                    }
                }
                
                if(g_msd_ep_out[0] == 0x00 || g_msd_ep_out[0] == 0xE5)
                {
                    for(i = 0; i < EEPROM_SIZE; i++) EEPROM_Write(i, 0xFF);
                    g_boot_reset = true;
                }
                #else
                if(user_firmware)
                {
                    if(g_msd_ep_out[0] == 0x00 || g_msd_ep_out[0] == 0xE5)
                    {
                        delete_file();  
                        g_boot_reset = true;
                    }
                }
                #endif
            }
        }
        #endif
    }
    
    if(boot_state == BOOT_LOAD_HEX)
    {
        if(g_msd_rw_10_vars.LBA >= DATA_SECT_ADDR)
        {
            for(i = 0; i < MSD_EP_SIZE; i++)
            {
                hex_result = hex_parse(g_msd_ep_out[i]);
                if(hex_result != HEX_PARSING)
                {
                    if(hex_result == HEX_FAULT) delete_file();
                    else {} // Write goto user instruction
                    boot_state = BOOT_FINISHED;
                    g_boot_reset = true;
                    break;
                }
            }
        }
    } 
}

static uint8_t hex_parse(uint8_t chr)
{
    static uint8_t  hex_state = HEX_START;
    static uint8_t  rec_len, chksum, chksum_calc, rectype, data[16], data_index, data_byte;
    static uint16_t load_offset, ULBA, char_cnt;
    static uint24_t ULBA_calc;
    
    if(hex_state != HEX_START)
    {
        // If this is not the start of the HEX file, and chr is 0-9 or A-F in ASCII, convert to HEX
        if(!hex_char_to_char(&chr))
        {
            hex_state = HEX_START;
            return HEX_FAULT; 
        }
    }
    
    switch(hex_state)
    {
        case HEX_START: // Wait for start of file ':'
            if(chr == '\r' || chr == '\n') break;
            if(chr == ':')
            {
                chksum_calc = 0;                
                char_cnt    = 2; // We are expecting 2 chars next
                rec_len     = 0;
                hex_state   = HEX_REC_LEN;
                return HEX_PARSING;
            }
            hex_state = HEX_START;
            return HEX_FAULT; 
        case HEX_REC_LEN: // Get Record length
            rec_len = (rec_len << 4) | chr;
            char_cnt--;
            if(char_cnt == 0)
            {
                if(rec_len > 16) // Support only up to 16 byte rows.
                {
                    hex_state = HEX_START;
                    return HEX_FAULT; 
                }
                chksum_calc += rec_len;
                char_cnt     = 4;
                load_offset  = 0;
                hex_state    = HEX_LOAD_OFFSET;
            }
            break;
        case HEX_LOAD_OFFSET: // Get Load Offset (offset address from current base address)
            load_offset = (load_offset << 4) | (uint16_t)chr;
            char_cnt--;
            if(char_cnt == 0)
            {
                //if(load_offset % 2){hex_state = HEX_START; return false;} // load_offset must be even.
                
                chksum_calc += load_offset >> 8;
                chksum_calc += load_offset;
                
                char_cnt  = 2;
                rectype   = 0;
                hex_state = HEX_RECTYPE;
            }
            break;
        case HEX_RECTYPE: // Get Record Type
            char_cnt--; // Only interested in the second received byte (lower nibble)
            if(char_cnt == 0)
            {
                rectype = chr;
                chksum_calc += rectype;
                switch(rectype)
                {
                    case DATA_REC: // Data Record
                        char_cnt   = 2;
                        data_index = 0;
                        data_byte  = 0;
                        hex_state  = HEX_DATA;
                        break;
                    case EOF_REC: // End of File Record
                        char_cnt  = 2;
                        chksum    = 0;
                        hex_state = HEX_CHKSUM;
                        break;
                    case ELA_REC: // Extended Linear Address Record
                        char_cnt  = 4;
                        ULBA      = 0;
                        hex_state = HEX_ELA;
                        break;
                    default:
                        hex_state = HEX_START;
                        return HEX_FAULT;
                }
            }
            break;
        case HEX_DATA:
            char_cnt--;
            if(char_cnt == 1){data_byte = chr; data_byte <<= 4;}
            if(char_cnt == 0)
            {
                data_byte       |= chr;
                data[data_index] = data_byte;
                chksum_calc     += data[data_index];
                data_index++;
                char_cnt = 2;
                if(data_index == rec_len)
                {
                    chksum    = 0;
                    hex_state = HEX_CHKSUM;
                }
            }
            break;
        case HEX_ELA:
            ULBA = (ULBA << 4) | (uint16_t)chr;
            char_cnt--;
            if(char_cnt == 0)
            {
                chksum_calc += ULBA;
                ULBA_calc = ((uint24_t)ULBA) << 16;
                char_cnt  = 2;
                chksum    = 0;
                hex_state = HEX_CHKSUM;
            }
            break;
        case HEX_CHKSUM:
            char_cnt--;
            if(char_cnt == 1){chksum = chr; chksum <<= 4;}
            if(char_cnt == 0)
            {
                chksum      |= chr;
                chksum_calc += chksum;
                if(chksum_calc)
                {
                    hex_state = HEX_START;
                    return HEX_FAULT;
                }
                switch(rectype)
                {
                    case DATA_REC: // Data Record
                        if(!update_erase_block(ULBA_calc + (uint24_t)load_offset, data, rec_len))
                        {
                            hex_state = HEX_START;
                            return HEX_FAULT;
                        }
                        break;
                    case EOF_REC: // End of File Record
                        if(m_prev_block_index != 0) // Data left in m_flash_block not yet written.
                        {
                            if(!safely_write_block(m_prev_flash_addr))
                            {
                                hex_state = HEX_START;
                                return HEX_FAULT;
                            }
                        }
                        hex_state = HEX_START;
                        return HEX_FINISHED;
                    case ELA_REC: // Extended Linear Address Record
                        break;
                }
                hex_state = HEX_START;
            }
            break;
    }
    return HEX_PARSING;
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
    Flash_Erase(PROG_REGION_START / 2, PROG_REGION_END / 2);
    Flash_Erase(USER_GOTO / 2, FLASH_END / 2);
#elif (__J_PART)
    Flash_Erase(PROG_REGION_START, CONFIG_PAGE_START);
#else
    Flash_Erase(PROG_REGION_START, END_OF_FLASH);
#endif
}

static bool safely_write_block(uint24_t start_addr)
{
#if defined(_PIC14E)
    if(start_addr == PROG_REGION_START)
    {
        // Write the goto bootloader instruction into m_flash_block
    }
    else if((start_addr < PROG_REGION_END) && (start_addr > PROG_REGION_START)) Flash_WriteBlock(start_addr / 2, m_flash_block);
    else if(start_addr == CONFIG_REGION_START){}
    else return false;
    return true;
#elif defined(__J_PART)
    if(start_addr < CONFIG_PAGE_START && start_addr >= PROG_REGION_START) Flash_WriteBlock(start_addr, m_flash_block);
    else if(start_addr < END_OF_FLASH){}      
    else return false;
    return true;
#else
    uint8_t i;
    if((start_addr < END_OF_FLASH) && (start_addr >= PROG_REGION_START)) Flash_WriteBlock(start_addr, m_flash_block);
    else if(start_addr == ID_REGION_START){}
    else if(start_addr == CONFIG_REGION_START){}
    #ifdef HAS_EEPROM
    else if((start_addr < END_OF_EEPROM) && (start_addr >= EEPROM_REGION_START))
    {
        start_addr &= 0xFF;
        for(i = 0; i < _FLASH_WRITE_SIZE; i++) EEPROM_Write(start_addr + i, m_flash_block[i]);
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