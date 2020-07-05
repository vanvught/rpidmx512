/**
 * @file ws28xxparamsset.cpp
 *
 */
/* Copyright (C) 2019-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <cassert>

#include "ws28xxdmxparams.h"
#include "ws28xxdmx.h"

void WS28xxDmxParams::Set(WS28xxDmx *pWS28xxDmx) {
	assert(pWS28xxDmx != nullptr);

	if (isMaskSet(WS28xxDmxParamsMask::LED_TYPE)) {
		pWS28xxDmx->SetLEDType(m_tWS28xxParams.tLedType);
	}

	if (isMaskSet(WS28xxDmxParamsMask::RGB_MAPPING)) {
		pWS28xxDmx->SetRgbMapping(static_cast<TRGBMapping>(m_tWS28xxParams.nRgbMapping));
	}

	if (isMaskSet(WS28xxDmxParamsMask::LOW_CODE)) {
		pWS28xxDmx->SetLowCode(m_tWS28xxParams.nLowCode);
	}

	if (isMaskSet(WS28xxDmxParamsMask::HIGH_CODE)) {
		pWS28xxDmx->SetHighCode(m_tWS28xxParams.nHighCode);
	}

	if (isMaskSet(WS28xxDmxParamsMask::LED_COUNT)) {
		pWS28xxDmx->SetLEDCount(m_tWS28xxParams.nLedCount);
	}

	if (isMaskSet(WS28xxDmxParamsMask::DMX_START_ADDRESS)) {
		pWS28xxDmx->SetDmxStartAddress(m_tWS28xxParams.nDmxStartAddress);
	}

	if (isMaskSet(WS28xxDmxParamsMask::SPI_SPEED)) {
		pWS28xxDmx->SetClockSpeedHz(m_tWS28xxParams.nSpiSpeedHz);
	}

	if (isMaskSet(WS28xxDmxParamsMask::GLOBAL_BRIGHTNESS)) {
		pWS28xxDmx->SetGlobalBrightness(m_tWS28xxParams.nGlobalBrightness);
	}
}
