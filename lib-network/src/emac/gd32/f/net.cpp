/**
 * net.c
 *
 */
/* Copyright (C) 2022-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#pragma GCC push_options
#pragma GCC optimize ("O3")

#include <cstdint>

#include "gd32.h"
#include "../src/net/net_packets.h"

#if defined (CONFIG_ENET_ENABLE_PTP)
extern enet_descriptors_struct *dma_current_ptp_rxdesc;
extern enet_descriptors_struct *dma_current_txdesc;
namespace net {
namespace globals {
extern uint32_t ptpTimestamp[2];
extern bool IsValidPtpTimestamp;
}  // namespace globals
}  // namespace net
#endif
extern enet_descriptors_struct  *dma_current_rxdesc;

extern "C" {
int console_error(const char *);

int emac_eth_recv(uint8_t **ppPacket) {
	const auto nLength = enet_desc_information_get(dma_current_rxdesc, RXDESC_FRAME_LENGTH);

	if (nLength > 0) {
#if defined (CONFIG_ENET_ENABLE_PTP)
		*ppPacket = (uint8_t *)(dma_current_ptp_rxdesc->buffer1_addr);
#else
		*ppPacket = (uint8_t *)(dma_current_rxdesc->buffer1_addr);
#endif
		return nLength;
	}

	return -1;
}

void emac_free_pkt() {
#if defined (CONFIG_ENET_ENABLE_PTP)
	const auto status = ENET_NOCOPY_PTPFRAME_RECEIVE_NORMAL_MODE(net::globals::ptpTimestamp);
	if (status != ERROR) {
		net::globals::IsValidPtpTimestamp = true;
	} else {
		console_error("ENET_NOCOPY_PTPFRAME_RECEIVE_NORMAL_MODE failed\n");
	}
#else
	ENET_NOCOPY_FRAME_RECEIVE();
#endif
}

#if defined (CONFIG_ENET_ENABLE_PTP)
static bool isPTP(void *pBuffer) {
	const auto *const ether = reinterpret_cast<struct ether_header *>(pBuffer);
	// Support for Ethernet level 2 only
	return (ether->type == __builtin_bswap16(ETHER_TYPE_PTP));
}
#endif

void emac_eth_send(void *pBuffer, int nLength) {
#if defined (CONFIG_ENET_ENABLE_PTP)
	auto nStatus = dma_current_txdesc->status;
	if (isPTP(pBuffer)) {
		nStatus |= ENET_TDES0_TTSEN;
	} else {
		nStatus &= ~ENET_TDES0_TTSEN;
	}
	dma_current_txdesc->status = nStatus;
	ErrStatus status = enet_ptpframe_transmit_normal_mode((uint8_t *)pBuffer, nLength, NULL);
#else
	ErrStatus status = enet_frame_transmit((uint8_t *)pBuffer, nLength);
#endif
	if (status == ERROR) {
		console_error("emac_eth_send failed\n");
	}
}
}
