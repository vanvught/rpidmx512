/**
 * @file ws28xxparamsset.cpp
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

#include <assert.h>

#include "ws28xxdmxparams.h"
#include "ws28xxdmx.h"

void WS28xxDmxParams::Set(WS28xxDmx *pWS28xxDmx) {
	assert(pWS28xxDmx != 0);

	if (isMaskSet(WS28XXDMX_PARAMS_MASK_LED_TYPE)) {
		pWS28xxDmx->SetLEDType(m_tWS28xxParams.tLedType);
	}

	if (isMaskSet(WS28XXDMX_PARAMS_MASK_LED_COUNT)) {
		pWS28xxDmx->SetLEDCount(m_tWS28xxParams.nLedCount);
	}

	if (isMaskSet(WS28XXDMX_PARAMS_MASK_DMX_START_ADDRESS)) {
		pWS28xxDmx->SetDmxStartAddress(m_tWS28xxParams.nDmxStartAddress);
	}

	if (isMaskSet(WS28XXDMX_PARAMS_MASK_SPI_SPEED)) {
		pWS28xxDmx->SetClockSpeedHz(m_tWS28xxParams.nSpiSpeedHz);
	}

	if (isMaskSet(WS28XXDMX_PARAMS_MASK_GLOBAL_BRIGHTNESS)) {
		pWS28xxDmx->SetGlobalBrightness(m_tWS28xxParams.nGlobalBrightness);
	}
}
