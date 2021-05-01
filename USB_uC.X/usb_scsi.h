/**
 * @file usb_scsi.h
 * @brief SCSI related definitions/enums and types.
 * @author John Izzard
 * @date 05/06/2020
 * 
 * USB uC - MSD Library (This file is for MSD Internal Example).
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
#ifndef USB_SCSI_H
#define USB_SCSI_H

#include <stdint.h>

/* ************************************************************************** */
/* ************************** SENSE DATA DEFINES **************************** */
/* ************************************************************************** */

// ASC and ASCQ Sense Data
#define ASC_NO_ADDITIONAL_SENSE_INFORMATION              0x00
#define ASCQ_NO_ADDITIONAL_SENSE_INFORMATION             0x00

#define ASC_INVALID_COMMAND_OPCODE                       0x20
#define ASCQ_INVALID_COMMAND_OPCODE                      0x00

#define ASC_INVALID_FIELD_IN_CBD                         0x24
#define ASCQ_INVALID_FIELD_IN_CBD                        0x00

#define ASC_LOGICAL_UNIT_NOT_SUPPORTED                   0x25
#define ASCQ_LOGICAL_UNIT_NOT_SUPPORTED                  0x00

#define ASC_LOGICAL_UNIT_DOES_NOT_RESPOND                0x05
#define ASCQ_LOGICAL_UNIT_DOES_NOT_RESPOND               0x00

#define ASC_NOT_READY_TO_READY_CHANGE                    0x28
#define ASCQ_MEDIUM_MAY_HAVE_CHANGED                     0x00

#define ASC_MEDIUM_NOT_PRESENT                           0x3a
#define ASCQ_MEDIUM_NOT_PRESENT                          0x00

#define ASC_LOGICAL_UNIT_NOT_READY_CAUSE_NOT_REPORTABLE  0x04
#define ASCQ_LOGICAL_UNIT_NOT_READY_CAUSE_NOT_REPORTABLE 0x00

#define ASC_LOGICAL_UNIT_IN_PROCESS                      0x04
#define ASCQ_LOGICAL_UNIT_IN_PROCESS                     0x01

#define ASC_LOGICAL_UNIT_NOT_READY_INIT_REQD             0x04
#define ASCQ_LOGICAL_UNIT_NOT_READY_INIT_REQD            0x02

#define ASC_LOGICAL_UNIT_NOT_READY_INTERVENTION_REQD     0x04
#define ASCQ_LOGICAL_UNIT_NOT_READY_INTERVENTION_REQD    0x03

#define ASC_LOGICAL_UNIT_NOT_READY_FORMATTING            0x04
#define ASCQ_LOGICAL_UNIT_NOT_READY_FORMATTING           0x04

#define ASC_LOGICAL_BLOCK_ADDRESS_OUT_OF_RANGE           0x21
#define ASCQ_LOGICAL_BLOCK_ADDRESS_OUT_OF_RANGE          0x00

#define ASC_WRITE_PROTECTED                              0x27
#define ASCQ_WRITE_PROTECTED                             0x00

// Sense Data Response Codes
#define CURRENT_FIXED       0x70
#define DEFERRED_FIXED      0x71
#define CURRENT_DESCRIPTOR  0x72
#define DEFERRED_DESCRIPTOR 0x73

// Sense Data Sense Key Error Codes
#define NO_SENSE        0x00
#define RECOVERED_ERROR 0x01
#define NOT_READY       0x02
#define MEDIUM_ERROR    0x03
#define HARDWARE_ERROR  0x04
#define ILLEGAL_REQUEST 0x05
#define UNIT_ATTENTION  0x06
#define DATA_PROTECT    0x07
#define BLANK_CHECK     0x08
#define VENDOR_SPECIFIC 0x09
#define COPY_ABORTED    0x0A
#define ABORTED_COMMAND 0x0B
#define OBSOLETE        0x0C
#define VOLUME_OVERFLOW 0x0D
#define MISCOMPARE      0x0E

/* ************************************************************************** */


/* ************************************************************************** */
/* ***************************** SCSI COMMANDS ****************************** */
/* ************************************************************************** */

// SBC-2 SCSI Commands (** Popular Commands)
#define FORTMAT_UNIT                 0x04 // Manditory, not supported.
#define INQUIRY                      0x12 // Manditory, supported.     **
#define LOCK_UNLOCK_CACHE_10         0x36 // Optional, not supported.
#define LOCK_UNLOCK_CACHE_16         0x92 // Optional, not supported.
#define LOG_SELECT                   0x4C // Optional, not supported.
#define LOG_SENSE                    0x4D // Optional, not supported.
#define MODE_SELECT_6                0x15 // Optional, not supported.
#define MODE_SELECT_10               0x55 // Optional, not supported.
#define MODE_SENSE_6                 0x1A // Optional, supported.      **
#define MODE_SENSE_10                0x5A // Optional, not supported.
#define MOVE_MEDIUM                  0xA7 // Optional, not supported.
#define PERSISTANT_RESERVE_IN        0x5E // Optional, not supported.
#define PERSISTANT_RESERVE_OUT       0x5F // Optional, not supported.
#define PRE_FETCH_10                 0x34 // Optional, not supported.
#define PRE_FETCH_16                 0x90 // Optional, not supported.
#define PREVENT_ALLOW_MEDIUM_REMOVAL 0x1E // Optional, not supported.  **
#define READ_6                       0x08 // Manditory, not supported.
#define READ_10                      0x28 // Manditory, supported.     **
#define READ_12                      0xA8 // Optional, not supported.
#define READ_16                      0x88 // Manditory, not supported.
#define READ_BUFFER                  0x3C // Optional, not supported.
#define READ_CAPACITY                0x25 // Manditory, supported.     **
#define READ_DEFECT_DATA_10          0x37 // Optional, not supported.
#define READ_DEFECT_DATA_12          0xB7 // Optional, not supported.
#define READ_ELEMENT_STATUS          0xB4 // Optional, not supported.
#define READ_LONG                    0x3E // Optional, not supported.
#define REASSIGN_BLOCKS              0x07 // Optional, not supported.
#define REBUILD_16                   0x81 // Optional, not supported.
#define REBUILD_32                   0x7F // Optional, not supported.
#define RECEIVE_DIAGNOSTIC_RESULTS   0x1C // Optional, not supported.
#define REGENERAGE_16                0x82 // Optional, not supported.
#define REGENERATE_32                0x7F // Optional, not supported.
#define RELEASE_6                    0x16 // Optional, not supported.
#define RELEASE_10                   0x57 // Manditory, not supported.
#define REPORT_LUNS                  0xA0 // Optional, not supported.
#define REQUEST_SENSE                0x03 // Manditory, supported.     **
#define RESERVE_6                    0x16 // Optional, not supported.
#define RESERVE_10                   0x56 // Manditory, not supported.
#define SEEK_10                      0x2B // Optional, not supported.
#define SEND_DIAGNOSTIC              0x1D // Manditory, not supported.
#define SET_LIMITS_10                0x33 // Optional, not supported.
#define SET_LIMITS_12                0xB3 // Optional, not supported.
#define START_STOP_UNIT              0x1B // Optional, supported.      **
#define SYNCHRONIZE_CACHE_10         0x35 // Optional, not supported.
#define SYNCHRONIZE_CACHE_16         0x91 // Optional, not supported.
#define TEST_UNIT_READY              0x00 // Manditory, supported.     **
#define VERIFY_10                    0x2F // Optional, supported.      **
#define VERIFY_12                    0xAF // Optional, not supported.
#define VERIFY_16                    0x8F // Optional, not supported.
#define WRITE_6                      0x0A // Optional, not supported.
#define WRITE_10                     0x2A // Optional, supported.      **
#define WRITE_12                     0xAA // Optional, not supported.
#define WRITE_16                     0x8A // Optional, not supported.
#define WRITE_AND_VERIFY_10          0x2E // Optional, not supported.
#define WRITE_AND_VERIFY_12          0xAE // Optional, not supported.
#define WRITE_AND_VERIFY_16          0x8E // Optional, not supported.
#define WRITE_BUFFER                 0x3B // Optional, not supported.
#define WRITE_LONG                   0x3F // Optional, not supported.
#define WRITE_SAME_10                0x41 // Optional, not supported.
#define WRITE_SAME_16                0x93 // Optional, not supported.
#define XDREAD_10                    0x52 // Optional, not supported.
#define XDREAD_32                    0x7F // Optional, not supported.
#define XDWRITE_10                   0x50 // Optional, not supported.
#define XDWRITE_32                   0x7F // Optional, not supported.
#define XDWRITEREAD_10               0x53 // Optional, not supported.
#define XDWRITEREAD_32               0x7F // Optional, not supported.
#define XDWRITE_EXTENDED_16          0x80 // Optional, not supported.
#define XDWRITE_EXTENDED_32          0x7F // Optional, not supported.
#define XDWRITE_EXTENDED_64          0x7F // Optional, not supported.
#define XPWRITE_10                   0x51 // Optional, not supported.
#define XPWRITE_32                   0x7F // Optional, not supported.

/* ************************************************************************** */


/* ************************************************************************** */
/* ********************************** TYPES ********************************* */
/* ************************************************************************** */

typedef union
{
    uint8_t BYTE[18];
    struct
    {
        unsigned RESPONSE_CODE:         7;
        unsigned VALID:                 1;
        uint8_t Obsolete;
        unsigned SENSE_KEY:             4;
        unsigned RESERVED:              1;
        unsigned ILI:                   1;
        unsigned EOM:                   1;
        unsigned FILEMARK:              1;
        uint8_t INFORMATION[4];
        uint8_t ADDITIONAL_SENSE_LENGTH;
        uint8_t COMMAND_SPECIFIC_INFORMATION[4];
        uint8_t ADDITIONAL_SENSE_CODE;
        uint8_t ADDITIONAL_SENSE_CODE_QUALIFIER;
        uint8_t FIELD_REPLACEABLE_UNIT_CODE;
        uint8_t SENSE_KEY_SPECIFIC[3];
    };
}scsi_fixed_format_sense_t;

// 0x03 Request Sense Command
typedef struct
{
    uint8_t  OPERTATION_CODE;
    unsigned DESC:                  1;
    unsigned:                       7;
    unsigned:                       8;
    unsigned:                       8;
    uint8_t ALLOCATION_LENGTH;
    uint8_t  CONTROL;
}scsi_request_sense_cmd_t;

// 0x12 Inquiry Command
typedef struct
{
    uint8_t  OPERTATION_CODE;
    unsigned EVPD:                  1;
    unsigned:                       7;
    uint8_t  PAGE_CODE;
    union
    {
        uint8_t ALLOCATION_LENGTH_BYTES[2];
        uint16_t ALLOCATION_LENGTH;
    };
    uint8_t  CONTROL;
}scsi_inquiry_cmd_t;

typedef struct
{
    uint8_t Data[8];
    uint8_t T10_VEND_ID[8];
    uint8_t PROD_ID[16];
    uint8_t PROD_REV[4];
}scsi_inquiry_t;

// Mode Sense (6) Command
typedef struct
{
    uint8_t OPERATION_CODE;
    unsigned: 3;
    unsigned DBD: 1;
    unsigned: 4;
    unsigned PAGE_CODE: 6;
    unsigned PC: 2;
    uint8_t SUBPAGE_CODE;
    uint8_t ALLOCATION_LENGTH;
    uint8_t CONTROL;
}scsi_mode_sense_6_cmd_t;

// 0x25 Read Capacity (10) Command
typedef struct
{
    uint8_t OPERATION_CODE;
    unsigned: 8;
    union
    {
        uint32_t LOGICAL_BLOCK_ADDRESS;
        uint8_t LOGICAL_BLOCK_ADDRESS_BYTES[4];
    };
    unsigned: 8;
    unsigned: 8;
    unsigned PMI: 1;
    unsigned: 7;
    uint8_t CONTROL;
}scsi_read_capacity_10_cmd_t;

typedef struct
{
    uint32_t RETURNED_LOGICAL_BLOCK_ADDRESS;
    uint32_t BLOCK_LENGTH_IN_BYTES;
}scsi_read_capacity_10_t;

// 0x28 READ (10) Command
typedef struct
{
    uint8_t OPERATION_CODE;
    unsigned: 1;
    unsigned FUA_NV: 1;
    unsigned: 1;
    unsigned FUA: 1;
    unsigned DPO: 1;
    unsigned RDPROTECT: 3;
    union
    {
        uint8_t LBA_BYTES[4];
        uint32_t LBA;
    };
    unsigned GROUP_NUMBER: 5;
    unsigned: 3;
    union
    {
        uint8_t TF_LEN_BYTES[2];
        uint16_t TF_LEN;
    };
    uint8_t CONTROL;
}scsi_read_10_cmd_t;

//0x2A Write (10) Command
typedef struct
{
    uint8_t OPERATION_CODE;
    unsigned: 1;
    unsigned: 1;
    unsigned: 1;
    unsigned FUA: 1;
    unsigned DPO: 1;
    unsigned WRPROTECT: 3;
    union
    {
        uint8_t LBA_BYTES[4];
        uint32_t LBA;
    };
    unsigned GROUP_NUMBER: 5;
    unsigned: 3;
    union
    {
        uint8_t TF_LEN_BYTES[2];
        uint16_t TF_LEN;
    };
    uint8_t CONTROL;
}scsi_write_10_cmd_t;

// Mode Select (6) Command
typedef struct
{
    uint8_t OPERATION_CODE;
    unsigned SP: 1;
    unsigned: 3;
    unsigned FP: 1;
    unsigned: 3;
    unsigned: 8;
    unsigned: 8;
    uint8_t PARAMETER_LIST_LENGTH;
    uint8_t CONTROL;
}scsi_mode_select_6_cmd_t;

typedef struct
{
    uint8_t MODE_DATA_LENGTH;
    uint8_t MEDIUM_TYPE;
    uint8_t DEVICE_SPECIFIC_PARAMETER;
    uint8_t BLOCK_DESCRIPTOR_LENGTH;
}scsi_mode_sense_t;

// Prevent Allow Media Removal Command
typedef struct
{
    uint8_t OPERATION_CODE;
    unsigned: 8;
    unsigned: 8;
    unsigned: 8;
    unsigned PREVENT: 2;
    unsigned: 6;
    unsigned CONTROL: 8;
}scsi_pamr_cmd_t;

/* ************************************************************************** */

#endif /* USB_SCSI_H */