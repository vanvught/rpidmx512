/**
 * @file tcnetreader.cpp
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

#include <stdint.h>
#include <string.h>
#include <assert.h>

#include "h3/tcnetreader.h"
#include "timecodeconst.h"

#include "c/led.h"

#include "arm/synchronize.h"
#include "h3_hs_timer.h"
#include "h3_timer.h"
#include "irq_timer.h"

// Output
#include "ltcleds.h"
#include "display.h"
#include "displaymax7219.h"
#include "artnetnode.h"
#include "rtpmidi.h"
#include "midi.h"
#include "h3/ltcsender.h"
#include "ntpserver.h"

// IRQ Timer0
static volatile uint32_t nUpdatesPerSecond = 0;
static volatile uint32_t nUpdatesPrevious = 0;
static volatile uint32_t nUpdates = 0;
// IRQ Timer1
static volatile bool IsMidiQuarterFrameMessage = false;

static void irq_timer0_update_handler(uint32_t clo) {
	nUpdatesPerSecond = nUpdates - nUpdatesPrevious;
	nUpdatesPrevious = nUpdates;
}

static void irq_timer1_midi_handler(uint32_t clo) {
	IsMidiQuarterFrameMessage = true;
}

TCNetReader::TCNetReader(struct TLtcDisabledOutputs *pLtcDisabledOutputs) :
	m_ptLtcDisabledOutputs(pLtcDisabledOutputs),
	m_nMidiQuarterFramePiece(0),
	m_nTimeCodePrevious(0xFF),
	m_tTimeCodeTypePrevious(TC_TYPE_INVALID)
{
	assert(m_ptLtcDisabledOutputs != 0);

	Ltc::InitTimeCode(m_aTimeCode);
}

TCNetReader::~TCNetReader(void) {
	Stop();
}

void TCNetReader::Start(void) {
	irq_timer_init();

	irq_timer_set(IRQ_TIMER_0, (thunk_irq_timer_t) irq_timer0_update_handler);
	H3_TIMER->TMR0_INTV = 0xB71B00; // 1 second
	H3_TIMER->TMR0_CTRL &= ~(TIMER_CTRL_SINGLE_MODE);
	H3_TIMER->TMR0_CTRL |= (TIMER_CTRL_EN_START | TIMER_CTRL_RELOAD);

	if (!m_ptLtcDisabledOutputs->bMidi) {
		irq_timer_set(IRQ_TIMER_1, (thunk_irq_timer_t) irq_timer1_midi_handler);
		H3_TIMER->TMR1_CTRL &= ~(TIMER_CTRL_SINGLE_MODE);
	}

	led_set_ticks_per_second(1000000 / 1);
}

void TCNetReader::Stop(void) {
	irq_timer_set(IRQ_TIMER_0, 0);
	irq_timer_set(IRQ_TIMER_1, 0);
}

void TCNetReader::Handler(const struct TTCNetTimeCode* pTimeCode) {
	char *pTimeCodeType;

	nUpdates++;

	memcpy(&m_tMidiTimeCode, pTimeCode, sizeof(struct _midi_send_tc));

	assert(((uint32_t )pTimeCode & 0x3) == 0); // Check if we can do 4-byte compare
	const uint32_t *p = (uint32_t *)pTimeCode;

	if (m_nTimeCodePrevious != *p) {
		if (!m_ptLtcDisabledOutputs->bLtc) {
			LtcSender::Get()->SetTimeCode((const struct TLtcTimeCode *) pTimeCode);
		}

		if (!m_ptLtcDisabledOutputs->bArtNet) {
			ArtNetNode::Get()->SendTimeCode((const struct TArtNetTimeCode *) pTimeCode);
		}

		if (!m_ptLtcDisabledOutputs->bRtpMidi) {
			RtpMidi::Get()->SendTimeCode(&m_tMidiTimeCode);
		}

		if (!m_ptLtcDisabledOutputs->bNtp) {
			NtpServer::Get()->SetTimeCode((const struct TLtcTimeCode *) pTimeCode);
		}
	}

	if ((m_tTimeCodeTypePrevious != (TTimecodeTypes) pTimeCode->nType)) {
		m_tTimeCodeTypePrevious = (TTimecodeTypes) pTimeCode->nType;

		if (!m_ptLtcDisabledOutputs->bMidi) {
			Midi::Get()->SendTimeCode((struct _midi_send_tc *) &m_tMidiTimeCode);
		}

		H3_TIMER->TMR1_INTV = TimeCodeConst::TMR_INTV[(int) pTimeCode->nType] / 4;
		H3_TIMER->TMR1_CTRL |= (TIMER_CTRL_EN_START | TIMER_CTRL_RELOAD);

		m_nMidiQuarterFramePiece = 0;

		pTimeCodeType = (char *) Ltc::GetType((TTimecodeTypes) pTimeCode->nType);

		if (!m_ptLtcDisabledOutputs->bDisplay) {
			Display::Get()->TextLine(2, pTimeCodeType, TC_TYPE_MAX_LENGTH);
		}

		LtcLeds::Get()->Show((TTimecodeTypes) pTimeCode->nType);
	}

	if (m_nTimeCodePrevious != *p) {
		m_nTimeCodePrevious = *p;

		Ltc::ItoaBase10((const struct TLtcTimeCode *) pTimeCode, m_aTimeCode);

		if (!m_ptLtcDisabledOutputs->bDisplay) {
			Display::Get()->TextLine(1, (const char *) m_aTimeCode, TC_CODE_MAX_LENGTH);
		}
		if (!m_ptLtcDisabledOutputs->bMax7219) {
			DisplayMax7219::Get()->Show((const char *) m_aTimeCode);
		}
	}
}

void TCNetReader::Run(void) {
	dmb();
	if (nUpdatesPerSecond >= 24) {
		dmb();
		if (__builtin_expect((IsMidiQuarterFrameMessage), 0)) {
			dmb();
			IsMidiQuarterFrameMessage = false;
			Midi::Get()->SendQf(&m_tMidiTimeCode, m_nMidiQuarterFramePiece);
		}
		led_set_ticks_per_second(1000000 / 3);
	} else {
		m_nTimeCodePrevious = TC_TYPE_INVALID;
		DisplayMax7219::Get()->ShowSysTime();
		led_set_ticks_per_second(1000000 / 1);
	}
}
