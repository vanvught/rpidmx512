/**
 * @file hwclock.cpp
 *
 */
/* Copyright (C) 2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <cassert>
#include <stdint.h>
#include <stdio.h>
#include <sys/time.h>

#include "hwclock.h"

#include "hardware.h"

#include "debug.h"

using namespace rtc;

HwClock *HwClock::s_pThis = nullptr;

HwClock::HwClock() {
	DEBUG_ENTRY

	assert(s_pThis == nullptr);
	s_pThis = this;

	RtcProbe();

	m_nLastHcToSysMillis = Hardware::Get()->Millis();

	DEBUG_EXIT
}

void HwClock::Print() {
	if (!m_bIsConnected) {
		puts("No RTC connected");
		return;
	}

	printf("%s\n", m_nType == MCP7941X ? "MCP7941X" : (m_nType == DS3231 ? "DS3231" : "Unknown"));


	struct rtc_time tm;
	RtcGet(&tm);
	printf("%.4d/%.2d/%.2d %.2d:%.2d:%.2d\n", 1900 + tm.tm_year, 1 + tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
}

/*
 * Set the System Clock from the Hardware Clock.
 */
void HwClock::HcToSys() {
	DEBUG_ENTRY
	if (!m_bIsConnected) {
		DEBUG_EXIT
		return;
	}

	assert(Hardware::Get() != nullptr);
	const bool bIsWatchdog = Hardware::Get()->IsWatchdog();

	if (bIsWatchdog) {
		Hardware::Get()->WatchdogStop();
	}

	struct rtc_time rtcT1;
	struct rtc_time rtcT2;

	struct timeval tvT1;
	struct timeval tvT2;

	RtcGet(&rtcT1);
	gettimeofday(&tvT1, nullptr);

	const int32_t nSecondsT1 = rtcT1.tm_sec + rtcT1.tm_min * 60;

	struct tm tm;

	tm.tm_sec = rtcT1.tm_sec;
	tm.tm_min = rtcT1.tm_min;
	tm.tm_hour = rtcT1.tm_hour;
	tm.tm_mday = rtcT1.tm_mday;
	tm.tm_mon = rtcT1.tm_mon;
	tm.tm_year = rtcT1.tm_year;

	const time_t nSeconds = mktime(&tm);

	while(true) {
		RtcGet(&rtcT2);

		const int32_t nSeconds2 = rtcT2.tm_sec + rtcT2.tm_min * 60;

		if (nSecondsT1 != nSeconds2) {
			gettimeofday(&tvT2, nullptr);
			break;
		}
	}

	struct timeval tv;
	tv.tv_sec = nSeconds;

	if (tvT2.tv_sec == tvT1.tv_sec) {
		tv.tv_usec = 1000000 - (tvT2.tv_usec - tvT1.tv_usec);
	} else {
		if (tvT2.tv_usec - tvT1.tv_usec >= 0) {
			tv.tv_usec = tvT2.tv_usec - tvT1.tv_usec;
		} else {
			tv.tv_usec = tvT1.tv_usec - tvT2.tv_usec;
		}
	}

	settimeofday(&tv, nullptr);

	m_nLastHcToSysMillis = Hardware::Get()->Millis();

	if (bIsWatchdog) {
		Hardware::Get()->WatchdogInit();
	}

	DEBUG_EXIT
}

/*
 * Set the Hardware Clock from the System Clock.
 */
void HwClock::SysToHc() {
	DEBUG_ENTRY
	if (!m_bIsConnected) {
		DEBUG_EXIT
		return;
	}

	assert(Hardware::Get() != nullptr);
	const bool bIsWatchdog = Hardware::Get()->IsWatchdog();

	if (bIsWatchdog) {
		Hardware::Get()->WatchdogStop();
	}

	struct timeval tv1;
	gettimeofday(&tv1, nullptr);

	while (true) {
		struct timeval tv2;
		gettimeofday(&tv2, nullptr);

		if (tv2.tv_sec >= (tv1.tv_sec + 1)) {
			const struct tm *tm = localtime(&tv2.tv_sec);

			struct rtc_time rtc;

			rtc.tm_sec = tm->tm_sec;
			rtc.tm_min = tm->tm_min;
			rtc.tm_hour = tm->tm_hour;
			rtc.tm_mday = tm->tm_mday;
			rtc.tm_mon = tm->tm_mon;
			rtc.tm_year = tm->tm_year;

			RtcSet(&rtc);
			break;
		}
	}

	if (bIsWatchdog) {
		Hardware::Get()->WatchdogInit();
	}

	DEBUG_EXIT
}
