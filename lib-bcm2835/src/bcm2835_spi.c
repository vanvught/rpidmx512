/**
 * @file bcm2835_spi.c
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

#include "arm/synchronize.h"

#include "bcm2835.h"
#include "bcm2835_gpio.h"
#include "bcm2835_spi.h"

/**
 * @ingroup SPI
 *
 * Start SPI operations.
 * Forces RPi SPI0 pins P1-19 (MOSI), P1-21 (MISO), P1-23 (CLK), P1-24 (CE0) and P1-26 (CE1)
 * alternate function ALT0, which enables those pins for SPI interface.
 * Default the chip select polarity to LOW
 * Default to \ref BCM2835_SPI_MODE0 (CPOL = 0, CPHA = 0).
 * Default the SPI speed to 100 kHz (\ref BCM2835_SPI_CLOCK_DIVIDER_2500).
 */
void bcm2835_spi_begin(void) {
	dsb();
	bcm2835_gpio_fsel(RPI_V2_GPIO_P1_26, BCM2835_GPIO_FSEL_INPT);	// SPI0_CE1
	bcm2835_gpio_fsel(RPI_V2_GPIO_P1_24, BCM2835_GPIO_FSEL_INPT);	// SPI0_CE0
	bcm2835_gpio_fsel(RPI_V2_GPIO_P1_21, BCM2835_GPIO_FSEL_INPT);	// SPI0_MISO
	bcm2835_gpio_fsel(RPI_V2_GPIO_P1_19, BCM2835_GPIO_FSEL_INPT);	// SPI0_MOSI
	bcm2835_gpio_fsel(RPI_V2_GPIO_P1_23, BCM2835_GPIO_FSEL_INPT);	// SPI0_SCLK

	bcm2835_gpio_fsel(RPI_V2_GPIO_P1_26, BCM2835_GPIO_FSEL_ALT0);	// SPI0_CE1
	bcm2835_gpio_fsel(RPI_V2_GPIO_P1_24, BCM2835_GPIO_FSEL_ALT0);	// SPI0_CE0
	bcm2835_gpio_fsel(RPI_V2_GPIO_P1_21, BCM2835_GPIO_FSEL_ALT0);	// SPI0_MISO
	bcm2835_gpio_fsel(RPI_V2_GPIO_P1_19, BCM2835_GPIO_FSEL_ALT0);	// SPI0_MOSI
	bcm2835_gpio_fsel(RPI_V2_GPIO_P1_23, BCM2835_GPIO_FSEL_ALT0);	// SPI0_SCLK
	dmb();

	dsb();
	BCM2835_SPI0->CS = 0;
	BCM2835_SPI0->CS = BCM2835_SPI0_CS_CLEAR;
#if 0
	bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS0, LOW);	// Chip select 0 active LOW
	bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS1, LOW);	// Chip select 1 active LOW
	bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);					// CPOL = 0, CPHA = 0
	BCM2835_SPI0->CLK = BCM2835_SPI_CLOCK_DIVIDER_2500;			// 100 kHz
#endif
	dmb();
}

/**
 * @ingroup SPI
 *
 * End SPI operations.
 * SPI0 pins P1-19 (MOSI), P1-21 (MISO), P1-23 (CLK), P1-24 (CE0) and P1-26 (CE1)
 * are returned to their default INPUT behavior.
 */
void bcm2835_spi_end(void) {
	dsb();
	bcm2835_gpio_fsel(RPI_V2_GPIO_P1_26, BCM2835_GPIO_FSEL_INPT);	// SPI0_CE1
	bcm2835_gpio_fsel(RPI_V2_GPIO_P1_24, BCM2835_GPIO_FSEL_INPT);	// SPI0_CE0
	bcm2835_gpio_fsel(RPI_V2_GPIO_P1_21, BCM2835_GPIO_FSEL_INPT);	// SPI0_MISO
	bcm2835_gpio_fsel(RPI_V2_GPIO_P1_19, BCM2835_GPIO_FSEL_INPT);	// SPI0_MOSI
	bcm2835_gpio_fsel(RPI_V2_GPIO_P1_23, BCM2835_GPIO_FSEL_INPT);	// SPI0_SCLK
	dmb();
}

/**
 * @ingroup SPI
 *
 * Sets the SPI bit order.
 * NOTE: has no effect. Not supported by SPI0.
 * Defaults to ::BCM2835_SPI_BIT_ORDER_MSBFIRST
 *
 * @param order
 */
void bcm2835_spi_setBitOrder(/*@unused@*/const uint8_t order) {
	// BCM2835_SPI_BIT_ORDER_MSBFIRST is the only one supported by SPI0
}

/**
 * @ingroup SPI
 *
 * Sets the chip select pin polarity for a given pin.
 *
 * @param cs The chip select pin to affect
 * @param active Whether the chip select pin is to be active HIGH or LOW
 */
void bcm2835_spi_setChipSelectPolarity(const uint8_t cs, const uint8_t active) {
	uint8_t shift = 21 + cs;
	// Mask in the appropriate CSPOLn bit
	dsb();
	BCM2835_PERI_SET_BITS(BCM2835_SPI0->CS, active << shift, 1 << shift);
	dmb();
}

/**
 * @ingroup SPI
 *
 *  Transfers any number of bytes to and from the currently selected SPI slave.
 *  Asserts the currently selected CS pins (as previously set by \ref bcm2835_spi_chipSelect)
 *  during the transfer.
 *
 * @param tbuf Buffer of bytes to send.
 * @param rbuf Received bytes will by put in this buffer.
 * @param len Number of bytes in the tbuf buffer, and the number of bytes to send/received.
 */
void bcm2835_spi_transfernb(char *tbuf, char *rbuf, const uint32_t len) {
	uint32_t fifo_writes = 0;
	uint32_t fifo_reads = 0;

	dsb();
	// Clear TX and RX fifos
	BCM2835_PERI_SET_BITS(BCM2835_SPI0->CS, BCM2835_SPI0_CS_CLEAR, BCM2835_SPI0_CS_CLEAR);
	// Set TA = 1
	BCM2835_PERI_SET_BITS(BCM2835_SPI0->CS, BCM2835_SPI0_CS_TA, BCM2835_SPI0_CS_TA);

	while ((fifo_writes < len) || (fifo_reads < len)) {

		while ((BCM2835_SPI0->CS & BCM2835_SPI0_CS_TXD) && (fifo_writes < len)) {
			if (tbuf) {
				BCM2835_SPI0->FIFO = (uint32_t) tbuf[fifo_writes];
			} else {
				BCM2835_SPI0->FIFO = 0;
			}
			fifo_writes++;
		}

		while ((BCM2835_SPI0->CS & BCM2835_SPI0_CS_RXD) && (fifo_reads < len)) {
			if (rbuf) {
				rbuf[fifo_reads] = (uint32_t) BCM2835_SPI0->FIFO;
			} else {
				(void) BCM2835_SPI0->FIFO;
			}
			fifo_reads++;
		}
	}

	// Wait for DONE to be set
	while (!(BCM2835_SPI0->CS & BCM2835_SPI0_CS_DONE))
		;

	// Set TA = 0
	BCM2835_PERI_SET_BITS(BCM2835_SPI0->CS, 0, BCM2835_SPI0_CS_TA);

	dmb();
}

/**
 * @ingroup SPI
 *
 * Transfers any number of bytes to and from the currently selected SPI slave
 * using \ref bcm2835_spi_transfernb.
 * The returned data from the slave replaces the transmitted data in the buffer.
 *
 * @param buf Buffer of bytes to send. Received bytes will replace the contents.
 * @param len Number of bytes in the buffer, and the number of bytes to send/received.
 */
void bcm2835_spi_transfern(char *buf, const uint32_t len) {
	bcm2835_spi_transfernb(buf, buf, len);
}
