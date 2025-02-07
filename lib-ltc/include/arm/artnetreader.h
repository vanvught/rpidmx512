/**
 * @file artnetreader.h
 *
 */
/* Copyright (C) 2019-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef ARM_ARTNETREADER_H_
#define ARM_ARTNETREADER_H_

#include <cstdint>

#include "artnettimecode.h"
#include "midi.h"

#include "ltcoutputs.h"

#include "hardware.h"

#include "arm/platform_ltc.h"

class ArtNetReader {
public:
	ArtNetReader() {
		assert(s_pThis == nullptr);
		s_pThis = this;
	}

	~ArtNetReader() = default;

	void Start();
	void Stop();

	void Run() {
		const auto nTimeStamp = Hardware::Get()->Millis();

		if ((nTimeStamp - m_nTimestamp) >= 50U) {
			LtcOutputs::Get()->ShowSysTime();
			Hardware::Get()->SetMode(hardware::ledblink::Mode::NORMAL);
			Reset(true);
		} else {
			Hardware::Get()->SetMode(hardware::ledblink::Mode::DATA);
			Reset(false);
		}
	}

	void static StaticCallbackFunction(const struct artnet::TimeCode *pTimeCode) {
		assert(s_pThis != nullptr);
		s_pThis->Handler(pTimeCode);
	}

private:
	void Handler(const struct artnet::TimeCode *);

	void Reset(const bool doReset) {
		if (m_doResetTimeCode != doReset) {
			m_doResetTimeCode = doReset;
			if (m_doResetTimeCode) {
				LtcOutputs::Get()->ResetTimeCodeTypePrevious();
			}
		}
	}

private:
	uint32_t m_nTimestamp { 0 };
	bool m_doResetTimeCode { true };
	static inline ArtNetReader *s_pThis;
};

#endif /* ARM_ARTNETREADER_H_ */
