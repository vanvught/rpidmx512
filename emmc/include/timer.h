#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

#define TIMER_CLO		0x20003004

typedef int useconds_t;

extern void udelay(uint32_t usec);
#define usleep(x) udelay((x))

#define TIMEOUT_WAIT(stop_if_true, usec) 		\
do {							\
	uint32_t compare = (*(volatile uint32_t *)(TIMER_CLO)) + usec; \
	do						\
	{						\
		if(stop_if_true)			\
			break;				\
	} while((*(volatile uint32_t *)(TIMER_CLO)) < compare);			\
} while(0);

#endif

