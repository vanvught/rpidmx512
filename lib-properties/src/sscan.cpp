/**
 * @file sscan.cpp
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

#include <cstdint>

#include "sscan.h"

uint8_t Sscan::fromHex(const char Hex[2]) {
		if (Hex[1] != 0) {
			const int nLow = (Hex[1] > '9' ? (Hex[1] | 0x20) - 'a' + 10 : Hex[1] - '0');
			const int nHigh = (Hex[0] > '9' ? (Hex[0] | 0x20) - 'a' + 10 : Hex[0] - '0');
			return static_cast<uint8_t>((nHigh << 4) | nLow);
		} else {
			return static_cast<uint8_t>(Hex[0] > '9' ? (Hex[0] | 0x20) - 'a' + 10 : Hex[0] - '0');
		}
}

const char *Sscan::checkName(const char *pBuffer, const char *pName) {
		while ((*pName != 0) && (*pBuffer != 0)) {
			if (*pName++ != *pBuffer++) {
				return nullptr;
			}
		}

		if (*pName != 0) {
			return nullptr;
		}

		if (*pBuffer++ != '=') {
			return nullptr;
		}

		if ((*pBuffer == ' ') || (*pBuffer == 0)) {
			return nullptr;
		}

		return pBuffer;
}
