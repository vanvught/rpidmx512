/**
 * @file hardware_rtc.c
 *
 */
/* Copyright (C) 2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include <assert.h>

#include "c/hardware.h"
#include "c/sys_time.h"

#include "mcp7941x.h"

void hardware_rtc_set(const struct hardware_time *tm_hw) {
	assert(tm_hw != 0);

	struct rtc_time tm_rtc;
	struct tm tmbuf;

	tm_rtc.tm_hour = (int) tm_hw->hour;
	tm_rtc.tm_min = (int) tm_hw->minute;
	tm_rtc.tm_sec = (int) tm_hw->second;
	tm_rtc.tm_mday = (int) tm_hw->day;
	//tm_rtc.tm_wday = // TODO tm_rtc.tm_wday
	tm_rtc.tm_mon = (int) tm_hw->month - 1;
	tm_rtc.tm_year = (int) tm_hw->year - 2000;	// RTC stores 2 digits only

	if (mcp7941x_start(0x00) != MCP7941X_ERROR) {
		mcp7941x_set_date_time(&tm_rtc);
	}

	tmbuf.tm_hour = tm_rtc.tm_hour;
	tmbuf.tm_min = tm_rtc.tm_min;
	tmbuf.tm_sec = tm_rtc.tm_sec;
	tmbuf.tm_mday = tm_rtc.tm_mday;
	//tmbuf.tm_wday = tm_rtc.tm_wday;
	tmbuf.tm_mon = (int) tm_hw->month;
	tmbuf.tm_year = tm_rtc.tm_year;
	tmbuf.tm_isdst = 0;

	sys_time_set(&tmbuf);
}
