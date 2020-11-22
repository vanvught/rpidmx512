/**
 * @file artnetreader.cpp
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
#include <cassert>

#include "ledblink.h"

#include "h3/artnetreader.h"
#include "ltc.h"
#include "timecodeconst.h"

#include "arm/synchronize.h"
#include "h3.h"
#include "h3_timer.h"
#include "irq_timer.h"

// Input
#include "artnettimecode.h"

// Output
#include "rtpmidi.h"
#include "h3/ltcsender.h"
#include "h3/ltcoutputs.h"

// IRQ Timer0
static volatile uint32_t nUpdatesPerSecond = 0;
static volatile uint32_t nUpdatesPrevious = 0;
static volatile uint32_t nUpdates = 0;

static void irq_timer0_update_handler(__attribute__((unused)) uint32_t clo) {
	nUpdatesPerSecond = nUpdates - nUpdatesPrevious;
	nUpdatesPrevious = nUpdates;
}

ArtNetReader::ArtNetReader(struct TLtcDisabledOutputs *pLtcDisabledOutputs) : m_ptLtcDisabledOutputs(pLtcDisabledOutputs) {
	assert(m_ptLtcDisabledOutputs != nullptr);
}

void ArtNetReader::Start() {
	irq_timer_init();

	irq_timer_set(IRQ_TIMER_0, reinterpret_cast<thunk_irq_timer_t>(irq_timer0_update_handler));
	H3_TIMER->TMR0_INTV = 0xB71B00; // 1 second
	H3_TIMER->TMR0_CTRL &= ~(TIMER_CTRL_SINGLE_MODE);
	H3_TIMER->TMR0_CTRL |= (TIMER_CTRL_EN_START | TIMER_CTRL_RELOAD);

	LtcOutputs::Get()->Init();

	LedBlink::Get()->SetFrequency(ltc::led_frequency::NO_DATA);
}

void ArtNetReader::Stop() {
	irq_timer_set(IRQ_TIMER_0, nullptr);
}

void ArtNetReader::Handler(const struct TArtNetTimeCode *ArtNetTimeCode) {
	nUpdates++;

	if (!m_ptLtcDisabledOutputs->bLtc) {
		LtcSender::Get()->SetTimeCode(reinterpret_cast<const struct TLtcTimeCode*>(ArtNetTimeCode));
	}

	if (!m_ptLtcDisabledOutputs->bRtpMidi) {
		RtpMidi::Get()->SendTimeCode(reinterpret_cast<const struct _midi_send_tc*>(ArtNetTimeCode));
	}

	memcpy(&m_tMidiTimeCode, ArtNetTimeCode, sizeof(struct _midi_send_tc));

	LtcOutputs::Get()->Update(reinterpret_cast<const struct TLtcTimeCode*>(ArtNetTimeCode));
}

void ArtNetReader::Run() {
	LtcOutputs::Get()->UpdateMidiQuarterFrameMessage(reinterpret_cast<const struct TLtcTimeCode*>(&m_tMidiTimeCode));

	dmb();
	if (nUpdatesPerSecond != 0) {
		LedBlink::Get()->SetFrequency(ltc::led_frequency::DATA);
	} else {
		LtcOutputs::Get()->ShowSysTime();
		LedBlink::Get()->SetFrequency(ltc::led_frequency::NO_DATA);
	}
}
