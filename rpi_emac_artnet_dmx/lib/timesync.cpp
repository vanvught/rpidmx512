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
#include <time.h>

#include "timesync.h"
#include "artnettimesync.h"

#include "c/hardware.h"
#include "console.h"

#include "util.h"


#define ROW		2		///<
#define COLUMN	80		///<

static char timesync[] ALIGNED =  "--:--:-- --/--/--";
static char systime[] ALIGNED =  "--:--:--";

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

TimeSync::TimeSync(void) : m_nSecondsPrevious(60) {
	// 60 : Force initial update for ShowSystemTime
}

TimeSync::~TimeSync(void) {
}

void TimeSync::Start(void) {
	this->Show();
}

void TimeSync::Handler(const struct TArtNetTimeSync *pArtNetTimeSync) {
	struct hardware_time hw_time;

	hw_time.second = pArtNetTimeSync->tm_sec;
	hw_time.minute = pArtNetTimeSync->tm_min;
	hw_time.hour = pArtNetTimeSync->tm_hour;
	hw_time.day = pArtNetTimeSync->tm_mday;
	hw_time.month = pArtNetTimeSync->tm_mon + (uint8_t) 1;
	hw_time.year = (uint16_t) 1900 + ((uint16_t) (pArtNetTimeSync->tm_year_hi) << 8) + (uint16_t) pArtNetTimeSync->tm_year_lo;

	hardware_rtc_set(&hw_time);

	itoa_base10(hw_time.hour, (char *) &timesync[0]);
	itoa_base10(hw_time.minute, (char *) &timesync[3]);
	itoa_base10(hw_time.second, (char *) &timesync[6]);
	itoa_base10(hw_time.year - (100 * (hw_time.year / 100)), (char *) &timesync[9]);
	itoa_base10(hw_time.month, (char *) &timesync[12]);
	itoa_base10(hw_time.day, (char *) &timesync[15]);

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

	itoa_base10(local_time->tm_hour, (char *) &systime[0]);
	itoa_base10(local_time->tm_min, (char *) &systime[3]);
	itoa_base10(local_time->tm_sec, (char *) &systime[6]);

	console_save_cursor();
	console_set_cursor(COLUMN, 0);
	console_set_fg_color(CONSOLE_WHITE);
	console_puts(systime);
	console_restore_cursor();
}
