/**
 * @file showsystime.cpp
 *
 */
/* Copyright (C) 2019-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <time.h>

#include "h3/showsystime.h"

#include "console.h"

static constexpr auto ROW = 0;
static constexpr auto COLUMN = 80;

static char systime[] __attribute__ ((aligned (4))) = "--:--:-- --/--/--";

static void itoa_base10(int nArg, char *pBuffer) {
	char *p = pBuffer;

	if (nArg == 0) {
		*p++ = '0';
		*p = '0';
		return;
	}

	*p++ = '0' + (nArg / 10);
	*p = '0' + (nArg % 10);
}

void ShowSystime::Run() {
	const time_t ltime = time(nullptr);
	const struct tm *pLocalTime = localtime(&ltime);

	if (__builtin_expect((m_nSecondsPrevious == pLocalTime->tm_sec), 0)) {
		return;
	}

	m_nSecondsPrevious = pLocalTime->tm_sec;

	itoa_base10(pLocalTime->tm_hour, &systime[0]);
	itoa_base10(pLocalTime->tm_min, &systime[3]);
	itoa_base10(pLocalTime->tm_sec, &systime[6]);

	itoa_base10(pLocalTime->tm_year - 100, &systime[9]);
	itoa_base10(pLocalTime->tm_mon + 1, &systime[12]);
	itoa_base10(pLocalTime->tm_mday, &systime[15]);

	console_save_cursor();
	console_set_cursor(COLUMN, ROW);
	console_set_fg_color(CONSOLE_WHITE);
	console_puts(systime);
	console_restore_cursor();
}
