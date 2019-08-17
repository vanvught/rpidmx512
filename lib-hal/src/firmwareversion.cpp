/**
 * @file firmwareversion.cpp
 *
 */
/* Copyright (C) 2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#include <stdio.h>
#include <assert.h>

#include <firmwareversion.h>

#include "hardware.h"

FirmwareVersion *FirmwareVersion::s_pThis = 0;

FirmwareVersion::FirmwareVersion(const char* pVersion, const char* pDate, const char* pTime) {
	assert(pVersion != 0);
	assert(pDate != 0);
	assert(pTime != 0);

	s_pThis = this;

	memcpy(m_tFirmwareVersion.SoftwareVersion, pVersion, SOFTWARE_VERSION_LENGTH);
	memcpy(m_tFirmwareVersion.BuildDate, pDate, GCC_DATE_LENGTH);
	memcpy(m_tFirmwareVersion.BuildTime, pTime, GCC_TIME_LENGTH);

	uint8_t nHwTextLength;

	assert((uint32_t) Hardware::Get() != 0);

	snprintf(m_aPrint, sizeof m_aPrint - 1, "[V%.*s] %s Compiled on %.*s at %.*s\n",
			SOFTWARE_VERSION_LENGTH, m_tFirmwareVersion.SoftwareVersion,
			Hardware::Get()->GetBoardName(nHwTextLength),
			GCC_DATE_LENGTH, m_tFirmwareVersion.BuildDate,
			GCC_TIME_LENGTH, m_tFirmwareVersion.BuildTime);
}

FirmwareVersion::~FirmwareVersion(void) {
}

void FirmwareVersion::Print(void) {
	printf("%s", m_aPrint);
}
