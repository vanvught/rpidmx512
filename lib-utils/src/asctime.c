/**
 * @file time.c
 *
 */
/* Copyright (C) 2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#include <stdio.h>
#include <time.h>

#define MAX_ASC_TIME	50

static char s_buffer[MAX_ASC_TIME + 1];
static const char *pMonthName[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
static const char *pDaysOfWeek[7] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

char *asctime(const struct tm *pTm) {
	if (pTm == NULL) {
		return NULL;
	}

	snprintf(s_buffer, MAX_ASC_TIME, "%s %s %2u %02u:%02u:%02u %04u\n",
			(0 <= pTm->tm_wday && pTm->tm_wday <= 6) ? pDaysOfWeek[pTm->tm_wday] : "???",
			(0 <= pTm->tm_mon && pTm->tm_mon <= 11) ? pMonthName[pTm->tm_mon] : "???",
			(unsigned) pTm->tm_mday, (unsigned) pTm->tm_hour,
			(unsigned) pTm->tm_min, (unsigned) pTm->tm_sec,
			(unsigned) pTm->tm_year + 1900);

	return s_buffer;
}
