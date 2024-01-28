/**
 * @file usb_app.h
 * @brief USB application header.
 * @author John Izzard
 * @date 17/12/2023
 * 
 * USB uC - USB Application file.
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