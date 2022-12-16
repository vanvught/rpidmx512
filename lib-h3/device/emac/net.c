/**
 * @file net.c
 *
 */
/* Copyright (C) 2018-2022 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "h3.h"
#include "emac.h"

#include "debug.h"

extern struct coherent_region *p_coherent_region;

__attribute__((hot)) int emac_eth_recv(uint8_t **packetp) {
	uint32_t status, desc_num = p_coherent_region->rx_currdescnum;
	struct emac_dma_desc *desc_p = &p_coherent_region->rx_chain[desc_num];
	int length;

	status = desc_p->status;

	/* Check for DMA own bit */
	if (!(status & (1U << 31))) {
		length = (desc_p->status >> 16) & 0x3FFF;

		if (length < 0x40) {
			DEBUG_PUTS("Bad Packet (length < 0x40)");
			return -1;
		} else {
			if (length > CONFIG_ETH_RXSIZE) {
				DEBUG_PRINTF("Received packet is too big (length=%d)\n", length);
				return -1;
			}

			*packetp = (uint8_t*) (uint32_t) desc_p->buf_addr;
#ifdef DEBUG_DUMP
			debug_dump((void*) *packetp, (uint16_t) length);
#endif
			return length;
		}
	}

	return -1;
}

void emac_eth_send(void *packet, int len) {
	uint32_t value;
	uint32_t desc_num = p_coherent_region->tx_currdescnum;
	struct emac_dma_desc *desc_p = &p_coherent_region->tx_chain[desc_num];
	uintptr_t data_start = (uintptr_t) desc_p->buf_addr;

	desc_p->st = (uint32_t)len;
	/* Mandatory undocumented bit */
	desc_p->st |= (1U << 24);

	h3_memcpy((void *) data_start, packet, (size_t)len);
#ifdef DEBUG_DUMP
	debug_dump((void *) data_start, (uint16_t) len);
#endif
	/* frame end */
	desc_p->st |= (1 << 30);
	desc_p->st |= (1U << 31);

	/*frame begin */
	desc_p->st |= (1 << 29);
	desc_p->status = (1U << 31);

	/* Move to next Descriptor and wrap around */
	if (++desc_num >= CONFIG_TX_DESCR_NUM) {
		desc_num = 0;
	}

	p_coherent_region->tx_currdescnum = desc_num;

	/* Start the DMA */
	value = H3_EMAC->TX_CTL1;
	value |= (1U << 31);/* mandatory */
	value |= (1 << 30);/* mandatory */
	H3_EMAC->TX_CTL1 = value;
}

void emac_free_pkt(void) {
	uint32_t desc_num = p_coherent_region->rx_currdescnum;
	struct emac_dma_desc *desc_p = &p_coherent_region->rx_chain[desc_num];

	/* Make the current descriptor valid again */
	desc_p->status |= (1U << 31);

	/* Move to next desc and wrap-around condition. */
	if (++desc_num >= CONFIG_RX_DESCR_NUM) {
		desc_num = 0;
	}

	p_coherent_region->rx_currdescnum = desc_num;
}
