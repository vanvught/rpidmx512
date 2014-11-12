/**
 * @file bcm2835_spi.c
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
#include "bcm2835_spi.h"

/**
 * @ingroup SPI
 *
 * Start SPI operations.
 * Forces RPi SPI0 pins P1-19 (MOSI), P1-21 (MISO), P1-23 (CLK), P1-24 (CE0) and P1-26 (CE1)
 * alternate function ALT0, which enables those pins for SPI interface.
 * Default the chip select polarity to \ref LOW
 * Default to \ref BCM2835_SPI_MODE0 (CPOL = 0, CPHA = 0).
 * Default the SPI speed to 100 kHz (\ref BCM2835_SPI_CLOCK_DIVIDER_2500).
 */
void bcm2835_spi_begin(void) {
	// Set the SPI0 pins to the Alt 0 function to enable SPI0 access on them
	bcm2835_gpio_fsel(RPI_V2_GPIO_P1_26, BCM2835_GPIO_FSEL_ALT0); // CE1
	bcm2835_gpio_fsel(RPI_V2_GPIO_P1_24, BCM2835_GPIO_FSEL_ALT0); // CE0
	bcm2835_gpio_fsel(RPI_V2_GPIO_P1_21, BCM2835_GPIO_FSEL_ALT0); // MISO
	bcm2835_gpio_fsel(RPI_V2_GPIO_P1_19, BCM2835_GPIO_FSEL_ALT0); // MOSI
	bcm2835_gpio_fsel(RPI_V2_GPIO_P1_23, BCM2835_GPIO_FSEL_ALT0); // CLK

	BCM2835_SPI0 ->CS = 0;
	BCM2835_SPI0 ->CS = BCM2835_SPI0_CS_CLEAR;

	bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS0, LOW);		// Chip select 0 active LOW
	bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS1, LOW);		// Chip select 1 active LOW
	bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);						// CPOL = 0, CPHA = 0
	bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_2500);	// 100 kHz
}

/**
 * @ingroup SPI
 *
 * End SPI operations.
 * SPI0 pins P1-19 (MOSI), P1-21 (MISO), P1-23 (CLK), P1-24 (CE0) and P1-26 (CE1)
 * are returned to their default INPUT behavior.
 */
void bcm2835_spi_end(void) {
	// Set all the SPI0 pins back to input
	bcm2835_gpio_fsel(RPI_V2_GPIO_P1_26, BCM2835_GPIO_FSEL_INPT); // CE1
	bcm2835_gpio_fsel(RPI_V2_GPIO_P1_24, BCM2835_GPIO_FSEL_INPT); // CE0
	bcm2835_gpio_fsel(RPI_V2_GPIO_P1_21, BCM2835_GPIO_FSEL_INPT); // MISO
	bcm2835_gpio_fsel(RPI_V2_GPIO_P1_19, BCM2835_GPIO_FSEL_INPT); // MOSI
	bcm2835_gpio_fsel(RPI_V2_GPIO_P1_23, BCM2835_GPIO_FSEL_INPT); // CLK
}

/**
 * @ingroup SPI
 *
 */
inline void bcm2835_spi_setBitOrder(const uint8_t order) {
	// BCM2835_SPI_BIT_ORDER_MSBFIRST is the only one supported by SPI0
}

/**
 * @ingroup SPI
 *
 * Sets the SPI clock divider and therefore the SPI clock speed.
 *
 * @param divider The desired SPI clock divider, one of ::bcm2835SPIClockDivider
 */
inline void bcm2835_spi_setClockDivider(const uint16_t divider) {
	BCM2835_SPI0 ->CLK = divider;
}

inline void bcm2835_spi_setDataMode(const uint8_t mode) {
	// Mask in the CPO and CPHA bits of CS
	BCM2835_PERI_SET_BITS(BCM2835_SPI0->CS, mode << 2, BCM2835_SPI0_CS_CPOL | BCM2835_SPI0_CS_CPHA);
}

/**
 * @ingroup SPI
 *
 * @param cs Specifies the CS pins(s) that are used to activate the desired slave.
 */
inline void bcm2835_spi_chipSelect(const uint8_t cs) {
	BCM2835_PERI_SET_BITS(BCM2835_SPI0->CS, cs, BCM2835_SPI0_CS_CS);
}

/**
 * @ingroup SPI
 *
 * Sets the chip select pin polarity for a given pin.
 *
 * @param cs The chip select pin to affect
 * @param active Whether the chip select pin is to be active HIGH or LOW
 */
inline void bcm2835_spi_setChipSelectPolarity(const uint8_t cs, const uint8_t active) {
	uint8_t shift = 21 + cs;
	// Mask in the appropriate CSPOLn bit
	BCM2835_PERI_SET_BITS(BCM2835_SPI0->CS, active << shift, 1 << shift);
}

/**
 * @ingroup SPI
 *
 * @param tbuf
 * @param rbuf
 * @param len
 */
inline void bcm2835_spi_transfernb(char* tbuf, char* rbuf, const uint32_t len) {
	// Clear TX and RX fifos
	BCM2835_PERI_SET_BITS(BCM2835_SPI0->CS, BCM2835_SPI0_CS_CLEAR, BCM2835_SPI0_CS_CLEAR);

    // Set TA = 1
	BCM2835_PERI_SET_BITS(BCM2835_SPI0->CS, BCM2835_SPI0_CS_TA, BCM2835_SPI0_CS_TA);

    uint32_t i;
    for (i = 0; i < len; i++)
    {
		// Maybe wait for TXD
		while (!(BCM2835_SPI0->CS & BCM2835_SPI0_CS_TXD))
			;

		BCM2835_SPI0->FIFO = tbuf[i];

		// Wait for RXD
		while (!(BCM2835_SPI0->CS & BCM2835_SPI0_CS_RXD))
			;

		// then read the data byte
		rbuf[i] = BCM2835_SPI0->FIFO;
    }
    // Wait for DONE to be set
    while (!(BCM2835_SPI0->CS & BCM2835_SPI0_CS_DONE))
        	;

    // Set TA = 0, and also set the barrier
    BCM2835_PERI_SET_BITS(BCM2835_SPI0->CS, 0, BCM2835_SPI0_CS_TA);
}

/**
 * @ingroup SPI
 *
 * @param buf
 * @param len
 */
void bcm2835_spi_transfern(char* buf, const uint32_t len) {
	bcm2835_spi_transfernb(buf, buf, len);
}

/**
 * @ingroup SPI
 *
 * @param tbuf
 * @param len
 */
void bcm2835_spi_writenb(char* tbuf, const uint32_t len) {
    // Clear TX and RX fifos
	BCM2835_PERI_SET_BITS(BCM2835_SPI0->CS, BCM2835_SPI0_CS_CLEAR, BCM2835_SPI0_CS_CLEAR);

    // Set TA = 1
	BCM2835_PERI_SET_BITS(BCM2835_SPI0->CS, BCM2835_SPI0_CS_TA, BCM2835_SPI0_CS_TA);

    uint32_t i;
	for (i = 0; i < len; i++)
	{
		// Maybe wait for TXD
		while (!(BCM2835_SPI0->CS & BCM2835_SPI0_CS_TXD))
			;

		// Write to FIFO, no barrier
		BCM2835_SPI0->FIFO =  tbuf[i];
	}

    // Wait for DONE to be set
    while (!(BCM2835_SPI0->CS & BCM2835_SPI0_CS_DONE))
    	;

    // Set TA = 0, and also set the barrier
	BCM2835_PERI_SET_BITS(BCM2835_SPI0->CS, 0, BCM2835_SPI0_CS_TA);
}

#if 0// moved to bcm2835_asm.S
inline void bcm2835_spi_write(const uint16_t data) {
    // Clear TX and RX fifos
	BCM2835_PERI_SET_BITS(BCM2835_SPI0->CS, BCM2835_SPI0_CS_CLEAR, BCM2835_SPI0_CS_CLEAR);

    // Set TA = 1
	BCM2835_PERI_SET_BITS(BCM2835_SPI0->CS, BCM2835_SPI0_CS_TA, BCM2835_SPI0_CS_TA);

	// Maybe wait for TXD
    while (!(BCM2835_SPI0->CS & BCM2835_SPI0_CS_TXD))
		;

	// Write to FIFO
    BCM2835_SPI0->FIFO = data >> 8;
    BCM2835_SPI0->FIFO = data & 0x0FF;

    // Wait for DONE to be set
	while (!(BCM2835_SPI0->CS & BCM2835_SPI0_CS_DONE))
    	;

    // Set TA = 0, and also set the barrier
	BCM2835_PERI_SET_BITS(BCM2835_SPI0->CS, 0, BCM2835_SPI0_CS_TA);
}
#endif
