/**
 * @file hwclockrun.cpp
 *
 */
/* Copyright (C) 2020-2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <cstdint>
#include <sys/time.h>
#include <time.h>

#include "hwclock.h"

#include "hardware.h"

#include "debug.h"

enum class Status {
	WAITING, SAMPLING
};

static Status Status = Status::WAITING;
static time_t nSeconds;
static int32_t nSecondsT1;
static struct tm rtcT1;
static struct timeval tvT1;
static struct tm rtcT2;
static struct timeval tvT2;

void HwClock::Process() {
	if (Status == Status::WAITING) {
		if (__builtin_expect(((Hardware::Get()->Millis() - m_nLastHcToSysMillis) > 7200 * 1000), 0)) {
			Status = Status::SAMPLING;

			RtcGet(&rtcT1);
			gettimeofday(&tvT1, nullptr);

			nSecondsT1 = rtcT1.tm_sec + rtcT1.tm_min * 60;
			nSeconds = mktime(&rtcT1);
		}

		return;
	}

	if (Status == Status::SAMPLING) {
		RtcGet(&rtcT2);

		const auto nSeconds2 = rtcT2.tm_sec + rtcT2.tm_min * 60;

		if (nSecondsT1 != nSeconds2) {
			gettimeofday(&tvT2, nullptr);

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
			Status = Status::WAITING;

			DEBUG_PRINTF("%d:%d (%d %d) (%d %d) -> %d", nSecondsT1, nSeconds2, static_cast<int>(tvT1.tv_sec), static_cast<int>(tvT1.tv_usec), static_cast<int>(tvT2.tv_sec), static_cast<int>(tvT2.tv_usec), static_cast<int>(tv.tv_usec));
		}

		return;
	}
}
