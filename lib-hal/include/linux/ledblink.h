/**
 * @file ledblink.h
 *
 */
/* Copyright (C) 2019-2022 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef LINUX_LEDBLINK_H_
#define LINUX_LEDBLINK_H_

#include <cstdint>

class LedBlink {
public:
	LedBlink();

	void SetFrequency(uint32_t nFreqHz);
	uint32_t GetFrequency() {
		return m_nFreqHz;
	}

	void SetMode(ledblink::Mode Mode);
	ledblink::Mode GetMode() {
		return m_Mode;
	}

	void Run() {} // Not needed

	void SetLedBlinkDisplay(LedBlinkDisplay *pLedBlinkDisplay) {
		m_pLedBlinkDisplay = pLedBlinkDisplay;
	}

	static LedBlink* Get() {
		return s_pThis;
	}

private:
	void PlatformInit() {}

private:
	uint32_t m_nFreqHz { 0 };
	ledblink::Mode m_Mode { ledblink::Mode::UNKNOWN };
	LedBlinkDisplay *m_pLedBlinkDisplay { nullptr };

	static LedBlink *s_pThis;
};

#endif /* LINUX_LEDBLINK_H_ */
