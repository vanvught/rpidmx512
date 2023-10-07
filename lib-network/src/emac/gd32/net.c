/**
 * net.c
 *
 */
/* Copyright (C) 2022 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include "gd32.h"

extern enet_descriptors_struct  *dma_current_rxdesc;

int emac_eth_recv(uint8_t **packet) {
	const uint32_t size = enet_rxframe_size_get();

	if (size > 0) {
		*packet = (uint8_t *) (enet_desc_information_get(dma_current_rxdesc, RXDESC_BUFFER_1_ADDR));
		return size;
	}

	return -1;
}

void emac_free_pkt(void) {
	ENET_NOCOPY_FRAME_RECEIVE();
}

void emac_eth_send(void *packet, int len) {
	enet_frame_transmit((uint8_t *) packet, len);
}
