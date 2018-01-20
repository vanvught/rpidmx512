/**
 * @file i2cdetect.c
 *
 */
/* Copyright (C) 2017-2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include "i2c.h"

#include "software_version.h"

int main(int argc, char **argv) {
	uint8_t first = 0x03, last = 0x77;
	uint8_t i, j;
	uint8_t address;

	if(!i2c_begin()) {
		return -1;
	}

	i2c_set_clockdivider(2500); // 100kHz

	printf("[V%s] Compiled on %s at %s\n\n", SOFTWARE_VERSION, __DATE__, __TIME__);

	for (address = 0x00; address <= 0x7F; address++) {
		/* Skip unwanted addresses */
		if (address < first || address > last) {
			continue;
		}

		if (i2c_is_connected(address)) {
			printf("0x%.2X : 0x%.2X : %s\n", (unsigned int) address, (unsigned int) (address << 1), i2c_lookup_device(address));
		}
	}


	puts("\n     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f");

	for (i = 0; i < 128; i += 16) {
		printf("%02x: ", (unsigned int) i);
		for (j = 0; j < 16; j++) {
			/* Skip unwanted addresses */
			if (i + j < first || i + j > last) {
				printf("   ");
				continue;
			}

			if (i2c_is_connected((i + j))) {
				printf("%02x ", (unsigned int) (i + j));

			} else {
				printf("-- ");
			}
		}

		puts("");
	}

	return 0;
}
