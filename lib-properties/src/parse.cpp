/**
 * @file parse.cpp
 *
 */
/* Copyright (C) 2018-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <cstdint>
#include <cctype>
#include <cassert>

#include "parse.h"

char *Parse::DmxSlotInfo(char *s, bool &isValid, uint8_t &nType, uint16_t &nCategory) {
	assert(s != nullptr);

	char *b = s;
	uint8_t i = 0;

	uint16_t nTmp = 0;

	while ((i < 2) && (*b != ':')) {
		if (isxdigit(static_cast<int>(*b)) == 0) {
			isValid = false;
			return nullptr;
		}

		uint8_t nibble = *b > '9' ?  static_cast<uint8_t> ((*b | 0x20) - 'a' + 10) :  static_cast<uint8_t> (*b - '0');
		nTmp = static_cast<uint16_t>((nTmp << 4) | nibble);
		b++;
		i++;
	}

	if ((i != 2) && (*b != ':')) {
		isValid = false;
		return nullptr;
	}

	nType = static_cast<uint8_t>(nTmp);

	i = 0;
	nTmp = 0;

	b++;

	while ((i < 4) && (*b != ',') && (*b != '\0')) {
		if (isxdigit(static_cast<int>(*b)) == 0) {
			isValid = false;
			return nullptr;
		}

		uint8_t nibble = *b > '9' ?  static_cast<uint8_t> ((*b | 0x20) - 'a' + 10) :  static_cast<uint8_t> (*b - '0');
		nTmp = static_cast<uint16_t>((nTmp << 4) | nibble);
		b++;
		i++;
	}

	if (i != 4) {
		isValid = false;
		return nullptr;
	}

	if ((*b != ',') && (*b != ' ') && (*b != '\0')) {
		isValid = false;
		return nullptr;
	}

	nCategory = nTmp;

	isValid = true;

	if (*b == '\0') {
		return nullptr;
	}

	return ++b;
}
