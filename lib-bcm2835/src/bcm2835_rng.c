/**
 * @file bcm2835_rng.c
 *
 */
/**
 * This code is inspired by:
 *
 * https://github.com/raspberrypi/linux/blob/rpi-3.6.y/drivers/char/hw_random/bcm2708-rng.c
 * https://github.com/rsta2/circle/blob/4354aa20130f1d7daf12e87616823de3d80a9e28/lib/bcmrandom.cpp
 */
/* Copyright (C) 2016-2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#define RNG_CTRL_EN			0x01
#define RNG_WARMUP_COUNT	0x40000

static bool is_initialized = false;

void bcm2835_rng_init(void) {
	if (!is_initialized) {
		is_initialized = true;
		BCM2835_HW_RNG->STATUS = (uint32_t) RNG_WARMUP_COUNT;
		BCM2835_HW_RNG->CTRL = (uint32_t) RNG_CTRL_EN;
	}
}

uint32_t bcm2835_rng_get_number(void) {
	while ((BCM2835_HW_RNG->STATUS >> 24) == 0)
		;

	return BCM2835_HW_RNG->DATA;
}
