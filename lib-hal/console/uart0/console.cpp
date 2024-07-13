/**
 * @file console.cpp
 *
 */
/* Copyright (C) 2018-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include "console.h"

extern "C" {
void uart0_init();
void uart0_putc(int);
void uart0_puts(const char *);

void console_putc(int c) {
	uart0_putc(c);
}

void console_puts(const char *s) {
	uart0_puts(s);
}

void console_error(const char *s) {
	uart0_puts("\x1b[31m");
	uart0_puts(s);
	uart0_puts("\x1b[37m");
}

void console_set_fg_color(uint16_t fg) {
	switch (fg) {
	case CONSOLE_BLACK:
		uart0_puts("\x1b[30m");
		break;
	case CONSOLE_RED:
		uart0_puts("\x1b[31m");
		break;
	case CONSOLE_GREEN:
		uart0_puts("\x1b[32m");
		break;
	case CONSOLE_YELLOW:
		uart0_puts("\x1b[33m");
		break;
	case CONSOLE_WHITE:
		uart0_puts("\x1b[37m");
		break;
	default:
		uart0_puts("\x1b[39m");
		break;
	}
}

void console_set_bg_color(uint16_t bg) {
	switch (bg) {
	case CONSOLE_BLACK:
		uart0_puts("\x1b[40m");
		break;
	case CONSOLE_RED:
		uart0_puts("\x1b[41m");
		break;
	case CONSOLE_WHITE:
		uart0_puts("\x1b[47m");
		break;
	default:
		uart0_puts("\x1b[49m");
		break;
	}
}

void console_write(const char *s, unsigned int n) {
	char c;

	while (((c = *s++) != 0) && (n-- != 0)) {
		console_putc(static_cast<int>(c));
	}
}

void console_status(uint32_t nColour, const char *s) {
	console_set_fg_color(static_cast<uint16_t>(nColour));
	console_set_bg_color(CONSOLE_BLACK);

	uart0_puts(s);
	console_putc('\n');

	console_set_fg_color(CONSOLE_WHITE);
}
}

void __attribute__((cold)) console_init() {
	uart0_init();

	console_set_fg_color(CONSOLE_WHITE);
	console_set_bg_color(CONSOLE_BLACK);
}
