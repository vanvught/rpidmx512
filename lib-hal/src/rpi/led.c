/**
 * @file led.c
 *
 */
/* Copyright (C) 2015-2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include "bcm2835.h"
#include "arm/synchronize.h"

#include "hardware.h"

static uint32_t ticks_per_second = (uint32_t) (1000000 / 2);

static uint32_t led_counter = 0;
static uint32_t micros_previous = 0;

void led_set_ticks_per_second(uint32_t ticks) {
	ticks_per_second = ticks;
}

uint32_t led_get_ticks_per_second(void) {
	return ticks_per_second;
}

void led_blink(void) {
	if (ticks_per_second == 0) {
		return;
	}

	dsb();
	const uint32_t micros_now = BCM2835_ST->CLO;
	dmb();

	if (micros_now - micros_previous < ticks_per_second) {
		return;
	}

	hardware_led_set((int)(led_counter++ & 0x01));
	micros_previous = micros_now;
}
