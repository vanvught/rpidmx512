#include <assert.h>
#include <circle/sched/scheduler.h>

#include "blinktask.h"

CBlinkTask::CBlinkTask (CActLED *pActLED, unsigned nFreqHz)
:	m_pActLED (pActLED), m_bStop(false)
{
	SetFrequency (nFreqHz);
}

CBlinkTask::~CBlinkTask (void)
{
	m_pActLED = 0;
}

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
