/**
 * @file bcm2835_i2c.c
 *
 */
/* Copyright (C) 2016-2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
	/* BSC1 is on GPIO 2 & 3 */
	bcm2835_gpio_fsel(RPI_V2_GPIO_P1_03, BCM2835_GPIO_FSEL_ALT0);
	bcm2835_gpio_fsel(RPI_V2_GPIO_P1_05, BCM2835_GPIO_FSEL_ALT0);

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
	/* BSC1 is on GPIO 2 & 3 */
	bcm2835_gpio_fsel(RPI_V2_GPIO_P1_03, BCM2835_GPIO_FSEL_INPT);
	bcm2835_gpio_fsel(RPI_V2_GPIO_P1_05, BCM2835_GPIO_FSEL_INPT);
}

/**
 * @ingroup I2C
 *
 * Write a data to I2C device.
 * @param buf buffer to write
 * @param len size of the buffer
 * @return ::BCM2835_I2C_REASON_OK if successful; BCM2835_I2C_REASON_ERROR_* otherwise. Reference \ref bcm2835I2CReasonCodes
 */
uint8_t bcm2835_i2c_write(/*@null@*/ const char *buf, uint32_t len) {
	uint32_t remaining = len;
	uint32_t i = 0;
	uint8_t reason = BCM2835_I2C_REASON_OK;

    // Clear FIFO
    BCM2835_BSC1->C = BCM2835_BSC_C_CLEAR_1;
    // Clear Status
    BCM2835_BSC1->S = (uint32_t)(BCM2835_BSC_S_CLKT | BCM2835_BSC_S_ERR | BCM2835_BSC_S_DONE);
	// Set Data Length
    BCM2835_BSC1->DLEN = len;
    // pre populate FIFO with max buffer
	while ((remaining != (uint32_t)0)  && (i < BCM2835_BSC_FIFO_SIZE)) {
		BCM2835_BSC1 ->FIFO = (uint32_t)buf[i];
		i++;
		remaining--;
	}

    // Enable device and start transfer
    BCM2835_BSC1->C = (uint32_t)(BCM2835_BSC_C_I2CEN | BCM2835_BSC_C_ST);

	// Transfer is over when BCM2835_BSC_S_DONE
	while (!(BCM2835_BSC1 ->S & BCM2835_BSC_S_DONE)) {
		while ((remaining != (uint32_t)0) && (BCM2835_BSC1 ->S & BCM2835_BSC_S_TXD)) {
			BCM2835_BSC1 ->FIFO = (uint32_t)buf[i];
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
	else if (remaining != (uint32_t)0) {
		reason = BCM2835_I2C_REASON_ERROR_DATA;
	}

	BCM2835_BSC1->S = BCM2835_BSC_S_DONE;

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
uint8_t bcm2835_i2c_read(char *buf, uint32_t len) {
	uint32_t remaining = len;
	uint8_t reason = BCM2835_I2C_REASON_OK;
	uint8_t *p = (uint8_t *)buf;

    // Clear FIFO
    BCM2835_BSC1->C = BCM2835_BSC_C_CLEAR_1;
    // Clear Status
    BCM2835_BSC1->S = (uint32_t)(BCM2835_BSC_S_CLKT | BCM2835_BSC_S_ERR | BCM2835_BSC_S_DONE);
	// Set Data Length
    BCM2835_BSC1->DLEN = len;
    // Start read
    BCM2835_BSC1->C = (uint32_t)(BCM2835_BSC_C_I2CEN | BCM2835_BSC_C_ST | BCM2835_BSC_C_READ);

	// wait for transfer to complete
	while (!(BCM2835_BSC1->S & BCM2835_BSC_S_DONE)) {
		// we must empty the FIFO as it is populated and not use any delay
		while (BCM2835_BSC1->S & BCM2835_BSC_S_RXD) {
			*p++ = (uint8_t) (BCM2835_BSC1->FIFO & 0xFF);
			remaining--;
		}
	}

	// transfer has finished - grab any remaining stuff in FIFO
	while ((remaining != (uint32_t)0) && (BCM2835_BSC1 ->S & BCM2835_BSC_S_RXD)) {
		*p++ = (uint8_t) (BCM2835_BSC1->FIFO & 0xFF);
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
	else if (remaining != (uint32_t)0) {
		reason = BCM2835_I2C_REASON_ERROR_DATA;
	}

	BCM2835_BSC1->S = BCM2835_BSC_S_DONE;

    return reason;
}

void bcm2835_i2c_set_baudrate(uint32_t baudrate) {
	uint32_t divider = ((uint32_t)BCM2835_CORE_CLK_HZ / baudrate);
	bcm2835_i2c_setClockDivider((uint16_t) divider);
}
