/**
 * @file ledblink.h
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

#ifndef H3_LEDBLINK_H_
#define H3_LEDBLINK_H_

#include <stdint.h>

#include "h3.h"

extern "C" {
void hardware_led_set(const int);
}

class LedBlink {
public:
	LedBlink(void);

	void SetFrequency(uint32_t nFreqHz) {
		m_nFreqHz = nFreqHz;

		switch (nFreqHz) {
		case 0:
			m_nTicksPerSecond = 0;
			hardware_led_set(0);
			break;
		case 1:
			m_nTicksPerSecond = (1000000 / 1);
			break;
		case 3:
			m_nTicksPerSecond = (1000000 / 3);
			break;
		case 5:
			m_nTicksPerSecond = (1000000 / 5);
			break;
		case 255:
			m_nTicksPerSecond = 0;
			hardware_led_set(1);
			break;
		default:
			m_nTicksPerSecond = (1000000 / nFreqHz);
			break;
		}
	}

	uint32_t GetFrequency(void) const {
		return m_nFreqHz;
	}

	void SetMode(tLedBlinkMode tMode);
	tLedBlinkMode GetMode(void) const {
		return m_tMode;
	}

	void Run(void) {
		if (__builtin_expect (m_nTicksPerSecond == 0, 0)) {
			return;
		}

		const uint32_t nMicros = H3_TIMER->AVS_CNT1;

		if (__builtin_expect ((nMicros - m_nMicrosPrevious < m_nTicksPerSecond), 0)) {
			return;
		}

		m_nMicrosPrevious = nMicros;

		m_nToggleLed ^= 0x1;
		hardware_led_set(m_nToggleLed);
	}

	void SetLedBlinkDisplay(LedBlinkDisplay *pLedBlinkDisplay) {
		m_pLedBlinkDisplay = pLedBlinkDisplay;
	}

public:
	static LedBlink* Get(void) {
		return s_pThis;
	}

private:
	uint32_t m_nFreqHz{0};
	tLedBlinkMode m_tMode{LEDBLINK_MODE_UNKNOWN};
	LedBlinkDisplay *m_pLedBlinkDisplay{nullptr};
	//
	uint32_t m_nTicksPerSecond{1000000 / 2};
	int32_t m_nToggleLed{0};
	uint32_t m_nMicrosPrevious{0};

	static LedBlink *s_pThis;
};

#endif /* H3_LEDBLINK_H_ */
