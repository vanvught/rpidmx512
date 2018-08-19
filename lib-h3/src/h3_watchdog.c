/**
 * @file h3_watchdog.c
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

#define CFG_WHOLE_SYSTEM	(0b01 << 0)
#define CFG_ONLY_INT		(0b10 << 0)

#define MODE_ENABLE			(1 << 0)

#define CTRL_RELOAD			((1 << 0) | (0x0a57 << 1))

#if 0
static const uint8_t interval_value_map[] = {
	[1] = 0x1,  /* 1s  */
	[2] = 0x2,  /* 2s  */
	[3] = 0x3,  /* 3s  */
	[4] = 0x4,  /* 4s  */
	[5] = 0x5,  /* 5s  */
	[6] = 0x6,  /* 6s  */
	[8] = 0x7,  /* 8s  */
	[10] = 0x8, /* 10s */
	[12] = 0x9, /* 12s */
	[14] = 0xA, /* 14s */
	[16] = 0xB, /* 16s */
};

#define INVERVAL_VALUE_MIN			1
#define INVERVAL_VALUE_MAX			16

#define INTERVAL_VALUE_MASK			0x0F
#define INTERVAL_VALUE_MAP_SHIFT	4
#endif

void h3_watchdog_enable(void) {
	H3_TIMER->WDOG0_CFG = CFG_WHOLE_SYSTEM;
	H3_TIMER->WDOG0_MODE = 0x10;		// TODO 1s What did we get from U-Boot?
	H3_TIMER->WDOG0_MODE |= MODE_ENABLE;
}

void h3_watchdog_restart(void) {
	H3_TIMER->WDOG0_CTRL = CTRL_RELOAD;
}

void h3_watchdog_disable(void) {
	H3_TIMER->WDOG0_MODE = 0;
}
