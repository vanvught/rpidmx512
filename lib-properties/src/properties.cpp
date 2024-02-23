/**
 * @file properties.cpp
 *
 */
/* Copyright (C) 2021-2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <cassert>
#include <ctype.h>

#include "debug.h"

namespace properties {
int convert_json_file(char *pBuffer, uint32_t nLength, const bool bSkipFileName) {
	DEBUG_ENTRY
	assert(pBuffer != nullptr);
	assert(nLength > 1);

	debug_dump(pBuffer, static_cast<uint16_t>(nLength));

	const auto *pSrc = pBuffer;
	auto *pDst = pBuffer;

	if (pSrc[0] != '{') {
		DEBUG_EXIT
		return -1;
	}

	uint32_t nNewLength = 1;
	uint32_t i = 1;

	if (!bSkipFileName) {
		pDst[0] = '#';
		pDst++;
		pSrc++;

		for (;i < nLength; i++) {
			if (*pSrc++ == '"') {
				break;
			}
		}

		// File name
		for (; (i < nLength) && (*pSrc != '"'); i++) {
			*pDst++ = *pSrc++;
			nNewLength++;
		}

		*pDst++ = '\n';
		nNewLength++;

		while ((*pSrc != '{') &&  (i++ < nLength)) {
			pSrc++;
		}
	} else {
		while ((*pSrc != '{') &&  (i++ < nLength)) {
			pSrc++;
		}
	}

	for (; i < nLength; i++) {
		// Name
		while (i++ < nLength) {
			if (*pSrc++ == '"') {
				break;
			}
		}

		while ((*pSrc != '"') &&  (i++ < nLength)) {
			*pDst++ = *pSrc++;
			nNewLength++;
		}


		if (*pSrc == '"') {
			*pDst++ = '=';
			nNewLength++;
		}

		// Value
		while (i++ < nLength) {
			if (*pSrc++ == ':') {
				break;
			}
		}

		while ((*pSrc == ' ') && (i++ < nLength)) {
			pSrc++;
		}

		if (*pSrc == '"') {
			pSrc++;
		}

		if (*pSrc == '"') {
			pSrc++;
		}

		while ((*pSrc != '"') && (*pSrc != ',') && (*pSrc != '}') &&  (i++ < nLength)) {
			const int c = *pSrc;
			if ( ((c >= static_cast<int>(' ') && c <= static_cast<int>('z')))) {
				*pDst++ = *pSrc++;
				nNewLength++;
			}
		}

		if ((*pSrc == '"') || (*pSrc == ',') || (*pSrc == '}')) {
			*pDst++ = '\n';
			nNewLength++;
		}

		pSrc++;
	}

	if (bSkipFileName) {
		nNewLength--;
	}

	debug_dump(pBuffer, static_cast<uint16_t>(nNewLength));
	DEBUG_EXIT
	return static_cast<int>(nNewLength);
}
}  // namespace properties
