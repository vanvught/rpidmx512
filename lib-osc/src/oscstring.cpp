/**
 * @file oscstring.cpp
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

#include <stdint.h>
#include <assert.h>

#ifdef __circle__
#include <circle/util.h>
#include "circle/oscutil.h"
#elif defined(__linux__) || defined (__CYGWIN__)
#include <string.h>
#else
#include "util.h"
#endif

#include "oscstring.h"
#include "osc.h"

unsigned OSCString::Validate(void *data, unsigned size) {
	unsigned i = 0, len = 0;
	char *pos = (char *) data;

	if (size < 0) {
		return -OSC_STRING_INVALID_SIZE;
	}

	for (i = 0; i < size; ++i) {
		if (pos[i] == '\0') {
			len = 4 * (i / 4 + 1);
			break;
		}
	}

	if (0 == len) {
		return -OSC_STRING_NOT_TERMINATED;
	}

	if (len > size) {
		return -OSC_STRING_INVALID_SIZE;
	}

	for (; i < len; ++i) {
		if (pos[i] != '\0') {
			return -OSC_STRING_NONE_ZERO_IN_PADDING;
		}
	}

	return len;
}

/**
 * @brief A function to calculate the amount of OSC message space required by a C char *.
 *
 * @param s
 *
 * @return Returns the storage size in bytes, which will always be a multiple of four.
 */
unsigned OSCString::Size(const char *s) {
	return 4 * (strlen(s) / 4 + 1);
}
