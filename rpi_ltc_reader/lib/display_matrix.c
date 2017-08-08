/**
 * @file display_matrix.c
 *
 */
/* Copyright (C) 2017 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include "d8x8matrix.h"
#include "device_info.h"

#include "util.h"

#define SEGMENTS	8

static device_info_t device_info;

static uint8_t buffer[SEGMENTS] ALIGNED;

/**
 *
 */
void display_matrix_init(const uint8_t intensity) {
	device_info.chip_select = 2;
	device_info.speed_hz = 0;

	d8x8matrix_init(&device_info, SEGMENTS, intensity);
	d8x8matrix_cls(&device_info);
}

/**
 *
 * @param timecode
 */
void display_matrix(const char *timecode) {
	buffer[0] = (uint8_t) (timecode[0]);
	buffer[1] = (uint8_t) (timecode[1]);
	buffer[2] = (uint8_t) (timecode[3]);
	buffer[3] = (uint8_t) (timecode[4]);
	buffer[4] = (uint8_t) (timecode[6]);
	buffer[5] = (uint8_t) (timecode[7]);
	buffer[6] = (uint8_t) (timecode[9]);
	buffer[7] = (uint8_t) (timecode[10]);

	d8x8matrix_write(&device_info, (char *)buffer, SEGMENTS);
}
