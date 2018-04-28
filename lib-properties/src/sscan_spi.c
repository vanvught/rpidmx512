/**
 * @file sscan_spi.c
 *
 */
/* Copyright (C) 2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#include <stdbool.h>
#include <ctype.h>
#include <assert.h>

#include "sscan.h"

int sscan_spi(const char *buf, char *spi, char *name, uint8_t *len, uint8_t *address, uint16_t *dmx, uint32_t *speed) {
	int i = 0;
	int k = 0;
	char c;
	uint16_t uint16;
	uint32_t uint32;
	char tmp[16];
	const char SPI[] = "SPI";
	uint8_t nibble_high;
	uint8_t nibble_low;

	for (i = 0; (buf[i] != (char) 0) && (buf[i] != (char) ',') ; i++) {
		if ((i < 3) && (buf[i] != SPI[i]))  {
			return SSCAN_NAME_ERROR;
		}
	}

	if (i > 4) {
		return SSCAN_NAME_ERROR;
	}

	c = buf[3];

	if (!isdigit((int)c) && (buf[4] != (char) ',')) {
		return SSCAN_NAME_ERROR;
	}

	*spi = c -  (char) '0';

	k = 0;
	i++;

	while ((buf[i] != (char) 0) && (k < (int) *len) && (buf[i] != (char) ',')) {
		name[k] = (char) tolower((int )buf[i]);
		k++;
		i++;
	}

	*len = (uint8_t) k;
	name[k] = (char) 0;

	if (buf[i] != (char) ',') {
		return SSCAN_NAME_ERROR;
	}

	k = 0;
	i++;

	while ((buf[i] != (char) 0) && (buf[i] != (char) ',') && (k < 2)) {
		if (!isxdigit((int)buf[i])) {
			return SSCAN_VALUE_ERROR;
		}
		tmp[k++] = buf[i++];
	}

	if ((k == 0) || (buf[i] != (char) ',')) {
		return SSCAN_VALUE_ERROR;
	}

	if (k == 2) {
		nibble_low = tmp[1] > '9' ? (tmp[1] | 0x20) - 'a' + 10 : tmp[1] - '0';
		nibble_high = (tmp[0] > '9' ? (tmp[0] | 0x20) - 'a' + 10 : tmp[0] - '0') << 4;
		*address = nibble_high | nibble_low;
	} else {
		nibble_low = tmp[0] > '9' ? (tmp[0] | 0x20) - 'a' + 10 : tmp[0] - '0';
		*address = nibble_low;
	}

	k = 0;
	i++;
	uint16 = 0;

	while ((buf[i] != (char) 0) && (buf[i] != (char) ',') && (k < 3)) {
		if (!isdigit((int)buf[i])) {
			return SSCAN_VALUE_ERROR;
		}
		uint16 = uint16 * (uint16_t)10 + (uint16_t)buf[i] - (uint16_t) '0';
		k++;
		i++;
	}

	if (k == 0 || (buf[i] != (char) ','))  {
		return SSCAN_VALUE_ERROR;
	}

	*dmx = uint16;

	k = 0;
	i++;
	uint32 = 0;

	while ((buf[i] != (char) 0) && (buf[i] != (char) '\n') && (k < 9) && (buf[i] != (char) ' ') && (buf[i] != (char) ':')) {
		if (!isdigit((int)buf[i])) {
			return SSCAN_VALUE_ERROR;
		}
		uint32 = uint32 * (uint32_t)10 + (uint32_t)buf[i] - (uint32_t) '0';
		k++;
		i++;
	}

	if ((k == 0) || ((buf[i] != (char) 0) && (buf[i] != (char) '\n') && (buf[i] != (char) ' ') && (buf[i] != (char) ':'))) {
		return SSCAN_VALUE_ERROR;
	}

	*speed = uint32;

	return SSCAN_OK;
}
