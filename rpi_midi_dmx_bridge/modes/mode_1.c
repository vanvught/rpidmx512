/**
 * @file mode_1.c
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

#define DEBUG

#include <stdint.h>
#include <stdbool.h>

#include "tables.h"
#include "bridge_params.h"
#include "midi.h"

static const struct _midi_message *midi_message;

#ifdef DEBUG
#include <stdio.h>
#include "console.h"
#include "bcm2835.h"

static uint32_t ticks_per_second = (uint32_t) (1E6 / 2);
static uint32_t micros_previous = 0;
static int counter = 0;
#endif

/**
 *
 */
void mode_1(void) {
#ifdef DEBUG
	const uint32_t micros_now = BCM2835_ST->CLO;

	if (micros_now - micros_previous < ticks_per_second) {
		return;
	}

	if (counter == 0) {
		console_newline();
	}

	printf("%d|", counter++);

	micros_previous = micros_now;
#endif
}

INITIALIZER(modes, mode_1)


/**
 *
 */
void mode_1_monitor(void) {
#ifdef DEBUG
	printf("%s\n", __FUNCTION__);
#endif
}

INITIALIZER(modes_monitor, mode_1_monitor)


/**
 *
 */
void mode_1_init(void) {
	midi_message = (const struct _midi_message *) midi_message_get();
#ifdef DEBUG
	printf("%s\n", __FUNCTION__);
#endif
}

INITIALIZER(modes_init, mode_1_init)
