
/**
 * @file smp.c
 *
 */
/* Copyright (C) 2016 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include <arm/smp.h>
#include <stdint.h>

#if defined (ARM_ALLOW_MULTI_CORE)
#include <stdbool.h>
#include "arm/synchronize.h"

static volatile bool core_is_started;		///<
static start_fn_t start_fn;					///<

/**
 *
 */
void smp_core_main(void) {
	start_fn_t temp_fn = start_fn;
	dmb();
	core_is_started = true;
	temp_fn();
	for (;;)
		;
}

/**
 *
 * @param core_number
 * @param start
 */
void smp_start_core(uint32_t core_number, start_fn_t start) {
	if (core_number == 0 || core_number > 3) {
		return;
	}
	start_fn = start;
	*(uint32_t *) (SMP_CORE_BASE + (core_number * 0x10)) = (uint32_t) _init_core;
	dmb();
	core_is_started = false;
	while (!core_is_started) {
		dmb();
	}
}
#endif

/**
 *
 * @return
 */
uint32_t smp_get_core_number(void) {
	uint32_t core_number;
	asm volatile ("mrc p15, 0, %0, c0, c0, 5" : "=r" (core_number));
	return (core_number & SMP_CORE_MASK);
}
