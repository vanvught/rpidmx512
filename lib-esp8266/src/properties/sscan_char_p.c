/**
 * @file sscan_char_p.c
 *
 */
/* Copyright (C) 2016-2019 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <assert.h>

#include "c/sscan.h"

extern char *get_name(const char *buf, const char *name);

int sscan_char_p(const char *buf, const char *name, char *value, uint8_t *len) {
	assert(buf != NULL);
	assert(name != NULL);
	assert(value != NULL);
	assert(len != NULL);

	char *b;
	int k;
	char *v = value;

	if ((b = get_name(buf, name)) == NULL) {
		return SSCAN_NAME_ERROR;
	}

	k = 0;

	while ((*b != (char) 0) && (k < (int) *len)) {
		*v++ = *b++;
		k++;
	}

	if ((k < (int) *len) || (*b == '\0') || (*b == '\n')) {
		*len = (uint8_t) k;
		return SSCAN_OK;
	}

	return SSCAN_VALUE_ERROR;
}

