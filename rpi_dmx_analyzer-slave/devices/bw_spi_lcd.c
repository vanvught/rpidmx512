/**
 * @file bw_spi_lcd.c
 *
 */
/* Copyright (C) 2014 by Arjan van Vught <pm @ http://www.raspberrypi.org/forum/>
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

#ifdef DEBUG
extern int printf(const char *format, ...);
#endif
#include "tables.h"
#include "dmx.h"
#include "bw_spi_lcd.h"
#include "util.h"

static const struct _rdm_personality rdm_sub_device = { 4, "LCD 4-slots", 11 };
static struct _rdm_sub_devices_info rdm_sub_devices_info = { 4, 1, 1, 0, 0, "bw_spi_lcd", 10, &rdm_sub_device };

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
inline static void display_data_hex(device_info_t *device_info, const uint16_t start_channel) {
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
static void bw_spi_lcd(dmx_device_info_t *dmx_device_info) {
	display_data_hex(&(dmx_device_info->device_info), dmx_device_info->dmx_start_address);
}

INITIALIZER(devices, bw_spi_lcd)

/**
 * @ingroup DEV
 *
 * @param dmx_device_info
 */
static void bw_spi_lcd_init(dmx_device_info_t *dmx_device_info) {
#ifdef DEBUG
	printf("device init <bw_spi_lcd_init>\n");
#endif
	(void)bw_spi_lcd_start(&(dmx_device_info->device_info));
	bw_spi_lcd_cls(&(dmx_device_info->device_info));
	display_channels(&(dmx_device_info->device_info), dmx_device_info->dmx_start_address);

	dmx_device_info->rdm_sub_devices_info = &rdm_sub_devices_info;
	rdm_sub_devices_info.dmx_start_address = dmx_device_info->dmx_start_address;
}

INITIALIZER(devices_init, bw_spi_lcd_init)
