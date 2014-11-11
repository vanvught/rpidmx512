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

#define BCM2835_SPI0_CS_LEN_LONG   0x02000000 ///< Enable Long data word in Lossi mode if DMA_LEN is set
#define BCM2835_SPI0_CS_DMA_LEN    0x01000000 ///< Enable DMA mode in Lossi mode
#define BCM2835_SPI0_CS_CSPOL2     0x00800000 ///< Chip Select 2 Polarity
#define BCM2835_SPI0_CS_CSPOL1     0x00400000 ///< Chip Select 1 Polarity
#define BCM2835_SPI0_CS_CSPOL0     0x00200000 ///< Chip Select 0 Polarity
#define BCM2835_SPI0_CS_RXF        0x00100000 ///< RXF - RX FIFO Full
#define BCM2835_SPI0_CS_RXR        0x00080000 ///< RXR RX FIFO needs Reading ( full)
#define BCM2835_SPI0_CS_TXD        0x00040000 ///< TXD TX FIFO can accept Data
#define BCM2835_SPI0_CS_RXD        0x00020000 ///< RXD RX FIFO contains Data
#define BCM2835_SPI0_CS_DONE       0x00010000 ///< Done transfer Done
#define BCM2835_SPI0_CS_TE_EN      0x00008000 ///< Unused
#define BCM2835_SPI0_CS_LMONO      0x00004000 ///< Unused
#define BCM2835_SPI0_CS_LEN        0x00002000 ///< LEN LoSSI enable
#define BCM2835_SPI0_CS_REN        0x00001000 ///< REN Read Enable
#define BCM2835_SPI0_CS_ADCS       0x00000800 ///< ADCS Automatically Deassert Chip Select
#define BCM2835_SPI0_CS_INTR       0x00000400 ///< INTR Interrupt on RXR
#define BCM2835_SPI0_CS_INTD       0x00000200 ///< INTD Interrupt on Done
#define BCM2835_SPI0_CS_DMAEN      0x00000100 ///< DMAEN DMA Enable
#define BCM2835_SPI0_CS_TA         0x00000080 ///< Transfer Active
#define BCM2835_SPI0_CS_CSPOL      0x00000040 ///< Chip Select Polarity
#define BCM2835_SPI0_CS_CLEAR      0x00000030 ///< Clear FIFO Clear RX and TX
#define BCM2835_SPI0_CS_CLEAR_RX   0x00000020 ///< Clear FIFO Clear RX
#define BCM2835_SPI0_CS_CLEAR_TX   0x00000010 ///< Clear FIFO Clear TX
#define BCM2835_SPI0_CS_CPOL      	0x00000008 ///< Clock Polarity
#define BCM2835_SPI0_CS_CPHA      	0x00000004 ///< Clock Phase
#define BCM2835_SPI0_CS_CS        	0x00000003 ///< Chip Select

void bcm2835_spi_begin(void) {
	// Set the SPI0 pins to the Alt 0 function to enable SPI0 access on them
	bcm2835_gpio_fsel(RPI_V2_GPIO_P1_26, BCM2835_GPIO_FSEL_ALT0); // CE1
	bcm2835_gpio_fsel(RPI_V2_GPIO_P1_24, BCM2835_GPIO_FSEL_ALT0); // CE0
	bcm2835_gpio_fsel(RPI_V2_GPIO_P1_21, BCM2835_GPIO_FSEL_ALT0); // MISO
	bcm2835_gpio_fsel(RPI_V2_GPIO_P1_19, BCM2835_GPIO_FSEL_ALT0); // MOSI
	bcm2835_gpio_fsel(RPI_V2_GPIO_P1_23, BCM2835_GPIO_FSEL_ALT0); // CLK

	BCM2835_SPI0 ->CS = 0;
	BCM2835_SPI0 ->CS = BCM2835_SPI0_CS_CLEAR;

	bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS0, LOW);
	bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS1, LOW);
	bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);						// CPOL = 0, CPHA = 0
	bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_2500);	// 100 kHz
}

void bcm2835_spi_end(void) {
	// Set all the SPI0 pins back to input
	bcm2835_gpio_fsel(RPI_V2_GPIO_P1_26, BCM2835_GPIO_FSEL_INPT); // CE1
	bcm2835_gpio_fsel(RPI_V2_GPIO_P1_24, BCM2835_GPIO_FSEL_INPT); // CE0
	bcm2835_gpio_fsel(RPI_V2_GPIO_P1_21, BCM2835_GPIO_FSEL_INPT); // MISO
	bcm2835_gpio_fsel(RPI_V2_GPIO_P1_19, BCM2835_GPIO_FSEL_INPT); // MOSI
	bcm2835_gpio_fsel(RPI_V2_GPIO_P1_23, BCM2835_GPIO_FSEL_INPT); // CLK
}

inline void bcm2835_spi_setBitOrder(const uint8_t order) {
	// BCM2835_SPI_BIT_ORDER_MSBFIRST is the only one supported by SPI0
}

inline void bcm2835_spi_setClockDivider(const uint16_t divider) {
	BCM2835_SPI0 ->CLK = divider;
}

inline void bcm2835_spi_setDataMode(const uint8_t mode) {
	// Mask in the CPO and CPHA bits of CS
	BCM2835_PERI_SET_BITS(BCM2835_SPI0->CS, mode << 2, BCM2835_SPI0_CS_CPOL | BCM2835_SPI0_CS_CPHA);
}

inline void bcm2835_spi_chipSelect(const uint8_t cs) {
	BCM2835_PERI_SET_BITS(BCM2835_SPI0->CS, cs, BCM2835_SPI0_CS_CS);
}

inline void bcm2835_spi_setChipSelectPolarity(const uint8_t cs, const uint8_t active) {
	uint8_t shift = 21 + cs;
	// Mask in the appropriate CSPOLn bit
	BCM2835_PERI_SET_BITS(BCM2835_SPI0->CS, active << shift, 1 << shift);
}

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

void bcm2835_spi_transfern(char* buf, const uint32_t len) {
	bcm2835_spi_transfernb(buf, buf, len);
}

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
