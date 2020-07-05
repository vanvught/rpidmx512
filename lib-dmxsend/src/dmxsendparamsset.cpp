/**
 * @file dmxsendparamsset.cpp
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

#include <stdint.h>
#include <cassert>

#include "dmxparams.h"

void DMXParams::Set(DMXSend *pDMXSend) {
	assert(pDMXSend != nullptr);

	if (isMaskSet(DmxSendParamsMask::BREAK_TIME)) {
		pDMXSend->SetDmxBreakTime(m_tDMXParams.nBreakTime);
	}

	if (isMaskSet(DmxSendParamsMask::MAB_TIME)) {
		pDMXSend->SetDmxMabTime(m_tDMXParams.nMabTime);
	}

	if (isMaskSet(DmxSendParamsMask::REFRESH_RATE)) {
		uint32_t period = 0;
		if (m_tDMXParams.nRefreshRate != 0) {
			period = static_cast<uint32_t>(1000000 / m_tDMXParams.nRefreshRate);
		}
		pDMXSend->SetDmxPeriodTime(period);
	}
}

#if defined (H3)
void DMXParams::Set(DMXSendMulti *pDMXSendMulti) {
	assert(pDMXSendMulti != nullptr);

	if (isMaskSet(DmxSendParamsMask::BREAK_TIME)) {
		pDMXSendMulti->SetDmxBreakTime(m_tDMXParams.nBreakTime);
	}

	if (isMaskSet(DmxSendParamsMask::MAB_TIME)) {
		pDMXSendMulti->SetDmxMabTime(m_tDMXParams.nMabTime);
	}
}
#endif
