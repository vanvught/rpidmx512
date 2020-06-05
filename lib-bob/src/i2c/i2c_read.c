/**
 * @file i2c_read.c
 *
 */
/* Copyright (C) 2017-2019 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "i2c.h"

#if defined(__linux__)
 #include "bcm2835.h"
 #define udelay bcm2835_delayMicroseconds
#elif defined(H3)
 #include "h3.h"
#else
 #include "bcm2835.h"
#endif

uint8_t i2c_read_uint8(void) {
	uint8_t buf[1] = { 0 };

	FUNC_PREFIX(i2c_read((char *) buf, 1));

	return buf[0];
}

uint16_t i2c_read_uint16(void) {
	uint8_t buf[2] = { 0, 0 };

	FUNC_PREFIX(i2c_read((char *) buf, 2));

	return (uint16_t) ((uint16_t) buf[0] << 8 | (uint16_t) buf[1]);
}

uint8_t i2c_read_reg_uint8(uint8_t reg) {
	uint8_t buf[2] = { 0, 0 };

	buf[0] = reg;

	FUNC_PREFIX(i2c_write((char *) &buf[0], 1));

	return i2c_read_uint8();
}

uint16_t i2c_read_reg_uint16(uint8_t reg) {
	uint8_t buf[2] = { 0, 0 };

	buf[0] = reg;

	FUNC_PREFIX(i2c_write((char *) &buf[0], 1));

	return i2c_read_uint16();
}

uint16_t i2c_read_reg_uint16_delayus(uint8_t reg, uint32_t delayus) {
	uint8_t buf[2] = { 0, 0 };

	buf[0] = reg;

	FUNC_PREFIX(i2c_write((char *) &buf[0], 1));

	udelay(delayus);

	return i2c_read_uint16();
}
