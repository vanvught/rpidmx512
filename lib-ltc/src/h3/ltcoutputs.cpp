/**
 * @file ltcoutputs.cpp
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

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <cassert>

#include "h3/ltcoutputs.h"

#include "ltc.h"
#include "timecodeconst.h"

#include "h3.h"
#include "h3_timer.h"
#include "irq_timer.h"
#include "arm/synchronize.h"

// Outputs
#include "h3/ltcsender.h"
#include "rtpmidi.h"
#include "midi.h"
#include "ntpserver.h"
#include "ltc7segment.h"
#include "display.h"
#include "ltcdisplaymax7219.h"
#include "ltcdisplayws28xx.h"

// IRQ Timer1
static volatile bool IsMidiQuarterFrameMessage = false;

static void irq_timer1_midi_handler(__attribute__((unused)) uint32_t clo) {
	IsMidiQuarterFrameMessage = true;
}

LtcOutputs *LtcOutputs::s_pThis = nullptr;

using namespace ltc;

LtcOutputs::LtcOutputs(const struct TLtcDisabledOutputs *pLtcDisabledOutputs, source tSource, bool bShowSysTime):
	m_bShowSysTime(bShowSysTime),
	m_tTimeCodeTypePrevious(ltc::type::INVALID),
	m_nMidiQuarterFramePiece(0),
	m_nSecondsPrevious(60)
{
	assert(pLtcDisabledOutputs != nullptr);

	assert(s_pThis == nullptr);
	s_pThis = this;

	memcpy(&m_tLtcDisabledOutputs, pLtcDisabledOutputs, sizeof(struct TLtcDisabledOutputs));

	m_tLtcDisabledOutputs.bMidi |= (tSource == source::MIDI);
	m_tLtcDisabledOutputs.bArtNet |= (tSource == source::ARTNET);
	m_tLtcDisabledOutputs.bLtc |= (tSource == source::LTC);
	m_tLtcDisabledOutputs.bRtpMidi |= (tSource == source::APPLEMIDI);

	Ltc::InitTimeCode(m_aTimeCode);
	Ltc::InitSystemTime(m_aSystemTime);
}

void LtcOutputs::Init() {
	if (!m_tLtcDisabledOutputs.bMidi) {
		irq_timer_set(IRQ_TIMER_1, static_cast<thunk_irq_timer_t>(irq_timer1_midi_handler));
	}

	if (!m_tLtcDisabledOutputs.bDisplay) {
		Display::Get()->TextLine(2, Ltc::GetType(ltc::type::UNKNOWN), TC_TYPE_MAX_LENGTH);
	}
}

void LtcOutputs::Update(const struct TLtcTimeCode *ptLtcTimeCode) {
	assert(ptLtcTimeCode != nullptr);

	if (!m_tLtcDisabledOutputs.bNtp) {
		NtpServer::Get()->SetTimeCode(ptLtcTimeCode);
	}

	if (ptLtcTimeCode->nType != static_cast<uint8_t>(m_tTimeCodeTypePrevious)) {
		m_tTimeCodeTypePrevious = static_cast<ltc::type>(ptLtcTimeCode->nType);

		if (!m_tLtcDisabledOutputs.bMidi) {
			Midi::Get()->SendTimeCode(reinterpret_cast<const struct _midi_send_tc*>(ptLtcTimeCode));
		}

		H3_TIMER->TMR1_INTV = TimeCodeConst::TMR_INTV[ptLtcTimeCode->nType] / 4;
		H3_TIMER->TMR1_CTRL |= (TIMER_CTRL_EN_START | TIMER_CTRL_RELOAD);

		m_nMidiQuarterFramePiece = 0;

		if (!m_tLtcDisabledOutputs.bDisplay) {
			Display::Get()->TextLine(2, Ltc::GetType(static_cast<ltc::type>(ptLtcTimeCode->nType)), TC_TYPE_MAX_LENGTH);
		}

		Ltc7segment::Get()->Show(static_cast<ltc::type>(ptLtcTimeCode->nType));
	}

	Ltc::ItoaBase10(ptLtcTimeCode, m_aTimeCode);

	if (!m_tLtcDisabledOutputs.bDisplay) {
		Display::Get()->TextLine(1, m_aTimeCode, TC_CODE_MAX_LENGTH);
	}

	if (!m_tLtcDisabledOutputs.bMax7219) {
		LtcDisplayMax7219::Get()->Show(m_aTimeCode);
	}

	if(!m_tLtcDisabledOutputs.bWS28xx) {
		LtcDisplayWS28xx::Get()->Show(m_aTimeCode);
	}
}

void LtcOutputs::UpdateMidiQuarterFrameMessage(const struct TLtcTimeCode *ptLtcTimeCode) {
	dmb();
	if (__builtin_expect((IsMidiQuarterFrameMessage), 0)) {
		IsMidiQuarterFrameMessage = false;
		Midi::Get()->SendQf(reinterpret_cast<const struct _midi_send_tc*>(ptLtcTimeCode), m_nMidiQuarterFramePiece);
	}
}

void LtcOutputs::ShowSysTime() {
	if (m_bShowSysTime) {
		const time_t tTime = time(nullptr);
		const struct tm *pLocalTime = localtime(&tTime);

		if (__builtin_expect((m_nSecondsPrevious == pLocalTime->tm_sec), 1)) {
			return;
		}

		m_nSecondsPrevious = pLocalTime->tm_sec;

		Ltc::ItoaBase10(pLocalTime, m_aSystemTime);

		if (!m_tLtcDisabledOutputs.bDisplay) {
			Display::Get()->TextLine(1, m_aSystemTime, TC_SYSTIME_MAX_LENGTH);
			Display::Get()->ClearLine(2);
		}

		Ltc7segment::Get()->Show(ltc::type::UNKNOWN);

		if (!m_tLtcDisabledOutputs.bMax7219) {
			LtcDisplayMax7219::Get()->ShowSysTime(m_aSystemTime);
		}

		if(!m_tLtcDisabledOutputs.bWS28xx) {
			LtcDisplayWS28xx::Get()->ShowSysTime(m_aSystemTime);
		}

		ResetTimeCodeTypePrevious();
	}
}

void LtcOutputs::PrintDisabled(bool IsDisabled, const char *pString) {
	if (IsDisabled) {
		printf(" %s output is disabled\n", pString);
	}
}

void LtcOutputs::Print() {
	PrintDisabled(m_tLtcDisabledOutputs.bLtc, "LTC");
	PrintDisabled(m_tLtcDisabledOutputs.bArtNet, "Art-Net");
	PrintDisabled(m_tLtcDisabledOutputs.bRtpMidi, "AppleMIDI");
	PrintDisabled(m_tLtcDisabledOutputs.bMidi, "MIDI");
	PrintDisabled(m_tLtcDisabledOutputs.bNtp, "NTP");
	PrintDisabled(m_tLtcDisabledOutputs.bDisplay, "OLED");
	PrintDisabled(m_tLtcDisabledOutputs.bMax7219, "Max7219");
	PrintDisabled(m_tLtcDisabledOutputs.bWS28xx, "WS28xx");
}
