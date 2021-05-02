/**
 * @file sscanipaddress.cpp
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

typedef union pcast32 {
	uint32_t u32;
	uint8_t u8[4];
} _pcast32;

Sscan::ReturnCode Sscan::IpAddress(const char *pBuffer, const char *pName, uint32_t& nIpAddress) {
	assert(pBuffer != nullptr);
	assert(pName != nullptr);

	_pcast32 cast32;

	const char *p;

	if ((p = Sscan::checkName(pBuffer, pName)) == nullptr) {
		 return Sscan::NAME_ERROR;
	}

	uint32_t i, j, k;

	for (i = 0; i < 3; ++i) {
		j = 0;
		k = 0;

		while ((*p != '.') && (*p != 0)) {
			if (j == 3) {
				return Sscan::VALUE_ERROR;
			}

			if (isdigit(*p) == 0) {
				return Sscan::VALUE_ERROR;
			}

			j++;
			k = k * 10 + static_cast<uint32_t>(*p) - '0';
			p++;
		}

		if (k > 255) {
			return Sscan::VALUE_ERROR;
		}

		cast32.u8[i] = static_cast<uint8_t>(k);
		p++;
	}

	j = 0;
	k = 0;

	while ((*p != ' ') && (*p != 0)) {
		if (j == 3) {
			return Sscan::VALUE_ERROR;
		}

		if (isdigit(*p) == 0) {
			return Sscan::VALUE_ERROR;
		}

		j++;
		k = k * 10 + static_cast<uint32_t>(*p) - '0';
		p++;
	}

	if (k > 255) {
		return Sscan::VALUE_ERROR;
	}

	cast32.u8[i] = static_cast<uint8_t>(k);

	nIpAddress = cast32.u32;

	return Sscan::OK;
}
