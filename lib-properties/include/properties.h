/**
 * @file properties.h
 *
 */
/* Copyright (C) 2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef PROPERTIES_H_
#define PROPERTIES_H_

#include <cstdint>

namespace properties {
int convert_json_file(char *pBuffer, uint16_t nLength) {
	assert(pBuffer != nullptr);
	assert(nLength > 1);

	auto *pSrc = pBuffer;
	auto *pDst = pBuffer;

	if (pSrc[0] != '{') {
		return -1;
	}

	uint16_t nNewLength = 1;

	pDst[0] = '#';
	pDst++;
	pSrc++;

	uint16_t i;

	for (i = 1; i < nLength; i++) {
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

		while ((*pSrc < '0') && (i++ < nLength)) {
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

	return nNewLength;
}
}  // namespace properties

#endif /* PROPERTIES_H_ */
