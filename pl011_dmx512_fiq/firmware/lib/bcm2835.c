/**
 * @file bcm2835.c
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

#include "bcm2835.h"
#include "bcm2835_gpio.h"

uint8_t inline bcm2835_gpio_lev(const uint8_t pin) {
	uint32_t value = BCM2835_GPIO ->GPLEV0;
	return (value & (1 << pin)) ? HIGH : LOW;
}

inline void bcm2835_gpio_pud(const uint8_t pud) {
	BCM2835_GPIO ->GPPUD = pud;
}

inline void bcm2835_gpio_pudclk(const uint8_t pin, const uint8_t on) {
	BCM2835_GPIO ->GPPUDCLK0 = (on ? 1 : 0) << pin;
}

inline void bcm2835_gpio_set_pud(const uint8_t pin, const uint8_t pud) {
	bcm2835_gpio_pud(pud);
	udelay(10);
	bcm2835_gpio_pudclk(pin, 1);
	udelay(10);
	bcm2835_gpio_pud(BCM2835_GPIO_PUD_OFF);
	bcm2835_gpio_pudclk(pin, 0);
}

void bcm2835_uart_begin(void) {
    BCM2835_UART1->ENABLE = 0x01;
    BCM2835_UART1->CNTL = 0x00;
    BCM2835_UART1->LCR = 0x03;
    BCM2835_UART1->MCR = 0x00;
    BCM2835_UART1->IER = 0x05;
    BCM2835_UART1->IIR = 0xC6;
    BCM2835_UART1->BAUD = 270;

    // Set the GPI0 pins to the Alt 5 function to enable UART1 access on them
    bcm2835_gpio_fsel(RPI_V2_GPIO_P1_08, BCM2835_GPIO_FSEL_ALT5); // UART1_TXD
    bcm2835_gpio_fsel(RPI_V2_GPIO_P1_10, BCM2835_GPIO_FSEL_ALT5); // UART1_RXD

    // Disable pull-up/down
    bcm2835_gpio_set_pud(RPI_V2_GPIO_P1_08, BCM2835_GPIO_PUD_OFF);
    bcm2835_gpio_set_pud(RPI_V2_GPIO_P1_10, BCM2835_GPIO_PUD_OFF);

    // turn on the uart for send and receive
    BCM2835_UART1->CNTL = 3;
}


void bcm2835_uart_send(const uint32_t c) {
	while ((BCM2835_UART1->LSR & 0x20) == 0)
		;
	BCM2835_UART1 ->IO = c;
}

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
	BCM2835_PL011 ->IBRD = PL011_BAUD_INT(115200);
	BCM2835_PL011 ->FBRD = PL011_BAUD_FRAC(115200);
	BCM2835_PL011 ->LCRH = PL011_LCRH_WLEN8; 		/* Set N, 8, 1, FIFO disabled */
	BCM2835_PL011 ->CR = 0x301; 					/* Enable UART */
}

void bcm2835_pl011_send(const uint32_t c) {
	while (1) {
		if ((BCM2835_PL011 ->FR & 0x20) == 0)
			break;
	}
	BCM2835_PL011 ->DR = c;
}
