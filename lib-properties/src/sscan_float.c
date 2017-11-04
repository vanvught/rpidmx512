/**
 * @file sscan_float.c
 *
 */
/* Copyright (C) 2017 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#include <stddef.h>
#include <ctype.h>
#include <assert.h>

#include "sscan.h"

int sscan_float(const char *buf, const char *name, float *value) {
	float k, f;
	uint32_t div;
	bool is_negative = false;

	const char *n = name;
	const char *b = buf;

	assert(buf != NULL);
	assert(name != NULL);
	assert(value != NULL);

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

	if (*b == (char) '-') {
		b++;
		is_negative = true;
	}

	if ((*b == ' ') || (*b == (char) 0) || (*b == '\n')) {
		return SSCAN_VALUE_ERROR;
	}

	k = 0;

	do {
		if (isdigit((int) *b) == 0) {
			return SSCAN_VALUE_ERROR;
		}
		k = k * 10 + (float) *b - (float) '0';
		b++;
	} while ((*b != '.') && (*b != ' ') && (*b != (char) 0) && (*b != '\n'));

	if (*b != '.') {
		if (is_negative) {
			*value = (float) 0 - k;
		} else {
			*value = k;
		}
		return SSCAN_OK;
	}

	f = k;

	k = 0;
	div = 1;
	b++;

	while ((*b != ' ') && (*b != (char) 0) && (*b != '\n')) {
		if (isdigit((int) *b) == 0) {
			return SSCAN_VALUE_ERROR;
		}
		k = k * 10 + (float) *b - (float) '0';
		div = div * 10;
		b++;
	}

	f = f + (k / div);

	if (is_negative) {
		*value = (float) 0 - f;
	} else {
		*value = f;
	}

	return SSCAN_OK;
}
