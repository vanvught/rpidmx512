/**
 * @file i2c_set.c
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

#include <stdint.h>

#if defined(__linux__)
 #include "bcm2835.h"
#elif defined(H3)
 #include "h3_i2c.h"
#else
 #include "bcm2835_i2c.h"
#endif

#if defined(H3)
 #define FUNC_PREFIX(x) h3_##x
#else
 #define FUNC_PREFIX(x) bcm2835_##x
#endif

void i2c_set_address(uint8_t address) {
	FUNC_PREFIX(i2c_setSlaveAddress(address));
}

void i2c_set_clockdivider(uint16_t divider) {
	FUNC_PREFIX(i2c_setClockDivider(divider));
}
void i2c_set_baudrate(uint32_t baudrate) {
	FUNC_PREFIX(i2c_set_baudrate(baudrate));
}
