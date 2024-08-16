/**
 * @file time.c
 *
 */
/* Copyright (C) 2021-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#pragma GCC push_options
#pragma GCC optimize ("O2")

#include <cstddef>
#include <sys/time.h>
#include <cstdint>
#include <cassert>

extern volatile uint32_t gv_nSysTickMillis;

static uint32_t nPreviousSysTickMillis;
static struct timeval s_tv;

extern "C" {
/*
 * number of seconds and microseconds since the Epoch,
 *     1970-01-01 00:00:00 +0000 (UTC).
 */

int gettimeofday(struct timeval *tv, __attribute__((unused))    struct timezone *tz) {
	assert(tv != 0);

	const auto nCurrentSysTickMillis = gv_nSysTickMillis;

	uint32_t nMillisElapsed;

	if (nCurrentSysTickMillis >= nPreviousSysTickMillis) {
		nMillisElapsed = nCurrentSysTickMillis - nPreviousSysTickMillis;
	} else {
		nMillisElapsed = (UINT32_MAX - nPreviousSysTickMillis) + nCurrentSysTickMillis + 1;
	}

	nPreviousSysTickMillis = nCurrentSysTickMillis;

	const auto nSeconds = nMillisElapsed / 1000U;
	const auto nMicroSeconds = (nMillisElapsed % 1000U) * 1000U;

	s_tv.tv_sec += static_cast<time_t>(nSeconds);
	s_tv.tv_usec += static_cast<suseconds_t>(nMicroSeconds);

	if (s_tv.tv_usec >= 1000000) {
		s_tv.tv_sec++;
		s_tv.tv_usec -= 1000000;
	}

	tv->tv_sec = s_tv.tv_sec;
	tv->tv_usec = s_tv.tv_usec;

	return 0;
}

int settimeofday(const struct timeval *tv, __attribute__((unused))  const struct timezone *tz) {
	assert(tv != 0);

	struct timeval g;
	gettimeofday(&g, nullptr);

	nPreviousSysTickMillis = gv_nSysTickMillis;

	s_tv.tv_sec = tv->tv_sec;
	s_tv.tv_usec = tv->tv_usec;

	return 0;
}

/*
 *  time() returns the time as the number of seconds since the Epoch,
       1970-01-01 00:00:00 +0000 (UTC).
 */
time_t time(time_t *__timer) {
	struct timeval tv;
	gettimeofday(&tv, nullptr);

	if (__timer != nullptr) {
		*__timer = tv.tv_sec;
	}

	return tv.tv_sec;
}
}
