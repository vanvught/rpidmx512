/**
 * @file sscanchar.cpp
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
#include <cassert>

#include "sscan.h"

Sscan::ReturnCode Sscan::Char(const char *pBuffer, const char *pName, char *pValue, uint32_t& nLength) {
	assert(pBuffer != nullptr);
	assert(pName != nullptr);
	assert(pValue != nullptr);

	const char *p;

	if ((p = Sscan::checkName(pBuffer, pName)) == nullptr) {
		return Sscan::NAME_ERROR;
	}

	uint16_t k = 0;

	while ((*p != 0) && (k < nLength)) {
		*pValue++ = *p++;
		k++;
	}

	if ((k < nLength) || (*p == '\0') || (*p == '\n')) {
		nLength = k;
		return Sscan::OK;
	}

	return Sscan::VALUE_ERROR;
}

