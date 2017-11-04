/**
 * @file wifi.c
 *
 */
/* Copyright (C) 2016-2017 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include <assert.h>
#include <stdint.h>

#include "wifi.h"

#include "esp8266.h"
#include "esp8266_cmd.h"

static char sdk_version[SDK_VERSION_MAX + 1] __attribute__((aligned(4))) = { 'U' , 'n', 'k' , 'n' , 'o' , 'w', 'n' , '\0'};


/**
 *
 * @return
 */
const char *system_get_sdk_version(void) {
	uint16_t len = SDK_VERSION_MAX;

	esp8266_write_4bits((uint8_t) CMD_SYSTEM_SDK_VERSION);
	esp8266_read_str(sdk_version, &len);

	return sdk_version;
}

