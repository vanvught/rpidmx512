/**
 * @file bcm2835_pl011.c
 *
 */
/* Copyright (C) 2016-2017 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include "arm/pl011.h"

#include "bcm2835.h"
#include "bcm2835_gpio.h"

static uint32_t pl011_baud_rate = 115200;

/**
 * @ingroup PL011
 *
 */
void bcm2835_pl011_begin(void) {
	BCM2835_PL011 ->CR = 0; /* Disable everything */

    // Set the GPI0 pins to the Alt 0 function to enable PL011 access on them
    bcm2835_gpio_fsel(RPI_V2_GPIO_P1_08, BCM2835_GPIO_FSEL_ALT0);	// PL011_TXD
    bcm2835_gpio_fsel(RPI_V2_GPIO_P1_10, BCM2835_GPIO_FSEL_ALT0);	// PL011_RXD

    // Disable pull-up/down
    bcm2835_gpio_set_pud(RPI_V2_GPIO_P1_08, BCM2835_GPIO_PUD_OFF);
    bcm2835_gpio_set_pud(RPI_V2_GPIO_P1_10, BCM2835_GPIO_PUD_OFF);

    /* Poll the "flags register" to wait for the UART to stop transmitting or receiving. */
	while (BCM2835_PL011 ->FR & PL011_FR_BUSY ) {
	}

    /* Flush the transmit FIFO by marking FIFOs as disabled in the "line control register". */
	BCM2835_PL011->LCRH &= ~PL011_LCRH_FEN;

    BCM2835_PL011->ICR = 0x7FF;						/* Clear all interrupt status */
	/*
	 * IBRD = UART_CLK / (16 * BAUD_RATE)
	 * FBRD = ROUND((64 * MOD(UART_CLK,(16 * BAUD_RATE))) / (16 * BAUD_RATE))
	 */
    // UART_CLK = 3000000
    // BAUD_RATE = 115200
	BCM2835_PL011 ->IBRD = PL011_BAUD_INT(pl011_baud_rate);
	BCM2835_PL011 ->FBRD = PL011_BAUD_FRAC(pl011_baud_rate);
	BCM2835_PL011 ->LCRH = PL011_LCRH_WLEN8; 		/* Set N, 8, 1, FIFO disabled */
	BCM2835_PL011 ->CR = 0x301; 					/* Enable UART */
}

/**
 * @ingroup PL011
 *
 * @param c
 */
void bcm2835_pl011_send(const uint8_t c) {
	while (1 == 1) {
		if ((BCM2835_PL011->FR & 0x20) == 0) {
			break;
		}
	}
	BCM2835_PL011->DR = (uint32_t)c;
}

/**
 * @ingroup PL011
 *
 * @param baudrate
 */
void bcm2835_pl011_set_baudrate(const uint32_t baudrate) {
	if (baudrate > 0) {
		pl011_baud_rate = baudrate;
	}
}
