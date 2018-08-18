/**
 * @file i2c_write.c
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

#if defined(H3)
#else
 #include "bcm2835.h"
#endif

#if defined(__linux__)
#elif defined(H3)
 #include "h3_i2c.h"
#else
 #include "bcm2835_i2c.h"
#endif

#include "i2c.h"

#if defined(H3)
 #define FUNC_PREFIX(x) h3_##x
#else
 #define FUNC_PREFIX(x) bcm2835_##x
#endif

void i2c_write_nb(const char *data, uint32_t length) {
	(void) FUNC_PREFIX(i2c_write(data, length));
}

#if defined(__linux__) || defined(H3)
void i2c_write(uint8_t data) {
	(void) FUNC_PREFIX(i2c_write((char *)&data, 1));
}

void i2c_write_reg_uint8(uint8_t reg, uint8_t data) {
	char buffer[2];

	buffer[0] = reg;
	buffer[1] = data;

	FUNC_PREFIX(i2c_write(buffer, 2));
}

void i2c_write_reg_uint16(uint8_t reg, uint16_t data) {
	char buffer[3];

	buffer[0] = reg;
	buffer[1] = (char) (data >> 8);
	buffer[2] = (char) (data & 0xFF);

	FUNC_PREFIX(i2c_write(buffer, 3));
}
#else
void i2c_write(uint8_t data) {
	BCM2835_BSC1->C = BCM2835_BSC_C_CLEAR_1;
	BCM2835_BSC1->S = (uint32_t) (BCM2835_BSC_S_CLKT | BCM2835_BSC_S_ERR | BCM2835_BSC_S_DONE);

	BCM2835_BSC1->DLEN = (uint32_t) 1;
	BCM2835_BSC1->FIFO = (uint32_t) data;

	BCM2835_BSC1->C = (uint32_t) (BCM2835_BSC_C_I2CEN | BCM2835_BSC_C_ST);

	while ((BCM2835_BSC1->S & BCM2835_BSC_S_DONE) != BCM2835_BSC_S_DONE) {
	}

	BCM2835_BSC1->S = BCM2835_BSC_S_DONE;
}

void i2c_write_uint16(uint16_t data) {
	BCM2835_BSC1->C = BCM2835_BSC_C_CLEAR_1;
	BCM2835_BSC1->S = (uint32_t) (BCM2835_BSC_S_CLKT | BCM2835_BSC_S_ERR | BCM2835_BSC_S_DONE);

	BCM2835_BSC1->DLEN = (uint32_t) 2;
	BCM2835_BSC1->FIFO = (uint32_t) data >> 8;
	BCM2835_BSC1->FIFO = (uint32_t) data & 0xFF;

	BCM2835_BSC1->C = (uint32_t) (BCM2835_BSC_C_I2CEN | BCM2835_BSC_C_ST);

	while ((BCM2835_BSC1->S & BCM2835_BSC_S_DONE) != BCM2835_BSC_S_DONE) {
	}

	BCM2835_BSC1->S = BCM2835_BSC_S_DONE;
}

void i2c_write_reg_uint8(uint8_t reg, uint8_t data) {
	BCM2835_BSC1->C = BCM2835_BSC_C_CLEAR_1;
	BCM2835_BSC1->S = (uint32_t) (BCM2835_BSC_S_CLKT | BCM2835_BSC_S_ERR | BCM2835_BSC_S_DONE);

	BCM2835_BSC1->DLEN = (uint32_t) 2;
	BCM2835_BSC1->FIFO = (uint32_t) reg;
	BCM2835_BSC1->FIFO = (uint32_t) data;

	BCM2835_BSC1->C = (uint32_t) (BCM2835_BSC_C_I2CEN | BCM2835_BSC_C_ST);

	while ((BCM2835_BSC1->S & BCM2835_BSC_S_DONE) != BCM2835_BSC_S_DONE) {
	}

	BCM2835_BSC1->S = BCM2835_BSC_S_DONE;
}

void i2c_write_reg_uint16(uint8_t reg, uint16_t data) {
	BCM2835_BSC1->C = BCM2835_BSC_C_CLEAR_1;
	BCM2835_BSC1->S = (uint32_t) (BCM2835_BSC_S_CLKT | BCM2835_BSC_S_ERR | BCM2835_BSC_S_DONE);

	BCM2835_BSC1->DLEN = (uint32_t) 3;
	BCM2835_BSC1->FIFO = (uint32_t) reg;
	BCM2835_BSC1->FIFO = (uint32_t) data >> 8;
	BCM2835_BSC1->FIFO = (uint32_t) data & 0xFF;

	BCM2835_BSC1->C = (uint32_t) (BCM2835_BSC_C_I2CEN | BCM2835_BSC_C_ST);

	while ((BCM2835_BSC1->S & BCM2835_BSC_S_DONE) != BCM2835_BSC_S_DONE) {
	}

	BCM2835_BSC1->S = BCM2835_BSC_S_DONE;
}
#endif

void i2c_write_reg_uint16_mask(uint8_t reg, uint16_t data, uint16_t mask) {
	uint16_t current;
	uint16_t new;

	current = i2c_read_reg_uint16(reg);

	new = (current & ~mask) | (data & mask);

	i2c_write_reg_uint16(reg, new);
}
