/**
 * @file bcm2835_uart.c
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
#include "bcm2835_uart.h"

void bcm2835_uart_begin(void) {
    BCM2835_UART1->ENABLE = 0x01;
    BCM2835_UART1->CNTL = 0x00;
    BCM2835_UART1->LCR = 0x03;
    BCM2835_UART1->MCR = 0x00;
    BCM2835_UART1->IER = 0x05;
    BCM2835_UART1->IIR = 0xC6;
    BCM2835_UART1->BAUD = 270;

    // Set the GPI0 pins to the Alt 5 function to enable UART1 access on them
    uint32_t value = BCM2835_GPIO->GPFSEL1;
    value &= ~(7 << 12);
    value |= BCM2835_GPIO_FSEL_ALT5 << 12;	// Pin 14 UART1_TXD
    value &= ~(7 << 15);
    value |= BCM2835_GPIO_FSEL_ALT5 << 15;	// Pin 15 UART1_RXD
    BCM2835_GPIO->GPFSEL1 = value;

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

