/**
 * @file gpsparamsdump.cpp
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

#if !defined(__clang__)	// Needed for compiling on MacOS
# pragma GCC push_options
# pragma GCC optimize ("Os")
#endif

#include <stdint.h>
#include <stdio.h>

#include "gpsparams.h"
#include "gpsparamsconst.h"
#include "gps.h"

#include "debug.h"

void GPSParams::Dump() {
#ifndef NDEBUG
	printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, GPSParamsConst::FILE_NAME);

	if (isMaskSet(GPSParamsMask::MODULE)) {
		printf(" %s=%d [%s]\n", GPSParamsConst::MODULE, static_cast<int>(m_tTGPSParams.nModule), GPS::GetModuleName(static_cast<GPSModule>(m_tTGPSParams.nModule)));
	}

	if (isMaskSet(GPSParamsMask::ENABLE)) {
		printf(" %s=%d [%s]\n", GPSParamsConst::ENABLE, static_cast<int>(m_tTGPSParams.nEnable), BOOL2STRING::Get(m_tTGPSParams.nEnable));
	}

	if (isMaskSet(GPSParamsMask::UTC_OFFSET)) {
		printf(" %s=%1.1f\n", GPSParamsConst::UTC_OFFSET, m_tTGPSParams.fUtcOffset);
	}
#endif
}
