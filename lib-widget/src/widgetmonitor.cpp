/**
 * @file widgetmonitor.cpp
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

#include <cstdio>
#include <cstdarg>
#include <cstddef>

#include "widgetmonitor.h"

#if !defined (NO_HDMI_OUTPUT)
# include "console.h"
#endif

void WidgetMonitor::Line([[maybe_unused]] int line, [[maybe_unused]] const char *fmt, ...) {
	// For H3, only enabled when NDEBUG is not defined
#if !(defined(NDEBUG) && defined(NO_HDMI_OUTPUT))
	va_list va;

#if !defined (NO_HDMI_OUTPUT)
	console_clear_line(line);
#endif

	if (fmt != nullptr) {
		va_start(va, fmt);
		vprintf(fmt, va);
		va_end(va);
	}
#endif
}
