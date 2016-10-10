/**
 * @file devices_params.c
 *
 */
/* Copyright (C) 2016 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include <devices_params.h>
#include <stdint.h>
#include <stdbool.h>

#include "ws28xx.h"
#include "read_config_file.h"
#include "sscan.h"
#include "util.h"

static const char PARAMS_FILE_NAME[] ALIGNED = "devices.txt";			///< Parameters file name
static const char PARAMS_LED_TYPE[] ALIGNED = "led_type";				///<
static const char PARAMS_LED_COUNT[] ALIGNED = "led_count";				///<

static const char led_types[3][8] ALIGNED = { "WS2801", "WS2812", "WS2812B" };

static _ws28xxx_type devices_params_led_type = WS2801;					///<
static uint16_t devices_params_led_count = 170;							///< 1 DMX Universe

/**
 *
 * @return
 */
const _ws28xxx_type devices_params_get_led_type(void) {
	return devices_params_led_type;
}

/**
 *
 * @return
 */
const uint16_t devices_params_get_led_count(void) {
	return devices_params_led_count;
}

/**
 *
 * @return
 */
const char *devices_params_get_led_type_string(void) {
	return led_types[devices_params_led_type];
}

/**
 *
 * @param line
 */
static void process_line_read(const char *line) {
	uint16_t value16;
	uint8_t len;
	char *p_type;

	p_type = (char *)led_types[WS2801];
	len = 6;
	if (sscan_char_p(line, PARAMS_LED_TYPE, p_type, &len) == 2) {
		devices_params_led_type = WS2801;
		return;
	}

	p_type = (char *)led_types[WS2812];
	len = 6;
	if (sscan_char_p(line, PARAMS_LED_TYPE, p_type, &len) == 2) {
		devices_params_led_type = WS2812;
		return;
	}

	p_type = (char *)led_types[WS2812B];
	len = 7;
	if (sscan_char_p(line, PARAMS_LED_TYPE, p_type, &len) == 2) {
		devices_params_led_type = WS2812B;
		return;
	}

	if (sscan_uint16_t(line, PARAMS_LED_COUNT, &value16) == 2) {
		if (value16 != 0 && value16 <= (4 * 170)) {
			devices_params_led_count = value16;
		}
	}
}

/**
 *
 * @return
 */
const bool devices_params_init(void) {
	return read_config_file(PARAMS_FILE_NAME, &process_line_read);
}
