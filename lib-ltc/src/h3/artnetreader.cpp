/**
 * @file artnetreader.cpp
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

#include "h3/artnetreader.h"
#include "ltc.h"
#include "timecodeconst.h"

#include "c/led.h"

#include "arm/synchronize.h"
#include "h3_hs_timer.h"
#include "h3_timer.h"
#include "irq_timer.h"

// Input
#include "artnettimecode.h"

// Output
#include "ltcleds.h"
#include "display.h"
#include "displaymax7219.h"
#include "displayws28xx.h"
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

ArtNetReader::ArtNetReader(struct TLtcDisabledOutputs *pLtcDisabledOutputs) :
	m_ptLtcDisabledOutputs(pLtcDisabledOutputs),
	m_tTimeCodeTypePrevious(TC_TYPE_INVALID),
	m_nMidiQuarterFramePiece(0)
{
	assert(m_ptLtcDisabledOutputs != 0);

	Ltc::InitTimeCode(m_aTimeCode);
}

ArtNetReader::~ArtNetReader(void) {
	Stop();
}

void ArtNetReader::Start(void) {
	irq_timer_init();

	irq_timer_set(IRQ_TIMER_0, (thunk_irq_timer_t) irq_timer0_update_handler);
	H3_TIMER->TMR0_INTV = 0xB71B00; // 1 second
	H3_TIMER->TMR0_CTRL &= ~(TIMER_CTRL_SINGLE_MODE);
	H3_TIMER->TMR0_CTRL |= (TIMER_CTRL_EN_START | TIMER_CTRL_RELOAD);

	if (!m_ptLtcDisabledOutputs->bMidi) {
		irq_timer_set(IRQ_TIMER_1, (thunk_irq_timer_t) irq_timer1_midi_handler);
		H3_TIMER->TMR1_CTRL &= ~TIMER_CTRL_SINGLE_MODE;
	}

	led_set_ticks_per_second(1000000 / 1);
}

void ArtNetReader::Stop(void) {
	irq_timer_set(IRQ_TIMER_0, 0);
	irq_timer_set(IRQ_TIMER_1, 0);
}

void ArtNetReader::Handler(const struct TArtNetTimeCode *ArtNetTimeCode) {
	char *pTimeCodeType;

	nUpdates++;

	if (!m_ptLtcDisabledOutputs->bLtc) {
		LtcSender::Get()->SetTimeCode((const struct TLtcTimeCode *)ArtNetTimeCode);
	}

	if (!m_ptLtcDisabledOutputs->bRtpMidi) {
		RtpMidi::Get()->SendTimeCode((const struct _midi_send_tc *)ArtNetTimeCode);
	}

	if (!m_ptLtcDisabledOutputs->bNtp) {
		NtpServer::Get()->SetTimeCode((const struct TLtcTimeCode *)ArtNetTimeCode);
	}

	memcpy(&m_tMidiTimeCode, ArtNetTimeCode, sizeof (struct _midi_send_tc ));

	if ((m_tTimeCodeTypePrevious != (TTimecodeTypes )ArtNetTimeCode->Type)) {
		m_tTimeCodeTypePrevious = (TTimecodeTypes) ArtNetTimeCode->Type;

		if (!m_ptLtcDisabledOutputs->bMidi) {
			Midi::Get()->SendTimeCode(&m_tMidiTimeCode);
		}

		H3_TIMER->TMR1_INTV = TimeCodeConst::TMR_INTV[(int) ArtNetTimeCode->Type] / 4;
		H3_TIMER->TMR1_CTRL |= (TIMER_CTRL_EN_START | TIMER_CTRL_RELOAD);

		m_nMidiQuarterFramePiece = 0;

		pTimeCodeType = (char *) Ltc::GetType((TTimecodeTypes) ArtNetTimeCode->Type);

		if (!m_ptLtcDisabledOutputs->bDisplay) {
			Display::Get()->TextLine(2, pTimeCodeType, TC_TYPE_MAX_LENGTH);
		}

		LtcLeds::Get()->Show((TTimecodeTypes) ArtNetTimeCode->Type);

	}

	Ltc::ItoaBase10((const struct TLtcTimeCode *) ArtNetTimeCode, m_aTimeCode);

	if (!m_ptLtcDisabledOutputs->bDisplay) {
		Display::Get()->TextLine(1, (const char *) m_aTimeCode, TC_CODE_MAX_LENGTH);
	}
	if (!m_ptLtcDisabledOutputs->bMax7219) {
		DisplayMax7219::Get()->Show((const char *) m_aTimeCode);
	} else	
		DisplayWS28xx::Get()->Show((const char *) m_aTimeCode);
	
	
}

void ArtNetReader::Run(void) {
	dmb();
	if (nUpdatesPerSecond >= 24) {
		dmb();
		if (__builtin_expect((IsMidiQuarterFrameMessage), 0)) {
			IsMidiQuarterFrameMessage = false;
			Midi::Get()->SendQf(&m_tMidiTimeCode, m_nMidiQuarterFramePiece);
		}
		led_set_ticks_per_second(1000000 / 3);
	} else {
		m_tTimeCodeTypePrevious = TC_TYPE_INVALID;
		DisplayMax7219::Get()->ShowSysTime();
		led_set_ticks_per_second(1000000 / 1);
	}
}
