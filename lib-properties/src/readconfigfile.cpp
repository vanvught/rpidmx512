/**
 * @file readconfigfile.cpp
 */
/* Copyright (C) 2017-2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "readconfigfile.h"

ReadConfigFile::ReadConfigFile(CallbackFunctionPtr cb, void *p) {
	assert(cb != 0);
	assert(p != 0);

    m_cb = cb;
    m_p = p;
}

ReadConfigFile::~ReadConfigFile(void) {
    m_cb = 0;
    m_p = 0;
}

bool ReadConfigFile::Read(const char *pFileName) {
	char buffer[128];
	FILE *fp;

	assert(pFileName != 0);

	fp = fopen(pFileName, "r");

	if (fp != NULL) {
		for (;;) {
			if (fgets(buffer, (int) sizeof(buffer) - 1, fp) != buffer) {
				break; // Error or end of file
			}

			if (buffer[0] >= 'a') {
				char *q = (char *) buffer;

				for (unsigned i = 0; (i < sizeof(buffer) - 1) && (*q != '\0'); i++) {
					if ((*q == '\r') || (*q == '\n')) {
						*q = '\0';
					}
					q++;
				}

				(void) m_cb(m_p, (const char *) buffer);
			}
		}
		(void) fclose(fp);
	} else {
		return false;
	}

	return true;
}

void ReadConfigFile::Read(const char* pBuffer, unsigned nLength) {
	assert(pBuffer != 0);
	assert(nLength != 0);

	char *p = new char[nLength + 1];
	char *pFree = p;
	memcpy(p, pBuffer, nLength);
	p[nLength] = '\0';

#ifndef NDEBUG
		printf("%s:%d [%s]\n", __FUNCTION__, __LINE__, p);
#endif

	while (nLength != 0) {
		char *pLine = (char *) p;

		while ((nLength != 0) && (*p != '\r') && (*p != '\n')) {
			p++;
			nLength--;
		}

		while ((nLength != 0) && ((*p == '\r') || (*p == '\n'))) {
			*p++ = '\0';
			nLength--;
		}

#ifndef NDEBUG
		printf("%s:%d [%s]\n", __FUNCTION__, __LINE__, pLine);
#endif

		if (*pLine >= 'a') {
			(void) m_cb(m_p, (const char *) pLine);
		}
	}

	delete [] pFree;
}
