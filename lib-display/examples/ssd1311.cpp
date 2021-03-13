/**
 * @file ssd1311.cpp
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

#include "bcm2835.h"

#include "ssd1311.h"

int main(int argc, char **argv) {
	if (getuid() != 0) {
		fprintf(stderr, "Error: Not started with 'root'\n");
		return -1;
	}

	if (bcm2835_init() != 1) {
		fprintf(stderr, "bcm2835_init() failed\n");
		return -2;
	}

	if (bcm2835_i2c_begin() != 1) {
		fprintf(stderr, "bcm2835_i2c_begin() failed\n");
		return -3;
	}

	Ssd1311 display;

	const bool isDetected = display.Start();

	printf("isDetected=%d\n", isDetected);

	if (isDetected) {
		display.PrintInfo();
		display.Cls();

		if (argc == 2) {
			const uint8_t nContrast = atoi(argv[1]);
			printf("Set contrast=%d\n", nContrast);
			display.SetContrast(nContrast);
		}

		display.TextLine(1, "Line 1", 6);
		display.Text(" Text", 5);
		display.TextLine(2, "Line 2", 6);
		display.PutString(" String");
		display.TextLine(3, "Line 3", 6);
		display.PutChar('!');
		display.TextLine(4, "Line 4", 6);
	}

	return 0;
}
