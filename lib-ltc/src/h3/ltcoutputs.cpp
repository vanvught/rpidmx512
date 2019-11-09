/**
 * @file ltcoutputs.cpp
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

// TODO Remove when using compressed firmware
#if !defined(__clang__)	// Needed for compiling on MacOS
 #pragma GCC push_options
 #pragma GCC optimize ("Os")
#endif

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#include "h3/ltcoutputs.h"
#include "ltc.h"
#include "timecodeconst.h"

#include "h3.h"
#include "h3_timer.h"
#include "irq_timer.h"
#include "arm/synchronize.h"

// Outputs
#include "h3/ltcsender.h"
#include "artnetnode.h"
#include "rtpmidi.h"
#include "midi.h"
#include "ntpserver.h"
#include "ltcleds.h"
#include "display.h"
#include "displaymax7219.h"
//#include "displayws28xx.h"

// IRQ Timer1
static volatile bool IsMidiQuarterFrameMessage = false;

static void irq_timer1_midi_handler(uint32_t clo) {
	IsMidiQuarterFrameMessage = true;
}

LtcOutputs *LtcOutputs::s_pThis = 0;

LtcOutputs::LtcOutputs(const struct TLtcDisabledOutputs *pLtcDisabledOutputs, TLtcReaderSource tSource):
	m_tTimeCodeTypePrevious(TC_TYPE_INVALID),
	m_nMidiQuarterFramePiece(0)
{
	assert(pLtcDisabledOutputs != 0);

	s_pThis = this;

	memcpy(&m_tLtcDisabledOutputs, pLtcDisabledOutputs, sizeof(struct TLtcDisabledOutputs));

	m_tLtcDisabledOutputs.bMidi |= (tSource == LTC_READER_SOURCE_MIDI);
	m_tLtcDisabledOutputs.bArtNet |= (tSource == LTC_READER_SOURCE_ARTNET);
	m_tLtcDisabledOutputs.bTCNet |= (tSource == LTC_READER_SOURCE_TCNET);
	m_tLtcDisabledOutputs.bLtc |= (tSource == LTC_READER_SOURCE_LTC);
	m_tLtcDisabledOutputs.bRtpMidi |= (tSource == LTC_READER_SOURCE_APPLEMIDI);

	Ltc::InitTimeCode(m_aTimeCode);
}

LtcOutputs::~LtcOutputs(void) {
}

void LtcOutputs::Init(void) {
	if (!m_tLtcDisabledOutputs.bMidi) {
		irq_timer_set(IRQ_TIMER_1, (thunk_irq_timer_t) irq_timer1_midi_handler);
	}
}

void LtcOutputs::Update(const struct TLtcTimeCode *ptLtcTimeCode) {
	assert(ptLtcTimeCode != 0);

	if (!m_tLtcDisabledOutputs.bNtp) {
		NtpServer::Get()->SetTimeCode((const struct TLtcTimeCode *)ptLtcTimeCode);
	}

	if (ptLtcTimeCode->nType != static_cast<uint8_t>(m_tTimeCodeTypePrevious)) {
		m_tTimeCodeTypePrevious = static_cast<TTimecodeTypes>(ptLtcTimeCode->nType);

		if (!m_tLtcDisabledOutputs.bMidi) {
			Midi::Get()->SendTimeCode((const struct _midi_send_tc *)ptLtcTimeCode);
		}

		H3_TIMER->TMR1_INTV = TimeCodeConst::TMR_INTV[(int) ptLtcTimeCode->nType] / 4;
		H3_TIMER->TMR1_CTRL |= (TIMER_CTRL_EN_START | TIMER_CTRL_RELOAD);

		m_nMidiQuarterFramePiece = 0;

		if (!m_tLtcDisabledOutputs.bDisplay) {
			Display::Get()->TextLine(2, Ltc::GetType((TTimecodeTypes) ptLtcTimeCode->nType), TC_TYPE_MAX_LENGTH);
		}

		LtcLeds::Get()->Show(static_cast<TTimecodeTypes>(ptLtcTimeCode->nType));
	}

	Ltc::ItoaBase10((const struct TLtcTimeCode *) ptLtcTimeCode, m_aTimeCode);

	if(!m_tLtcDisabledOutputs.bWS28xx) {
		//DisplayWS28xx::Get()->Show((const char *) m_aTimeCode);
	}

	if (!m_tLtcDisabledOutputs.bDisplay) {
		Display::Get()->TextLine(1, (const char *) m_aTimeCode, TC_CODE_MAX_LENGTH);
	}

	if (!m_tLtcDisabledOutputs.bMax7219) {
		DisplayMax7219::Get()->Show((const char *) m_aTimeCode);
	}
}

void LtcOutputs::UpdateMidiQuarterFrameMessage(const struct TLtcTimeCode *ptLtcTimeCode) {
	dmb();
	if (__builtin_expect((IsMidiQuarterFrameMessage), 0)) {
		IsMidiQuarterFrameMessage = false;
		Midi::Get()->SendQf((const struct _midi_send_tc *)ptLtcTimeCode, m_nMidiQuarterFramePiece);
	}
}

void LtcOutputs::Print(void) {
	PrintDisabled(m_tLtcDisabledOutputs.bLtc, "LTC");
	PrintDisabled(m_tLtcDisabledOutputs.bTCNet, "TCNet");
	PrintDisabled(m_tLtcDisabledOutputs.bRtpMidi, "AppleMIDI");
	PrintDisabled(m_tLtcDisabledOutputs.bMidi, "MIDI");
	PrintDisabled(m_tLtcDisabledOutputs.bArtNet, "Art-Net");
	PrintDisabled(m_tLtcDisabledOutputs.bNtp, "NTP");
	PrintDisabled(m_tLtcDisabledOutputs.bDisplay, "OLED");
	PrintDisabled(m_tLtcDisabledOutputs.bMax7219, "Max7219");
	PrintDisabled(m_tLtcDisabledOutputs.bWS28xx, "WS28xx");
}

void LtcOutputs::PrintDisabled(bool IsDisabled, const char *p) {
	if (IsDisabled) {
		printf(" %s output is disabled\n", p);
	}
}
