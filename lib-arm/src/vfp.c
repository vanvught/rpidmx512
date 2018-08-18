/**
 * @file vfp.c
 *
 */
/* Copyright (C) 2016-2017 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include "arm/synchronize.h"

void vfp_init(void) {
	unsigned cacr;		// Coprocessor Access Control Register
	__asm volatile ("mrc p15, 0, %0, c1, c0, 2" : "=r" (cacr));
	cacr |= 3 << 20;	// bit 20/21, Full Access, CP10
	cacr |= 3 << 22;	// bit 22/23, Full Access, CP11
	__asm volatile ("mcr p15, 0, %0, c1, c0, 2" : : "r" (cacr));
	isb();

#define VFP_FPEXC_EN	(1 << 30)
	__asm volatile ("fmxr fpexc, %0" : : "r" (VFP_FPEXC_EN));

	__asm volatile ("fmxr fpscr, %0" : : "r" (0));
}
