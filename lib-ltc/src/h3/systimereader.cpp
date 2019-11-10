/**
 * @file systimereader.h
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
#include <assert.h>

#include "h3/systimereader.h"

#include "ltc.h"
#include "timecodeconst.h"

#include "c/led.h"

#include "arm/arm.h"
#include "arm/synchronize.h"

#include "h3_hs_timer.h"
#include "h3_timer.h"
#include "irq_timer.h"

#include "debug.h"

// IRQ Timer0
static volatile bool bTimeCodeAvailable;

static void irq_timer0_handler(uint32_t clo) {
	bTimeCodeAvailable = true;
}

SystimeReader::SystimeReader(struct TLtcDisabledOutputs *pLtcDisabledOutputs, enum TTimecodeTypes tTimecodeType) :
	m_ptLtcDisabledOutputs(pLtcDisabledOutputs),
	m_tTimecodeType(tTimecodeType)
{
	assert(m_ptLtcDisabledOutputs != 0);

	m_nFps = TimeCodeConst::FPS[(int) m_tTimecodeType];
	m_nTimer0Interval = TimeCodeConst::TMR_INTV[(int) m_tTimecodeType];
}

SystimeReader::~SystimeReader(void) {
}

void SystimeReader::Start(void) {
	irq_timer_init();
	irq_timer_set(IRQ_TIMER_0, (thunk_irq_timer_t) irq_timer0_handler);

	H3_TIMER->TMR0_INTV = m_nTimer0Interval;
	H3_TIMER->TMR0_CTRL &= ~(TIMER_CTRL_SINGLE_MODE);
	H3_TIMER->TMR0_CTRL |= (TIMER_CTRL_EN_START | TIMER_CTRL_RELOAD);

	led_set_ticks_per_second(LED_TICKS_DATA);
}

void SystimeReader::Run(void) {
}
