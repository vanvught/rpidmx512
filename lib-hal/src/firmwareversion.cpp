/**
 * @file firmwareversion.cpp
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

#if !defined(__clang__)	// Needed for compiling on MacOS
# pragma GCC push_options
# pragma GCC optimize ("Os")
#endif

#include <cstdint>
#include <cstring>
#include <cstdio>
#include <cassert>

#include "firmwareversion.h"

#include "hardware.h"

#include "debug.h"

using namespace firmwareversion;

FirmwareVersion *FirmwareVersion::s_pThis = nullptr;

FirmwareVersion::FirmwareVersion(const char *pVersion, const char *pDate, const char *pTime) {
	DEBUG_ENTRY

	assert(pVersion != nullptr);
	assert(pDate != nullptr);
	assert(pTime != nullptr);

	assert(s_pThis == nullptr);
	s_pThis = this;

	memcpy(m_tFirmwareVersion.SoftwareVersion, pVersion, length::SOFTWARE_VERSION);
	memcpy(m_tFirmwareVersion.BuildDate, pDate, length::GCC_DATE);
	memcpy(m_tFirmwareVersion.BuildTime, pTime, length::GCC_TIME);

	uint8_t nHwTextLength;

	snprintf(m_aPrint, sizeof m_aPrint - 1, "[V%.*s] %s Compiled on %.*s at %.*s",
			length::SOFTWARE_VERSION, m_tFirmwareVersion.SoftwareVersion,
			Hardware::Get()->GetBoardName(nHwTextLength),
			length::GCC_DATE, m_tFirmwareVersion.BuildDate,
			length::GCC_TIME, m_tFirmwareVersion.BuildTime);

	DEBUG_EXIT
}

void FirmwareVersion::Print(const char *pTitle) {
	printf("%s\n", m_aPrint);

	if (pTitle != nullptr) {
		puts(pTitle);
	}
}
