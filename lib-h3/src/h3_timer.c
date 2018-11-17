/**
 * @file h3_timer.c
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

#include "h3.h"

#define SCLK_GATING		(1 << 31)

#define AVS_CNT0_EN		(1 << 0)
#define AVS_CNT1_EN		(1 << 1)

#define AVS_CNT0_D_SHIFT	0
#define AVS_CNT1_D_SHIFT	16

#define DIV_N_CNT0	0x2EE0	// 24MHz / 2 / 12000 = 1KHz, period 1ms
#define DIV_N_CNT1	0xC		// 24MHz / 2 / 12 = 1MHz, period 1us

void h3_timer_init(void) {
	H3_CCU->AVS_CLK_CFG |= SCLK_GATING;

	H3_TIMER->AVS_CTRL = AVS_CNT1_EN |AVS_CNT0_EN;
	H3_TIMER->AVS_DIV = (DIV_N_CNT1 << AVS_CNT1_D_SHIFT) | DIV_N_CNT0;

	H3_TIMER->AVS_CNT0 = 0;
	H3_TIMER->AVS_CNT1 = 0;
}

void __msdelay(uint32_t ms) {
	uint32_t t1, t2;

	t1 = H3_TIMER->AVS_CNT0;
	t2 = t1 + ms;
	do {
		t1 = H3_TIMER->AVS_CNT0;
	} while (t2 >= t1);

	return;
}

void __usdelay(uint32_t us) {
	uint32_t t1, t2;

	t1 = H3_TIMER->AVS_CNT1;
	t2 = t1 + us;
	do {
		t1 = H3_TIMER->AVS_CNT1;
	} while (t2 >= t1);

	return;
}
