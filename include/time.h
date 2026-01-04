/**
 * @file time.h
 *
 */
/* Copyright (C) 2017-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef TIME_H_
#define TIME_H_

#ifndef _TIME_T
#define	_TIME_T
typedef long int time_t;
#endif	/* _TIME_T */

#define CLOCKS_PER_SEC	(1000000U)

struct tm {
	int tm_sec;		///< Seconds.		[0-60]	(1 leap second)
	int tm_min;		///< Minutes.		[0-59]
	int tm_hour;	///< Hours.			[0-23]
	int tm_mday;	///< Day.		 	[1-31]
	int tm_mon;		///< Month.			[0-11]
	int tm_year;	///< Year - 1900
	int tm_wday;	///< Day of week.	[0-6]
	int tm_yday;	///< Days in year.	[0-365]
	int tm_isdst;	///< DST.			[-1/0/1]
};

struct timespec {
	time_t tv_sec;	/* seconds */
	long tv_nsec;	/* nanoseconds */
};

typedef enum {
	CLOCK_REALTIME,
	CLOCK_MONOTONIC
} clockid_t;

#ifdef __cplusplus
extern "C" {
#endif

extern time_t time(time_t *t);
extern time_t mktime(struct tm *tm);
extern struct tm *gmtime(const time_t *timep);
extern struct tm *localtime(const time_t *timep);
extern char *asctime(const struct tm *tm);

extern int clock_getres(clockid_t clockid, struct timespec *res);
extern int clock_gettime(clockid_t clockid, struct timespec *tp);
extern int clock_settime(clockid_t clockid, const struct timespec *tp);

#ifdef __cplusplus
}
#endif

#endif /* TIME_H_ */
