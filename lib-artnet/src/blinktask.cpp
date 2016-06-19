//
// blinktask.cpp
//
// Circle - A C++ bare metal environment for Raspberry Pi
// Copyright (C) 2016  R. Stange <rsta2@o2online.de>
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#include <assert.h>

#if defined (__circle__)
#include <circle/sched/scheduler.h>
#else
extern "C" {
#include "led.h"
}
#endif

#include "blinktask.h"

#if defined (__circle__)
/**
 *
 * @param pActLED
 * @param nFreqHz
 */
CBlinkTask::CBlinkTask (CActLED *pActLED, unsigned nFreqHz)
:	m_pActLED (pActLED), m_bStop(false)
{
	SetFrequency (nFreqHz);
}

/**
 *
 */
CBlinkTask::~CBlinkTask (void)
{
	m_pActLED = 0;
}

/**
 *
 * @param nFreqHz
 */
void CBlinkTask::SetFrequency (unsigned nFreqHz)
{
	if (nFreqHz == 0) {
		m_bStop = true;
		m_pActLED->Off();
	} else {
		m_bStop = false;
		m_nusPeriod = 1000000 / nFreqHz;
	}
}

/**
 *
 */
void CBlinkTask::Run (void)
{
	while (1)
	{
		assert (m_pActLED != 0);

		if (m_bStop)
		{
			CScheduler::Get ()->Sleep (1);
		}
		else
		{
			m_pActLED->On ();
			CScheduler::Get ()->usSleep (m_nusPeriod);

			m_pActLED->Off ();
			CScheduler::Get ()->usSleep (m_nusPeriod);
		}
	}
}
#else
CBlinkTask::CBlinkTask (unsigned nFreqHz)
:	m_bStop(false)
{
	SetFrequency (nFreqHz);
}

/**
 *
 */
CBlinkTask::~CBlinkTask (void)
{

}

/**
 *
 * @param nFreqHz
 */
void CBlinkTask::SetFrequency (unsigned nFreqHz)
{
	if (nFreqHz == 0)
	{
		led_set_ticks_per_second(0);
	} else
	{
		led_set_ticks_per_second(1000000 / nFreqHz);
	}
}
#endif
