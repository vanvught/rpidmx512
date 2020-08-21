/**
 * @file time.c
 *
 */
/* Copyright (C) 2019 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
static const char mon_name[][4] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
static const char wday_name[][4] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

char *asctime(const struct tm *pTm) {
	if (pTm == NULL) {
		return NULL;
	}

	snprintf(s_buffer, MAX_ASC_TIME - 1, "%s %s %2d %02d:%02d:%02d %04d\n",
			(0 <= pTm->tm_wday && pTm->tm_wday <= 6) ? wday_name[pTm->tm_wday] : "???",
			(0 <= pTm->tm_mon && pTm->tm_mon <= 11) ? mon_name[pTm->tm_mon] : "???",
			pTm->tm_mday, pTm->tm_hour,
			pTm->tm_min,  pTm->tm_sec,
			pTm->tm_year + 1900);

	return s_buffer;
}
