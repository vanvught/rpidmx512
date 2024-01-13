/**
 * @file oscstring.h
 *
 */
/* Copyright (C) 2016-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef OSCSTRING_H_
#define OSCSTRING_H_

#include <cstring>

#include "osc.h"

/*
 * OSC-string
 * A sequence of non-null ASCII characters followed by a null,
 * followed by 0-3 additional null characters to make the total number of bits a multiple of 32.
 */

namespace osc {
inline static int string_validate(void *pData, unsigned nSize) {
	unsigned nLength = 0;
	char *pSrc = reinterpret_cast<char*>(pData);

	unsigned i = 0;

	for (i = 0; i < nSize; ++i) {
		if (pSrc[i] == '\0') {
			nLength = 4 * (i / 4 + 1);
			break;
		}
	}

	if (0 == nLength) {
		return -osc::validate::NOT_TERMINATED;
	}

	if (nLength > nSize) {
		return -osc::validate::INVALID_SIZE;
	}

	for (; i < nLength; ++i) {
		if (pSrc[i] != '\0') {
			return -osc::validate::NONE_ZERO_IN_PADDING;
		}
	}

	return static_cast<int>(nLength);
}

inline static unsigned string_size(const char *pString) {
	return 4 * (strlen(pString) / 4 + 1);
}
}  // namespace osc

#endif /* OSCSTRING_H_ */
