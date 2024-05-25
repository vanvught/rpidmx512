/**
 * @file showfile_filename.cpp
 *
 */
/* Copyright (C) 2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <cstdio>
#include <cstring>
#include <cctype>
#include <cassert>

#include "showfile.h"

#include "debug.h"

namespace showfile {
bool filename_copyto(char *pShowFileName, const uint32_t nLength, const uint32_t nShowFileNumber) {
	assert(nLength == showfile::FILE_NAME_LENGTH + 1);

	if (nShowFileNumber <= showfile::FILE_MAX_NUMBER) {
		snprintf(pShowFileName, nLength, "show%.2u.txt", static_cast<unsigned int>(nShowFileNumber));
		return true;
	}

	return false;
}

bool filename_check(const char *pShowFileName, uint32_t &nShowFileNumber) {
	DEBUG_PRINTF("pShowFileName=[%s]", pShowFileName);

	if ((pShowFileName == nullptr) || (strlen(pShowFileName) != showfile::FILE_NAME_LENGTH)) {
		DEBUG_EXIT
		return false;
	}

	if (memcmp(pShowFileName, SHOWFILE_PREFIX, sizeof(SHOWFILE_PREFIX) - 1) != 0) {
		DEBUG_EXIT
		return false;
	}

	if (memcmp(&pShowFileName[showfile::FILE_NAME_LENGTH - sizeof(SHOWFILE_SUFFIX) + 1], SHOWFILE_SUFFIX, sizeof(SHOWFILE_SUFFIX) - 1) != 0) {
		DEBUG_EXIT
		return false;
	}

	char cDigit = pShowFileName[sizeof(SHOWFILE_PREFIX) - 1];
	DEBUG_PRINTF("cDigit=%c", cDigit);

	if (!isdigit(cDigit)) {
		DEBUG_EXIT
		return false;
	}

	nShowFileNumber = static_cast<uint32_t>(10 * (cDigit - '0'));

	cDigit = pShowFileName[sizeof(SHOWFILE_PREFIX)];
	DEBUG_PRINTF("cDigit=%c", cDigit);

	if (!isdigit(cDigit)) {
		DEBUG_EXIT
		return false;
	}

	nShowFileNumber += static_cast<uint32_t>(cDigit - '0');

	DEBUG_EXIT
	return true;
}


}  // namespace showfile
