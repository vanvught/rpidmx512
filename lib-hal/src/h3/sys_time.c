/**
 * @file sys_time.c
 *
 */
/* Copyright (C) 2018-2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#include <stddef.h>
#include <time.h>

#include "h3.h"

#include "rtc.h"

#include "debug.h"

static volatile uint64_t sys_time_init_seconds = 0;
static volatile time_t rtc_startup_seconds = 0;

void sys_time_init(void) {
	struct tm tmbuf;
	struct tm tm_rtc;

	sys_time_init_seconds = h3_read_cnt64() / (uint64_t) 24 / (uint64_t) 1000000;

	/*
	 * The mktime function ignores the specified contents of the tm_wday and tm_yday members of the broken- down time structure.
	 */

	if (!rtc_start(RTC_PROBE)) {
		tmbuf.tm_hour = 0;
		tmbuf.tm_min = 0;
		tmbuf.tm_sec = 0;
		tmbuf.tm_mday = 1;
		tmbuf.tm_mon = 0;
		tmbuf.tm_year = 20;
		tmbuf.tm_isdst = 0; // 0 (DST not in effect, just take RTC time)

		rtc_startup_seconds = mktime(&tmbuf);
		return;
	}

	rtc_get_date_time(&tm_rtc);
	rtc_startup_seconds = mktime(&tm_rtc);

	DEBUG_PUTS("RTC found");
	DEBUG_PRINTF("%.4d/%.2d/%.2d %.2d:%.2d:%.2d", tm_rtc.tm_year, tm_rtc.tm_mon, tm_rtc.tm_mday, tm_rtc.tm_hour, tm_rtc.tm_min, tm_rtc.tm_sec);
	DEBUG_PRINTF("sys_time_init_startup_seconds=%u, rtc_startup_seconds=%u", sys_time_init_seconds, rtc_startup_seconds);
	DEBUG_PRINTF("%s", asctime(localtime((const time_t *) &rtc_startup_seconds)));
}

void sys_time_set(const struct tm *tmbuf) {
	sys_time_init_seconds = h3_read_cnt64() / (uint64_t) 24 / (uint64_t) 1000000;
	rtc_startup_seconds = mktime((struct tm *) tmbuf);

	DEBUG_PRINTF("%.4d/%.2d/%.2d %.2d:%.2d:%.2d", tmbuf->tm_year, tmbuf->tm_mon, tmbuf->tm_mday, tmbuf->tm_hour, tmbuf->tm_min, tmbuf->tm_sec);
	DEBUG_PRINTF("sys_time_init_startup_seconds=%u, rtc_startup_seconds=%u", sys_time_init_seconds, rtc_startup_seconds);
	DEBUG_PRINTF("%s", asctime(localtime((const time_t *) &rtc_startup_seconds)));
}

void sys_time_set_systime(time_t seconds) {
	sys_time_init_seconds = h3_read_cnt64() / (uint64_t) 24 / (uint64_t) 1000000;
	rtc_startup_seconds = seconds;

	DEBUG_PRINTF("sys_time_init_startup_seconds=%u, rtc_startup_seconds=%u", sys_time_init_seconds, rtc_startup_seconds);
	DEBUG_PRINTF("%s", asctime(localtime((const time_t *) &rtc_startup_seconds)));
}

uint32_t millis(void) {
	return (uint32_t) ((uint64_t) (h3_read_cnt64() / (uint64_t) 24 / (uint64_t) 1000));
}

time_t time(time_t *__timer) {
	time_t elapsed = (time_t) ((uint64_t) (h3_read_cnt64() / (uint64_t) 24 / (uint64_t) 1000000) - sys_time_init_seconds);

	elapsed = elapsed + rtc_startup_seconds;

	if (__timer != NULL) {
		*__timer = elapsed;
	}

	return elapsed;
}
