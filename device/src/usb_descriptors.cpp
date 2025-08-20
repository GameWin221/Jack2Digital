/*
 * This file is based on the usb_descriptors.c file that can be found in the tinyusb cdc_msc example:
 * https://github.com/hathach/tinyusb/blob/master/examples/device/cdc_msc/src/usb_descriptors.c
 * 
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include "bsp/board_api.h"
#include "tusb.h"

// https://github.com/raspberrypi/usb-pid
#define USB_PID 0x000A // CDC RP2040
#define USB_VID 0x2E8A
#define USB_BCD 0x0200

#define EPNUM_CDC_NOTIF 0x81
#define EPNUM_CDC_OUT   0x02
#define EPNUM_CDC_IN    0x82

#define CONFIG_TOTAL_LEN (TUD_CONFIG_DESC_LEN + TUD_CDC_DESC_LEN)

enum : uint8_t {
    ITF_NUM_CDC = 0,
    ITF_NUM_CDC_DATA,
    ITF_NUM_TOTAL
};

enum : uint8_t {
    STRID_LANGID = 0,
    STRID_MANUFACTURER,
    STRID_PRODUCT,
    STRID_SERIAL
};

static const char *string_desc_arr[] = {
    (const char[]){ 0x09, 0x04 },  // 0: is supported language is English (0x0409)
    "Mateusz Antkiewicz",          // 1: Manufacturer
    "Jack2Digital",                // 2: Product
    nullptr,                       // 3: Serial unique ID (provided later in tud_descriptor_string_cb(...))
    "Pi Pico CDC",                 // 4: CDC Interface
};

// full speed configuration
static uint8_t const desc_fs_conf[] = {
    // Config number, interface count, string index, total length, attribute, power in mA
    TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN, 0x80, 100),
    
    // Interface number, string index, EP notification address and size, EP data address (out, in) and size.
    TUD_CDC_DESCRIPTOR(ITF_NUM_CDC, 4, EPNUM_CDC_NOTIF, 8, EPNUM_CDC_OUT, EPNUM_CDC_IN, 64),
};

#if TUD_OPT_HIGH_SPEED
// Per USB specs: high speed capable device must report device_qualifier and other_speed_configuration

// high speed configuration
static uint8_t const desc_hs_conf[] = {
    // Config number, interface count, string index, total length, attribute, power in mA
    TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN, 0x80, 100),

    // Interface number, string index, EP notification address and size, EP data address (out, in) and size.
    TUD_CDC_DESCRIPTOR(ITF_NUM_CDC, 4, EPNUM_CDC_NOTIF, 8, EPNUM_CDC_OUT, EPNUM_CDC_IN, 512),
};

// Invoked when received GET DEVICE QUALIFIER DESCRIPTOR request
// Application return pointer to descriptor, whose contents must exist long enough for transfer to complete.
// device_qualifier descriptor describes information about a high-speed capable device that would
// change if the device were operating at the other speed. If not highspeed capable stall this request.
uint8_t const *tud_descriptor_device_qualifier_cb(void) {
    static tusb_desc_device_qualifier_t const desc_device_qualifier{
        .bLength            = sizeof(tusb_desc_device_qualifier_t),
        .bDescriptorType    = TUSB_DESC_DEVICE_QUALIFIER,
        .bcdUSB             = USB_BCD,
        .bDeviceClass       = TUSB_CLASS_MISC,
        .bDeviceSubClass    = MISC_SUBCLASS_COMMON,
        .bDeviceProtocol    = MISC_PROTOCOL_IAD,
        .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,
        .bNumConfigurations = 0x01,
        .bReserved          = 0x00
    };

    return (uint8_t const *)&desc_device_qualifier;
}

// Invoked when received GET OTHER SEED CONFIGURATION DESCRIPTOR request
// Application return pointer to descriptor, whose contents must exist long enough for transfer to complete
// Configuration descriptor in the other speed e.g if high speed then this is for full speed and vice versa
uint8_t const *tud_descriptor_other_speed_configuration_cb(uint8_t index) {
    (void) index; // for multiple configurations

    static uint8_t desc_other_speed_config[CONFIG_TOTAL_LEN];

    // if link speed is high return fullspeed config, and vice versa
    // Note: the descriptor type is OHER_SPEED_CONFIG instead of CONFIG
    memcpy(desc_other_speed_config, (tud_speed_get() == TUSB_SPEED_HIGH) ? desc_fs_conf : desc_hs_conf, CONFIG_TOTAL_LEN);

    desc_other_speed_config[1] = TUSB_DESC_OTHER_SPEED_CONFIG;

    return desc_other_speed_config;
}
#endif

// Invoked when received GET DEVICE DESCRIPTOR
// Application return pointer to descriptor
uint8_t const *tud_descriptor_device_cb(void) {
    static const tusb_desc_device_t desc_device{
        .bLength            = sizeof(tusb_desc_device_t),
        .bDescriptorType    = TUSB_DESC_DEVICE,
        .bcdUSB             = USB_BCD,
        .bDeviceClass       = TUSB_CLASS_MISC,
        .bDeviceSubClass    = MISC_SUBCLASS_COMMON,
        .bDeviceProtocol    = MISC_PROTOCOL_IAD,
        .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,
        .idVendor           = USB_VID,
        .idProduct          = USB_PID,
        .bcdDevice          = 0x0100,
        .iManufacturer      = STRID_MANUFACTURER,
        .iProduct           = STRID_PRODUCT,
        .iSerialNumber      = STRID_SERIAL,
        .bNumConfigurations = 0x01
    };

    return (uint8_t const*)&desc_device;
}

// Invoked when received GET CONFIGURATION DESCRIPTOR
// Application return pointer to descriptor
// Descriptor contents must exist long enough for transfer to complete
uint8_t const *tud_descriptor_configuration_cb(uint8_t index) {
    (void) index; // for multiple configurations

#if TUD_OPT_HIGH_SPEED
    // Although we are highspeed, host may be fullspeed.
    return (tud_speed_get() == TUSB_SPEED_HIGH) ? desc_hs_conf : desc_fs_conf;
#else
    return desc_fs_conf;
#endif
}

// Invoked when received GET STRING DESCRIPTOR request
// Application return pointer to descriptor, whose contents must exist long enough for transfer to complete
uint16_t const *tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
    (void)langid;
    size_t chr_count;

    static uint16_t desc_str[32 + 1];

    switch (index) {
        case STRID_LANGID:
            memcpy(&desc_str[1], string_desc_arr[0], 2);
            chr_count = 1;
            break;

        case STRID_SERIAL:
            chr_count = board_usb_get_serial(desc_str + 1, 32);
            break;

        default:
            if(index >= (sizeof(string_desc_arr) / sizeof(string_desc_arr[0]))) {
                return nullptr;
            }

            const char *str = string_desc_arr[index];

            chr_count = strlen(str);
            const size_t max_count = sizeof(desc_str) / sizeof(desc_str[0]) - 1; // -1 for string type
            if (chr_count > max_count) {
                chr_count = max_count;
            }

            // Convert ASCII string into UTF-16
            for (size_t i = 0; i < chr_count; i++) {
                desc_str[1 + i] = str[i];
            }
            break;
    }

    // first byte is length (including header), second byte is string type
    desc_str[0] = (uint16_t)((sizeof(uint16_t) * (chr_count + 1)) | (TUSB_DESC_STRING << 8));

    return desc_str;
}
