#ifndef _blinktask_h
#define _blinktask_h

#include <circle/actled.h>
#include <circle/sched/task.h>

#include "ledblink.h"

class CBlinkTask : public LedBlink, CTask
{
public:
	CBlinkTask (CActLED *pActLED, unsigned nFreqHz);
	~CBlinkTask (void);

	void SetFrequency (unsigned nFreqHz);

	void Run (void);

private:
	CActLED *m_pActLED;
	unsigned m_nusPeriod;
	bool m_bStop;
};

#endif /* */
