/**
 * @file h3_spinlock.c
 *
 */
/* Copyright (C) 2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <assert.h>

#include "h3.h"
#include "h3_ccu.h"

#define MAX_LOCKS	32

void __attribute__((cold)) h3_spinlock_init(void) {
	H3_CCU->BUS_CLK_GATING1 |= CCU_BUS_CLK_GATING1_SPINLOCK;
	H3_CCU->BUS_SOFT_RESET1 |= CCU_BUS_SOFT_RESET1_SPINLOCK;
}

uint32_t h3_spinlock_check(uint32_t lock) {
	assert(lock < MAX_LOCKS);

	return H3_SPINLOCK->STATUS & (1U << lock);
}

void h3_spinlock_lock(uint32_t lock) {
	assert(lock < MAX_LOCKS);

	for (;;) {
		if ( H3_SPINLOCK->LOCK[lock] == 0) {
			break;
		}
	}
}

void h3_spinlock_unlock(uint32_t lock) {
	assert(lock < MAX_LOCKS);

	H3_SPINLOCK->LOCK[lock] = 0;
}
