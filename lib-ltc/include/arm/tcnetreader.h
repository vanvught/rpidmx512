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

#include "tcnettimecode.h"

#include "midi.h"
#include "ltc.h"

#include "ltcoutputs.h"

#include "hardware.h"

class TCNetReader final : public TCNetTimeCode {
public:
	void Start();
	void Stop();

	void Run() {
		LtcOutputs::Get()->UpdateMidiQuarterFrameMessage(reinterpret_cast<const struct ltc::TimeCode*>(&m_tMidiTimeCode));

		__DMB();
		if (gv_ltc_nUpdatesPerSecond != 0) {
			Hardware::Get()->SetMode(hardware::ledblink::Mode::DATA);
		} else {
			LtcOutputs::Get()->ShowSysTime();
			Hardware::Get()->SetMode(hardware::ledblink::Mode::NORMAL);
			m_nTimeCodePrevious = static_cast<uint32_t>(~0);
		}

		HandleUdpRequest();
	}

	void Handler(const struct TTCNetTimeCode *pTimeCode) override;

private:
	void HandleUdpRequest();

private:
	midi::Timecode m_tMidiTimeCode;
	uint32_t m_nTimeCodePrevious { 0xFF };
	int32_t m_nHandle { -1 };

	static inline char *s_pUdpBuffer;
};

#endif /* ARM_TCNETREADER_H_ */
