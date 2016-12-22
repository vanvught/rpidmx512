/**
 * @file devicesparams.cpp
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

#include <stdint.h>

#include "deviceparams.h"
#include "ws28xx.h"
#include "read_config_file.h"
#include "sscan.h"
#include "util.h"

static const char PARAMS_FILE_NAME[] ALIGNED = "devices.txt";			///< Parameters file name
static const char PARAMS_LED_TYPE[] ALIGNED = "led_type";				///<
static const char PARAMS_LED_COUNT[] ALIGNED = "led_count";				///<

#define LED_TYPES_COUNT 			5	///<
#define LED_TYPES_MAX_NAME_LENGTH 	8	///<
static const char led_types[LED_TYPES_COUNT][LED_TYPES_MAX_NAME_LENGTH] ALIGNED = { "WS2801\0", "WS2811\0", "WS2812\0", "WS2812B", "WS2813\0" };

static _ws28xxx_type devices_params_led_type = WS2801;					///<
static uint16_t devices_params_led_count = 170;							///< 1 DMX Universe = 512 / 3

/**
 *
 * @param line
 */
static void process_line_read(const char *line) {
	uint16_t value16;
	uint8_t len;
	char buffer[16] ALIGNED;

	len = 7;
	if (sscan_char_p(line, PARAMS_LED_TYPE, buffer, &len) == 2) {
		uint8_t i;
		for (i = 0; i < LED_TYPES_COUNT; i++) {
			if (memcmp(buffer, led_types[i], len) == 0) {
				devices_params_led_type = (_ws28xxx_type) i;
				return;
			}
		}
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
 */
DeviceParams::DeviceParams(void) {
	devices_params_led_type = WS2801;
	devices_params_led_count = 170;
}

/**
 *
 */
DeviceParams::~DeviceParams(void) {
}

/**
 *
 * @return
 */
bool DeviceParams::Load(void) {
	return read_config_file(PARAMS_FILE_NAME, &process_line_read);
}

/**
 *
 * @return
 */
const _ws28xxx_type DeviceParams::GetLedType(void) {
	return devices_params_led_type;
}

/**
 *
 * @return
 */
const uint16_t DeviceParams::GetLedCount(void) {
	return devices_params_led_count;
}

/**
 *
 * @return
 */
const char* DeviceParams::GetLedTypeString(void) {
	return led_types[devices_params_led_type];
}
