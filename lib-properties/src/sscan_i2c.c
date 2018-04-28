/**
 * @file scan_i2c.c
 *
 */
/* Copyright (C) 2017-2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#include <stddef.h>
#include <ctype.h>
#include <assert.h>

#include "sscan.h"

int sscan_i2c(const char *buf, char *name, uint8_t *len, uint8_t *address, uint8_t *channel) {
	int k = 0;
	char *n = name;
	const char *b = buf;
	char tmp[8];
	uint8_t nibble_high;
	uint8_t nibble_low;
	uint8_t uint8;

	assert(buf != NULL);
	assert(name != NULL);
	assert(len != NULL);
	assert(address != NULL);
	assert(channel != NULL);

	tmp[0] = (char) 0;
	tmp[1] = (char) 0;

	k = 0;

	while ((*b != (char) 0) && (*b != (char) ',') && (k < (int) *len)) {
		*n++ = *b++;
		k++;
	}

	*len = (uint8_t) k;
	*n = (char) SSCAN_NAME_ERROR;


	b++;
	k = 0;

	while ((*b != (char) '\n') && (*b != (char) '\0') && (*b != (char) ',') && (k < 2)) {
		if (isxdigit((int) *b) == 0) {
			return SSCAN_NAME_ERROR;
		}
		tmp[k] = *b;
		k++;
		b++;
	}

	if ((*b != (char) ':') && (*b != (char) '\n') && (*b != (char) '\0') && (*b != (char) ' ')) {
		return SSCAN_NAME_ERROR;
	}

	if (k == 2) {
		nibble_low = (uint8_t) (tmp[1] > '9' ? (tmp[1] | (char) 0x20) - 'a' + (char) 10 : tmp[1] - '0');
		nibble_high = (uint8_t) (tmp[0] > '9' ? (tmp[0] | (char) 0x20) - 'a' + (char) 10 : tmp[0] - '0') << 4;
		*address = nibble_high | nibble_low;
	} else {
		nibble_low = (uint8_t) (tmp[0] > '9' ? (tmp[0] | (char) 0x20) - 'a' + (char) 10 : tmp[0] - '0');
		*address = nibble_low;
	}

	*channel = (uint8_t) 0;

	if (*b++ != (char) ':') {
		return 1;
	}

	if ((*b == (char) '\n') || (*b == (char) '\0') || (*b == (char) ' ')) {
		return 1;
	}

	if (isdigit((int) *b) == 0) {
		return 0;
	}

	uint8 = (uint8_t) *b - (uint8_t) '0';

	if (uint8 > (uint8_t) 7) {
		return 0;
	}

	*channel = uint8;

	return 2;
}
