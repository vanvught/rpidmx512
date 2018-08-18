/**
 * @file bcm2835_spi_write.c
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

#include "bcm2835_spi.h"
#include "arm/synchronize.h"


/**
 * @ingroup SPI
 *
 * Transfers uint16_t (2 bytes) to the currently selected SPI slave.
 * Asserts the currently selected CS pins (as previously set by \ref bcm2835_spi_chipSelect)
 * during the transfer.
 *
 * @param data uint16_t
 */
void bcm2835_spi_write(const uint16_t data) {
	dsb();
	// Clear TX and RX fifos
	BCM2835_PERI_SET_BITS(BCM2835_SPI0->CS, BCM2835_SPI0_CS_CLEAR, BCM2835_SPI0_CS_CLEAR);
	// Set TA = 1
	BCM2835_PERI_SET_BITS(BCM2835_SPI0->CS, BCM2835_SPI0_CS_TA, BCM2835_SPI0_CS_TA);
	// Maybe wait for TXD
	while (!(BCM2835_SPI0->CS & BCM2835_SPI0_CS_TXD))
		;
	// Write to FIFO
	BCM2835_SPI0->FIFO = (uint32_t) data >> 8;
	BCM2835_SPI0->FIFO = (uint32_t) data & 0xFF;
	// Wait for DONE to be set
	while (!(BCM2835_SPI0->CS & BCM2835_SPI0_CS_DONE))
		;
	// Set TA = 0
	BCM2835_PERI_SET_BITS(BCM2835_SPI0->CS, 0, BCM2835_SPI0_CS_TA);
	dmb();
}
