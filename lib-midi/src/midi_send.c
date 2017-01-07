/**
 * @file midi_send.c
 */
/* Copyright (C) 2017 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#include <stdbool.h>

#include "arm/synchronize.h"
#include "arm/pl011.h"

#include "bcm2835.h"
#include "bcm2835_gpio.h"
#include "bcm2835_vc.h"

#include "midi.h"
#include "midi_send.h"

#define PL011_BAUD_INT_3(x) 		(3000000 / (16 * (x)))
#define PL011_BAUD_FRAC_3(x) 		(int)((((3000000.0 / (16.0 * (x))) - PL011_BAUD_INT_3(x)) * 64.0) + 0.5)
#define PL011_BAUD_INT_48(x) 		(48000000 / (16 * (x)))
#define PL011_BAUD_FRAC_48(x) 		(int)((((48000000.0 / (16.0 * (x))) - PL011_BAUD_INT_48(x)) * 64.0) + 0.5)

static void pl011_init(void) {
	uint32_t ibrd = PL011_BAUD_INT_48(MIDI_BAUDRATE_DEFAULT);			// Default UART CLOCK 48Mhz
	uint32_t fbrd = PL011_BAUD_FRAC_48(MIDI_BAUDRATE_DEFAULT);			// Default UART CLOCK 48Mhz
	dmb();

	// Work around BROADCOM firmware bug
	if (bcm2835_vc_get_clock_rate(BCM2835_VC_CLOCK_ID_UART) != 48000000) {
		(void) bcm2835_vc_set_clock_rate(BCM2835_VC_CLOCK_ID_UART, 3000000);// Set UART clock rate to 3000000 (3MHz)
		ibrd = PL011_BAUD_INT_3(MIDI_BAUDRATE_DEFAULT);
		fbrd = PL011_BAUD_FRAC_3(MIDI_BAUDRATE_DEFAULT);
	}

	BCM2835_PL011->CR = 0;												// Disable everything

    bcm2835_gpio_fsel(RPI_V2_GPIO_P1_08, BCM2835_GPIO_FSEL_ALT0);		// PL011_TXD
    bcm2835_gpio_fsel(RPI_V2_GPIO_P1_10, BCM2835_GPIO_FSEL_ALT0);		// PL011_RXD

	bcm2835_gpio_set_pud(RPI_V2_GPIO_P1_08, BCM2835_GPIO_PUD_OFF);		// Disable pull-up/down
	bcm2835_gpio_set_pud(RPI_V2_GPIO_P1_10, BCM2835_GPIO_PUD_OFF);		// Disable pull-up/down

	while ((BCM2835_PL011->FR & PL011_FR_BUSY) != 0)
		;																// Poll the "flags register" to wait for the UART to stop transmitting or receiving

	BCM2835_PL011->LCRH &= ~PL011_LCRH_FEN;								// Flush the transmit FIFO by marking FIFOs as disabled in the "line control register"
	BCM2835_PL011->ICR = 0x7FF;											// Clear all interrupt status
	BCM2835_PL011->IBRD = ibrd;
	BCM2835_PL011->FBRD = fbrd;
	BCM2835_PL011->LCRH = PL011_LCRH_WLEN8 | PL011_LCRH_FEN;			// Set N, 8, 1, FIFO enabled

	BCM2835_PL011->CR = PL011_CR_TXE | PL011_CR_UARTEN;					// Enable UART, TX only

	dmb();
}

/**
 *
 * @param data
 * @param length
 */
static void send(const uint8_t *data, uint16_t length) {
	const uint8_t *p = data;

	while (length > 0) {
		while (BCM2835_PL011->FR & PL011_FR_TXFF) {
		}
		BCM2835_PL011->DR = (uint32_t) *p;
		p++;
		length--;
	}
}

void midi_send_tc(const struct _midi_send_tc *tc) {
	uint8_t data[10] = {0xF0, 0x7F, 0x7F, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0xF7};

	data[5] = (((tc->rate) & 0x03) << 5) | (tc->hour & 0x1F);
	data[6] = tc->minute & 0x3F;
	data[7] = tc->second & 0x3F;
	data[8] = tc->frame & 0x1F;

	send(data, 10);
}

/**
 *
 * @param data
 * @param length
 */
void midi_send_raw(const uint8_t *data, const int16_t length) {
	send(data, length);
}

/**
 * * @ingroup midi
 *
 */
void midi_send_init(void) {
	pl011_init();
}
