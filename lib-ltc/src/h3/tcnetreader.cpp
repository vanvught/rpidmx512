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
#include "h3.h"
#include "h3_timer.h"
#include "irq_timer.h"

// Output
#include "artnetnode.h"
#include "rtpmidi.h"
#include "h3/ltcsender.h"
#include "displaymax7219.h"
//
#include "h3/ltcoutputs.h"

// IRQ Timer0
static volatile uint32_t nUpdatesPerSecond = 0;
static volatile uint32_t nUpdatesPrevious = 0;
static volatile uint32_t nUpdates = 0;

static void irq_timer0_update_handler(uint32_t clo) {
	nUpdatesPerSecond = nUpdates - nUpdatesPrevious;
	nUpdatesPrevious = nUpdates;
}

TCNetReader::TCNetReader(struct TLtcDisabledOutputs *pLtcDisabledOutputs) :
	m_ptLtcDisabledOutputs(pLtcDisabledOutputs),
	m_nTimeCodePrevious(0xFF)
{
	assert(m_ptLtcDisabledOutputs != 0);
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

	LtcOutputs::Get()->Init();

	led_set_ticks_per_second(LED_TICKS_NO_DATA);
}

void TCNetReader::Stop(void) {
	irq_timer_set(IRQ_TIMER_0, 0);
}

void TCNetReader::Handler(const struct TTCNetTimeCode* pTimeCode) {
	nUpdates++;

	assert(((uint32_t )pTimeCode & 0x3) == 0); // Check if we can do 4-byte compare
	const uint32_t *p = (uint32_t *)pTimeCode;

	if (m_nTimeCodePrevious != *p) {
		m_nTimeCodePrevious = *p;

		if (!m_ptLtcDisabledOutputs->bLtc) {
			LtcSender::Get()->SetTimeCode((const struct TLtcTimeCode *) pTimeCode);
		}

		if (!m_ptLtcDisabledOutputs->bArtNet) {
			ArtNetNode::Get()->SendTimeCode((const struct TArtNetTimeCode *) pTimeCode);
		}

		if (!m_ptLtcDisabledOutputs->bRtpMidi) {
			RtpMidi::Get()->SendTimeCode((const struct _midi_send_tc *)pTimeCode);
		}

		memcpy(&m_tMidiTimeCode, pTimeCode, sizeof(struct _midi_send_tc));

		LtcOutputs::Get()->Update((const struct TLtcTimeCode *)pTimeCode);
	}
}

void TCNetReader::Run(void) {
	LtcOutputs::Get()->UpdateMidiQuarterFrameMessage((const struct TLtcTimeCode *)&m_tMidiTimeCode);

	dmb();
	if (nUpdatesPerSecond >= 24) {
		led_set_ticks_per_second(LED_TICKS_DATA);
	} else {
		LtcOutputs::Get()->ShowSysTime();
		led_set_ticks_per_second(LED_TICKS_NO_DATA);
	}
}
