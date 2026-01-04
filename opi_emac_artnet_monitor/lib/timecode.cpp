/**
 * @file timecode.cpp
 *
 */
/* Copyright (C) 2016-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include <cstdint>
#include <string.h>

#include "timecode.h"
#include "artnettimecode.h"
#include "console.h"

static char s_timecode[] = "--:--:--.-- -----";
static uint8_t nTypePrevious = 0xFF;	///< Invalid type. Force initial update.

static constexpr auto kRow = 1;
static constexpr auto kColumn = 80;
static constexpr auto kTcLength = sizeof(s_timecode) - 1;
static constexpr char kTcTypes[4][8] __attribute__ ((aligned (4))) = { "Film ", "EBU  ", "DF   ", "SMPTE" };

static void Itoa(uint32_t value, char *buffer) {
	auto *dst = buffer;

	if (value == 0) {
		*dst++ = '0';
		*dst = '0';
		return;
	}

	*dst++ = static_cast<char>('0' + (value / 10));
	*dst = static_cast<char>('0' + (value % 10));
}

void TimeCode::Start() {
	console::SaveCursor();
	console::SetCursor(kColumn, kRow);
	console::SetFgColour(console::Colours::kConsoleCyan);
	console::Puts(s_timecode);
	console::RestoreCursor();
}

void TimeCode::Stop() {
	console::SetCursor(kColumn, kRow);
	console::Puts("                 ");
}

void TimeCode::Handler(const struct artnet::TimeCode* time_code)
{
    Itoa(time_code->hours, &s_timecode[0]);
	Itoa(time_code->minutes, &s_timecode[3]);
	Itoa(time_code->seconds, &s_timecode[6]);
	Itoa(time_code->frames, &s_timecode[9]);

	if ((nTypePrevious != time_code->type) && (time_code->type < 4)) {
		memcpy(&s_timecode[12], kTcTypes[time_code->type], 5);
		nTypePrevious = time_code->type;
	}

	console::SaveCursor();
	console::SetCursor(kColumn, kRow);
	console::SetFgColour(console::Colours::kConsoleYellow);
	console::Write(s_timecode, kTcLength);
	console::RestoreCursor();
}
