/**
 * @file ltcmidisystemrealtime.h
 *
 */
/* Copyright (C) 2020-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef ARM_LTCMIDISYSTEMREALTIME_H_
#define ARM_LTCMIDISYSTEMREALTIME_H_

#include <cassert>

#include "midi.h"
#include "net/rtpmidi.h"

#include "arm/ltcoutputs.h"

class LtcMidiSystemRealtime {
public:
	LtcMidiSystemRealtime() {
		assert(s_pThis == nullptr);
		s_pThis = this;
	}

	void Start();
	void Stop();

	void SendStart() {
		Send(midi::Types::START);
	}

	void SendStop() {
		Send(midi::Types::STOP);
	}

	void SendContinue() {
		Send(midi::Types::CONTINUE);
	}

	void SetBPM(uint32_t nBPM);

	void Input(const uint8_t *pBuffer, uint32_t nSize, uint32_t nFromIp, uint16_t nFromPort);

	static LtcMidiSystemRealtime *Get() {
		return s_pThis;
	}

private:
	void Send(const midi::Types type) {
//		if (!ltc::g_DisabledOutputs.bRtpMidi) {
		if (ltc::Destination::IsEnabled(ltc::Destination::Output::RTPMIDI)) {
			RtpMidi::Get()->SendRaw(type);
		}

//		if (!ltc::g_DisabledOutputs.bMidi) {
		if (ltc::Destination::IsEnabled(ltc::Destination::Output::MIDI)) {
			Midi::Get()->SendRaw(type);
		}
	}

	void ShowBPM(const uint32_t nBPM) {
		LtcOutputs::Get()->ShowBPM(nBPM);
	}

	void static StaticCallbackFunctionInput(const uint8_t *pBuffer, uint32_t nSize, uint32_t nFromIp, uint16_t nFromPort) {
		s_pThis->Input(pBuffer, nSize, nFromIp, nFromPort);
	}

private:
	int32_t m_nHandle { -1 };
	uint32_t m_nBPMPrevious { 999 };

	static inline LtcMidiSystemRealtime *s_pThis;
};

#endif /* ARM_LTCMIDISYSTEMREALTIME_H_ */
