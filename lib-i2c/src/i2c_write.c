/**
 * @file i2c_write.c
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

#include "bcm2835.h"
#include "bcm2835_i2c.h"

#include "i2c.h"

/**
 *
 * @param data
 */
void i2c_write(const uint8_t data) {
	BCM2835_BSC1->C = BCM2835_BSC_C_CLEAR_1;
	BCM2835_BSC1->S = (uint32_t) (BCM2835_BSC_S_CLKT | BCM2835_BSC_S_ERR | BCM2835_BSC_S_DONE);

	BCM2835_BSC1->DLEN = (uint32_t) 1;
	BCM2835_BSC1->FIFO = (uint32_t) data;

	BCM2835_BSC1->C = (uint32_t) (BCM2835_BSC_C_I2CEN | BCM2835_BSC_C_ST);

	while (!(BCM2835_BSC1->S & BCM2835_BSC_S_DONE)) {
	}

	BCM2835_BSC1->S = BCM2835_BSC_S_DONE;
}

/**
 *
 * @param data
 */
void i2c_write_uint16(const uint16_t data) {
	BCM2835_BSC1->C = BCM2835_BSC_C_CLEAR_1;
	BCM2835_BSC1->S = (uint32_t) (BCM2835_BSC_S_CLKT | BCM2835_BSC_S_ERR | BCM2835_BSC_S_DONE);

	BCM2835_BSC1->DLEN = (uint32_t) 2;
	BCM2835_BSC1->FIFO = (uint32_t) data >> 8;
	BCM2835_BSC1->FIFO = (uint32_t) data & 0xFF;

	BCM2835_BSC1->C = (uint32_t) (BCM2835_BSC_C_I2CEN | BCM2835_BSC_C_ST);

	while (!(BCM2835_BSC1->S & BCM2835_BSC_S_DONE)) {
	}

	BCM2835_BSC1->S = BCM2835_BSC_S_DONE;
}

/**
 *
 * @param reg
 * @param data
 */
void i2c_write_reg_uint8(const uint8_t reg, const uint8_t data) {
	BCM2835_BSC1->C = BCM2835_BSC_C_CLEAR_1;
	BCM2835_BSC1->S = (uint32_t) (BCM2835_BSC_S_CLKT | BCM2835_BSC_S_ERR | BCM2835_BSC_S_DONE);

	BCM2835_BSC1->DLEN = (uint32_t) 2;
	BCM2835_BSC1->FIFO = (uint32_t) reg;
	BCM2835_BSC1->FIFO = (uint32_t) data;

	BCM2835_BSC1->C = (uint32_t) (BCM2835_BSC_C_I2CEN | BCM2835_BSC_C_ST);

	while (!(BCM2835_BSC1->S & BCM2835_BSC_S_DONE)) {
	}

	BCM2835_BSC1->S = BCM2835_BSC_S_DONE;
}

/**
 *
 * @param reg
 * @param data
 */
void i2c_write_reg_uint16(const uint8_t reg, const uint16_t data) {
	BCM2835_BSC1->C = BCM2835_BSC_C_CLEAR_1;
	BCM2835_BSC1->S = (uint32_t) (BCM2835_BSC_S_CLKT | BCM2835_BSC_S_ERR | BCM2835_BSC_S_DONE);

	BCM2835_BSC1->DLEN = (uint32_t) 3;
	BCM2835_BSC1->FIFO = (uint32_t) reg;
	BCM2835_BSC1->FIFO = (uint32_t) data >> 8;
	BCM2835_BSC1->FIFO = (uint32_t) data & 0xFF;

	BCM2835_BSC1->C = (uint32_t) (BCM2835_BSC_C_I2CEN | BCM2835_BSC_C_ST);

	while (!(BCM2835_BSC1->S & BCM2835_BSC_S_DONE)) {
	}

	BCM2835_BSC1->S = BCM2835_BSC_S_DONE;
}

/**
 *
 * @param reg
 * @param data
 * @param mask
 */
void i2c_write_reg_uint16_mask(const uint8_t reg, const uint16_t data, const uint16_t mask) {
	uint16_t current;
	uint16_t new;

	current = i2c_read_reg_uint16(reg);

	new = (current & ~mask) | (data & mask);

	i2c_write_reg_uint16(reg, new);
}
