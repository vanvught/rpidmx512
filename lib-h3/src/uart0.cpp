/**
 * @file h3_uart0.cpp
 *
 */
/* Copyright (C) 2018-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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
#include <cstdio>
#include <cstdarg>

#include "h3.h"
#include "h3_uart.h"

void __attribute__((cold)) uart0_init() {
	h3_uart_begin(H3_UART0_BASE, 115200, H3_UART_BITS_8, H3_UART_PARITY_NONE, H3_UART_STOP_1BIT);

	while ((H3_UART0->USR & UART_USR_BUSY) == UART_USR_BUSY) {
		(void) H3_UART0->O00.RBR;
	}
}

void uart0_putc(int c) {
	if (c == '\n') {
		while (!(H3_UART0->LSR & UART_LSR_THRE))
			;
		H3_UART0->O00.THR = static_cast<uint32_t>('\r');
	}

	while (!(H3_UART0->LSR & UART_LSR_THRE))
		;
	
	H3_UART0->O00.THR = static_cast<uint32_t>(c);
}

void uart0_puts(const char *s) {
	while (*s != '\0') {
		if (*s == '\n') {
			uart0_putc('\r');
		}
		uart0_putc(*s++);
	}

//	uart0_putc('\n'); //TODO Add '\n'
}

int uart0_getc() {
	if (__builtin_expect(((H3_UART0->LSR & UART_LSR_DR) != UART_LSR_DR), 1)) {
		return EOF;
	}

	const auto c = static_cast<int>(H3_UART0->O00.RBR);

#if defined (UART0_ECHO)
	uart0_putc(c);
#endif

	return c;
}

static char s_buffer[128];

int uart0_printf(const char *fmt, ...) {
	va_list arp;

	va_start(arp, fmt);

	int i = vsnprintf(s_buffer, sizeof(s_buffer) - 1, fmt, arp);

	va_end(arp);

	char *s = s_buffer;

	while (*s != '\0') {
		if (*s == '\n') {
			uart0_putc('\r');
		}
		uart0_putc(*s++);
	}

	return i;
}
