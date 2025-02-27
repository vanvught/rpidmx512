/**
 * @file timecode.cpp
 *
 */
/* Copyright (C) 2016-2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "timecode.h"
#include "artnettimecode.h"

#include "console.h"

#define ROW		1
#define COLUMN	80

static char timecode[] =  "--:--:--;-- -----";
#define TIMECODE_LENGTH		(sizeof(timecode) - 1)

constexpr char types[4][8] = {"Film " , "EBU  " , "DF   " , "SMPTE" };

static uint8_t prev_type = 0xFF;	///< Invalid type. Force initial update.

static void itoa_base10(int arg, char *pBuffer) {
	auto *n = pBuffer;

	if (arg == 0) {
		*n++ = '0';
		*n = '0';
		return;
	}

	*n++ = static_cast<char>('0' + (arg / 10));
	*n = static_cast<char>('0' + (arg % 10));
}

void TimeCode::Start() {
	console_save_cursor();
	console_set_cursor(COLUMN, ROW);
	console_set_fg_color(CONSOLE_CYAN);
	console_puts(timecode);
	console_restore_cursor();
}

void TimeCode::Stop() {
	console_set_cursor(COLUMN, ROW);
	console_puts("                 ");
}

void TimeCode::Handler(const struct artnet::TimeCode *pTimeCode) {
	itoa_base10(pTimeCode->Hours, &timecode[0]);
	itoa_base10(pTimeCode->Minutes, &timecode[3]);
	itoa_base10(pTimeCode->Seconds, &timecode[6]);
	itoa_base10(pTimeCode->Frames, &timecode[9]);

	if ((prev_type != pTimeCode->Type) && (pTimeCode->Type < 4)) {
		memcpy(&timecode[12], types[pTimeCode->Type], 5);
		prev_type = pTimeCode->Type;
	}

	console_save_cursor();
	console_set_cursor(COLUMN, ROW);
	console_set_fg_color(CONSOLE_CYAN);
	console_write(timecode, TIMECODE_LENGTH);
	console_restore_cursor();
}
