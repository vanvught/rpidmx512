/**
 * @file showsystime.cpp
 *
 */
/* Copyright (C) 2019-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include <time.h>

#include "h3/showsystime.h"
#include "h3/console_fb.h"

static constexpr auto kRow = 0;
static constexpr auto kColumn = 80;

static char systime[] __attribute__ ((aligned (4))) = "--:--:-- --/--/--";

static void Itoa(int v, char *buffer) {
	auto *p = buffer;

	if (v == 0) {
		*p++ = '0';
		*p = '0';
		return;
	}

	*p++ = static_cast<char>('0' + (v / 10));
	*p = static_cast<char>('0' + static_cast<char>(v % 10));
}

void ShowSystime::Run() {
	const auto kTime = time(nullptr);
	const auto *local_time = localtime(&kTime);

	if (__builtin_expect((seconds_previous_ == local_time->tm_sec), 0)) {
		return;
	}

	seconds_previous_ = local_time->tm_sec;

	Itoa(local_time->tm_hour, &systime[0]);
	Itoa(local_time->tm_min, &systime[3]);
	Itoa(local_time->tm_sec, &systime[6]);

	Itoa(local_time->tm_year - 100, &systime[9]);
	Itoa(local_time->tm_mon + 1, &systime[12]);
	Itoa(local_time->tm_mday, &systime[15]);

	console::SaveCursor();
	console::SetCursor(kColumn, kRow);
	console::SetFgColour(console::Colours::kConsoleWhite);
	console::Puts(systime);
    console::RestoreCursor();
}
