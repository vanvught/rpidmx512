/**
 * @file i2cdetect.cpp
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

#include "i2cdetect.h"

#include "hal_i2c.h"

#if defined(H3)
# include "h3_uart0_debug.h"
# define printf uart0_printf
#endif

inline static bool i2c_is_connected(uint8_t address) {
	uint8_t ret;
	char buf;

	FUNC_PREFIX(i2c_set_address(address));

	if ((address >= 0x30 && address <= 0x37) || (address >= 0x50 && address <= 0x5F)) {
		ret = FUNC_PREFIX(i2c_read(&buf, 1));
	} else {
		/* This is known to corrupt the Atmel AT24RF08 EEPROM */
		ret = FUNC_PREFIX(i2c_write(nullptr, 0));
	}

	return (ret == 0) ? true : false;
}

I2cDetect::I2cDetect() {
	uint8_t first = 0x03, last = 0x77;
	uint8_t i, j;

	FUNC_PREFIX(i2c_set_baudrate(100000));

	puts("\n     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f");

	for (i = 0; i < 128; i += 16) {
		printf("%02x: ", i);
		for (j = 0; j < 16; j++) {
			/* Skip unwanted addresses */
			if (i + j < first || i + j > last) {
				printf("   ");
				continue;
			}

			if (i2c_is_connected((i + j))) {
				printf("%02x ", (i + j));
			} else {
				printf("-- ");
			}
		}

		puts("");
	}
}
