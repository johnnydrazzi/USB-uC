/**
 * @file usb_ch9.h
 * @brief <i>USB Device Framework</i> defines and types.
 * @author John Izzard
 * @date 05/06/2020
 * 
 * USB uC - USB Stack.
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

#ifndef USB_CH9_H
#define USB_CH9_H

#include <stdint.h>


/* ************************************************************************** */
/* ************************ SETUP PACKET DEFINES **************************** */
/* ************************************************************************** */

/// bmRequestType D7: Data transfer direction
#define HOST_TO_DEVICE 0x00
#define DEVICE_TO_HOST 0x01

/// bmRequestType D6...5: Type
#define STANDARD 0x00
#define CLASS    0x01
#define VENDOR   0x02

/// bmRequestType D4...0: Recipient
#define DEVICE    0x00
#define INTERFACE 0x01
#define ENDPOINT  0x02
#define OTHER     0x03

///  Standard Request Codes (bRequest)
#define GET_STATUS        0x00
#define CLEAR_FEATURE     0x01
#define SET_FEATURE       0x03
#define SET_ADDRESS       0x05
#define GET_DESCRIPTOR    0x06
#define SET_DESCRIPTOR    0x07
#define GET_CONFIGURATION 0x08
#define SET_CONFIGURATION 0x09
#define GET_INTERFACE     0x0A
#define SET_INTERFACE     0x0B
#define SYNC_FRAME        0x0C

/* ************************************************************************** */


/* ************************************************************************** */
/* ********************** SET / CLEAR FEATURE DEFINES *********************** */
/* ************************************************************************** */

/// Standard Feature Selectors
#define ENDPOINT_HALT        0
#define DEVICE_REMOTE_WAKEUP 1
#define TEST_MODE            2

/// Test Mode Selectors
#define Test_J            1
#define Test_K            2
#define Test_SE0_NAK      3
#define Test_Packet       4
#define Test_Force_Enable 5

/* ************************************************************************** */


/* ************************************************************************** */
/* *********************** DESCRIPTOR TYPE DEFINES ************************** */
/* ************************************************************************** */

/// Descriptor Types
#define DEVICE_DESC                    1
#define CONFIGURATION_DESC             2
#define STRING_DESC                    3
#define INTERFACE_DESC                 4
#define ENDPOINT_DESC                  5
#define DEVICE_QUALIFIER_DESC          6
#define OTHER_SPEED_CONFIGURATION_DESC 7
#define INTERFACE_POWER_DESC           8

/* ************************************************************************** */


/* ************************************************************************** */
/* ************************** CLASS CODE DEFINES **************************** */
/* ************************************************************************** */

/// Class Codes
#define NA  0
#define CDC 2
#define HID 3
#define MSC 8

/* ************************************************************************** */


/* ************************************************************************** */
/* ********************************* TYPES ********************************** */
/* ************************************************************************** */

/// SETUP DATA
typedef union
{
    uint8_t array[8];
    struct
    {
        union
        {
            struct
            {
                unsigned Recipient              :5;
                unsigned Type                   :2;
                unsigned DataTransferDirection  :1;
            }bmRequestType_bits;
            uint8_t bmRequestType;
        };
        uint8_t  bRequest;
        uint16_t wValue;
        uint16_t wIndex;
        uint16_t wLength;
    };
}ch9_setup_t;

/// SET/CLEAR FEATURE REQUEST DATA
typedef struct
{
    union
    {
        struct
        {
            unsigned Recipient:5;
            unsigned Type:2;
            unsigned Direction:1;
        }bmRequestType_bits;
        uint8_t bmRequestType;
    };
    unsigned :8;
    uint16_t FeatureSelector;
    union
    {
        uint16_t ZeroInterfaceEndpoint;
        union
        {
            uint16_t Zero;
            uint16_t Interface;
            uint16_t Endpoint;
            struct
            {
                uint8_t InterfaceNumber;
                uint8_t TestSelector;
            }Interface_bits;  
            struct
            {
                unsigned EndpointNumber         :4;
                unsigned Reserved_ALWAYS_CLEAR  :3;
                unsigned Direction              :1;
                uint8_t  TestSelector;
            }Endpoint_bits;
        }ZeroInterfaceEndpoint_bits;  
    };
    uint16_t Zero;
}ch9_set_clear_feature_t;

/// GET CONFIGURATION REQUEST DATA
typedef struct
{
    unsigned :8;
    unsigned :8;
    uint32_t Zero;
    uint16_t One;
}ch9_get_configuration_t;

/// GET DESCRIPTOR REQUEST DATA
typedef struct
{
    unsigned :8;
    unsigned :8;
    uint8_t  DescriptorIndex;
    uint8_t  DescriptorType;
    uint16_t LanguageID_ZERO;
    uint16_t DescriptorLength;
}ch9_get_descriptor_t;

/// SET DESCRIPTOR REQUEST DATA
typedef struct
{
    unsigned :8;
    unsigned :8;
    uint8_t  DescriptorIndex;
    uint8_t  DescriptorType;
    uint16_t LanguageID_ZERO;
    uint16_t DescriptorLength;
}ch9_set_descriptor_t;

/// GET INTERFACE REQUEST DATA
typedef struct
{
    unsigned :8;
    unsigned :8;
    uint16_t Zero;
    uint8_t  Interface;
    unsigned Reserved_ALWAYS_CLEAR :8;
    uint16_t One;
}ch9_get_interface_t;

/// SET INTERFACE REQUEST DATA
typedef struct
{
    unsigned :8;
    unsigned :8;
    uint16_t AlternateSetting;
    uint8_t  Interface;
    unsigned Reserved_ALWAYS_CLEAR :8;
    uint16_t Zero;
}ch9_set_interface_t;

/// GET STATUS REQUEST DATA
typedef struct
{
    union
    {
        struct
        {
            unsigned Recipient  :5;
            unsigned Type       :2;
            unsigned Direction  :1;
        }bmRequestType_bits;
        uint8_t bmRequestType;
    };
    unsigned :8;
    uint16_t Zero;
    union
    {
        uint16_t ZeroInterfaceEndpoint;
        union
        {
            uint16_t Zero;
            union
            {
                uint16_t Interface;
                struct
                {
                    uint8_t InterfaceNumber;
                    uint8_t Reserved_ALWAYS_CLEAR;
                }Interface_bits;
            };
            union
            {
                uint16_t Endpoint;
                struct
                {
                    unsigned EndpointNumber          :4;
                    unsigned Reserved0_ALWAYS_CLEAR  :3;
                    unsigned Direction               :1;
                    uint8_t  Reserved1_ALWAYS_CLEAR;
                }Endpoint_bits;
            };
        }ZeroInterfaceEndpoint_bits;  
    };
    uint16_t Two;
}ch9_get_status_t;

/// SET ADDRESS REQUEST DATA
typedef struct
{
    unsigned :8;
    unsigned :8;
    uint16_t DeviceAddress;
    uint32_t Zero;
}ch9_set_address_t;

/// SET CONFIGURATION REQUEST DATA
typedef struct
{
    unsigned :8;
    unsigned :8;
    uint16_t ConfigurationValue;
    uint32_t Zero;
}ch9_set_configuration_t;

/// SYNC FRAME REQUEST DATA
typedef struct
{
    unsigned                        :8;
    unsigned                        :8;
    uint16_t Zero;
    unsigned Endpoint               :4;
    unsigned Reserved_ALWAYS_CLEAR  :4;
    unsigned Direction              :1;
    uint16_t Two;
}ch9_sync_frame_t;

/// Device Descriptor Structure
typedef struct
{
    uint8_t  bLength;
    uint8_t  bDescriptorType;
    uint16_t bcdUSB;
    uint8_t  bDeviceClass;
    uint8_t  bDeviceSubClass;
    uint8_t  bDeviceProtocol;
    uint8_t  bMaxPacketSize0;
    uint16_t idVendor;
    uint16_t idProduct;
    uint16_t bcdDevice;
    uint8_t  iManufacturer;
    uint8_t  iProduct;
    uint8_t  iSerialNumber;
    uint8_t  bNumConfigurations;
}ch9_device_descriptor_t;

/// Device Qualifier Descriptor Structure
typedef struct
{ // *** ONLY USED IN HIGH SPEED
    uint8_t  bLength;
    uint8_t  bDescriptorType;
    uint16_t bcdUSB;
    uint8_t  bDeviceClass;
    uint8_t  bDeviceSubClass;
    uint8_t  bDeviceProtocol;
    uint8_t  bMaxPacketSize0;
    uint8_t  bNumConfigurations;
    uint8_t  bReserved;
}ch9_device_qualifier_descriptor_t;

/// Configuration Descriptor Structure
typedef struct
{
    uint8_t  bLength;
    uint8_t  bDescriptorType;
    uint16_t wTotalLength;
    uint8_t  bNumInterfaces;
    uint8_t  bConfigurationValue;
    uint8_t  iConfiguration;
    uint8_t  bmAttributes;
    uint8_t  bMaxPower;
}ch9_configutarion_descriptor_t;

/// Other Speed Configuration Descriptor Structure
typedef struct
{ // *** ONLY USED IN HIGH SPEED
    uint8_t  bLength;
    uint8_t  bDescriptorType;
    uint16_t wTotalLength;
    uint8_t  bNumInterfaces;
    uint8_t  bConfigurationValue;
    uint8_t  iConfiguration;
    uint8_t  bmAttributes;
    uint8_t  bMaxPower;
}ch9_other_speed_configutarion_descriptor_t;

/// Standard Interface Descriptor Structure
typedef struct
{
    uint8_t  bLength;
    uint8_t  bDescriptorType;
    uint8_t  bInterfaceNumber;
    uint8_t  bAlternateSetting;
    uint8_t  bNumEndpoints;
    uint8_t  bInterfaceClass;
    uint8_t  bInterfaceSubClass;
    uint8_t  bInterfaceProtocol;
    uint8_t  iInterface;
}ch9_standard_interface_descriptor_t;

/// Standard Endpoint Descriptor Structure
typedef struct
{
    uint8_t  bLength;
    uint8_t  bDescriptorType;
    uint8_t  bEndpointAddress;
    uint8_t  bmAttributes;
    uint16_t wMaxPacketSize;
    uint8_t  bInterval;
}ch9_standard_endpoint_descriptor_t;

/* ************************************************************************** */

#endif /* USB_CH9_H */