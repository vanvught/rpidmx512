/**
 * @file pwmled.cpp
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

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

#include "tlc59711.h"

#define MAKE16BITS(x)	((uint16_t) ((uint16_t)(x) & 0xFF) << 8 | ((x) & 0xFF))


int main(int argc, char **argv) {
	if (getuid() != 0) {
		fprintf(stderr, "Program is not started as \'root\' (sudo)\n");
		return -1;
	}

	TLC59711 pwmled;

	const uint8_t gbc = 0x30;

	pwmled.SetGbcRed(gbc);
	pwmled.SetGbcGreen(gbc);
	pwmled.SetGbcBlue(gbc);

	pwmled.Dump();

	uint32_t j = 0xFF;

	for (;;) {
		uint8_t c1 = (uint8_t) j;
		uint8_t c2 = (uint8_t) (j >> 8);
		uint8_t c3 = (uint8_t) (j >> 16);

		pwmled.Set(0, c1);
		pwmled.Set(1, c2);
		pwmled.Set(2, c3);

		pwmled.Set(3, c3);
		pwmled.Set(4, c1);
		pwmled.Set(5, c2);

		pwmled.Set(6, c2);
		pwmled.Set(7, c3);
		pwmled.Set(8, c1);

		pwmled.Set(9, (uint8_t) (0xFF - c1));
		pwmled.Set(10, (uint8_t) (0xFF - c2));
		pwmled.Set(11, (uint8_t) (0xFF - c3));

		j <<= 8;

		if (j == 0) {
			j = 0xFF;
		}

		pwmled.Dump();
		pwmled.Update();
		sleep(1);
	}
}

