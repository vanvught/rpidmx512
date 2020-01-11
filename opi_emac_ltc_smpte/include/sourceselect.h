/**
 * @file sourceselect.h
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

#ifndef SOURCESELECT_H_
#define SOURCESELECT_H_

#include <stdbool.h>

#include "ltcparams.h"

#include "rotaryencoder.h"

enum TRunStatus {
	RUN_STATUS_IDLE,
	RUN_STATUS_CONTINUE,
	RUN_STATUS_REBOOT
};

class SourceSelect {
public:
	SourceSelect(TLtcReaderSource tLtcReaderSource, struct TLtcDisabledOutputs *ptLtcDisabledOutputs);
	~SourceSelect(void);

	bool Check(void);
	bool Wait(TLtcReaderSource& tLtcReaderSource);

	bool IsConnected(void) {
		return m_bIsConnected;
	}

	void Run(void);

private:
	void LedBlink(uint8_t nPortB);
	void HandleActionLeft(TLtcReaderSource& tLtcReaderSource);
	void HandleActionRight(TLtcReaderSource& tLtcReaderSource);
	void HandleRotary(uint8_t nInputAB, TLtcReaderSource& tLtcReaderSource);
	void UpdateDisaplays(TLtcReaderSource tLtcReaderSource);
	void HandleActionSelect(void);
	void SetRunState(TRunStatus tRunState);

private:
	TLtcReaderSource m_tLtcReaderSource;
	struct TLtcDisabledOutputs *m_ptLtcDisabledOutputs;
	bool m_bIsConnected;
	uint8_t m_nPortA;
	uint8_t m_nPortAPrevious;
	uint8_t m_nPortB;
	uint32_t m_nMillisPrevious;
	RotaryEncoder m_RotaryEncoder;
	TRotaryDirection m_tRotaryDirection;
	TRunStatus m_tRunStatus;
	uint32_t m_nSelectMillis;
};

#endif /* SOURCESELECT_H_ */
