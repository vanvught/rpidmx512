/**
 * @file arm_dump_vector_table.c
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

#include <stdio.h>

#include "arm/synchronize.h"

void arm_dump_vector_table(void) {
	unsigned vector_table;
	asm volatile ("mrc p15, 0, %0, c12, c0, 0" : "=r" (vector_table));
	isb();
	printf("Vector table at address: %p\n", vector_table);

	const unsigned *p = (unsigned *)vector_table;

	printf(" Reset: %p\n", p[0]);
	printf(" Undefined Instruction: %p\n", p[1]);
	printf(" SWI: %p\n", p[2]);
	printf(" Prefetch Abort: %p\n", p[3]);
	printf(" Data Abort: %p\n", p[4]);
	printf(" Reserved: %p\n", p[5]);
	printf(" IRQ: %p\n", p[6]);
	printf(" FIQ: %p\n", p[7]);
}
