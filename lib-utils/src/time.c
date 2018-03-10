/**
 * @file time.c
 *
 */
/* Copyright (C) 2016-2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include <stddef.h>
#include <time.h>

static const unsigned days_of_month[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

static int isleapyear(const unsigned year) {
	if (year % 100 == 0) {
		return year % 400 == 0;
	}

	return year % 4 == 0;
}

static unsigned getdaysofmonth(const unsigned month, const unsigned year) {
	if ((month == 1) && isleapyear(year)) {
		return 29;
	}

	return days_of_month[month];
}

static struct tm Tm;

struct tm *localtime(const time_t *pTime) {
	unsigned nYear;
	unsigned nMonth;

	if (pTime == 0) {
		return NULL;
	}

	time_t Time = *pTime;

	Tm.tm_sec = Time % 60;
	Time /= 60;
	Tm.tm_min = Time % 60;
	Time /= 60;
	Tm.tm_hour = Time % 24;
	Time /= 24;

	Tm.tm_wday = (Time + 4) % 7;

	nYear = 1970;
	while (1) {
		unsigned nDaysOfYear = isleapyear(nYear) ? 366 : 365;
		if (Time < nDaysOfYear) {
			break;
		}

		Time -= nDaysOfYear;
		nYear++;
	}

	Tm.tm_year = nYear - 1900;
	Tm.tm_yday = Time;

	nMonth = 0;
	while (1) {
		unsigned nDaysOfMonth = getdaysofmonth(nMonth, nYear);
		if (Time < nDaysOfMonth) {
			break;
		}

		Time -= nDaysOfMonth;
		nMonth++;
	}

	Tm.tm_mon = nMonth;
	Tm.tm_mday = Time + 1;

	return &Tm;
}

time_t mktime(struct tm *pTm) {
	unsigned year, month;
	time_t result = 0;

	if (pTm == NULL) {
		return (time_t) -1;
	}

	pTm->tm_year = pTm->tm_year + 2000;

	if (pTm->tm_year < 1970 || pTm->tm_year > 2099) {
		return (time_t) -1;
	}

	for (year = 1970; year < pTm->tm_year; year++) {
		result += isleapyear(year) ? 366 : 365;
	}

	if (pTm->tm_mon < 0 || pTm->tm_mon > 11) {
		return (time_t) -1;
	}

	for (month = 0; month < pTm->tm_mon; month++) {
		result += getdaysofmonth(month, (unsigned) pTm->tm_year);
	}

	if (pTm->tm_mday < 1 || pTm->tm_mday > getdaysofmonth((unsigned) pTm->tm_mon, (unsigned) pTm->tm_year)) {
		return (time_t) -1;
	}

	result += pTm->tm_mday - 1;
	result *= 24;

	if (pTm->tm_hour < 0 || pTm->tm_hour > 23) {
		return (time_t) -1;
	}

	result += pTm->tm_hour;
	result *= 60;

	if (pTm->tm_min < 0 || pTm->tm_min > 59) {
		return (time_t) -1;
	}

	result += pTm->tm_min;
	result *= 60;

	if (pTm->tm_sec < 0 || pTm->tm_sec > 59) {
		return (time_t) -1;
	}

	result += pTm->tm_sec;

	return result;
}
