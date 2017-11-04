/**
 * @file blinktask.cpp
 *
 */
/* Copyright (C) 2017 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include <assert.h>
#include <circle/sched/scheduler.h>

#include "circle/blinktask.h"

CBlinkTask::CBlinkTask(CActLED *pActLED, unsigned nFreqHz) : m_pActLED(pActLED), m_bStop(false) {
	SetFrequency(nFreqHz);
}

CBlinkTask::~CBlinkTask(void) {
	m_pActLED = 0;
}

void CBlinkTask::SetFrequency(unsigned nFreqHz) {
	if (nFreqHz == 0) {
		m_bStop = true;
		m_pActLED->Off();
	} else {
		m_bStop = false;
		m_nusPeriod = 1000000 / nFreqHz;
	}
}

void CBlinkTask::Run(void) {
	while (1) {
		assert(m_pActLED != 0);

		if (m_bStop) {
			CScheduler::Get()->Sleep(1);
		} else {
			m_pActLED->On();
			CScheduler::Get()->usSleep(m_nusPeriod);

			m_pActLED->Off();
			CScheduler::Get()->usSleep(m_nusPeriod);
		}
	}
}
