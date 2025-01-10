/**
 * @file debug_exception.cpp
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

#include <cstdio>

#include "h3.h"

void console_set_fg_color(uint32_t);

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

extern "C" void debug_exception(unsigned int type, unsigned int address) {
	__sync_synchronize();

	console_set_fg_color(CONSOLE_RED);

	if (type == 0) {
		printf("\nUndefined exception at address: %p\n", (void *)address);
	} else if (type == 1) {
		printf("\nPrefetch abort at address: %p\n", (void *)address);
	} else if (type == 2) {
		volatile unsigned int datafaultaddr;
		asm volatile ("mrc p15, 0, %[dfa], c6, c0, 0\n\t" : [dfa] "=r" (datafaultaddr));
		printf("\nData abort at address: %p -> %p\n", (void *)address, (void *)datafaultaddr);
	} else {
		printf("\nUnknown exception! [%u]\n", type);
	}

	console_set_fg_color(CONSOLE_WHITE);

	H3_TIMER->WDOG0_MODE = 0;

	for(;;);
}
