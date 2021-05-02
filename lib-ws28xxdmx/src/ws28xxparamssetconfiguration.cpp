/**
 * @file ws28xxparamssetconfiguration.cpp
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
#include <cassert>

#include "ws28xxdmxparams.h"

#include "pixeldmxconfiguration.h"

using namespace pixel;

void WS28xxDmxParams::Set(PixelDmxConfiguration *pPixelDmxConfiguration) {
	assert(pPixelDmxConfiguration != nullptr);

	// Pixel

	if (isMaskSet(WS28xxDmxParamsMask::TYPE)) {
		pPixelDmxConfiguration->SetType(static_cast<Type>(m_tWS28xxParams.nType));
	}

	if (isMaskSet(WS28xxDmxParamsMask::COUNT)) {
		pPixelDmxConfiguration->SetCount(m_tWS28xxParams.nCount);
	}

	if (isMaskSet(WS28xxDmxParamsMask::MAP)) {
		pPixelDmxConfiguration->SetMap(static_cast<Map>(m_tWS28xxParams.nMap));
	}

	if (isMaskSet(WS28xxDmxParamsMask::LOW_CODE)) {
		pPixelDmxConfiguration->SetLowCode(m_tWS28xxParams.nLowCode);
	}

	if (isMaskSet(WS28xxDmxParamsMask::HIGH_CODE)) {
		pPixelDmxConfiguration->SetHighCode(m_tWS28xxParams.nHighCode);
	}

#if defined (PARAMS_INLCUDE_ALL) || !defined(OUTPUT_PIXEL_MULTI)
	if (isMaskSet(WS28xxDmxParamsMask::SPI_SPEED)) {
		pPixelDmxConfiguration->SetClockSpeedHz(m_tWS28xxParams.nSpiSpeedHz);
	}

	if (isMaskSet(WS28xxDmxParamsMask::GLOBAL_BRIGHTNESS)) {
		pPixelDmxConfiguration->SetGlobalBrightness(m_tWS28xxParams.nGlobalBrightness);
	}
#endif

	// Dmx

	if (isMaskSet(WS28xxDmxParamsMask::GROUPING_ENABLED)) {
		pPixelDmxConfiguration->SetGroupingEnabled(true);
	}

	if (isMaskSet(WS28xxDmxParamsMask::GROUPING_COUNT)) {
		pPixelDmxConfiguration->SetGroupingCount(m_tWS28xxParams.nGroupingCount);
	}

#if defined (PARAMS_INLCUDE_ALL) || defined(OUTPUT_PIXEL_MULTI)
	if (isMaskSet(WS28xxDmxParamsMask::ACTIVE_OUT)) {
		pPixelDmxConfiguration->SetOutputPorts(m_tWS28xxParams.nActiveOutputs);
	}
#endif
}
