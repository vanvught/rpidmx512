/**
 * @file ledblink.h
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

#ifndef LINUX_LEDBLINK_H_
#define LINUX_LEDBLINK_H_

#include <stdint.h>

class LedBlink {
public:
	LedBlink(void);
	~LedBlink(void);

	void SetFrequency(uint32_t nFreqHz);
	uint32_t GetFrequency(void) {
		return m_nFreqHz;
	}

	void SetMode(tLedBlinkMode Mode);
	tLedBlinkMode GetMode(void) {
		return m_tMode;
	}

	void Run(void) {} // Not needed

public:
	static LedBlink* Get(void) {
		return s_pThis;
	}

private:
	uint32_t m_nFreqHz;
	tLedBlinkMode m_tMode;

	static LedBlink *s_pThis;
};

#endif /* LINUX_LEDBLINK_H_ */
