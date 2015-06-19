/**
 * @file sys_time.c
 *
 */
/* Copyright (C) 2015 by Arjan van Vught <pm @ http://www.raspberrypi.org/forum/>
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
#include <time.h>

#include "bcm2835.h"
#include "mcp7941x.h"

static volatile uint64_t sys_time_init_startup_micros = 0;
static volatile uint32_t rtc_startup_seconds = 0;

/**
 * Read time from RTC MCP7941x and set EPOCH time in seconds
 */
void sys_time_init(void) {
	sys_time_init_startup_micros = bcm2835_st_read();
	struct rtc_time tm_rtc;
	struct tm tmbuf;

	if (mcp7941x_start(0x00) == MCP7941X_ERROR)
	{
		tmbuf.tm_hour = 0;
		tmbuf.tm_min = 0;
		tmbuf.tm_sec = 0;
		tmbuf.tm_mday = 1;
		tmbuf.tm_mon = 1;
		tmbuf.tm_year = 15;
		tmbuf.tm_isdst = 0; // 0 (DST not in effect, just take RTC time)

		rtc_startup_seconds = mktime(&tmbuf);
		return;
	}

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

/**
 *
 * @param tmbuf
 */
void sys_time_set(const struct tm *tmbuf)
{
	sys_time_init_startup_micros = bcm2835_st_read();
	rtc_startup_seconds = mktime((struct tm *)tmbuf);
}

/**
 *
 * @param __timer
 * @return
 */
time_t sys_time(time_t *__timer) {
	time_t elapsed = (time_t)((bcm2835_st_read() - sys_time_init_startup_micros) / 1E6);

	elapsed = elapsed + rtc_startup_seconds;

	if (__timer != NULL )
		*__timer = elapsed;

	return elapsed;
}
