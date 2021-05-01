;*******************************************************************************
;                                                                              *
;    This program is free software: you can redistribute it and/or modify      *
;    it under the terms of the GNU General Public License as published by      *
;    the Free Software Foundation, either version 3 of the License, or         *
;    (at your option) any later version.                                       *
;                                                                              *
;    This program is distributed in the hope that it will be useful,           *
;    but WITHOUT ANY WARRANTY; without even the implied warranty of            *
;    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
;    GNU General Public License for more details.                              *
;                                                                              *
;    You should have received a copy of the GNU General Public License         *
;    along with this program.  If not, see <https://www.gnu.org/licenses/>.    *
;                                                                              *
;*******************************************************************************
;                                                                              *
;    Filename: bootloader.asm                                                  *
;    Date: 29/04/2020                                                          *
;    File Version: 1.00                                                        *
;    Author: John Izzard                                                       *
;    Company: N/A                                                              *
;    Description: The meat of the booaloader code. All the file system         *
;                 emulation, file emulation, bootloader state machine, hex     *
;                 file parsing and reading and writing to flash code is here.  *
;                                                                              *
;*******************************************************************************
;                                                                              *
;    Revision History:                                                         *
;                                                                              *
;*******************************************************************************
    
    include "p16f1454.inc"
    include "config.inc"
    include "usb.inc"
    include "usb_ch9.inc"
    include "usb_msd.inc"
    include "bootloader.inc"
    radix dec
 
    global boot_state
; Boot Variables.
BOOT_VARS udata BOOT_VARS_ADDRESS
boot_state       res 1
load_hex_i       res 1
hex_state        res 1
result           res 1
chr              res 1
chksum_calc      res 1
char_cnt         res 1
rec_len          res 1
load_offset      res 2
rectype          res 1
data_byte        res 1
data_cnt         res 1
chksum           res 1
ULBA             res 1
data_FSR1L       res 1
data_            res 16
reset_row_cnt    res 1
flash_addr       res 2
prev_flash_addr  res 2
row_index        res 1
prev_row_index   res 1
start_addr       res 2
EP1_out_FSR0L    res 1

; Flash writing variables.
FLASH_VARS udata FLASH_VARS_ADDRESS
word_cnt         res 1

; Flash row (buffer) variable.
FLASH_ROW_VARS udata FLASH_ROW_VARS_ADDRESS 
flash_row        res 64
	
; External Variable Symbols.
    extern user_firmware

; External Function Symbols.
    extern msd_byte_of_sect
    extern LBA, START_LBA, cnt, usb_flags
    extern EP1_in, EP1_out
    extern mem_set, mem_copy
    
; Global MSD Function Symbols.
    global msd_tx_sector, msd_rx_sector

bootloaderCode code
;*******************************************************************************
; MSD RX Sector
;*******************************************************************************
; This function is used for emulating storage being read during data transfer
; from a SCSI READ_10 command.
msd_rx_sector:
    ; Clear EP1_in.
    movlw       64
    movwf       cnt
    movlw       high EP1_in
    movwf       FSR0H
    movlw       low EP1_in
    movwf       FSR0L
    clrw        ; WREG holds value to set.
    call        mem_set
    ; Check LBA to be read.
    movfw       LBA+1
    btfss       STATUS,Z
    return ; If MSB's of LBA != 0, out of bounds, just return zeros in EP1_in.
    banksel     0
    ; Might be using FSR1 to access EP1_in, set now to save space.
    movlw       high EP1_in
    movwf       FSR1H
    movlw       low EP1_in
    movwf       FSR1L
    movfw       LBA
    bz          _rx_boot_sect
    xorlw       FAT_SECT_ADDR
    bz          _rx_FAT_sect
    movfw       LBA
    xorlw       ROOT_SECT_ADDR
    bz          _rx_root_sect
    if USE_ABOUT_FILE
    movfw       LBA
    xorlw       ABOUT_SECT_ADDR
    bz          _rx_about_sect
    endif
    if USE_PROG_MEM_READ
    ; LBA >= PROG_MEM_SECT_ADDR.
    movfw       LBA
    sublw       PROG_MEM_SECT_ADDR-1
    bnc         _rx_prog_mem_sect
    endif
    return
_rx_boot_sect:
    ; Check which part of boot sector is being read.
    movfw       msd_byte_of_sect
    iorwf       msd_byte_of_sect+1,W
    bnz         $+13
    ; Addressing the first 64 bytes of boot sector (msd_byte_of_sect == 0).
    ; Copy 62 bytes of the boot sector to EP1_in.
    movlw       low boot16
    movwf       FSR0L
    movlw       high (boot16 + 0x8000)
    movwf       FSR0H
    movlw       24
    movwf	cnt
    call        mem_copy
    movlw       14 ; Skip zeros in boot sector (see boot16_prt2).
    addwf       FSR1L,F
    movlw       24
    movwf	cnt
    goto        mem_copy
    ; Check if addressing the last 64 bytes so as to add 0x55 and 0xAA.
    movfw       msd_byte_of_sect+1
    xorlw       0x01
    btfss       STATUS,Z
    return
    movfw       msd_byte_of_sect
    xorlw       0xC0
    btfss       STATUS,Z
    return
    ; Addressing the last 64 bytes. Add 0x55 and 0xAA to the 510 and 511 bytes
    ; of the boot sector.
    banksel     EP1_in
    movlw       0x55
    movwf       EP1_in+62
    movlw       0xAA
    movwf       EP1_in+63
    return
_rx_FAT_sect:
    ; Check which part of FAT sector is being read.
    movfw       msd_byte_of_sect
    iorwf       msd_byte_of_sect+1,W
    btfss       STATUS,Z
    return
    ; Addressing the first 64 bytes of FAT sector (msd_byte_of_sect == 0).
    ; Generate the FAT.
    banksel     EP1_in
    movlw       0xF8
    movwf       EP1_in
    movlw       0xFF
    movwf       EP1_in+1
    movwf       EP1_in+2
    movwf       EP1_in+3
    if USE_ABOUT_FILE == 1
    ; ABOUT File.
    movwf       EP1_in+4
    movwf       EP1_in+5
    endif
    if USE_PROG_MEM_FILE == 1
    movlw       PROG_MEM_CLUST+1
    movwf       FSR0L ; Stores cluster number.
    banksel     0
    btfss       user_firmware,0
    return
    ; PROG_MEM File.
    if USE_ABOUT_FILE == 1
    movlw       low (EP1_in+6)
    else
    movlw       low (EP1_in+4)
    endif
    movwf       FSR1L
    movlw       FILE_CLUSTERS-1
    movwf       cnt
    ; Fill FAT loop.
    movfw       FSR0L
    movwi       FSR1++
    incf        FSR1L,F
    incf        FSR0L,F
    decfsz      cnt,F
    goto        $-5
    movlw       0xFF
    movwi       FSR1++
    movwi       FSR1++
    endif
    return
_rx_root_sect:
    if USE_ABOUT_FILE && USE_PROG_MEM_FILE
    ; if MSB of msd_byte_of_sect != 0, nothing exists here, return all 0s.
    movfw       msd_byte_of_sect+1
    btfss       STATUS,Z
    return
    ; Check if reading first 64 bytes if root sect.
    movfw       msd_byte_of_sect
    bnz         _rx_root_sect_user ; Possible trying to read byte 64 to 128.
    else
    ; Check if reading first 64 bytes if root sect.
    movfw       msd_byte_of_sect
    iorwf       msd_byte_of_sect+1,W
    btfss       STATUS,Z
    return
    endif
    ; Addressing the first 64 bytes of root sector (msd_byte_of_sect == 0).
    ; Copy "PIC16" from boot sector to EP1_in. All other information for the 
    ; volume directory entry can be 0 except for DIR_Attr which is
    ; ATTR_VOLUME_ID (0x08).
    banksel     EP1_in
    movlw       low _volumeDIR
    movwf       FSR0L
    movlw       high (_volumeDIR + 0x8000)
    movwf       FSR0H
    movlw       10
    movwf	cnt
    call        mem_copy
    bsf         EP1_in+11,3 ; ATTR_VOLUME_ID (0x08).
    ; Add the ABOUT file directory entry if enabled in settings.
    if USE_ABOUT_FILE == 1
    movlw       low aboutDIR
    movwf       FSR0L
    movlw       high (aboutDIR + 0x8000)
    movwf       FSR0H
    movlw       low (EP1_in+32)
    movwf       FSR1L
    movlw       11
    movwf	cnt
    call        mem_copy
    movlw       0x21        ; ATTR_READ_ONLY (0x01) | ATTR_ARCHIVE (0x20).
    movwf       EP1_in+43
    bsf         EP1_in+58,1 ; DIR_FstClusLO = 2.
    movlw       ABOUT_FILE_SIZE
    movwf       EP1_in+60   ; DIR_FileSize = ABOUT_FILE_SIZE.
    return
    endif
    if USE_PROG_MEM_FILE == 1
_rx_root_sect_user:
    ; Check if user firmware is present.
    banksel     0
    btfss       user_firmware,0
    return
    ; User firmware is present.
    movfw       msd_byte_of_sect
    ; If about file is enabled in settings, PROG_MEM files directory entry is
    ; located at the 64-128 byte range, otherwise its 0-63 byte range.
    if USE_ABOUT_FILE == 1
    xorlw       64
    endif
    btfss       STATUS,Z
    return
    ; Populate the PROG_MEM entry.
    banksel     EP1_in
    if USE_ABOUT_FILE == 1
    movlw       low EP1_in
    movwf       FSR1L
    movlw       0x21           ; ATTR_READ_ONLY (0x01) | ATTR_ARCHIVE (0x20).
    movwf       EP1_in+11
    movlw       PROG_MEM_CLUST ; DIR_FstClusLO = PROG_MEM_CLUST.
    movwf       EP1_in+26
    movlw       low FILE_SIZE  ; DIR_FileSize.
    movwf       EP1_in+28
    movlw       high FILE_SIZE ; DIR_FileSize.
    movwf       EP1_in+29
    else
    movlw       low (EP1_in+32)
    movwf       FSR1L
    movlw       0x21           ; ATTR_READ_ONLY (0x01) | ATTR_ARCHIVE (0x20).
    movwf       EP1_in+43
    movlw       PROG_MEM_CLUST ; DIR_FstClusLO = PROG_MEM_CLUST.
    movwf       EP1_in+58
    movlw       low FILE_SIZE  ; DIR_FileSize.
    movwf       EP1_in+60
    movlw       high FILE_SIZE ; DIR_FileSize.
    movwf       EP1_in+61
    endif
    movlw       low prog_memDIR
    movwf       FSR0L
    movlw       high (prog_memDIR + 0x8000)
    movwf       FSR0H
    movlw       11
    movwf	cnt
    goto        mem_copy
    endif
    if USE_ABOUT_FILE
_rx_about_sect:
    if ABOUT_FILE_SIZE > 64
    ; if upper byte of msd_byte_of_sect != 0, return all 0s.
    movfw       msd_byte_of_sect+1
    btfss       STATUS,Z
    return
    ; Check if msd_byte_of_sect == 0.
    movfw       msd_byte_of_sect
    btfss       STATUS,Z
    goto        $+7
    ; msd_byte_of_sect == 0.
    movlw       low about_file
    movwf       FSR0L
    movlw       high (about_file + 0x8000)
    movwf       FSR0H
    movlw       64
    goto        $+9
    ; Check if msd_byte_of_sect == 64.
    xorlw       64
    btfss       STATUS,Z
    return ; Gone past the file, return all 0s.
    ; msd_byte_of_sect == 64, send remaining.
    movlw       low about_file_prt2
    movwf       FSR0L
    movlw       high (about_file_prt2 + 0x8000)
    movwf       FSR0H
    movlw       ABOUT_FILE_SIZE-64
    movwf	cnt
    goto        mem_copy
    else
    ; ABOUT_FILE_SIZE <= 64
    ; Check if msd_byte_of_sect == 0.
    movfw       msd_byte_of_sect+1
    iorwf       msd_byte_of_sect,W
    btfss       STATUS,Z
    return
    ; msd_byte_of_sect == 0.
    movlw       low about_file
    movwf       FSR0L
    movlw       high (about_file + 0x8000)
    movwf       FSR0H
    movlw       ABOUT_FILE_SIZE
    movwf	cnt
    goto        mem_copy
    endif
    endif
    if USE_PROG_MEM_READ
_rx_prog_mem_sect:
    ; Convert LBA to flash address. Don't care if LBA is too large and overflows
    ; the flash address. File will only show it's DIR_FileSize worth of data.
    ; (((LBA - PROG_MEM_SECT_ADDR) * 512) + (USER_PROGRAM<<1) + msd_byte_of_sect)>>1
    ;((LBA - PROG_MEM_SECT_ADDR) * 512).
    movlw       PROG_MEM_SECT_ADDR
    subwf       LBA,W
    banksel     PMADR
    movwf       PMADRH
    lslf        PMADRH,F
    ; + USER_PROGRAM<<1.
    movlw       high (USER_PROGRAM<<1)
    addwf       PMADRH,F
    movlw       low (USER_PROGRAM<<1)
    movwf       PMADRL
    ; + msd_byte_of_sect.
    banksel     0
    movfw       msd_byte_of_sect
    banksel     PMADR
    addwf       PMADRL,F
    btfsc       STATUS,C
    incf        PMADRH,F
    banksel     0
    movfw       msd_byte_of_sect+1
    banksel     PMADR
    addwf       PMADRH,F
    ; >> 1.
    lsrf        PMADRH,F
    rrf         PMADRL,F
    ; Read Flash.
    movlw       0x80
    movwf       PMCON1
    movlw       32
    movwf       cnt
    ; Read Flash Loop.
    bsf         PMCON1,RD
    nop
    nop
    movfw       PMDATL
    movwi       FSR1++
    movfw       PMDATH
    movwi       FSR1++
    incf        PMADRL,F
    btfsc       STATUS,Z
    incf        PMADRH,F
    decfsz      cnt,F
    goto        $-11
    return
    endif

;*******************************************************************************
; MSD TX Sector
;*******************************************************************************
; This function is used for detecting when the host is trying to write a hex
; file to the drive, it will then parse the hex file and write the user firmware
; to flash. Also used to detect when the user is deleting the PROM_MEM.BIN file.
msd_tx_sector:
    ; Check boot_state == BOOT_DUMMY, waiting for condition to start load hex.
    banksel     boot_state
    btfss       boot_state,BOOT_DUMMY
    goto        _check_load_hex
    ; boot_state == BOOT_DUMMY.
    ; If MSB of LBA != 0, out of bounds.
    movfw       LBA+1
    btfss       STATUS,Z
    return
    ; If the USE_PROG_MEM_DELETE feature is enabled.
    if USE_PROG_MEM_FILE && USE_PROG_MEM_DELETE
    movfw       LBA
    xorlw       ROOT_SECT_ADDR
    btfss       STATUS,Z
    goto        _tx_data_sect
_tx_root_sect:
    banksel     0
    ; If MSB of msd_byte_of_sect != 0, out of bounds.
    movfw       msd_byte_of_sect+1
    btfss       STATUS,Z
    return
    movfw       msd_byte_of_sect
    if USE_ABOUT_FILE
    xorlw       64 ; When using ABOUT file, PROG_MEM directory entry starts at
    endif          ; byte 64, otherwise it's at byte 32.
    btfss       STATUS,Z
    return
    ; The PROG_MEM directory entry only exists if the user firmware is present.
    banksel     0
    btfss       user_firmware,0
    return
    banksel     EP1_out
    ; Check first byte of new PROG_MEM directory entry, if 0x00 or 0xE5,
    ; the OS (user) is trying to delete it. Erase the user flash and reset.
    if USE_ABOUT_FILE == 1
    movfw       EP1_out
    else
    movfw       EP1_out+32
    endif
    btfsc       STATUS,Z
    goto        $+4
    xorlw       0xE5
    btfss       STATUS,Z
    return
    call        delete_file
    bsf         usb_flags,NEED_RESET
    return
    endif
_tx_data_sect:
    ; Check if this is the first sector, and it's in the DATA sector.
    ; if(LBA == START_LBA && LBA >= DATA_SECT_ADDR).
    movlw       DATA_SECT_ADDR
    subwf       LBA,W
    btfss       STATUS,C
    return ; LBA < DATA_SECT_ADDR.
    movfw       LBA
    xorwf       START_LBA,W
    btfss       STATUS,Z
    return
    ; This is the first sector, and it's in the DATA sector.
    ; Check if writing to the first 64 bytes of sector, (msd_byte_of_sect == 0).
    banksel     0
    movfw       msd_byte_of_sect
    iorwf       msd_byte_of_sect+1,W
    btfss       STATUS,Z
    return
    ; Check if the first byte is the hex start code ':' (EP1_out[0] == ':').
    banksel     EP1_out
    movfw       EP1_out
    xorlw       ':'
    btfss       STATUS,Z
    return
    ; Found ':'. erase user program memory.
    call        delete_file
    ; Prep for parsing the hex file and programming.
    ; prev_flash_addr = PROG_REGION_START.
    banksel     boot_state
    movlw       high (USER_PROGRAM<<1)
    movwf       prev_flash_addr+1
    movlw       low (USER_PROGRAM<<1)
    movwf       prev_flash_addr
    ; prev_row_index = 0.
    clrf        prev_row_index
    ; Set flash row to all 0xFF.
    call        reset_row
    ; hex_state = HEX_START.
    movlw       (1<<HEX_START)
    movwf       hex_state
    ; boot_state = BOOT_LOAD_HEX.
    movlw       (1<<BOOT_LOAD_HEX)
    movwf       boot_state
_check_load_hex:
    ; Check if boot_state == BOOT_LOAD_HEX.
    btfss       boot_state,BOOT_LOAD_HEX
    return
    ; Just in case, make sure this 64 bytes of data belongs in the DATA sector 
    ; (LBA >= DATA_SECT_ADDR).
    movfw       LBA+1
    btfss       STATUS,Z
    return
    movlw       DATA_SECT_ADDR
    subwf       LBA,W
    btfss       STATUS,C
    return
    ; Prep for transfer of characters from EP1_out.
    movlw       low EP1_out
    movwf       EP1_out_FSR0L
    clrf        load_hex_i
    clrf        result
_load_hex:
    ; Need to parse all 64 characters.
    btfsc       load_hex_i,6 ; Test for 64th iteration.
    return
    movlw       high EP1_out
    movwf       FSR0H
    movfw       EP1_out_FSR0L
    movwf       FSR0L
    ; Get character.
    movfw       INDF0
    movwf       chr
    incf        EP1_out_FSR0L,F
    ; Parse hex file.
    call        hex_parse
    ; Test if finished parsing hex file.
    btfss       result,0
    goto        $+3
    incf        load_hex_i,F
    goto        _load_hex
    ; Finished parsing hex file.
    btfss       result,1    ; Result bit 1 is set if successful parse.
    call        delete_file ; Fault occured, delete user space.
    movlw       (1<<BOOT_FINISHED)
    movwf       boot_state
    bsf         usb_flags,NEED_RESET
    return
    
;*******************************************************************************
; Hex Parse
;*******************************************************************************
; This function forms a state machine that will parse the hex file one character
; at a time.
hex_parse:
    btfsc       hex_state,HEX_START
    goto        _HEX_START
    ; hex_state != HEX_START.
_hex_char_2_num:
    ; Is it a number?
    movfw       chr
    sublw       '/'
    bc          $+7
    movlw       ':'
    subwf       chr,W
    bc          $+3
    ; It's a number.
    movlw       48
    goto        _hex_char_2_num_ok
    ; Is it a uppercase hex letter?
    movfw       chr
    sublw       '@'
    ;bc          _hex_finished
    btfsc       STATUS,C
    goto        _hex_finished
    movlw       'G'
    subwf       chr,W
    ;bc          _hex_finished
    btfsc       STATUS,C
    goto        _hex_finished
    ; It's a letter.
    movlw       55
_hex_char_2_num_ok:
    subwf       chr,F
_check_hex_state:
    ; Check the hex state machine's state.
    btfsc       hex_state,HEX_REC_LEN
    goto        _HEX_REC_LEN
    btfsc       hex_state,HEX_LOAD_OFFSET
    goto        _HEX_LOAD_OFFSET
    btfsc       hex_state,HEX_RECTYPE
    goto        _HEX_RECTYPE
    btfsc       hex_state,HEX_DATA
    goto        _HEX_DATA
    btfsc       hex_state,HEX_ELA
    goto        _HEX_ELA
    btfsc       hex_state,HEX_CHKSUM
    goto        _HEX_CHKSUM
    goto        _hex_finished ; Should never happen.
_HEX_START:
    ; Ignore "carrage return" or "new line"
    movfw       chr
    xorlw       '\r'
    bz          _hex_still_parsing
    movfw       chr
    xorlw       '\n'
    bz          _hex_still_parsing
    ; Expecting chr == ':', otherwise this is a fault.
    movfw       chr
    xorlw       ':'
    ;bnz         _hex_finished
    btfss       STATUS,Z
    goto        _hex_finished
    ; After the ':' (start code) is the "record length", prep for this.
    clrf        chksum_calc ; Clear the checksum summing variable.
    movlw       2           ; rec_len is derived from two characters.
    movwf       char_cnt
    clrf        rec_len     ; Clear rec_len in preparation.
    ; hex_state = HEX_REC_LEN.
    movlw       (1<<HEX_REC_LEN)
    movwf       hex_state
    goto        _hex_still_parsing
_HEX_REC_LEN:
    ; rec_len = (rec_len << 4) | chr
    swapf       rec_len,F
    movfw       chr
    iorwf       rec_len,F
    ; if(--char_cnt) goto _hex_still_parsing
    decfsz      char_cnt,F
    goto        _hex_still_parsing
    ; Received rec_len, check that rec_len <= 16. (only up to 16 data supported)
    sublw       16
    ;bnc         _hex_finished
    btfss       STATUS,C
    goto        _hex_finished
    ; Copy rec_len to data_cnt.
    movfw       rec_len
    movwf       data_cnt
    ; Add rec_len to chksum_calc.
    addwf       chksum_calc,F
    ; After "record length" is the "load offset" field, prep for this.
    movlw       4
    movwf       char_cnt
    clrf        load_offset+1
    clrf        load_offset
    ; hex_state = HEX_LOAD_OFFSET.
    movlw       (1<<HEX_LOAD_OFFSET)
    movwf       hex_state
    goto        _hex_still_parsing
_HEX_LOAD_OFFSET:
    ; Check which character.
    movfw       char_cnt
    sublw       2
    bc          $+6
    ; char_cnt > 2, load_offset[1] = (load_offset[1] << 4) | chr.
    swapf       load_offset+1,F
    movfw       chr
    iorwf       load_offset+1,F
    decf        char_cnt,F
    goto        _hex_still_parsing
    ; char_cnt <= 2, load_offset[0] = (load_offset[0] << 4) | chr.
    swapf       load_offset,F
    movfw       chr
    iorwf       load_offset,F
    decfsz      char_cnt,F
    goto        _hex_still_parsing
    ; Received load_offset, add load_offset to chksum_calc.
    movfw       load_offset+1
    addwf       chksum_calc,F
    movfw       load_offset
    addwf       chksum_calc,F
    ; After "load offset" is the "record type" field, prep for this.
    movlw       2
    movwf       char_cnt
    clrf        rectype
    ; hex_state = HEX_RECTYPE.
    movlw       (1<<HEX_RECTYPE)
    movwf       hex_state
    goto        _hex_still_parsing
_HEX_RECTYPE:
    ; if(--char_cnt) goto _hex_still_parsing
    decfsz      char_cnt,F
    goto        _hex_still_parsing
    ; Received rectype (note that ignored the upper nibble of rectype).
    movfw       chr
    movwf       rectype
    ; Add rectype to chksum_calc.
    addwf       chksum_calc,F
    movfw       rectype
    bnz         $+9
    ; rectype == DATA_REC, record is a "data" record, prep for this.
    movlw       2
    movwf       char_cnt
    clrf        data_byte
    movlw       low  data_
    movwf       data_FSR1L
    movlw       (1<<HEX_DATA) ; hex_state = HEX_DATA.
    movwf       hex_state
    goto        _hex_still_parsing
    ; Check EOF_REC
    xorlw       EOF_REC
    bnz         $+7
    ; rectype == EOF_REC, record is a "End Of File" record,
    ; the next field is the "Checksum", prep for this.
    movlw       2
    movwf       char_cnt
    clrf        chksum
    movlw       (1<<HEX_CHKSUM) ; hex_state = HEX_CHKSUM.
    movwf       hex_state
    goto        _hex_still_parsing
    ; Check ELA_REC
    movfw       chr
    xorlw       ELA_REC
    ;bnz         _hex_finished
    btfss       STATUS,Z
    goto        _hex_finished
    ; rectype == ELA_REC, record is a "Extended Linear Address" record,
    ; the next field is the "Extended Linear Address" value, prep for this.
    movlw       4
    movwf       char_cnt
    clrf        ULBA
    movlw       (1<<HEX_ELA) ; hex_state = HEX_ELA.
    movwf       hex_state
    goto        _hex_still_parsing
_HEX_DATA:
    ; Get two data characters.
    decfsz      char_cnt,F
    goto        $+18
    ; Received 2 data characters, store them in data_, prep for next two.
    clrf        FSR1H
    movfw       data_FSR1L
    movwf       FSR1L
    movfw       chr
    iorwf       data_byte,W
    movwf       INDF1
    addwf       chksum_calc,F
    incf        data_FSR1L,F
    clrf        data_byte
    movlw       2
    movwf       char_cnt
    ; if(--data_cnt) goto _hex_still_parsing.
    decfsz      data_cnt,F
    goto        _hex_still_parsing
    ; Reveived all of the "data" record, next is the "Checksum" field, prep.
    clrf        chksum
    movlw       (1<<HEX_CHKSUM) ; hex_state = HEX_CHKSUM.
    movwf       hex_state
    goto        _hex_still_parsing
    ; Received 1 of 2 characters. data_byte = (char << 4).
    swapf       chr,W
    movwf       data_byte
    goto        _hex_still_parsing
_HEX_ELA:
    ; Check which character.
    movfw       char_cnt
    sublw       1
    bc          $+6
    ; char_cnt > 1, if any upper 3 nibbles of ULBA != 0, this is out of bounds.
    movfw       chr
    ;bnz         _hex_finished
    btfss       STATUS,Z
    goto        _hex_finished
    decf        char_cnt,F
    goto        _hex_still_parsing
    ; char_cnt <= 1
    movfw       chr
    movwf       ULBA
    ; Received ULBA, if ULBA > 1, this is out of bounds.
    andlw       0xFE
    ;bnz         _hex_finished
    btfss       STATUS,Z
    goto        _hex_finished
    ; Add ULBA to chksum_calc.
    movfw       ULBA
    addwf       chksum_calc,F
    ; Next is the "Checksum" field, prep for that.
    movlw       2
    movwf       char_cnt
    clrf        chksum
    movlw       (1<<HEX_CHKSUM) ; hex_state = HEX_CHKSUM.
    movwf       hex_state
    goto        _hex_still_parsing
_HEX_CHKSUM:
    ; Get chksum.
    swapf       chksum,F
    movfw       chr
    iorwf       chksum,F
    decfsz      char_cnt,F
    goto        _hex_still_parsing
    ; Received chksum, add to chksum_calc.
    movfw       chksum
    addwf       chksum_calc,F
    ;bnz         _hex_finished ; Checksum is correct if the result is 0.
    btfss       STATUS,Z
    goto        _hex_finished
    ; Checksum was correct, check the "record type".
    movfw       rectype
    bnz         $+5
    ; rectype == DATA_REC, if ULBA == 0, this is in general flash area.
    movfw       ULBA
    bz          _update_row
    goto        _HEX_CHKSUM_exit
    ; Check EOF_REC.
    btfss       rectype,0
    goto        _HEX_CHKSUM_exit ; rectype == ELA_REC.
    ; rectype == EOF_REC.
    movfw       prev_row_index
    bz          $+7 
    ; prev_row_index != 0, must be data left to be written in flash_row.
    ; start_addr = prev_flash_addr.
    movfw       prev_flash_addr+1
    movwf       start_addr+1
    movfw       prev_flash_addr
    movwf       start_addr
    call        safely_write_row
    btfsc       result,0
    bsf         result,1   ; Marks success (delete not required).
    goto        _hex_finished
_HEX_CHKSUM_exit:
    movlw       (1<<HEX_START)
    movwf       hex_state
    bsf         result,0
    return
_hex_still_parsing:
    bsf         result,0
    return
_hex_finished:
    movlw       (1<<HEX_START)
    movwf       hex_state
    bcf         result,0
    return
; This part of the code will run when finnished parsing a data record.
; It will load the flash_row buffer with the data and will write it to flash
; when the buffer is full or when needing to store data for a different row.
_update_row:
    ; row_index = load_offset & 0x00001F.
    movfw       load_offset
    andlw       0x3F
    movwf       row_index
    ; flash_addr = load_offset & 0xFFE0.
    movfw       load_offset+1
    movwf       flash_addr+1
    movfw       load_offset
    andlw       0xC0
    movwf       flash_addr
_check_new_row:
    ; if(flash_addr != prev_flash_addr && prev_row_index != 0).
    movfw       flash_addr+1
    subwf       prev_flash_addr+1,W
    bnz         $+5 
    movfw       flash_addr
    subwf       prev_flash_addr,W
    bz          _add_2_row
    ; flash_addr != prev_flash_addr.
    movfw       prev_row_index
    bz          _add_2_row
    ; prev_row_index != 0.
    ; start_addr = prev_flash_addr.
    movfw       prev_flash_addr+1
    movwf       start_addr+1
    movfw       prev_flash_addr
    movwf       start_addr
    ; Write row to flash.
    call        safely_write_row
    btfss       result,0
    goto        _hex_finished
    ; Reset flash_row to all 0xFF
    call        reset_row
_add_2_row:
    ; Prep to move data_ to row.
    ; FSR0 = data_.
    clrf        FSR0H ; data_ is < 0x100.
    movlw       low data_
    movwf       FSR0L
    ; FSR1 = flash_row + row_index.
    movlw       high flash_row
    movwf       FSR1H
    movlw       low flash_row
    addwf       row_index,W
    movwf       FSR1L
_add_2_row_loop:
    ; while(rec_len)
    movfw       rec_len
    bz          _update_row_exit ; row_index < 32 and all last of data.
    moviw       FSR0++
    movwi       FSR1++
    incf        row_index,F
    btfsc       row_index,6 ; if(row_index == 64) break.
    goto        _row_finished
    decf        rec_len,F
    goto        _add_2_row_loop
_row_finished:
    ; start_addr = flash_addr.
    movfw       flash_addr+1
    movwf       start_addr+1
    movfw       flash_addr
    movwf       start_addr
    ; row_index == 64, write row to flash.
    call        safely_write_row
    btfss       result,0
    goto        _hex_finished
    ; Reset flash_row to all 0xFF.
    call        reset_row
    ; row_index = 0.
    clrf        row_index
    ; flash_addr += 64.
    movlw       64
    addwf       flash_addr,F
    btfsc       STATUS,C
    incf        flash_addr+1,F
    ; rec_len != 0, more to add.
    decfsz      rec_len,F
    goto        $+2
    goto        _update_row_exit
    ; More left in data_, reset FSR1 to start of flash_row.
;    movlw       high flash_row
;    movwf       FSR1H
    movlw       low flash_row
    movwf       FSR1L
    goto        _add_2_row_loop
_update_row_exit:
    ; prev_row_index = row_index
    movfw       row_index+1
    movwf       prev_row_index+1
    movfw       row_index
    movwf       prev_row_index
    ; prev_flash_addr = flash_adr
    movfw       flash_addr+1
    movwf       prev_flash_addr+1
    movfw       flash_addr
    movwf       prev_flash_addr
    goto        _HEX_CHKSUM_exit
    
;*******************************************************************************
; Safely Write Row
;*******************************************************************************
; Will make sure the row to be written is in the user area, and only then write 
; it to flash.
safely_write_row:
    ; if((start_addr < END_OF_FLASH) && (start_addr >= USER_PROGRAM))
    movlw       high (END_OF_FLASH<<1)
    subwf       start_addr+1,W
    bc          _safely_write_row_fault ; start_addr >= END_OF_FLASH
    movlw       high (USER_PROGRAM<<1)
    subwf       start_addr+1,W
    bnc         _safely_write_row_fault ; start_addr < USER_PROGRAM
_write_row:
    ; PMCON1 = 0xA4 (Unimplemented = 1, CFGS = 0, LWLO = 1, FREE = 0, WREN = 1).
    banksel     PMCON1
    movlw       0xA4 ; 
    movwf       PMCON1
    ; PMADR = start_addr
    banksel     start_addr
    lsrf        start_addr+1,W
    banksel     PMCON1
    movwf       PMADRH
    banksel     start_addr
    rrf         start_addr,W
    banksel     PMCON1
    movwf       PMADRL
    ; FSR0 = flash_row
    movlw       high flash_row
    movwf       FSR1H
    movlw       low flash_row
    movwf       FSR1L
    ; FSR1L = 32
    movlw       32
    movwf       word_cnt
_write_row_loop:
    ; PMDATL = flash_row[i++]
    moviw       FSR1++
    movwf       PMDATL
    ; PMDATH = flash_row[i++]
    moviw       FSR1++
    movwf       PMDATH
    ; if(--word_cnt == 0) PMCON1.LWLO = 0
    decfsz      word_cnt,F
    goto        $+2
    bcf         PMCON1,LWLO
    ; Unlock
    movlw       0x55
    movwf       PMCON2
    movlw       0xAA
    movwf       PMCON2
    ; PMCON1.WR = 1
    bsf         PMCON1,WR
    nop
    nop
    ; PMADR++
    incf        PMADRL,F
    bnz         $+2
    incf        PMADRH,F
    ; if(word_cnt) goto _write_row_loop
    movfw       word_cnt
    bnz         _write_row_loop
    ; PMCON1.WREN = 0
    bcf         PMCON1,WREN
_safely_write_row_exit:
    banksel     boot_state
    bsf         result,0
    return
_safely_write_row_fault:
    bcf         result,0
    return
    
;*******************************************************************************
; Reset Row
;*******************************************************************************
; Clears the flash_row buffer to all 0xFF in preparation to be filled with data.
; You only want to program areas that need to be programmed, anything other than
; 0xFF will be programmed.
reset_row:
    movlw       64
    movwf       reset_row_cnt
    movlw       high flash_row
    movwf       FSR1H
    movlw       low flash_row
    movwf       FSR1L
    movlw       0xFF
    movwi       FSR1++
    decfsz      reset_row_cnt,F
    goto        $-2
    return
    
; Erases the entire user area of flash.
delete_file:
    ; PMCON1 = 0x84
    banksel     PMCON1
    movlw       0x84
    movwf       PMCON1
    ; PMADR = USER_PROGRAM
    movlw       low USER_PROGRAM
    movwf       PMADRL
    movlw       high USER_PROGRAM
    movwf       PMADRH
_delete_file_loop:
    ; if(PMADR == END_OF_FLASH) goto _delete_file_exit
    btfsc       PMADRH,5 ; PMADRH == high END_OF_FLASH (0x20)
    goto        _delete_file_exit
    ; PMCON1.FREE = 1
    bsf         PMCON1,FREE
    ; Unlock
    movlw       0x55
    movwf       PMCON2
    movlw       0xAA
    movwf       PMCON2
    ; PMCON1 = WR
    bsf         PMCON1,WR
    nop
    nop
    movlw       32
    addwf       PMADRL,F
    btfsc       STATUS,Z
    incf        PMADRH,F
    goto        _delete_file_loop
_delete_file_exit:
    ; PMCON1.WREN = 0
    bcf         PMCON1,WREN
    banksel     boot_state
    return
    
;*******************************************************************************
; BOOT16 Sector
;*******************************************************************************
; Boot sector constant values.
boot16:
    dt 0xEB ,0x3C, 0x90
    dt "MSDOS5.0"
    dt low BYTES_PER_BLOCK_LE, high BYTES_PER_BLOCK_LE
    dt 1
    dt 1, 0
    dt 1
    dt low ROOT_ENTRY_COUNT, high ROOT_ENTRY_COUNT
    dt low VOL_CAPACITY_IN_BLOCKS, high VOL_CAPACITY_IN_BLOCKS
    dt 0xF8
    dt low FAT_SIZE, high FAT_SIZE
    ;dt 0, 0 ; To reduce size these values are not needed. EP1_in is cleared,
    ;dt 0, 0 ; so just need to skip these values.
    ;dt 0, 0, 0, 0
    ;dt 0, 0, 0, 0
    ;dt 0
    ;dt 0
boot16_prt2:
    dt 0x29
    dt 0x86, 0xE8, 0xA3, 0x56
_volumeDIR: ; Reusing boot16_prt2.
    ifdef PIC16F1454
    dt "PIC16F1454 "
    endif
    ifdef PIC16F1455
    dt "PIC16F1455 "
    endif
    ifdef PIC16F1459
    dt "PIC16F1459 "
    endif
    dt "FAT16   "
    
;*******************************************************************************
; ABOUT Directory Entry
;*******************************************************************************
    if USE_ABOUT_FILE == 1
aboutDIR:
    dt "ABOUT   HTM"
    ;dt 0x01
    ;dt 0
    ;dt 0
    ;dt 0, 0
    ;dt 0, 0
    ;dt 0, 0
    ;dt 0, 0
    ;dt 0, 0
    ;dt 0, 0
    ;dt 0, 0
    ;dt ABOUT_FILE_SIZE, 0, 0, 0
about_file:
    dt ABOUT_STR_1
    if ABOUT_FILE_SIZE > 64
about_file_prt2:
    dt ABOUT_STR_2
    endif
    endif
    
;*******************************************************************************
; PROG_MEM Directory Entry
;*******************************************************************************
    if USE_PROG_MEM_FILE == 1
prog_memDIR:
    dt "PROG_MEMBIN"
    ;dt 0x01
    ;dt 0
    ;dt 0
    ;dt 0, 0
    ;dt 0, 0
    ;dt 0, 0
    ;dt 0, 0
    ;dt 0, 0
    ;dt 0, 0
    ;dt PROG_MEM_CLUST, 0
    ;dt low FILE_SIZE, high FILE_SIZE, 0, 0
    endif
    
    end