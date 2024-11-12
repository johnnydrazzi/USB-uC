/**
 * @file usb_app.c
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
 * - Fixed: Line 66 returns false.
 *
 * File Version 1.0.0 - 2020-06-28
 * - Added: Initial release of the software.
 */

#include <stdbool.h>
#include "usb.h"
#include "usb_app.h"
#include "usb_msd.h"


bool usb_service_class_request(void)
{
    return msd_class_request();
}

bool usb_get_class_descriptor(const uint8_t** descriptor, uint16_t* size)
{
    return false; // No class descriptors for MSD.
}


void usb_app_init(void)
{
    msd_init();
}


void usb_app_tasks(void)
{
    switch(TRANSACTION_EP)
    {
        case MSD_EP:
            msd_add_task();
            break;
    }
}


void usb_app_clear_halt(uint8_t bd_table_index, uint8_t ep, uint8_t dir)
{
    msd_clear_halt(bd_table_index, ep, dir);
}


bool usb_app_set_interface(uint8_t alternate_setting, uint8_t interface)
{
#if NUM_ALT_INTERFACES != 0
    if(g_usb_set_interface.Interface < NUM_INTERFACES) return true;
#else
    if(alternate_setting != 0 || interface != 0) return false;
    
    msd_clear_ep_toggle();
    return true;
#endif
}


bool usb_app_get_interface(uint8_t* alternate_setting_result, uint8_t interface)
{
#if NUM_ALT_INTERFACES != 0
    if(g_get_interface.Interface < NUM_INTERFACES) return true;
#endif
    return false;
}


bool usb_out_control_finished(void)
{
    return false;
}