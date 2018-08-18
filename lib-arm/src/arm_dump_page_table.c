/**
 * @file arm_dump_page_table.c
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
#include <stdio.h>

#include "arm/synchronize.h"

#define MEGABYTE		0x100000

static void _print_bits(uint32_t u) {
	unsigned i;

	for (i = 0; i < 32; i++) {
		if ((u & 1) == 1) {
			printf("%-2d ", i);
		}
		u = u >> 1;
	}

	printf("\n");
}

static void _dump_page_table(const uint32_t *p) {
	unsigned i;
	uint32_t prev = 0;

	for (i = 0; i < 4096; i++) {
		uint32_t data =  (uint32_t) (*p & 0xFFFFF);
		if (data != prev) {
			printf(" %.4d : %p : 0x%.8x : %.5x -> bits ", i, p, i * MEGABYTE, data);
			_print_bits(data);
			prev = data;
		}
		p++;
	}
	p--;
	i--;

	printf(" %.4d : %p : 0x%.8x : %.5x -> bits ", i, p, i * MEGABYTE, (uint32_t) (*p & 0xFFFFF));
	_print_bits((uint32_t) (*p & 0xFFFFF));
}


void arm_dump_page_table(void) {
	register uint32_t page_table;

	asm volatile ("mrc p15, 0, %0, c2, c0, 0" : "=r" (page_table));
	isb();

	page_table = page_table & ~0xFF;
	printf("TTBR0 at address: %p\n", page_table);

	_dump_page_table((const uint32_t *) page_table);
}
