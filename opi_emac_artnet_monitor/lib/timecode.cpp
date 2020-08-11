/**
 * @file timecode.cpp
 *
 */
/* Copyright (C) 2016-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

static char s_aTimecode[] = "--:--:--.-- -----";
static uint8_t nTypePrevious = 0xFF;	///< Invalid type. Force initial update.

static constexpr auto ROW = 1;
static constexpr auto COLUMN = 80;
static constexpr auto TC_LENGTH = sizeof(s_aTimecode) - 1;
static constexpr char TC_TYPES[4][8] __attribute__ ((aligned (4))) = { "Film ", "EBU  ", "DF   ", "SMPTE" };

inline static void itoa_base10(uint32_t nArg, char *pBuffer) {
	char *nDst = pBuffer;

	if (nArg == 0) {
		*nDst++ = '0';
		*nDst = '0';
		return;
	}

	*nDst++ = '0' + (nArg / 10);
	*nDst = '0' + (nArg % 10);
}

TimeCode::TimeCode() {
}

void TimeCode::Start() {
	console_save_cursor();
	console_set_cursor(COLUMN, ROW);
	console_set_fg_color(CONSOLE_CYAN);
	console_puts(s_aTimecode);
	console_restore_cursor();
}

void TimeCode::Stop() {
	console_set_cursor(COLUMN, ROW);
	console_puts("                 ");
}

void TimeCode::Handler(const struct TArtNetTimeCode *ArtNetTimeCode) {
	itoa_base10(ArtNetTimeCode->Hours, &s_aTimecode[0]);
	itoa_base10(ArtNetTimeCode->Minutes, &s_aTimecode[3]);
	itoa_base10(ArtNetTimeCode->Seconds, &s_aTimecode[6]);
	itoa_base10(ArtNetTimeCode->Frames, &s_aTimecode[9]);

	if ((nTypePrevious != ArtNetTimeCode->Type) && (ArtNetTimeCode->Type < 4)) {
		memcpy(&s_aTimecode[12], TC_TYPES[ArtNetTimeCode->Type], 5);
		nTypePrevious = ArtNetTimeCode->Type;
	}

	console_save_cursor();
	console_set_cursor(COLUMN, ROW);
	console_set_fg_color(CONSOLE_YELLOW);
	console_write(s_aTimecode, TC_LENGTH);
	console_restore_cursor();
}
