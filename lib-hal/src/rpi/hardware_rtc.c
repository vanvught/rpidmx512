/**
 * @file hardware_rtc.c
 *
 */
/* Copyright (C) 2016-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <stdbool.h>
#include <assert.h>

#include "c/sys_time.h"

#include "../rtc/rtc.h"

#include "debug.h"

void hardware_rtc_set(const struct tm *tm_rtc) {
	DEBUG_PRINTF("%.4d/%.2d/%.2d %.2d:%.2d:%.2d", tm_rtc->tm_year, tm_rtc->tm_mon, tm_rtc->tm_mday, tm_rtc->tm_hour, tm_rtc->tm_min, tm_rtc->tm_sec);

	if (rtc_start(RTC_PROBE)) {
		rtc_set_date_time(tm_rtc);
	}

	sys_time_set(tm_rtc);
}
