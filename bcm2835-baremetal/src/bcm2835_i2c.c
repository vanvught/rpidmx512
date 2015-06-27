/**
 * @file bcm2835_i2c.c
 *
 */
/* Copyright (C) 2014 by Arjan van Vught <pm @ http://www.raspberrypi.org/forum/>
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
#include "bcm2835_gpio.h"
#include "bcm2835_i2c.h"

/**
 * @ingroup I2C
 *
 * Start I2C operations.
 * Forces BSC1 pins P1-3 (SDA), P1-5 (SCL)
 * alternate function ALT0, which enables those pins for I2C interface.
 * Default the I2C speed to 100 kHz.
 */
void bcm2835_i2c_begin(void) {
	uint32_t value;
	/* BSC1 is on GPIO 2 & 3 */

	value = BCM2835_GPIO->GPFSEL0;
	value &= ~(7 << 6);
	value |= BCM2835_GPIO_FSEL_INPT << 6;	// Pin 2, GPIO Input
	value &= ~(7 << 9);
	value |= BCM2835_GPIO_FSEL_INPT << 9;	// Pin 3, GPIO Input
	BCM2835_GPIO->GPFSEL0 = value;

	value = BCM2835_GPIO->GPFSEL0;
	value &= ~(7 << 6);
	value |= BCM2835_GPIO_FSEL_ALT0 << 6;	// Pin 2, Set SDA pin to alternate function 0 for I2C
	value &= ~(7 << 9);
	value |= BCM2835_GPIO_FSEL_ALT0 << 9;	// Pin 3, Set SCL pin to alternate function 0 for I2C
	BCM2835_GPIO->GPFSEL0 = value;

	BCM2835_BSC1->DIV = BCM2835_I2C_CLOCK_DIVIDER_2500; // Default the I2C speed to 100 kHz
}

/**
 * @ingroup I2C
 *
 * End I2C operations.
 * BSC1 pins pins P1-3 (SDA), P1-5 (SCL)
 * are returned to their default INPUT behavior.
 */
void bcm2835_i2c_end(void) {
	uint32_t value;
	/* BSC1 is on GPIO 2 & 3 */

	value = BCM2835_GPIO->GPFSEL0;
	value &= ~(7 << 6);
	value |= BCM2835_GPIO_FSEL_INPT << 6;	// Pin 2, GPIO Input
	value &= ~(7 << 9);
	value |= BCM2835_GPIO_FSEL_INPT << 9;	// Pin 3, GPIO Input
	BCM2835_GPIO->GPFSEL0 = value;
}

/**
 * @ingroup I2C
 *
 * Sets the I2C slave address
 * @param addr buffer for read.
 */
void bcm2835_i2c_setSlaveAddress(const uint8_t addr) {
	BCM2835_BSC1 ->A = addr;
}

/**
 * @ingroup I2C
 *
 * Sets the I2C clock divider and therefore the I2C clock speed.
 * @param divider The desired I2C clock divider, one of \ref bcm2835I2CClockDivider
 */
void bcm2835_i2c_setClockDivider(const uint16_t divider) {
	BCM2835_BSC1 ->DIV = divider;
}

/**
 * @ingroup I2C
 *
 * Write a data to I2C device.
 * @param buf buffer to write
 * @param len size of the buffer
 * @return ::BCM2835_I2C_REASON_OK if successful; BCM2835_I2C_REASON_ERROR_* otherwise. Reference \ref bcm2835I2CReasonCodes
 */
uint8_t bcm2835_i2c_write(/*@null@*/ const char * buf, const uint32_t len) {
	uint32_t remaining = len;
	uint32_t i = 0;
	uint8_t reason = BCM2835_I2C_REASON_OK;

    // Clear FIFO
    BCM2835_BSC1->C = BCM2835_BSC_C_CLEAR_1;
    // Clear Status
    BCM2835_BSC1->S = BCM2835_BSC_S_CLKT | BCM2835_BSC_S_ERR | BCM2835_BSC_S_DONE;
	// Set Data Length
    BCM2835_BSC1->DLEN = len;
    // pre populate FIFO with max buffer
	while (remaining && (i < BCM2835_BSC_FIFO_SIZE)) {
		BCM2835_BSC1 ->FIFO = buf[i];
		i++;
		remaining--;
	}

    // Enable device and start transfer
    BCM2835_BSC1->C = BCM2835_BSC_C_I2CEN | BCM2835_BSC_C_ST;

	// Transfer is over when BCM2835_BSC_S_DONE
	while (!(BCM2835_BSC1 ->S & BCM2835_BSC_S_DONE)) {
		while (remaining && (BCM2835_BSC1 ->S & BCM2835_BSC_S_TXD)) {
			// Write to FIFO
			BCM2835_BSC1 ->FIFO = buf[i];
			i++;
			remaining--;
    	}
    }

	// Received a NACK
	if (BCM2835_BSC1 ->S & BCM2835_BSC_S_ERR) {
		BCM2835_BSC1 ->S = BCM2835_BSC_S_ERR;
		reason = BCM2835_I2C_REASON_ERROR_NACK;
	}

	// Received Clock Stretch Timeout
	else if (BCM2835_BSC1 ->S & BCM2835_BSC_S_CLKT) {
		reason = BCM2835_I2C_REASON_ERROR_CLKT;
	}

	// Not all data is sent
	else if (remaining) {
		reason = BCM2835_I2C_REASON_ERROR_DATA;
	}

	BCM2835_BSC1->C = BCM2835_BSC_S_DONE;

    return reason;
}

/**
 * @ingroup I2C
 *
 * Read data from I2C device.
 * @param buf buffer for read
 * @param len size of the buffer
 * @return ::BCM2835_I2C_REASON_OK if successful; BCM2835_I2C_REASON_ERROR_* otherwise. Reference \ref bcm2835I2CReasonCodes
 */
uint8_t bcm2835_i2c_read(char* buf, const uint32_t len) {
	uint32_t remaining = len;
	uint32_t i = 0;
	uint8_t reason = BCM2835_I2C_REASON_OK;

    // Clear FIFO
    BCM2835_BSC1->C = BCM2835_BSC_C_CLEAR_1;
    // Clear Status
    BCM2835_BSC1->S = BCM2835_BSC_S_CLKT | BCM2835_BSC_S_ERR | BCM2835_BSC_S_DONE;
	// Set Data Length
    BCM2835_BSC1->DLEN = len;
    // Start read
    BCM2835_BSC1->C = BCM2835_BSC_C_I2CEN | BCM2835_BSC_C_ST | BCM2835_BSC_C_READ;

	// wait for transfer to complete
	while (!(BCM2835_BSC1->S & BCM2835_BSC_S_DONE)) {
		// we must empty the FIFO as it is populated and not use any delay
		while (BCM2835_BSC1->S & BCM2835_BSC_S_RXD) {
			// Read from FIFO, no barrier
			buf[i] = BCM2835_BSC1 ->FIFO;
			i++;
			remaining--;
		}
	}

	// transfer has finished - grab any remaining stuff in FIFO
	while (remaining && (BCM2835_BSC1 ->S & BCM2835_BSC_S_RXD)) {
		// Read from FIFO, no barrier
		buf[i] = BCM2835_BSC1 ->FIFO;
		i++;
		remaining--;
	}

	// Received a NACK
	if (BCM2835_BSC1 ->S & BCM2835_BSC_S_ERR) {
		BCM2835_BSC1 ->S = BCM2835_BSC_S_ERR;
		reason = BCM2835_I2C_REASON_ERROR_NACK;
	}

	// Received Clock Stretch Timeout
	else if (BCM2835_BSC1 ->S & BCM2835_BSC_S_CLKT) {
		reason = BCM2835_I2C_REASON_ERROR_CLKT;
	}

	// Not all data is received
	else if (remaining) {
		reason = BCM2835_I2C_REASON_ERROR_DATA;
	}

	BCM2835_BSC1->C = BCM2835_BSC_S_DONE;

    return reason;
}
