/**
 * @file rtpmidireader.h
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

#ifndef ARM_RTPMIDIREADER_H_
#define ARM_RTPMIDIREADER_H_

#include <cstdint>

#include "net/rtpmidihandler.h"
#include "ltc.h"

#include "midibpm.h"

class RtpMidiReader final : public RtpMidiHandler {
public:
	void Start();
	void Stop();
	void Run();

	void MidiMessage(const struct midi::Message *ptMidiMessage) override;

private:
	void HandleMtc(const struct midi::Message *ptMidiMessage);
	void HandleMtcQf(const struct midi::Message *ptMidiMessage);
	void Update();

private:
	struct ltc::TimeCode m_LtcTimeCode;
	uint8_t m_nPartPrevious { 0 };
	bool m_bDirection { true };
	uint32_t m_nMtcQfFramePrevious { 0 };
	uint32_t m_nMtcQfFramesDelta { 0 };
	MidiBPM m_MidiBPM;
};

#endif /* ARM_RTPMIDIREADER_H_ */
