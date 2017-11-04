/**
 * @file bcm2835_uart.c
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

#include "bcm2835.h"
#include "bcm2835_gpio.h"
#include "bcm2835_aux.h"
#include "bcm2835_uart.h"

#define UART1_LCR_7BITS		0x02 // 7 bits mode
#define UART1_LCR_8BITS		0x03 // 8 bits mode
#define UART1_LCR_BREAK		0x40 // send break
#define UART1_LCR_DLAB		0x80 // DLAB access

#define UART1_LSR_DR		0x01 // Receive Data ready
#define UART1_LSR_OE		0x02 // Receiver overrun error
#define UART1_LSR_THRE		0x20 // Transmitter holding register
#define UART1_LSR_TEMT 		0x40 // Transmitter empty

#define UART1_CNTL_REC_ENBL	0x01 // receiver enable
#define UART1_CNTL_TRN_ENBL	0x02 // transmitter enable
#define UART1_CNTL_AUTO_RTR	0x04 // RTR set by RX FF level
#define UART1_CNTL_AUTO_CTS	0x08 // CTS auto stops transmitter
#define UART1_CNTL_FLOW3	0x00 // Stop on RX FF 3 entries left
#define UART1_CNTL_FLOW2	0x10 // Stop on RX FF 2 entries left
#define UART1_CNTL_FLOW1	0x20 // Stop on RX FF 1 entries left
#define UART1_CNTL_FLOW4	0x30 // Stop on RX FF 4 entries left
#define UART1_CNTL_AURTRINV	0x40 // Invert AUTO RTR polarity
#define UART1_CNTL_AUCTSINV	0x80 // Invert AUTO CTS polarity

/**
 * @ingroup UART
 *
 */
void bcm2835_uart_begin(void) {
	uint32_t value;

    BCM2835_UART1->ENABLE = BCM2835_AUX_ENABLE_UART1;
    BCM2835_UART1->CNTL = 0x00;
    BCM2835_UART1->LCR = UART1_LCR_8BITS;
    BCM2835_UART1->MCR = 0x00;
    BCM2835_UART1->IER = 0x05;
    BCM2835_UART1->IIR = 0xC6;
    BCM2835_UART1->BAUD = 270;	// // Baud rate = sysclk/(8*(BAUD_REG+1))

    // Set the GPI0 pins to the Alt 5 function to enable UART1 access on them
    value = BCM2835_GPIO->GPFSEL1;
    value &= ~(7 << 12);
    value |= BCM2835_GPIO_FSEL_ALT5 << 12;	// Pin 14 UART1_TXD
    value &= ~(7 << 15);
    value |= BCM2835_GPIO_FSEL_ALT5 << 15;	// Pin 15 UART1_RXD
    BCM2835_GPIO->GPFSEL1 = value;

    // Disable pull-up/down
    bcm2835_gpio_set_pud(RPI_V2_GPIO_P1_08, BCM2835_GPIO_PUD_OFF);
    bcm2835_gpio_set_pud(RPI_V2_GPIO_P1_10, BCM2835_GPIO_PUD_OFF);

    // turn on the uart for send and receive
    BCM2835_UART1->CNTL = UART1_CNTL_REC_ENBL | UART1_CNTL_TRN_ENBL;
}

/**
 * @ingroup UART
 *
 * @param c
 */
void bcm2835_uart_send(const uint8_t c) {
	while ((BCM2835_UART1->LSR & UART1_LSR_THRE) == 0)
		;
	BCM2835_UART1->IO = (uint32_t) c;
}

/**
 * @ingroup UART
 */
uint8_t bcm2835_uart_receive(void) {
	while (1 == 1) {
		if (BCM2835_UART1->LSR & UART1_LSR_DR)
			break;
	}
	return (uint8_t) (BCM2835_UART1->IO);
}

