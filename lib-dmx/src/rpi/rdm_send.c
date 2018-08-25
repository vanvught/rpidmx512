/**
 * @file rdm_send.c
 *
 */
/* Copyright (C) 2017-2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include "arm/pl011.h"
#include "bcm2835.h"

#include "rdm.h"
#include "dmx.h"

void rdm_send_data(const uint8_t *data, uint16_t data_length) {
	uint16_t i;

	BCM2835_PL011->LCRH &= ~PL011_LCRH_FEN;
	BCM2835_PL011->LCRH = PL011_LCRH_WLEN8 | PL011_LCRH_STP2 | PL011_LCRH_BRK;
	udelay(RDM_TRANSMIT_BREAK_TIME);	// Break Time

	BCM2835_PL011->LCRH = PL011_LCRH_WLEN8 | PL011_LCRH_STP2;
	udelay(RDM_TRANSMIT_MAB_TIME);		// Mark After Break

	for (i = 0; i < data_length; i++) {
		while ((BCM2835_PL011->FR & PL011_FR_TXFF) == PL011_FR_TXFF) {
		}
		BCM2835_PL011->DR = data[i];
	}

	while ((BCM2835_PL011->FR & PL011_FR_BUSY) != 0)
		;

}

void rdm_send_no_break(const uint8_t *data, uint16_t data_length) {
	uint16_t i;

	BCM2835_PL011->LCRH &= ~PL011_LCRH_FEN;
	BCM2835_PL011->LCRH = PL011_LCRH_WLEN8 | PL011_LCRH_STP2;

	for (i = 0; i < data_length; i++) {
		while ((BCM2835_PL011->FR & PL011_FR_TXFF) != 0)
			;
		BCM2835_PL011->DR = data[i];
	}

	while ((BCM2835_PL011->FR & PL011_FR_BUSY) != 0)
		;
}

void rdm_send_discovery_respond_message(const uint8_t *data, uint16_t data_length) {
	const uint32_t delay = BCM2835_ST->CLO - rdm_get_data_receive_end();

	// 3.2.2 Responder Packet spacing
	if (delay < RDM_RESPONDER_PACKET_SPACING) {
		udelay(RDM_RESPONDER_PACKET_SPACING - delay);
	}

	dmx_set_port_direction(DMX_PORT_DIRECTION_OUTP, false);

	rdm_send_no_break(data, data_length);
	udelay(RDM_RESPONDER_DATA_DIRECTION_DELAY);

	dmx_set_port_direction(DMX_PORT_DIRECTION_INP, true);
}
