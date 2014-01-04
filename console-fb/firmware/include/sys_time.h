#ifndef SYS_TIME_C_
#define SYS_TIME_C_

#include <time.h>

extern volatile uint64_t st_startup_micros;
extern volatile uint32_t rtc_startup_seconds;

extern void sys_time_init(void);
extern time_t sys_time (time_t *__timer);

#endif /* SYS_TIME_C_ */
