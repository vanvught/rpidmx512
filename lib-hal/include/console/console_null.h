/**
 * @file console_null.h
 *
 */
/* Copyright (C) 2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef CONSOLE_CONSOLE_NULL_H_
#define CONSOLE_CONSOLE_NULL_H_

#if !defined (CONSOLE_NULL)
# error File should not be included
#endif

#include <cstdint>

typedef enum {
	CONSOLE_BLACK = 0,
	CONSOLE_RED = 1,
	CONSOLE_GREEN = 2,
	CONSOLE_YELLOW = 3,
	CONSOLE_BLUE = 4,
	CONSOLE_MAGENTA = 5
,	CONSOLE_CYAN = 6,
	CONSOLE_WHITE = 7,
	CONSOLE_DEFAULT = 9
} _console_colors;

inline void console_init() {}
inline void console_putc([[maybe_unused]] int i) {}
inline void console_puts([[maybe_unused]] const char *p) {}
inline void console_write([[maybe_unused]] const char *p, [[maybe_unused]] unsigned int i) {}
inline void console_status([[maybe_unused]] uint32_t i, [[maybe_unused]] const char *p) {}
inline void console_error([[maybe_unused]] const char *p) {}

#endif /* INCLUDE_CONSOLE_CONSOLE_NULL_H_ */
