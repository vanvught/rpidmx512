/**
 * @file widgetmonitor.cpp
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

#include <cstdio>
#include <cstdarg>
#include <cstddef>

#include "widgetmonitor.h"

#include "hardware.h"
#include "console.h"

void WidgetMonitor::Uptime(uint32_t nLine) {
	auto nUptime = Hardware::Get()->GetUpTime();
	auto ltime = time(nullptr);
	auto *pLocalTime = localtime(&ltime);

	console_save_cursor();
	console_set_cursor(0, nLine);

	const uint32_t nDays = nUptime / (24 * 3600);
	nUptime -= nDays * (24 * 3600);
	const uint32_t nHours = nUptime / 3600;
	nUptime -= nHours * 3600;
	const uint32_t nMinutes = nUptime / 60;
	const uint32_t nSeconds = nUptime - nMinutes * 60;

	printf("Local time %.2d:%.2d:%.2d, uptime %d days, %02d:%02d:%02d",
			pLocalTime->tm_hour, pLocalTime->tm_min, pLocalTime->tm_sec, nDays,
			nHours, nMinutes, nSeconds);

	console_restore_cursor();
}

void WidgetMonitor::Line(__attribute__((unused)) int line, __attribute__((unused)) const char *fmt, ...) {
	// For H3, only enabled when NDEBUG is not defined
#if !(defined(NDEBUG) && defined(H3))
	va_list va;

	console_clear_line(line);

	if (fmt != nullptr) {
		va_start(va, fmt);
		vprintf(fmt, va);
		va_end(va);
	}
#endif
}
