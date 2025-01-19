/**
 * @file tcnetreader.h
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

#ifndef ARM_TCNETREADER_H_
#define ARM_TCNETREADER_H_

#include <cassert>

#include "tcnettimecode.h"

#include "midi.h"
#include "ltc.h"

#include "ltcoutputs.h"

#include "hardware.h"

class TCNetReader {
public:
	TCNetReader() {
		assert(s_pThis == nullptr);
		s_pThis = this;
	}

	~TCNetReader() = default;

	void Start();
	void Stop();

	void Run();

	void Input(const uint8_t *pBuffer, uint32_t nSize, uint32_t nFromIp, uint16_t nFromPort);

	void static StaticCallbackFunctionHandler(const struct tcnet::TimeCode *pTimeCode) {
		assert(s_pThis != nullptr);
		s_pThis->Handler(pTimeCode);
	}

private:
	void static StaticCallbackFunctionInput(const uint8_t *pBuffer, uint32_t nSize, uint32_t nFromIp, uint16_t nFromPort) {
		s_pThis->Input(pBuffer, nSize, nFromIp, nFromPort);
	}

	void Handler(const struct tcnet::TimeCode *pTimeCode);

	void Reset(const bool doReset) {
		if (m_doResetTimeCode != doReset) {
			m_doResetTimeCode = doReset;
			if (m_doResetTimeCode) {
				LtcOutputs::Get()->ResetTimeCodeTypePrevious();
			}
		}
	}

	void ResetTimer(const bool doReset, const struct tcnet::TimeCode *pTimeCode);

private:
	int32_t m_nHandle { -1 };

	tcnet::TimeCode m_timeCode;

	uint8_t m_nTypePrevious { UINT8_MAX };
	uint8_t m_nFramePrevious { UINT8_MAX };

	bool m_doResetTimeCode { true };
	bool m_doResetTimer { true };

	static inline TCNetReader *s_pThis;
};

#endif /* ARM_TCNETREADER_H_ */
