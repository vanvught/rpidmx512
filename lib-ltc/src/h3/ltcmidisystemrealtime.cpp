/**
 * @file ltcmidisystemrealtime.cpp
 *
 */
/* Copyright (C) 2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <cstdint>
#include <cassert>

#include "ltcmidisystemrealtime.h"

#include "midi.h"
#include "rtpmidi.h"

#include "irq_timer.h"
#include "h3/ltcoutputs.h"

static struct TLtcDisabledOutputs *s_ptLtcDisabledOutputs;

static void timer_handler() {
	if (!s_ptLtcDisabledOutputs->bRtpMidi) {
		RtpMidi::Get()->SendRaw(midi::Types::CLOCK);
	}

	if (!s_ptLtcDisabledOutputs->bMidi) {
		Midi::Get()->SendRaw(midi::Types::CLOCK);
	}
}

LtcMidiSystemRealtime *LtcMidiSystemRealtime::s_pThis = nullptr;

LtcMidiSystemRealtime::LtcMidiSystemRealtime(struct TLtcDisabledOutputs *ptLtcDisabledOutputs) {
	assert(ptLtcDisabledOutputs != nullptr);
	s_ptLtcDisabledOutputs = ptLtcDisabledOutputs;

	assert(s_pThis == nullptr);
	s_pThis = this;
}

void LtcMidiSystemRealtime::Send(midi::Types tType) {
	if (!s_ptLtcDisabledOutputs->bRtpMidi) {
		RtpMidi::Get()->SendRaw(tType);
	}

	if (!s_ptLtcDisabledOutputs->bMidi) {
		Midi::Get()->SendRaw(tType);
	}
}

void LtcMidiSystemRealtime::SetBPM(uint32_t nBPM) {
	if ((!s_ptLtcDisabledOutputs->bRtpMidi) || (!s_ptLtcDisabledOutputs->bMidi)) {
		if (nBPM != m_nBPMPrevious) {
			m_nBPMPrevious = nBPM;
			if (nBPM == 0) {
				irq_timer_arm_virtual_set(nullptr, 0);
			} else if ((nBPM >= midi::bpm::MIN) && (nBPM <= midi::bpm::MAX)) {
				const uint32_t nValue =  60000000 / nBPM;
				irq_timer_arm_virtual_set(reinterpret_cast<thunk_irq_timer_arm_t>(timer_handler), nValue);
			}
		}
	}
}

void LtcMidiSystemRealtime::ShowBPM(uint32_t nBPM) {
	LtcOutputs::Get()->ShowBPM(nBPM);
}
