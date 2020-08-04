/**
 * @file sys_time.c
 *
 */
/* Copyright (C) 2018-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <stdbool.h>
#include <time.h>

#include "h3.h"

#include "../rtc/rtc.h"

#include "debug.h"

static time_t rtc_seconds = 0;
static time_t elapsed_previous = 0;
static uint32_t millis_init = 0;
static bool have_rtc = false;

void __attribute__((weak)) __attribute__((cold)) sys_time_init(void) {
	struct tm tmbuf;
	struct tm tm_rtc;

	millis_init = H3_TIMER->AVS_CNT0;

	DEBUG_PRINTF("millis_init=%u", millis_init);

	/*
	 * The mktime function ignores the specified contents of the tm_wday and tm_yday members of the broken- down time structure.
	 */

	if (!rtc_start(RTC_PROBE)) {
		tmbuf.tm_hour = 0;
		tmbuf.tm_min = 0;
		tmbuf.tm_sec = 0;
		tmbuf.tm_mday = _TIME_STAMP_DAY_;
		tmbuf.tm_mon = _TIME_STAMP_MONTH_ - 1;
		tmbuf.tm_year = _TIME_STAMP_YEAR_ - 1900;
		tmbuf.tm_isdst = 0; // 0 (DST not in effect, just take RTC time)

		rtc_seconds = mktime(&tmbuf);

		DEBUG_PRINTF("%.4d/%.2d/%.2d %.2d:%.2d:%.2d", tmbuf.tm_year, tmbuf.tm_mon, tmbuf.tm_mday, tmbuf.tm_hour, tmbuf.tm_min, tmbuf.tm_sec);
		DEBUG_PRINTF("%s", asctime(localtime((const time_t *) &rtc_seconds)));

		return;
	}

	rtc_get_date_time(&tm_rtc);
	rtc_seconds = mktime(&tm_rtc);
	have_rtc = true;

	DEBUG_PUTS("RTC found");
	DEBUG_PRINTF("%.4d/%.2d/%.2d %.2d:%.2d:%.2d", tm_rtc.tm_year, tm_rtc.tm_mon, tm_rtc.tm_mday, tm_rtc.tm_hour, tm_rtc.tm_min, tm_rtc.tm_sec);
	DEBUG_PRINTF("millis_init/1000=%u, rtc_startup_seconds=%u", millis_init/1000, rtc_seconds);
	DEBUG_PRINTF("%s", asctime(localtime((const time_t *) &rtc_seconds)));
}

void sys_time_set(const struct tm *tmbuf) {
	millis_init = H3_TIMER->AVS_CNT0;
	rtc_seconds = mktime((struct tm *) tmbuf);

	DEBUG_PRINTF("%.4d/%.2d/%.2d %.2d:%.2d:%.2d", tmbuf->tm_year, tmbuf->tm_mon, tmbuf->tm_mday, tmbuf->tm_hour, tmbuf->tm_min, tmbuf->tm_sec);
	DEBUG_PRINTF("millis_init/1000=%u, rtc_startup_seconds=%u", millis_init/1000, rtc_seconds);
	DEBUG_PRINTF("%s", asctime(localtime((const time_t *) &rtc_seconds)));
}

void sys_time_set_systime(time_t seconds) {
	millis_init = H3_TIMER->AVS_CNT0;
	rtc_seconds = seconds;

	DEBUG_PRINTF("millis_init/1000=%u, rtc_startup_seconds=%u", millis_init/1000, rtc_seconds);
	DEBUG_PRINTF("%s", asctime(localtime((const time_t *) &rtc_seconds)));
}

uint32_t millis(void) {
	return H3_TIMER->AVS_CNT0;
}

__attribute__((weak)) time_t time(time_t *__timer) {
	struct tm tm_rtc;

	time_t elapsed = (time_t) (H3_TIMER->AVS_CNT0 - millis_init) / 1000;

	elapsed = elapsed + rtc_seconds;

	if (have_rtc && ((elapsed - elapsed_previous) > (60 * 60))) {
		if (rtc_is_connected()) {

			elapsed_previous = elapsed;

			rtc_get_date_time(&tm_rtc);
			rtc_seconds = mktime(&tm_rtc);

			millis_init = H3_TIMER->AVS_CNT0;
			elapsed = rtc_seconds;

			DEBUG_PRINTF("Updated with RTC [%u]", rtc_seconds);
		} else {
			DEBUG_PUTS("RTC not connected (anymore)");
		}
	}

	if (__timer != NULL) {
		*__timer = elapsed;
	}

	return elapsed;
}
