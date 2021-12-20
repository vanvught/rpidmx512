/**
 * @file get_file_content.cpp
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

#include <cstdint>
#include <cstring>

#include "debug.h"

#include "../http/content/content.h"

int get_file_content(const char *fileName, char *pDst) {
	for (uint32_t i = 0; i < sizeof(HttpContent) / sizeof(HttpContent[0]) ; i++) {
		if (strcmp(fileName, HttpContent[i].pFileName) == 0) {
			const auto *pSrc =  HttpContent[i].pContent;
			auto *s = pDst;

			while ((*s++ = *pSrc++) != '\0')
				;

			const int nBytes = s - pDst - 1;

			DEBUG_PRINTF("%s -> %d", HttpContent[i].pFileName, nBytes);
			return nBytes;
		}
	}

	DEBUG_EXIT
	return -1;
}
