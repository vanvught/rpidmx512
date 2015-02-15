/**
 * @file bcm2835_pl011_dmx512.c
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

#include "bcm2835_gpio.h"
#include "bcm2835_vc.h"
#include "bcm2835_pl011.h"

/**
 * @ingroup DMX512
 *
 */
void bcm2835_pl011_dmx512_begin(void) {
	// Set UART clock rate to 4000000 (4MHz)
	bcm2835_vc_set_clock_rate(BCM2835_VC_CLOCK_ID_UART, 4000000);
	//
	BCM2835_PL011->CR	= 0;										// Disable everything
	//
    uint32_t value = BCM2835_GPIO->GPFSEL1;
    value &= ~(7 << 12);
    value |= BCM2835_GPIO_FSEL_ALT0 << 12;							// Pin 14 PL011_TXD
    value &= ~(7 << 15);
    value |= BCM2835_GPIO_FSEL_ALT0 << 15;							// Pin 15 PL011_RXD
    BCM2835_GPIO->GPFSEL1 = value;
	//
	bcm2835_gpio_set_pud(RPI_V2_GPIO_P1_08, BCM2835_GPIO_PUD_OFF);	// Disable pull-up/down
	bcm2835_gpio_set_pud(RPI_V2_GPIO_P1_10, BCM2835_GPIO_PUD_OFF);	// Disable pull-up/down
	//
	while (BCM2835_PL011 ->FR & PL011_FR_BUSY ) {}					// Poll the "flags register" to wait for the UART to stop transmitting or receiving
	//
	BCM2835_PL011->LCRH &= ~PL011_LCRH_FEN;							// Flush the transmit FIFO by marking FIFOs as disabled in the "line control register"
	BCM2835_PL011->ICR 	= 0x7FF;									// Clear all interrupt status
	BCM2835_PL011->IBRD = 1;										// UART Clock
	BCM2835_PL011->FBRD = 0;										// 4000000 (4MHz)
	BCM2835_PL011->LCRH = PL011_LCRH_WLEN8 | PL011_LCRH_STP2 ;		// Set 8, N, 2, FIFO disabled
	BCM2835_PL011->CR 	= 0x301;									// Enable UART
}

/**
 * @ingroup DMX512
 *
 * @param data
 * @param data_length
 */
void bcm2835_pl011_dmx512_send(const uint8_t *data, const uint16_t data_length)
{
	BCM2835_PL011->LCRH = PL011_LCRH_WLEN8 | PL011_LCRH_STP2 | PL011_LCRH_BRK;
	udelay(88);						// Break Time
	BCM2835_PL011->LCRH = PL011_LCRH_WLEN8 | PL011_LCRH_STP2;
	udelay(8);						// Mark After Break
	uint16_t i =0;
	BCM2835_PL011->DR = 0;
	for (i = 0; i < data_length; i++)
	{
		while (1)
		{
			if ((BCM2835_PL011->FR & 0x20) == 0)
				break;
		}
		BCM2835_PL011->DR = i;
	}
}
