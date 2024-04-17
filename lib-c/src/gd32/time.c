/**
 * @file time.c
 *
 */
/* Copyright (C) 2021-2024 by Arjan van Vught mailto:info@gd32-dmx.nl
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

#include <stddef.h>
#include <sys/time.h>
#include <stdint.h>
#include <assert.h>

extern volatile uint32_t s_nSysTickMillis;

static uint32_t set_timer;
static struct timeval s_tv;

/*
 * number of seconds and microseconds since the Epoch,
 *     1970-01-01 00:00:00 +0000 (UTC).
 */

int gettimeofday(struct timeval *tv, __attribute__((unused))   struct timezone *tz) {
	assert(tv != 0);

	const uint32_t timer = s_nSysTickMillis; // Millis timer

	uint32_t millis_elapsed;

	if (set_timer >= timer) {
		millis_elapsed = set_timer - timer;
	} else {
		millis_elapsed = timer - set_timer;
	}

	set_timer = timer;

	uint32_t sec = millis_elapsed / 1000U;
	const uint32_t usec = (millis_elapsed - (sec * 1000U)) * 1000U;

	s_tv.tv_sec += (time_t)sec;
	s_tv.tv_usec += (suseconds_t)usec;

	if (s_tv.tv_usec >= 1000000) {
		s_tv.tv_sec++;
		s_tv.tv_usec = 1000000 - s_tv.tv_usec;
	}

	tv->tv_sec = s_tv.tv_sec;
	tv->tv_usec = s_tv.tv_usec;

	return 0;
}

int settimeofday(const struct timeval *tv, __attribute__((unused))  const struct timezone *tz) {
	assert(tv != 0);

	set_timer = s_nSysTickMillis;

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
	gettimeofday(&tv, 0);

	if (__timer != NULL) {
		*__timer = tv.tv_sec;
	}

	return tv.tv_sec;
}
