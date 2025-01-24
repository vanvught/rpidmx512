/**
 * @file ltcoutputs.h
 *
 */
/* Copyright (C) 2019-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef ARM_LTCOUTPUTS_H_
#define ARM_LTCOUTPUTS_H_

#include "ltc.h"

class LtcOutputs {
public:
	LtcOutputs(const ltc::Source source, const bool bShowSysTime);

	void Init(const bool bDisableRtpMidi = false);
	void Update(const struct ltc::TimeCode *ptLtcTimeCode);

	void ShowSysTime();
	void ShowBPM(uint32_t nBPM);

	void ResetTimeCodeTypePrevious() {
		m_TypePrevious = ltc::Type::INVALID;
	}

	void Print();

	static LtcOutputs *Get() {
		return s_pThis;
	}

private:
	bool m_bShowSysTime;
	bool m_bMidiQuarterFramePieceRunning { false };
	bool m_bEnableRtpMidi { false };

	ltc::Type m_TypePrevious { ltc::Type::INVALID };
	int32_t m_nSecondsPrevious { 60 };

	char m_aTimeCode[ltc::timecode::CODE_MAX_LENGTH];
	char m_aSystemTime[ltc::timecode::SYSTIME_MAX_LENGTH];
	char m_cBPM[9];

	static inline LtcOutputs *s_pThis;
};

#endif /* ARM_LTCOUTPUTS_H_ */
