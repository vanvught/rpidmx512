#if !defined(CONSOLE_ILI9340)
/**
 * @file console.h
 *
 */
/* Copyright (C) 2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include "console.h"
#include "h3_uart0_debug.h"

static _console_colors cur_fore = CONSOLE_WHITE;
static _console_colors cur_back = CONSOLE_BLACK;

int console_init(void) {
	uart0_init();

	console_set_fg_color(cur_fore);
	console_set_bg_color(cur_back);

	return CONSOLE_OK;
}

int console_putc(int c) {
	if (c == '\n') {
		uart0_putc('\r');
	}
	uart0_putc(c);
	return c;
}

int console_puts(const char *s) {
	char c;
	int i = 0;;

	while ((c = *s++) != (char) 0) {
		i++;
		(void) console_putc((int) c);
	}

	return i;
}

void console_newline(void) {
	console_putc('\n');
}

void console_set_fg_color(uint16_t fg) {
	switch (fg) {
	case CONSOLE_BLACK:
		console_puts("\x1b[30m");
		break;
	case CONSOLE_RED:
		console_puts("\x1b[31m");
		break;
	case CONSOLE_GREEN:
		console_puts("\x1b[32m");
		break;
	case CONSOLE_YELLOW:
		console_puts("\x1b[33m");
		break;
	case CONSOLE_WHITE:
		console_puts("\x1b[37m");
		break;
	default:
		console_puts("\x1b[39m");
		break;
	}

	cur_fore = fg;
}

void console_set_bg_color(uint16_t bg) {
	switch (bg) {
	case CONSOLE_BLACK:
		console_puts("\x1b[40m");
		break;
	case CONSOLE_RED:
		console_puts("\x1b[41m");
		break;
	case CONSOLE_WHITE:
		console_puts("\x1b[47m");
		break;
	default:
		console_puts("\x1b[49m");
		break;
	}

	cur_back = bg;
}

void console_write(const char *s, unsigned int n) {
	char c;

	while (((c = *s++) != (char) 0) && (n-- != 0)) {
		(void) console_putc((int) c);
	}
}

int console_error(const char *s) {
	(void) console_puts("\x1b[31m");
	const int i = console_puts(s);
	(void) console_puts("\x1b[37m");

	return i;
}

int console_status(uint16_t color, const char *s) {
	char c;
	int i = 0;

	console_set_fg_color(color);
	console_set_bg_color(CONSOLE_BLACK);

	while ((c = *s++) != (char) 0) {
		i++;
		(void) console_putc((int) c);
	}

	(void) console_putc('\n');

	console_set_fg_color(CONSOLE_WHITE);
//	console_set_bg_color(cur_back);

	return i;
}


#define TO_HEX(i)	((i) < 10) ? (uint8_t)'0' + (i) : (uint8_t)'A' + ((i) - (uint8_t)10)

void console_puthex(uint8_t data) {
	(void) console_putc((int) (TO_HEX(((data & 0xF0) >> 4))));
	(void) console_putc((int) (TO_HEX(data & 0x0F)));
}

void console_puthex_fg_bg(uint8_t data, uint16_t fg, uint16_t bg) {
	console_set_fg_color(fg);
	console_set_bg_color(bg);

	(void) console_putc((int) (TO_HEX(((data & 0xF0) >> 4))));
	(void) console_putc((int) (TO_HEX(data & 0x0F)));

	console_set_fg_color(cur_fore);
	console_set_bg_color(cur_back);
}

#endif
