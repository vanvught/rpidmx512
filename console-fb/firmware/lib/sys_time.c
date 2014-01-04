#include <stdint.h>
#include <time.h>

#include <bcm2835.h>
#include <mcp7941x.h>

volatile uint64_t st_startup_micros = 0;
volatile uint32_t rtc_startup_seconds = 0;

void sys_time_init(void) {
	st_startup_micros = bcm2835_st_read();
	struct rtc_time tm_rtc;
	struct tm tmbuf;

	mcp7941x_start(0x00);
	mcp7941x_get_date_time(&tm_rtc);

	tmbuf.tm_hour = tm_rtc.tm_hour;
	tmbuf.tm_min = tm_rtc.tm_min;
	tmbuf.tm_sec = tm_rtc.tm_sec;

	tmbuf.tm_mday = tm_rtc.tm_mday;
	tmbuf.tm_wday = tm_rtc.tm_wday;
	tmbuf.tm_mon = tm_rtc.tm_mon;
	tmbuf.tm_year = tm_rtc.tm_year;

	tmbuf.tm_isdst = 0; // 0 (DST not in effect, just take RTC time)

	rtc_startup_seconds = mktime(&tmbuf);
}

time_t sys_time(time_t *__timer) {
	// http://www.hackersdelight.org/magic.htm
	// http://stackoverflow.com/questions/1269994/nanoseconds-to-milliseconds-fast-division-by-1000000
	time_t elapsed = (unsigned long long)((bcm2835_st_read() - st_startup_micros) * 0x431bde83) >> (0x12 + 32);

	elapsed = elapsed + rtc_startup_seconds;

	if (__timer != NULL )
		*__timer = elapsed;

	return elapsed;
}
