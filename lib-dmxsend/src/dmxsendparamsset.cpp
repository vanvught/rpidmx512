/**
 * @file dmxsendparamsset.cpp
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
#include <assert.h>

#include "dmxparams.h"

void DMXParams::Set(DMXSend *pDMXSend) {
	assert(pDMXSend != 0);

	if (isMaskSet(DMX_SEND_PARAMS_MASK_BREAK_TIME)) {
		pDMXSend->SetDmxBreakTime(m_tDMXParams.nBreakTime);
	}

	if (isMaskSet(DMX_SEND_PARAMS_MASK_MAB_TIME)) {
		pDMXSend->SetDmxMabTime(m_tDMXParams.nMabTime);
	}

	if (isMaskSet(DMX_SEND_PARAMS_MASK_REFRESH_RATE)) {
		uint32_t period = (uint32_t) 0;
		if (m_tDMXParams.nRefreshRate != (uint8_t) 0) {
			period = (uint32_t) (1000000 / m_tDMXParams.nRefreshRate);
		}
		pDMXSend->SetDmxPeriodTime(period);
	}
}

#if defined (H3)
void DMXParams::Set(DMXSendMulti *pDMXSendMulti) {
	assert(pDMXSendMulti != 0);

	if (isMaskSet(DMX_SEND_PARAMS_MASK_BREAK_TIME)) {
		pDMXSendMulti->SetDmxBreakTime(m_tDMXParams.nBreakTime);
	}

	if (isMaskSet(DMX_SEND_PARAMS_MASK_MAB_TIME)) {
		pDMXSendMulti->SetDmxMabTime(m_tDMXParams.nMabTime);
	}
}
#endif
