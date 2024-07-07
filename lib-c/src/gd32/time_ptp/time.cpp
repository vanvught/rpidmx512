/**
 * @file time.cpp
 *
 */
/* Copyright (C) 2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include <cstdint>
#include <time.h>
#include <sys/time.h>
#include <cassert>

#include "gd32.h"
#include "gd32_ptp.h"

#if defined (GD32H7XX)
# define enet_ptp_timestamp_function_config(x)		enet_ptp_timestamp_function_config(ENETx, x)
# define enet_ptp_timestamp_update_config(x,y,z)	enet_ptp_timestamp_update_config(ENETx, x, y, z)
# define enet_ptp_system_time_get(x)				enet_ptp_system_time_get(ENETx, x)
#endif

extern "C" {
/*
 * number of seconds and microseconds since the Epoch,
 *     1970-01-01 00:00:00 +0000 (UTC).
 */

int gettimeofday(struct timeval *tv, [[maybe_unused]] struct timezone *tz) {
	assert(tv != 0);

	enet_ptp_systime_struct systime;
	enet_ptp_system_time_get(&systime);

	tv->tv_sec = systime.second;

#if !defined (GD32F4XX)
	const auto nNanoSecond = systime.nanosecond;
#else
	const auto nNanoSecond = gd32::ptp_subsecond_2_nanosecond(systime.subsecond);
#endif

	tv->tv_usec = nNanoSecond / 1000U;

	return 0;
}

int settimeofday(const struct timeval *tv, [[maybe_unused]] const struct timezone *tz) {
	assert(tv != 0);

	const uint32_t nSign = ENET_PTP_ADD_TO_TIME;
	const uint32_t nSecond = tv->tv_sec;
	const uint32_t nNanoSecond = tv->tv_usec * 1000U;
	const auto nSubSecond = gd32::ptp_nanosecond_2_subsecond(nNanoSecond);

	enet_ptp_timestamp_update_config(nSign, nSecond, nSubSecond);

	if (SUCCESS == enet_ptp_timestamp_function_config(ENET_PTP_SYSTIME_INIT)) {
		return 0;
	}

	return -1;
}

/*
 *  time() returns the time as the number of seconds since the Epoch,
       1970-01-01 00:00:00 +0000 (UTC).
 */
time_t time(time_t *__timer) {
	struct timeval tv;
	gettimeofday(&tv, 0);

	if (__timer != nullptr) {
		*__timer = tv.tv_sec;
	}

	return tv.tv_sec;
}
}
