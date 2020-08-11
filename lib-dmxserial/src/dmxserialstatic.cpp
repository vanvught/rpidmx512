/**
 * @file dmxserialstatic.cpp
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

#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <cassert>

#include "dmxserial.h"

#include "debug.h"

bool DmxSerial::FileNameCopyTo(char *pFileName, uint32_t nLength, int16_t nFileNumber) {
	assert(nLength == DmxSerialFile::NAME_LENGTH + 1);

	if ((nFileNumber >= DmxSerialFile::MIN_NUMBER) && (nFileNumber <= DmxSerialFile::MAX_NUMBER)) {
		snprintf(pFileName, nLength, DMXSERIAL_FILE_PREFIX "%.3d" DMXSERIAL_FILE_SUFFIX, nFileNumber);
		return true;
	}

	return false;
}

bool DmxSerial::CheckFileName(const char *pFileName, int16_t &nFileNumber) {
	DEBUG_PRINTF("pFileName=[%s]", pFileName);

	if ((pFileName == nullptr) || (strlen(pFileName) != DmxSerialFile::NAME_LENGTH)) {
		DEBUG_EXIT
		return false;
	}

	if (memcmp(pFileName, DMXSERIAL_FILE_PREFIX, sizeof(DMXSERIAL_FILE_PREFIX) - 1) != 0) {
		DEBUG_EXIT
		return false;
	}

	if (memcmp(&pFileName[DmxSerialFile::NAME_LENGTH - sizeof(DMXSERIAL_FILE_SUFFIX) + 1], DMXSERIAL_FILE_SUFFIX, sizeof(DMXSERIAL_FILE_SUFFIX) - 1) != 0) {
		DEBUG_EXIT
		return false;
	}

	char cDigit = pFileName[sizeof(DMXSERIAL_FILE_PREFIX) - 1];

	if (!isdigit(cDigit)) {
		DEBUG_EXIT
		return false;
	}

	nFileNumber = 100 * (cDigit - '0');

	if (nFileNumber > DmxSerialFile::MAX_NUMBER) {
		DEBUG_EXIT
		return false;
	}

	cDigit = pFileName[sizeof(DMXSERIAL_FILE_PREFIX)];

	if (!isdigit(cDigit)) {
		DEBUG_EXIT
		return false;
	}

	nFileNumber += (10 * (cDigit - '0'));

	cDigit = pFileName[sizeof(DMXSERIAL_FILE_PREFIX) + 1];

	if (!isdigit(cDigit)) {
		DEBUG_EXIT
		return false;
	}

	nFileNumber += (cDigit - '0');

	if (nFileNumber > DmxSerialFile::MAX_NUMBER) {
		DEBUG_EXIT
		return false;
	}

	DEBUG_EXIT
	return true;
}
