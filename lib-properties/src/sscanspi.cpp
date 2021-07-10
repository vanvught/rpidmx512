/**
 * @file sscanspi.cpp
 *
 */
/* Copyright (C) 2020-2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#if !defined(__clang__)	// Needed for compiling on MacOS
# pragma GCC push_options
# pragma GCC optimize ("Os")
#endif

#include <cstdint>
#include <cctype>
#include <cassert>

#include "sscan.h"

Sscan::ReturnCode Sscan::Spi(const char *pBuffer, char &nChipSelect, char *pName, uint8_t &nLength, uint8_t &nAddress, uint16_t &nDmxStartAddress, uint32_t &nSpeedHz) {
	assert(pBuffer != nullptr);
	assert(pName != nullptr);

	const char *b = pBuffer;
	const char c = *b++;

	if (!isdigit(c) && (*b != ',')) {
		return Sscan::VALUE_ERROR;
	}

	nChipSelect = static_cast<char>(c - '0');

	uint32_t k = 0;
	b++;

	while ((*b != 0) && (k < nLength) && (*b != ',')) {
		pName[k++] = *b++;
	}

	nLength = static_cast<uint8_t>(k);
	pName[k] = 0;

	if (*b != ',') {
		return Sscan::NAME_ERROR;
	}

	char tmp[2];

	k = 0;
	b++;
	tmp[1] = 0;

	while ((*b != 0) && (*b != ',') && (k < 2)) {
		if (!isxdigit(*b)) {
			return Sscan::VALUE_ERROR;
		}
		tmp[k++] = *b++;
	}

	if ((k == 0) || (*b != ',')) {
		return Sscan::VALUE_ERROR;
	}

	nAddress = fromHex(tmp);

	k = 0;
	b++;
	uint16_t uint16 = 0;

	while ((*b != 0) && (*b != ',') && (k < 3)) {
		if (!isdigit(*b)) {
			return Sscan::VALUE_ERROR;
		}
		uint16 = static_cast<uint16_t>((uint16 * 10) + (*b - '0'));
		k++;
		b++;
	}

	if (k == 0 || (*b != ',')) {
		return Sscan::VALUE_ERROR;
	}

	nDmxStartAddress = uint16;

	if (uint16 > 512) {
		return Sscan::VALUE_ERROR;
	}

	k = 0;
	b++;
	uint32_t uint32 = 0;

	while ((*b != 0) && (k < 9) && (*b != ' ')) {
		if (!isdigit(*b)) {
			return Sscan::VALUE_ERROR;
		}
		uint32 = uint32 * 10 + static_cast<uint32_t>(*b - '0');
		k++;
		b++;
	}

	if ((k == 0) || ((*b != 0) && (*b != ' '))) {
		return Sscan::VALUE_ERROR;
	}

	nSpeedHz = uint32;

	return Sscan::OK;
}
