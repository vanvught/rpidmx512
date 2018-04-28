/**
 * @file sscan_i2c_address.c
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

int sscan_i2c_address(const char *buf, const char *name, uint8_t *address) {
	int k = 0;
	char *n = (char *)name;
	const char *b = buf;
	char tmp[8];
	uint8_t nibble_high;
	uint8_t nibble_low;

	assert(buf != NULL);
	assert(name != NULL);
	assert(address != NULL);

	tmp[0] = (char) 0;
	tmp[1] = (char) 0;

	while ((*n != (char) 0) && (*b != (char) 0)) {
		if (*n++ != *b++) {
			return SSCAN_NAME_ERROR;
		}
	}

	if (*n != (char) 0) {
		return SSCAN_NAME_ERROR;
	}

	if (*b++ != (char) '=') {
		return SSCAN_NAME_ERROR;
	}

	if ((*b == ' ') || (*b == (char) 0) || (*b == '\n')) {
		return SSCAN_VALUE_ERROR;
	}

	k = 0;


	while ((*b != (char) '\n') && (*b != (char) '\0') && (k < 2)) {
		if (isxdigit((int) *b) == 0) {
			return SSCAN_NAME_ERROR;
		}
		tmp[k] = *b;
		k++;
		b++;
	}

	if ((*b != (char) '\n') && (*b != (char) '\0') && (*b != (char) ' ')) {
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

	if (*address >= 0x7f) {
		return SSCAN_VALUE_ERROR;
	}

	return SSCAN_OK;
}
