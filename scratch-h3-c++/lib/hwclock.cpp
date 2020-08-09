
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

/*
 * PoC Code -> Do not use in production
 */

#include <cassert>
#include <stdint.h>
#include <stdio.h>
#include <sys/time.h>

#include "hwclock.h"
#include "../lib-hal/rtc/rtc.h"
#include "hardware.h"

#include "debug.h"

HwClock *HwClock::s_pThis = nullptr;

HwClock::HwClock() {
	DEBUG_ENTRY

	assert(s_pThis == nullptr);
	s_pThis = this;

	rtc_start(RTC_PROBE);

	m_bIsConnected = rtc_is_connected();

	DEBUG_EXIT
}

void HwClock::Print() {
	if (!m_bIsConnected) {
		puts("No RTC connected");
		return;
	}

	struct tm tm;
	rtc_get_date_time(&tm);
	printf("%.4d/%.2d/%.2d %.2d:%.2d:%.2d\n", 1900 + tm.tm_year, tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
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

	struct tm tmT1;
	struct tm tmT2;
	uint32_t MicrosT2;

	rtc_get_date_time(&tmT1);

	const uint32_t MicrosT1 = Hardware::Get()->Micros();
	const int32_t nSecondsT1 = tmT1.tm_sec + tmT1.tm_min * 60;
	const time_t nSeconds = mktime(&tmT1);

	while(true) {
		rtc_get_date_time(&tmT2);

		const int32_t nSeconds2 = tmT2.tm_sec + tmT2.tm_min * 60;

		if (nSecondsT1 != nSeconds2) {
			MicrosT2 = Hardware::Get()->Micros();
			break;
		}
	}

	const auto nDelay = MicrosT2 - MicrosT1;

	struct timeval tv;
	tv.tv_sec = nSeconds;
	tv.tv_usec = 1000000 -  static_cast<suseconds_t>(nDelay);

	settimeofday(&tv, nullptr);

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
			rtc_set_date_time(tm);
			break;
		}
	}

	if (bIsWatchdog) {
		Hardware::Get()->WatchdogInit();
	}

	DEBUG_EXIT
}
