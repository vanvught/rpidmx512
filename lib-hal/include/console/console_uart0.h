/**
 * @file console_uart0.h
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

#ifndef CONSOLE_CONSOLE_UART0_H_
#define CONSOLE_CONSOLE_UART0_H_

#if defined (CONSOLE_FB) || defined (CONSOLE_NULL) || defined (CONSOLE_I2C)
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

void console_init();
void console_putc(int);
void console_puts(const char *);
void console_write(const char *, unsigned int);
void console_status(uint32_t, const char *);
void console_set_fg_color(uint32_t);
void console_set_bg_color(uint32_t);
void console_error(const char *);

#endif /* CONSOLE_CONSOLE_UART0_H_ */
