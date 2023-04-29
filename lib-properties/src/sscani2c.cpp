/**
 * @file sscani2c.cpp
 *
 */
/* Copyright (C) 2020-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

Sscan::ReturnCode Sscan::I2c(const char *pBuffer, char *pName, uint8_t &nLength, uint8_t &nAddress, uint8_t &nReserved) {
	assert(pBuffer != nullptr);
	assert(pName != nullptr);

	uint32_t k = 0;

	const char *b = pBuffer;
	char *n = pName;

	while ((*b != 0) && (*b != '=') && (k < nLength)) {
		*n++ = *b++;
		k++;
	}

	nLength = static_cast<uint8_t>(k);

	if ((*b != 0) && (*b != '=')) {
		return Sscan::NAME_ERROR;
	}

	char tmp[2];

	b++;
	k = 0;
	tmp[1] = 0;

	while ((*b != '\0') && (*b != ':') && (k < 2)) {
		if (isxdigit(*b) == 0) {
			return Sscan::VALUE_ERROR;
		}
		tmp[k++] = *b++;
	}

	if (k == 0) {
		return Sscan::VALUE_ERROR;
	}

	nAddress = fromHex(tmp);
	nReserved = 0xFF;

	if ((*b == 0) || (*b == ' ')) {
		return Sscan::OK;
	}

	if (*b++ != ':') {
		return Sscan::VALUE_ERROR;
	}

	if (isdigit(*b) == 0) {
		return Sscan::VALUE_ERROR;
	}

	const auto uint8 = static_cast<uint8_t>(*b - '0');

	if (uint8 > 7) {
		return Sscan::VALUE_ERROR;
	}

	nReserved = uint8;

	return Sscan::OK;
}
