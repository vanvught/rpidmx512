/**
 * @file time.c
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

#include <stddef.h>
#include <sys/time.h>
#include <assert.h>

#include "h3.h"

#include "debug.h"

int32_t correction_seconds = 0;
int32_t correction_micros = 0;

/*
 * number of seconds and microseconds since the Epoch,
 *     1970-01-01 00:00:00 +0000 (UTC).
 */

int gettimeofday(struct timeval *tv, __attribute__((unused)) struct timezone *tz) {
	assert(tv != 0);

	const uint32_t micros = H3_TIMER->AVS_CNT0;

	tv->tv_sec = (time_t) (micros / 1000);
	tv->tv_usec = (suseconds_t) micros - (1000 * tv->tv_sec);

	if (correction_seconds < 0) {
		if (__builtin_expect(((time_t) (-correction_seconds) > tv->tv_sec), 0)) {
			DEBUG_PUTS(">ERROR<");
		} else {
			tv->tv_sec += correction_seconds;
		}
	} else {
		tv->tv_sec += correction_seconds;
	}

	if (correction_micros < 0) {
		if ((suseconds_t) (-correction_micros) > tv->tv_usec) {
			DEBUG_PUTS("");
		} else {
			tv->tv_usec += correction_micros;
		}
	} else {
		tv->tv_usec += correction_micros;
	}

	return 0;
}

int settimeofday(const struct timeval *tv, __attribute__((unused)) const struct timezone *tz) {
	assert(tv != 0);

	const uint32_t micros_avs = H3_TIMER->AVS_CNT0;
	const uint32_t micros_avs_seconds = micros_avs / 1000;
	const uint32_t micros_avs_micros = micros_avs - (micros_avs_seconds * 1000);

	correction_seconds = tv->tv_sec - (time_t) micros_avs_seconds;
	correction_micros = tv->tv_usec - (suseconds_t) micros_avs_micros;

	return 0;
}

/*
 *  time() returns the time as the number of seconds since the Epoch,
       1970-01-01 00:00:00 +0000 (UTC).
 */
time_t time(time_t *__timer) {
	time_t elapsed = (time_t) (H3_TIMER->AVS_CNT0 / 1000);

	if (correction_seconds < 0) {
		if (__builtin_expect(((time_t) (-correction_seconds) > elapsed), 0)) {
			DEBUG_PUTS("");
		} else {
			elapsed += correction_seconds;
		}
	} else {
		elapsed += correction_seconds;
	}

	if (__timer != NULL) {
		*__timer = elapsed;
	}

	return elapsed;
}
