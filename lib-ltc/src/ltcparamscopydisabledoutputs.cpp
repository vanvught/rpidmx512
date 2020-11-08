/**
 * @file ltcparamscopydisabledoutputs.cpp
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

#include "ltcparams.h"
#include "ltc.h"

void LtcParams::CopyDisabledOutputs(struct TLtcDisabledOutputs *pLtcDisabledOutputs) {
	pLtcDisabledOutputs->bOled = isDisabledOutputMaskSet(LtcParamsMaskDisabledOutputs::DISPLAY);
	pLtcDisabledOutputs->bMax7219 = isDisabledOutputMaskSet(LtcParamsMaskDisabledOutputs::MAX7219);
	pLtcDisabledOutputs->bMidi = isDisabledOutputMaskSet(LtcParamsMaskDisabledOutputs::MIDI);
	pLtcDisabledOutputs->bArtNet = isDisabledOutputMaskSet(LtcParamsMaskDisabledOutputs::ARTNET);
	pLtcDisabledOutputs->bLtc = isDisabledOutputMaskSet(LtcParamsMaskDisabledOutputs::LTC);
	pLtcDisabledOutputs->bNtp = (m_tLtcParams.nEnableNtp == 0);
	pLtcDisabledOutputs->bRtpMidi = isDisabledOutputMaskSet(LtcParamsMaskDisabledOutputs::RTPMIDI);
	pLtcDisabledOutputs->bWS28xx = (m_tLtcParams.nRgbLedType != TLtcParamsRgbLedType::WS28XX);
	pLtcDisabledOutputs->bRgbPanel = (m_tLtcParams.nRgbLedType != TLtcParamsRgbLedType::RGBPANEL);

	assert (pLtcDisabledOutputs->bWS28xx || pLtcDisabledOutputs->bRgbPanel);
}
