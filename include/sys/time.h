/**
 * @file time.h
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

#ifndef SYS_TIME_H_
#define SYS_TIME_H_

#ifndef _TIME_T
#define	_TIME_T
typedef long time_t;
#endif	/* _TIME_T */

#ifndef	_SUSECONDS_T
#define	_SUSECONDS_T
typedef long suseconds_t;
#endif	/* _SUSECONDS_T */

struct timeval {
	time_t tv_sec; /* seconds */
	suseconds_t tv_usec; /* microseconds */
};

struct timezone {
	int tz_minuteswest; /* minutes west of Greenwich */
	int tz_dsttime; /* type of DST correction */
};

#ifdef __cplusplus
extern "C" {
#endif

extern int gettimeofday(struct timeval *tv, struct timezone *tz);
extern int settimeofday(const struct timeval *tv, const struct timezone *tz);

#ifdef __cplusplus
}
#endif

#endif /* SYS_TIME_H_ */
