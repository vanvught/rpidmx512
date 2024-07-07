/**
 * @file hwclock.cpp
 *
 */
/* Copyright (C) 2020-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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
#include <cstdint>
#include <cstdio>
#include <sys/time.h>

#include "hwclock.h"

#include "hardware.h"

#include "debug.h"

HwClock *HwClock::s_pThis;

HwClock::HwClock() {
	assert(s_pThis == nullptr);
	s_pThis = this;
}

void HwClock::Print() {
	if (!m_bIsConnected) {
		puts("No RTC connected");
		return;
	}

	const char *pType = "Unknown";

	switch (m_Type) {
	case rtc::Type::MCP7941X:
		pType = "MCP7941X";
		break;
	case rtc::Type::DS3231:
		pType = "DS3231";
		break;
	case rtc::Type::PCF8563:
		pType = "PCF8563";
		break;
	case rtc::Type::SOC_INTERNAL:
		pType = "SOC_INTERNAL";
		break;
	default:
		break;
	}

	struct tm tm;
	RtcGet(&tm);
	printf("%s %.4d/%.2d/%.2d %.2d:%.2d:%.2d\n", pType, 1900 + tm.tm_year, 1 + tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
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

	const auto bIsWatchdog = Hardware::Get()->IsWatchdog();

	if (bIsWatchdog) {
		Hardware::Get()->WatchdogStop();
	}

	struct tm rtcT1;
	struct timeval tvT1;

	RtcGet(&rtcT1);
	gettimeofday(&tvT1, nullptr);

	const auto nSecondsT1 = rtcT1.tm_sec + rtcT1.tm_min * 60;
	const auto nSeconds = mktime(&rtcT1);

	struct tm rtcT2;
	struct timeval tvT2;

	while(true) {
		RtcGet(&rtcT2);

		const auto nSeconds2 = rtcT2.tm_sec + rtcT2.tm_min * 60;

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

	const auto bIsWatchdog = Hardware::Get()->IsWatchdog();

	if (bIsWatchdog) {
		Hardware::Get()->WatchdogStop();
	}

	struct timeval tv1;
	gettimeofday(&tv1, nullptr);

	while (true) {
		struct timeval tv2;
		gettimeofday(&tv2, nullptr);

		if (tv2.tv_sec >= (tv1.tv_sec + 1)) {
			const auto *tm = gmtime(&tv2.tv_sec);
			RtcSet(tm);
			break;
		}
	}

	if (bIsWatchdog) {
		Hardware::Get()->WatchdogInit();
	}

	DEBUG_EXIT
}
