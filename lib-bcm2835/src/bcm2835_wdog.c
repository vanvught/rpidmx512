/**
 * @file bcm2835_wdog.c
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

#include <stdint.h>
#include <stdbool.h>

#include "bcm2835.h"

#define BCM2835_PM_WDOG_PASSWORD				((uint32_t)0x5a000000)	///<
#define BCM2835_PM_WDOG_TIME_SET            	((uint32_t)0x000fffff)	///<
#define BCM2835_PM_WDOG_RSTC_RESET              ((uint32_t)0x00000102)	///<
#define BCM2835_PM_WDOG_RSTC_WRCFG_CLR          ((uint32_t)0xffffffcf)	///<
#define BCM2835_PM_WDOG_RSTC_WRCFG_FULL_RESET	((uint32_t)0x00000020)	///<

inline static void bcm2835_wdog_start(const uint32_t timeout) {
	uint32_t rstc;

	BCM2835_PM_WDOG->WDOG = BCM2835_PM_WDOG_PASSWORD | (timeout & BCM2835_PM_WDOG_TIME_SET);
	rstc = BCM2835_PM_WDOG->RSTC;
	BCM2835_PM_WDOG->RSTC = BCM2835_PM_WDOG_PASSWORD | (rstc & BCM2835_PM_WDOG_RSTC_WRCFG_CLR) | BCM2835_PM_WDOG_RSTC_WRCFG_FULL_RESET;
}

static bool is_started = false;

void bcm2835_watchdog_stop(void) {
	BCM2835_PM_WDOG->RSTC = BCM2835_PM_WDOG_PASSWORD | BCM2835_PM_WDOG_RSTC_RESET;
	is_started = false;
}

#define WDOG_TIMEOUT 0x0FFFF

void bcm2835_watchdog_init(void) {
	bcm2835_wdog_start(WDOG_TIMEOUT);
	is_started = true;
}

void bcm2835_watchdog_feed(void) {
	if (is_started) {
		bcm2835_wdog_start(WDOG_TIMEOUT);
	}
}
