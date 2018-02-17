/**
 * @file parse.cpp
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
#include <ctype.h>
#include <assert.h>

#include "parse.h"

char* Parse::DmxSlotInfo(char* s, bool& isValid, uint8_t& nType, uint16_t& nCategory) {
	assert(s != 0);

	char *b = s;
	uint8_t i = 0;

	uint16_t nTmp = 0;

	while ((i < 2) && (*b != ':')) {
		if (isxdigit((int)*b) == 0) {
			isValid = false;
			return 0;
		}

		uint8_t nibble = *b > '9' ? ((uint8_t) *b | (uint8_t) 0x20) - (uint8_t) 'a' + (uint8_t) 10 : (uint8_t) (*b - '0');
		nTmp = (nTmp << 4) | nibble;
		b++;
		i++;
	}

	if ((i != 2) && (*b != ':')) {
		isValid = false;
		return 0;
	}

	nType = nTmp;

	i = 0;
	nTmp = 0;

	b++;

	while ((i < 4) && (*b != ',') && (*b != '\0')) {
		if (isxdigit((int)*b) == 0) {
			isValid = false;
			return 0;
		}

		uint8_t nibble = *b > '9' ? ((uint8_t) *b | (uint8_t) 0x20) - (uint8_t) 'a' + (uint8_t) 10 : (uint8_t) (*b - '0');
		nTmp = (nTmp << 4) | nibble;
		b++;
		i++;
	}

	if (i != 4) {
		isValid = false;
		return 0;
	}

	if ((*b != ',') && (*b != ' ') && (*b != '\n') && (*b != '\0')) {
		isValid = false;
		return 0;
	}

	nCategory = nTmp;

	isValid = true;

	if ((*b == '\0') || (*b == '\n')) {
		return 0;
	}

	return ++b;
}
