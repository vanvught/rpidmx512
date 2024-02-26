/**
 * @file debug_exception.c
 *
 */
/* Copyright (C) 2018-2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <stdio.h>

#include "console.h"

#if defined (H3)
# include "h3.h"
#else
void bcm2835_watchdog_stop(void);
#endif

void debug_exception(unsigned int type, unsigned int address) {
	__sync_synchronize();

	console_set_fg_color(CONSOLE_RED);

	if (type == 0) {
		printf("\nUndefined exception at address: %p\n",address);
	} else if (type == 1) {
		printf("\nPrefetch abort at address: %p\n",address);
	} else if (type == 2) {
		volatile unsigned int datafaultaddr;
		asm volatile ("mrc p15, 0, %[dfa], c6, c0, 0\n\t" : [dfa] "=r" (datafaultaddr));
		printf("\nData abort at address: %p -> %p\n", address, datafaultaddr);
	} else {
		printf("\nUnknown exception! [%d]\n", type);
	}

	console_set_fg_color(CONSOLE_WHITE);

#if defined (H3)
	H3_TIMER->WDOG0_MODE = 0;
#else
	bcm2835_watchdog_stop();
#endif

	for(;;);
}
