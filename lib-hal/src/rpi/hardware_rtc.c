/**
 * @file hardware_rtc.c
 *
 */
/* Copyright (C) 2016-2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include <stddef.h>
#include <assert.h>

#include "c/hardware.h"
#include "c/sys_time.h"

#include "rtc.h"

void hardware_rtc_set(const struct hardware_time *tm_hw) {
	assert(tm_hw != 0);

	struct tm tm_rtc;
	struct tm tmbuf;

	tm_rtc.tm_hour = (int) tm_hw->hour;
	tm_rtc.tm_min = (int) tm_hw->minute;
	tm_rtc.tm_sec = (int) tm_hw->second;
	tm_rtc.tm_mday = (int) tm_hw->day;
	tm_rtc.tm_mon = (int) tm_hw->month;
	tm_rtc.tm_year = (int) tm_hw->year;

	if (rtc_start(RTC_PROBE)) {
		rtc_set_date_time(&tm_rtc);
	}

	tmbuf.tm_hour = (int) tm_hw->hour;
	tmbuf.tm_min = (int) tm_hw->minute;
	tmbuf.tm_sec = (int) tm_hw->second;
	tmbuf.tm_mday = (int) tm_hw->day;
	tmbuf.tm_mon = (int) tm_hw->month;
	tmbuf.tm_year = (int) tm_hw->year;
	tmbuf.tm_isdst = 0;

	sys_time_set(&tmbuf);
}
