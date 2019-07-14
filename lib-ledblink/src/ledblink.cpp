/**
 * @file ledblink.h
 *
 */
/* Copyright (C) 2017-2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include "ledblink.h"

enum tFreqMode {
	FREQ_MODE_OFF = 0,
	FREQ_MODE_NORMAL = 1,
	FREQ_MODE_DATA = 3,
	FREQ_MODE_FAST = 5
};

LedBlink *LedBlink::s_pThis = 0;

LedBlink::LedBlink(void):
	m_nFreqHz(0),
	m_tMode(LEDBLINK_MODE_OFF)
{
	s_pThis = this;
}

LedBlink::~LedBlink(void) {

}

void LedBlink::SetMode(tLedBlinkMode Mode) {
	m_tMode = Mode;

	switch (Mode) {
	case LEDBLINK_MODE_OFF:
		SetFrequency(FREQ_MODE_OFF);
		break;
	case LEDBLINK_MODE_NORMAL:
		SetFrequency(FREQ_MODE_NORMAL);
		break;
	case LEDBLINK_MODE_DATA:
		SetFrequency(FREQ_MODE_DATA);
		break;
	case LEDBLINK_MODE_FAST:
		SetFrequency(FREQ_MODE_FAST);
		break;
	default:
		SetFrequency(FREQ_MODE_OFF);
		break;
	}
}
