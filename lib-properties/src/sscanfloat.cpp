/**
 * @file sscanfloat.cpp
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

Sscan::ReturnCode Sscan::Float(const char *pBuffer, const char *pName, float &fValue) {
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
	}

	if ((*p == ' ') || (*p == 0)) {
		return Sscan::VALUE_ERROR;
	}

	uint32_t k = 0;

	do {
		if (isdigit(*p) == 0) {
			return Sscan::VALUE_ERROR;
		}
		k = k * 10 + static_cast<uint32_t>(*p - '0');
		p++;
	} while ((*p != '.') && (*p != ' ') && (*p != 0));

	fValue = static_cast<float>(k);

	if (*p != '.') {
		if (bIsNegatieve) {
			fValue = -fValue;
		}
		return Sscan::OK;
	}

	k = 0;
	p++;
	uint32_t div = 1;

	while ((*p != ' ') && (*p != 0)) {
		if (isdigit(*p) == 0) {
			return Sscan::VALUE_ERROR;
		}
		k = k * 10 + static_cast<uint32_t>(*p - '0');
		div *= 10;
		p++;
	}

	fValue += (static_cast<float>(k) / static_cast<float>(div));

	if (bIsNegatieve) {
		fValue = -fValue;
	}

	return Sscan::OK;
}
