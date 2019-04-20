/**
 * @file tlc59711dmxparamsset.cpp
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

#include "tlc59711dmxparams.h"
#include "tlc59711dmx.h"

void TLC59711DmxParams::Set(TLC59711Dmx* pTLC59711Dmx) {
	assert(pTLC59711Dmx != 0);

	if(isMaskSet(TLC59711DMX_PARAMS_MASK_LED_TYPE)) {
		pTLC59711Dmx->SetLEDType(m_tTLC59711Params.LedType);
	}

	if(isMaskSet(TLC59711DMX_PARAMS_MASK_LED_COUNT)) {
		pTLC59711Dmx->SetLEDCount(m_tTLC59711Params.nLedCount);
	}

	if(isMaskSet(TLC59711DMX_PARAMS_MASK_START_ADDRESS)) {
		pTLC59711Dmx->SetDmxStartAddress(m_tTLC59711Params.nDmxStartAddress);
	}

	if(isMaskSet(TLC59711DMX_PARAMS_MASK_SPI_SPEED)) {
		pTLC59711Dmx->SetSpiSpeedHz(m_tTLC59711Params.nSpiSpeedHz);
	}
}
