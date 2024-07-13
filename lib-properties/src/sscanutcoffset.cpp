/**
 * @file sscanutcoffset.cpp
 *
 */
/* Copyright (C) 2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

Sscan::ReturnCode Sscan::UtcOffset(const char *pBuffer, const char *pName, int8_t& nHours, uint8_t& nMinutes) {
	assert(pBuffer != nullptr);
	assert(pName != nullptr);

	const char *p;

	if ((p = checkName(pBuffer, pName)) == nullptr) {
		return Sscan::NAME_ERROR;
	}

	auto bIsNegatieve = false;

	if (*p == '-') {
		p++;
		bIsNegatieve = true;
	} else if (*p == '+') {
		p++;
	}

	if ((*p == ' ') || (*p == 0)) {
		return Sscan::VALUE_ERROR;
	}

	nHours = 0;

	if ((*p == '0') || (*p == '1')) {
		if (*p == '1') {
			nHours = 10;
		}
		p++;
	} else {
		return Sscan::VALUE_ERROR;
	}

	if (isdigit(*p) == 0) {
		return Sscan::VALUE_ERROR;
	}

	nHours = static_cast<int8_t>(nHours + (*p - '0'));

	p++;

	if (*p != ':') {
		return Sscan::VALUE_ERROR;
	}

	p++;

	if ((isdigit(p[0]) == 0) || (isdigit(p[1]) == 0)) {
		return Sscan::VALUE_ERROR;
	}

	if ((p[2] == ' ') || (p[2] == 0)) {
		nMinutes = static_cast<uint8_t>((p[0] - '0') * 10);
		nMinutes = static_cast<uint8_t>(nMinutes + (p[1] - '0'));

		if (bIsNegatieve) {
			nHours = -nHours;
		}

		return Sscan::OK;
	}

	return Sscan::VALUE_ERROR;
}
