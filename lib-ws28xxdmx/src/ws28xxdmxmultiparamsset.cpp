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
#include "ws28xxdmxmulti.h"

void WS28xxDmxParams::Set(WS28xxDmxMulti *pWS28xxDmxMulti) {
	assert(pWS28xxDmxMulti != 0);

	if (isMaskSet(WS28XXDMX_PARAMS_MASK_LED_TYPE)) {
		pWS28xxDmxMulti->SetLEDType((TWS28xxMultiType)m_tWS28xxParams.tLedType);
	}

	if (isMaskSet(WS28XXDMX_PARAMS_MASK_LED_COUNT)) {
		pWS28xxDmxMulti->SetLEDCount(m_tWS28xxParams.nLedCount);
	}

	if (isMaskSet(WS28XXDMX_PARAMS_MASK_ACTIVE_OUT)) {
		pWS28xxDmxMulti->SetActivePorts(m_tWS28xxParams.nActiveOutputs);
	}

	if (isMaskSet(WS28XXDMX_PARAMS_MASK_USE_SI5351A)) {
		pWS28xxDmxMulti->SetUseSI5351A(m_tWS28xxParams.bUseSI5351A);
	}
}
