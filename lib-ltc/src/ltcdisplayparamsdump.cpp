/**
 * @file ltcdisplayparamsdump.cpp
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

#include "ltcdisplayparams.h"
#include "ltcdisplayparamsconst.h"

#include "devicesparamsconst.h"

void LtcDisplayParams::Dump() {
#ifndef NDEBUG
	printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, LtcDisplayParamsConst::FILE_NAME);

	if (isMaskSet(LtcDisplayParamsMask::WS28XX_TYPE)) {
		printf(" %s=%d [%s]\n", LtcDisplayParamsConst::WS28XX_TYPE, m_tLtcDisplayParams.nWS28xxType, m_tLtcDisplayParams.nWS28xxType == LTCDISPLAYWS28XX_TYPE_7SEGMENT ? "7segment" : "matrix");
	}

	if (isMaskSet(LtcDisplayParamsMask::LED_TYPE)) {
		printf(" %s=%s [%d]\n", DevicesParamsConst::LED_TYPE,
				WS28xx::GetLedTypeString(
						static_cast<TWS28XXType>(m_tLtcDisplayParams.nLedType)), static_cast<int>(m_tLtcDisplayParams.nLedType));
	}

	if (isMaskSet(LtcDisplayParamsMask::RGB_MAPPING)) {
		printf(" %s=%s [%d]\n", DevicesParamsConst::LED_RGB_MAPPING,
				RGBMapping::ToString(static_cast<TRGBMapping>(m_tLtcDisplayParams.nRgbMapping)), static_cast<int>(m_tLtcDisplayParams.nRgbMapping));
	}

	if (isMaskSet(LtcDisplayParamsMask::WS28XX_INTENSITY)) {
		printf(" %s=%d\n", LtcDisplayParamsConst::WS28XX_INTENSITY, m_tLtcDisplayParams.nWS28xxIntensity);
	}

	if (isMaskSet(LtcDisplayParamsMask::WS28XX_COLON_BLINK_MODE)) {
		printf(" %s=%d\n", LtcDisplayParamsConst::WS28XX_COLON_BLINK_MODE, m_tLtcDisplayParams.nWS28xxColonBlinkMode);
	}

	for (uint32_t nIndex = 0; nIndex < LTCDISPLAYWS28XX_COLOUR_INDEX_LAST; nIndex++) {
		if (isMaskSet((LtcDisplayParamsMask::WS28XX_COLOUR_INDEX << nIndex))) {
			printf(" %s=%.6x\n", LtcDisplayParamsConst::WS28XX_COLOUR[nIndex], m_tLtcDisplayParams.aWS28xxColour[nIndex]);
		}
	}

	if (isMaskSet(LtcDisplayParamsMask::GLOBAL_BRIGHTNESS)) {
		printf(" %s=%d\n", DevicesParamsConst::GLOBAL_BRIGHTNESS, static_cast<int>(m_tLtcDisplayParams.nGlobalBrightness));
	}

	if (isMaskSet(LtcDisplayParamsMask::MAX7219_TYPE)) {
		printf(" %s=%d [%s]\n", LtcDisplayParamsConst::MAX7219_TYPE, m_tLtcDisplayParams.nMax7219Type, m_tLtcDisplayParams.nMax7219Type == LTCDISPLAYMAX7219_TYPE_7SEGMENT ? "7segment" : "matrix");
	}

	if (isMaskSet(LtcDisplayParamsMask::MAX7219_INTENSITY)) {
		printf(" %s=%d\n", LtcDisplayParamsConst::MAX7219_INTENSITY, m_tLtcDisplayParams.nMax7219Intensity);
	}
#endif
}
