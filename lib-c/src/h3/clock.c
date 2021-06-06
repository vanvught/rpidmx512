/**
 * @file clock.c
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

#include <time.h>
#include <errno.h>
#include <sys/time.h>
#include <assert.h>

#include <h3.h>

int clock_getres(clockid_t clockid, struct timespec *res) {
	if ((clockid <= CLOCK_MONOTONIC) && (res != 0)) {
		res->tv_sec = 0;
		res->tv_nsec = 1000;
		return 0;
	}

	errno = EINVAL;
	return -1;
}

static uint64_t s_micros = 0;
static uint32_t set_timer = 0;

#define MICROS_SECONDS	1000000

int clock_gettime(clockid_t clockid, struct timespec *tp) {
	assert(tp != 0);

	if (clockid == CLOCK_REALTIME) {
		struct timeval tv;
		gettimeofday(&tv, 0);
		tp->tv_sec = tv.tv_sec;
		tp->tv_nsec = tv.tv_usec * 1000;

		return 0;
	}

	if (clockid == CLOCK_MONOTONIC) {
		const uint32_t timer = H3_TIMER->AVS_CNT0; // Millis timer

		uint32_t timer_elapsed;

		if (set_timer >= timer) {
			timer_elapsed = set_timer - timer;
		} else {
			timer_elapsed = timer - set_timer;
		}

		set_timer = timer;
		s_micros += (timer_elapsed * 1000);

		tp->tv_sec = (time_t)(s_micros / MICROS_SECONDS);
		tp->tv_nsec = 1000 * (suseconds_t) (s_micros - ((uint64_t) tp->tv_sec * MICROS_SECONDS));

		return 0;
	}

	errno = EINVAL;
	return -1;
}

int clock_settime(clockid_t clockid, const struct timespec *tp) {
	assert(tp != 0);

	if (clockid == CLOCK_REALTIME) {
		struct timeval tv;
		tv.tv_sec = tp->tv_sec;
		tv.tv_usec = tp->tv_nsec / 1000;
		settimeofday(&tv, 0);
		return 0;
	}

	errno = EINVAL;
	return -1;
}
