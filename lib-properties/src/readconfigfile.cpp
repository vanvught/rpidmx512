/**
 * @file readconfigfile.cpp
 */
/* Copyright (C) 2017-2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <cstddef>
#include <cstdio>
#include <cstring>
#include <cassert>

#include "readconfigfile.h"

#include "debug.h"

//TODO Check this with GD32 64->128
static constexpr auto MAX_LINE_LENGTH = 128;	// Including '\0'

ReadConfigFile::ReadConfigFile(CallbackFunctionPtr callBack, void *p) {
	assert(callBack != nullptr);
	assert(p != nullptr);

    m_pCallBack = callBack;
    m_p = p;
}

ReadConfigFile::~ReadConfigFile() {
    m_pCallBack = nullptr;
    m_p = nullptr;
}

#if !defined(DISABLE_FS)
bool ReadConfigFile::Read(const char *pFileName) {
	assert(pFileName != nullptr);

	char buffer[MAX_LINE_LENGTH];

	FILE *fp = fopen(pFileName, "r");

	if (fp != nullptr) {
		for (;;) {
			if (fgets(buffer, static_cast<int>(sizeof(buffer)) - 1, fp) != buffer) {
				break; // Error or end of file
			}

			if (buffer[0] >= 'a') {
				char *q = buffer;

				for (unsigned i = 0; (i < sizeof(buffer) - 1) && (*q != '\0'); i++) {
					if ((*q == '\r') || (*q == '\n')) {
						*q = '\0';
					}
					q++;
				}

				m_pCallBack(m_p, buffer);
			}
		}

		fclose(fp);
	} else {
		return false;
	}

	return true;
}
#endif

void ReadConfigFile::Read(const char *pBuffer, unsigned nLength) {
	DEBUG_ENTRY

	assert(pBuffer != nullptr);
	assert(nLength != 0);

	const auto *pSrc = const_cast<char *>(pBuffer);
	char buffer[MAX_LINE_LENGTH];
	buffer[0] = '\n';

	debug_dump(pBuffer, nLength);

	while (nLength != 0) {
		char *pLine = &buffer[0];

		while ((nLength != 0) && (*pSrc != '\r') && (*pSrc != '\n')) {
			*pLine++ = *pSrc++;

			if ((pLine - buffer) >= MAX_LINE_LENGTH) {
				DEBUG_PRINTF("%128s", &buffer[0]);
				assert(0);
				return;
			}

			nLength--;
		}

		while ((nLength != 0) && ((*pSrc == '\r') || (*pSrc == '\n'))) {
			pSrc++;
			nLength--;
		}

		if (buffer[0] >= '0') {
			*pLine = '\0';
			DEBUG_PUTS(&buffer[0]);
			m_pCallBack(m_p, &buffer[0]);
		}
	}

	DEBUG_EXIT
}
