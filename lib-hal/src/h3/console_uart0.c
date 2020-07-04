#if !(defined (CONSOLE_NONE) || defined (CONSOLE_FB))
/**
 * @file console_uart0.c
 *
 */
/* Copyright (C) 2018-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

int __attribute__((cold)) console_init(void) {
	uart0_init();

	console_set_fg_color(cur_fore);
	console_set_bg_color(cur_back);

	return CONSOLE_OK;
}

void console_putc(int c) {
	if (c == '\n') {
		uart0_putc('\r');
	}
	uart0_putc(c);
}

void console_puts(const char *s) {
	uart0_puts((char *)s);
}

void console_newline(void) {
	console_putc('\n');
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

	cur_fore = fg;
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

	cur_back = bg;
}

void console_write(const char *s, unsigned int n) {
	char c;

	while (((c = *s++) != (char) 0) && (n-- != 0)) {
		console_putc((int) c);
	}
}

void console_error(const char *s) {
	uart0_puts("\x1b[31m");
	uart0_puts((char *)s);
	uart0_puts("\x1b[37m");
}

void console_status(uint32_t color, const char *s) {
	console_set_fg_color(color);
	console_set_bg_color(CONSOLE_BLACK);

	uart0_puts((char *)s);

	console_putc('\n');

	console_set_fg_color(CONSOLE_WHITE);
//	console_set_bg_color(cur_back);
}


#define TO_HEX(i)	((i) < 10) ? (uint8_t)'0' + (i) : (uint8_t)'A' + ((i) - (uint8_t)10)

void console_puthex(uint8_t data) {
	console_putc((TO_HEX(((data & 0xF0) >> 4))));
	console_putc((TO_HEX(data & 0x0F)));
}

void console_puthex_fg_bg(uint8_t data, uint16_t fg, uint16_t bg) {
	console_set_fg_color(fg);
	console_set_bg_color(bg);

	console_putc((TO_HEX(((data & 0xF0) >> 4))));
	console_putc((TO_HEX(data & 0x0F)));

	console_set_fg_color(cur_fore);
	console_set_bg_color(cur_back);
}

void console_putpct_fg_bg(uint8_t data, uint16_t fore, uint16_t back) {
	uint16_t fore_current = cur_fore;
	uint16_t back_current = cur_back;

	cur_fore = fore;
	cur_back = back;

	if (data < 100) {
		console_putc('0' + (data / 10));
		console_putc('0' + (data % 10));
	} else {
		uart0_puts("%%");
	}

	cur_fore = fore_current;
	cur_back = back_current;
}

void console_put3dec_fg_bg(uint8_t data, uint16_t fore, uint16_t back) {
	uint16_t fore_current = cur_fore;
	uint16_t back_current = cur_back;

	cur_fore = fore;
	cur_back = back;

	const uint8_t i = data / 100;

	console_putc('0' + i);

	data -= (i * 100);

	console_putc('0' + (data / 10));
	console_putc('0' + (data % 10));

	cur_fore = fore_current;
	cur_back = back_current;
}
#else
 typedef int ISO_C_forbids_an_empty_translation_unit;
#endif
