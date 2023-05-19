/**
 * @file i2cdetect.cpp
 *
 */
/* Copyright (C) 2020-2022 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <cstdio>

#include "i2cdetect.h"

#include "hal_i2c.h"

inline static bool i2c_is_connected(uint8_t nAddress) {
	uint8_t nResult;
	char buffer;

	FUNC_PREFIX(i2c_set_address(nAddress));

	if ((nAddress >= 0x30 && nAddress <= 0x37) || (nAddress >= 0x50 && nAddress <= 0x5F)) {
		nResult = FUNC_PREFIX(i2c_read(&buffer, 1));
	} else {
		/* This is known to corrupt the Atmel AT24RF08 EEPROM */
		nResult = FUNC_PREFIX(i2c_write(nullptr, 0));
	}

	return (nResult == 0) ? true : false;
}

static constexpr uint32_t FIRST = 0x03;
static constexpr uint32_t LAST = 0x77;

I2cDetect::I2cDetect() {
	FUNC_PREFIX(i2c_set_baudrate(100000));

	puts("\n     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f");

	for (uint32_t i = 0; i < 128; i = (i + 16)) {
		printf("%02x: ", i);
		for (uint32_t j = 0; j < 16; j++) {
			/* Skip unwanted addresses */
			if ((i + j < FIRST) || (i + j > LAST)) {
				printf("   ");
				continue;
			}

			if (i2c_is_connected(static_cast<uint8_t>(i + j))) {
				printf("%02x ", i + j);
			} else {
				printf("-- ");
			}
		}

		puts("");
	}

	FUNC_PREFIX(i2c_begin());
}
