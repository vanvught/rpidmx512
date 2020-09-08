/**
 * @file timesync.cpp
 *
 */
/* Copyright (C) 2017-2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#include <string.h>
#include <time.h>

#include "timesync.h"
#include "artnettimesync.h"

#include "hardware.h"
#include "console.h"

#define ROW		2
#define COLUMN	80

static char timesync[] =  "--:--:-- --/--/--";
static char systime[] =  "--:--:--";

static void itoa_base10(int arg, char *pBuffer) {
	char *n = pBuffer;

	if (arg == 0) {
		*n++ = '0';
		*n = '0';
		return;
	}

	*n++ = '0' + (arg / 10);
	*n = '0' + (arg % 10);
}

TimeSync::TimeSync(void) : m_nSecondsPrevious(60) {
	// 60 : Force initial update for ShowSystemTime
}

void TimeSync::Start(void) {
	this->Show();
}

void TimeSync::Handler(const struct TArtNetTimeSync *pArtNetTimeSync) {
	struct tm hw_time;

	hw_time.tm_sec = pArtNetTimeSync->tm_sec;
	hw_time.tm_min = pArtNetTimeSync->tm_min;
	hw_time.tm_hour = pArtNetTimeSync->tm_hour;
	hw_time.tm_mday = pArtNetTimeSync->tm_mday;
	hw_time.tm_mon = pArtNetTimeSync->tm_mon;
	hw_time.tm_year = (pArtNetTimeSync->tm_year_hi << 8) +  pArtNetTimeSync->tm_year_lo;

	Hardware::Get()->SetTime(&hw_time);

	itoa_base10(hw_time.tm_hour, &timesync[0]);
	itoa_base10(hw_time.tm_min, &timesync[3]);
	itoa_base10(hw_time.tm_sec, &timesync[6]);
	itoa_base10(hw_time.tm_year - (100 * (hw_time.tm_year / 100)), &timesync[9]);
	itoa_base10(hw_time.tm_mon, &timesync[12]);
	itoa_base10(hw_time.tm_mday, &timesync[15]);

	this->Show();
}

void TimeSync::Show(void) {
	console_save_cursor();
	console_set_cursor(COLUMN, ROW);
	console_set_fg_color(CONSOLE_BLUE);
	console_puts(timesync);
	console_restore_cursor();
}

void TimeSync::ShowSystemTime(void) {
	time_t ltime;
	struct tm *local_time;

	ltime = time(NULL);
	local_time = localtime(&ltime);

	if (m_nSecondsPrevious == local_time->tm_sec) {
		return;
	}

	m_nSecondsPrevious = local_time->tm_sec;

	itoa_base10(local_time->tm_hour, &systime[0]);
	itoa_base10(local_time->tm_min, &systime[3]);
	itoa_base10(local_time->tm_sec, &systime[6]);

	console_save_cursor();
	console_set_cursor(COLUMN, 0);
	console_set_fg_color(CONSOLE_WHITE);
	console_puts(systime);
	console_restore_cursor();
}
