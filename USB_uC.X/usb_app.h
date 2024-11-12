/**
 * @file usb_app.h
 * @author John Izzard
 * @date 2024-11-12
 * 
 * @brief USB uC - USB Application file (This file is for MSD Bootloader).
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
 * File Version 2.0.1 - 2024-11-12
 * - Changed: MIT License.
 *
 * File Version 2.0.0 - 2024-01-26
 * - Changed: API for usb_get_class_descriptor.
 *
 * File Version 1.0.0 - 2020-06-28
 * - Added: Initial release of the software.
 */

#ifndef USB_APP_H
#define USB_APP_H

#include <stdbool.h>
#include <stdint.h>


/* ************************************************************************** */
/* ************************** APP FUNCTIONS ********************************* */
/* ************************************************************************** */

/**
 * @fn bool usb_service_class_request(void)
 * 
 * @brief Used to service Class Requests on EP0.
 * 
 * @return Returns success (true) or failure (false) to execute the Request.
 */
bool usb_service_class_request(void);

/**
 * @fn bool usb_get_class_descriptor(const uint8_t** descriptor, uint16_t* size)
 * 
 * @brief Used to service Get Class Descriptor Requests.
 * 
 * @param[out] descriptor Descriptor to respond with.
 * @param[out] size Size of the response descriptor.
 * 
 * @return Returns success (true) or failure (false) to execute the Request.
 */
bool usb_get_class_descriptor(const uint8_t** descriptor, uint16_t* size);

/**
 * @fn void usb_app_init(void)
 * 
 * @brief Used to initialize class libraries.
 */
void usb_app_init(void);

/**
 * @fn void usb_app_clear_halt(uint8_t bd_table_index, uint8_t ep, uint8_t dir)
 * 
 * @brief Clear Endpoints used by the Application.
 * 
 * Used to inform the Application to clear the halt/stall condition on one of
 * it's endpoints.
 */
void usb_app_clear_halt(uint8_t bd_table_index, uint8_t ep, uint8_t dir);

/**
 * @fn void usb_app_tasks(void)
 * 
 * @brief Services the Application.
 * 
 * The device state machine calls this whenever an endpoint other than EP0 is used.
 */
void usb_app_tasks(void);

/**
 * @fn bool usb_app_set_interface(uint8_t alternate_setting, uint8_t interface)
 * 
 * @brief Services a SetInterface Request.
 * 
 * Services a SetInterface Request.
 * 
 * @param alternate_setting Alternate Setting to change to.
 * @param interface Interface the request is directed at.
 * @return Returns true if successful and false if unsuccessful.
 */
bool usb_app_set_interface(uint8_t alternate_setting, uint8_t interface);

/**
 * @fn bool usb_app_get_interface(uint8_t* alternate_setting_result, uint8_t interface)
 * 
 * @brief Services a GetInterface Request.
 * 
 * Returns the Alternate Setting used for the specified Interface.
 * 
 * @param alternate_setting_result Alternate Setting return.
 * @param interface Interface the request is directed at.
 * @return Returns true if successful and false if unsuccessful.
 */
bool usb_app_get_interface(uint8_t* alternate_setting_result, uint8_t interface);

/* ************************************************************************** */

#endif /* USB_APP_H */