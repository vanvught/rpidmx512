/**
 * @file bw_spi_lcd.c
 *
 */
/* Copyright (C) 2014, 2015, 2016 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <stdint.h>

#include "tables.h"
#include "util.h"
#include "dmx.h"

#include "bw_lcd.h"
#include "bw_spi_lcd.h"

#define DMX_FOOTPRINT	4

static const char device_label[] = "bw_spi_lcd";
static const uint8_t device_label_len = MIN(sizeof(device_label) / sizeof(device_label[0]), RDM_DEVICE_LABEL_MAX_LENGTH);

static struct _rdm_personality rdm_personality = { DMX_FOOTPRINT, "LCD 4-slots", 11 };
static struct _rdm_sub_devices_info sub_device_info = {DMX_FOOTPRINT, 1, 1, /* start address */0, /* sensor count */0, "", 0, &rdm_personality};

/**
 *
 * @param arg
 * @param buf
 */
inline static void itoa_base10(uint16_t arg, char buf[]) {
	char *n = buf + 2;

	if (arg == 0) *n = '0';

	while (arg != 0) {
		*n = (char)'0' + (char)(arg % 10);
		n--;
		arg /= 10;
	}
}

/**
 * @ingroup DEV
 *
 * @param spi_lcd_info
 * @param start_channel
 */
inline static void display_channels(device_info_t *spi_lcd_info, uint16_t start_channel) {
	char text[BW_LCD_MAX_CHARACTERS];
	int i = 0;
	int offset = 0;

	for (i = 0; i < 4; i++) {
		offset = i * 4;
		text[offset] = ' ';
		text[offset + 1] = ' ';
		text[offset + 2] = ' ';
		text[offset + 3] = ' ';
		itoa_base10(start_channel + i, &text[offset]);
	}

	bw_spi_lcd_text_line_1(spi_lcd_info, text, BW_LCD_MAX_CHARACTERS);
}

/**
 * @ingroup DEV
 *
 * @param device_info
 * @param start_channel
 */
inline static void display_data_hex(device_info_t *device_info, const uint16_t start_channel, const uint8_t *dmx_data) {
	char text[BW_LCD_MAX_CHARACTERS];
	int i = 0;
	int offset = 0;

	for (i = 0; i < 4; i++) {
		uint8_t data;
		offset = i * 4;
		data = dmx_data[start_channel + i];
		text[offset    ] = ' ';
		text[offset + 1] = TO_HEX((data & (char)0xF0) >> 4);
		text[offset + 2] = TO_HEX(data & (char)0x0F);
		text[offset + 3] = ' ';
	}

	text[15] = 'H';
	bw_spi_lcd_text_line_2(device_info, text, BW_LCD_MAX_CHARACTERS);
}

/**
 *
 * @param dmx_device_info
 */
static void bw_spi_lcd(dmx_device_info_t *dmx_device_info, const uint8_t *dmx_data) {
	display_data_hex(&(dmx_device_info->device_info), dmx_device_info->dmx_start_address, dmx_data);
}

INITIALIZER(devices, bw_spi_lcd)

/**
 *
 * @param dmx_device_info
 */
static void bw_spi_lcd_zero(dmx_device_info_t *dmx_device_info, const uint8_t *dmx_data) {
	bw_spi_lcd_cls(&(dmx_device_info->device_info));
}

INITIALIZER(devices_zero, bw_spi_lcd_zero)

/**
 * @ingroup DEV
 *
 * @param dmx_device_info
 */
static void bw_spi_lcd_init(dmx_device_info_t *dmx_device_info, const uint8_t *dmx_data) {
	struct _rdm_sub_devices_info *rdm_sub_devices_info =  &(dmx_device_info)->rdm_sub_devices_info;

	(void) bw_spi_lcd_start(&(dmx_device_info->device_info));
	bw_spi_lcd_cls(&(dmx_device_info->device_info));
	display_channels(&(dmx_device_info->device_info), dmx_device_info->dmx_start_address);

	(void *) memcpy(rdm_sub_devices_info, &sub_device_info, sizeof(struct _rdm_sub_devices_info));
	dmx_device_info->rdm_sub_devices_info.dmx_start_address = dmx_device_info->dmx_start_address;
	(void *) memcpy(dmx_device_info->rdm_sub_devices_info.device_label, device_label, device_label_len);
	dmx_device_info->rdm_sub_devices_info.device_label_length = device_label_len;
}

INITIALIZER(devices_init, bw_spi_lcd_init)
