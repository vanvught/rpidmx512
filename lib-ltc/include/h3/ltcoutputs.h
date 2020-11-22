/**
 * @file ltcoutputs.h
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

#ifndef H3_LTCOUTPUTS_H_
#define H3_LTCOUTPUTS_H_

#include "ltc.h"

class LtcOutputs {
public:
	LtcOutputs(struct TLtcDisabledOutputs *pLtcDisabledOutputs, ltc::source tSource, bool bShowSysTime);

	void Init();
	void Update(const struct TLtcTimeCode *ptLtcTimeCode);
	void UpdateMidiQuarterFrameMessage(const struct TLtcTimeCode *ptLtcTimeCode);

	void ShowSysTime();

	void ResetTimeCodeTypePrevious() {
		m_tTimeCodeTypePrevious = ltc::type::INVALID;
	}

	void Print();

	static LtcOutputs* Get() {
		return s_pThis;
	}

private:
	void PrintDisabled(bool IsDisabled, const char *p);

private:
	struct TLtcDisabledOutputs *m_ptLtcDisabledOutputs;
	bool m_bShowSysTime;
	ltc::type m_tTimeCodeTypePrevious{ltc::type::INVALID};
	uint32_t m_nMidiQuarterFramePiece {0};
	char m_aTimeCode[TC_CODE_MAX_LENGTH];
	char m_aSystemTime[TC_SYSTIME_MAX_LENGTH];
	int32_t m_nSecondsPrevious{60};

	static LtcOutputs *s_pThis;
};

#endif /* H3_LTCOUTPUTS_H_ */
