/**
 * @file sscanuint8.cpp
 *
 */
/* Copyright (C) 2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

Sscan::ReturnCode Sscan::Uint8(const char *pBuffer, const char *pName, uint8_t &nValue) {
	assert(pBuffer != nullptr);
	assert(pName != nullptr);

	const char *p;

	if ((p = checkName(pBuffer, pName)) == nullptr) {
		return Sscan::NAME_ERROR;
	}

	uint32_t k = 0;

	do {
		if (isdigit(*p) == 0) {
			return Sscan::VALUE_ERROR;
		}
		k = k * 10 + static_cast<uint32_t>(*p) - '0';
		p++;
	} while ((*p != ' ') && (*p != 0));

	if (k > static_cast<uint32_t>(static_cast<uint8_t>(~0))) {
		return Sscan::VALUE_ERROR;
	}

	nValue = static_cast<uint8_t>(k);

	return Sscan::OK;
}
