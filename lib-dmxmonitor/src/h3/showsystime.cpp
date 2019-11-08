/**
 * @file showsystime.cpp
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

#include <stdint.h>
#include <stddef.h>
#include <time.h>

#include "h3/showsystime.h"

#include "console.h"

#ifndef ALIGNED
 #define ALIGNED __attribute__ ((aligned (4)))
#endif

#define ROW		0
#define COLUMN	80

static char systime[] ALIGNED =  "--:--:-- --/--/--";

static void itoa_base10(int arg, char *buf) {
	char *n = buf;

	if (arg == 0) {
		*n++ = '0';
		*n = '0';
		return;
	}

	*n++ = (char) '0' + (char) (arg / 10);
	*n = (char) '0' + (char) (arg % 10);
}

ShowSystime::ShowSystime(void): m_nSecondsPrevious(60) {
}

ShowSystime::~ShowSystime(void) {
}

void ShowSystime::Run(void) {
	time_t ltime;
	struct tm *local_time;

	ltime = time(NULL);
	local_time = localtime(&ltime);

	if (m_nSecondsPrevious == local_time->tm_sec) {
		return;
	}

	m_nSecondsPrevious = local_time->tm_sec;

	itoa_base10(local_time->tm_hour, (char *) &systime[0]);
	itoa_base10(local_time->tm_min, (char *) &systime[3]);
	itoa_base10(local_time->tm_sec, (char *) &systime[6]);

	itoa_base10(local_time->tm_year - 100, (char *) &systime[9]);
	itoa_base10(local_time->tm_mon + 1, (char *) &systime[12]);
	itoa_base10(local_time->tm_mday, (char *) &systime[15]);

	console_save_cursor();
	console_set_cursor(COLUMN, ROW);
	console_set_fg_color(CONSOLE_WHITE);
	console_puts(systime);
	console_restore_cursor();
}
