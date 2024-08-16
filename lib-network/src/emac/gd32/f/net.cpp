/**
 * net.cpp
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

#if !defined (CONFIG_REMOTECONFIG_MINIMUM)
# pragma GCC push_options
# pragma GCC optimize ("O2")
# pragma GCC optimize ("no-tree-loop-distribute-patterns")
#endif

#include <cstdint>
#include <cassert>

#include "gd32.h"
#include "../src/net/net_memcpy.h"

#include "debug.h"

#if defined (CONFIG_ENET_ENABLE_PTP)
#include "gd32_ptp.h"

extern enet_descriptors_struct *dma_current_ptp_rxdesc;
extern enet_descriptors_struct *dma_current_ptp_txdesc;
namespace net {
namespace globals {
extern uint32_t ptpTimestamp[2];
}  // namespace globals
}  // namespace net
#endif
extern enet_descriptors_struct *dma_current_rxdesc;
extern enet_descriptors_struct *dma_current_txdesc;

int emac_eth_recv(uint8_t **ppPacket) {
	const auto nLength = enet_desc_information_get(dma_current_rxdesc, RXDESC_FRAME_LENGTH);

	if (nLength > 0) {
#if defined (CONFIG_ENET_ENABLE_PTP)
		*ppPacket = reinterpret_cast<uint8_t *>(dma_current_ptp_rxdesc->buffer1_addr);
#else
		*ppPacket = reinterpret_cast<uint8_t *>(dma_current_rxdesc->buffer1_addr);
#endif
		return nLength;
	}

	return -1;
}

#if defined (CONFIG_ENET_ENABLE_PTP)
static void ptpframe_receive_normal_mode() {
	net::globals::ptpTimestamp[0] = dma_current_rxdesc->buffer1_addr;
	net::globals::ptpTimestamp[1] = dma_current_rxdesc->buffer2_next_desc_addr;

	dma_current_rxdesc->buffer1_addr = dma_current_ptp_rxdesc->buffer1_addr;
	dma_current_rxdesc->buffer2_next_desc_addr = dma_current_ptp_rxdesc->buffer2_next_desc_addr;
	dma_current_rxdesc->status = ENET_RDES0_DAV;

	if (0 != (ENET_DMA_STAT & ENET_DMA_STAT_RBU)) {
		/* clear RBU flag */
		ENET_DMA_STAT = ENET_DMA_STAT_RBU;
		/* resume DMA reception by writing to the RPEN register*/
		ENET_DMA_RPEN = 0U;
	}

	 assert(0 != (dma_current_rxdesc->control_buffer_size & ENET_RDES1_RCHM));

	 dma_current_rxdesc = reinterpret_cast<enet_descriptors_struct *>(dma_current_ptp_rxdesc->buffer2_next_desc_addr);
	 /* if it is the last ptp descriptor */
	 if (0 != dma_current_ptp_rxdesc->status) {
		 /* pointer back to the first ptp descriptor address in the desc_ptptab list address */
		 dma_current_ptp_rxdesc = reinterpret_cast<enet_descriptors_struct *>(dma_current_ptp_rxdesc->status);
	 } else {
		 /* ponter to the next ptp descriptor */
		 dma_current_ptp_rxdesc++;
	 }
}
#else
static void frame_receive() {
	dma_current_rxdesc->status = ENET_RDES0_DAV;

	/* check Rx buffer unavailable flag status */
	if (0 != (ENET_DMA_STAT & ENET_DMA_STAT_RBU)) {
		/* clear RBU flag */
		ENET_DMA_STAT = ENET_DMA_STAT_RBU;
		/* resume DMA reception by writing to the RPEN register*/
		ENET_DMA_RPEN = 0U;
	}

	assert(0 != (dma_current_rxdesc->control_buffer_size & ENET_RDES1_RCHM));

	dma_current_rxdesc = reinterpret_cast<enet_descriptors_struct *>(dma_current_rxdesc->buffer2_next_desc_addr);
}
#endif

void emac_free_pkt() {
    while(0 != (dma_current_rxdesc->status & ENET_RDES0_DAV)) {
        __DMB();
    }
#if defined (CONFIG_ENET_ENABLE_PTP)
    ptpframe_receive_normal_mode();
#else
    frame_receive();
#endif
}

#if defined (CONFIG_ENET_ENABLE_PTP)
inline static void ptpframe_transmit(const uint8_t *pBuffer, const uint32_t nLength, const bool bCaptureTimestamp) {
	assert (nullptr != pBuffer);
	assert(nLength <= ENET_MAX_FRAME_SIZE);

	auto *pDst = reinterpret_cast<uint8_t *>(dma_current_ptp_txdesc->buffer1_addr);
	net::memcpy(pDst, pBuffer, nLength);

    dma_current_txdesc->control_buffer_size = (nLength & (uint32_t)0x1FFF);
    /* set the segment of frame, frame is transmitted in one descriptor */
    dma_current_txdesc->status |= ENET_TDES0_LSG | ENET_TDES0_FSG;
    /* enable the DMA transmission */
    dma_current_txdesc->status |= ENET_TDES0_DAV;

    /* check Tx buffer unavailable flag status */
    const auto dma_tbu_flag = (ENET_DMA_STAT & ENET_DMA_STAT_TBU);
    const auto dma_tu_flag = (ENET_DMA_STAT & ENET_DMA_STAT_TU);

	if ((0 != dma_tbu_flag) || (0 != dma_tu_flag)) {
		/* clear TBU and TU flag */
		ENET_DMA_STAT = (dma_tbu_flag | dma_tu_flag);
		/* resume DMA transmission by writing to the TPEN register*/
		ENET_DMA_TPEN = 0U;
	}

    uint32_t timeout = 0;
    uint32_t tdes0_ttmss_flag;

    if (bCaptureTimestamp) {
         do {
           tdes0_ttmss_flag = (dma_current_txdesc->status & ENET_TDES0_TTMSS);
           __DMB();
            timeout++;
        } while((0 == tdes0_ttmss_flag) && (timeout < UINT32_MAX));

        DEBUG_PRINTF("timeout=%x %d", timeout, (dma_current_txdesc->status & ENET_TDES0_TTMSS));

        dma_current_txdesc->status &= ~ENET_TDES0_TTMSS;

        net::globals::ptpTimestamp[0] = dma_current_txdesc->buffer1_addr;
        net::globals::ptpTimestamp[1] = dma_current_txdesc->buffer2_next_desc_addr;
    }

	dma_current_txdesc->buffer1_addr = dma_current_ptp_txdesc->buffer1_addr;
	dma_current_txdesc->buffer2_next_desc_addr = dma_current_ptp_txdesc->buffer2_next_desc_addr;

    assert(0 != (dma_current_txdesc->status & ENET_TDES0_TCHM)); /* chained mode */

    /* update the current TxDMA descriptor pointer to the next descriptor in TxDMA descriptor table */
    dma_current_txdesc = reinterpret_cast<enet_descriptors_struct *>(dma_current_ptp_txdesc->buffer2_next_desc_addr);
    /* if it is the last ptp descriptor */
    if(0 != dma_current_ptp_txdesc->status) {
    	/* pointer back to the first ptp descriptor address in the desc_ptptab list address */
    	dma_current_ptp_txdesc = reinterpret_cast<enet_descriptors_struct *>(dma_current_ptp_txdesc->status);
    } else {
    	/* pointer to the next ptp descriptor */
    	dma_current_ptp_txdesc++;
    }
}

void emac_eth_send(void *pBuffer, uint32_t nLength) {
	while (0 != (dma_current_txdesc->status & ENET_TDES0_DAV)) {
        __DMB();
	}

	auto nStatus = dma_current_txdesc->status;
	nStatus &= ~ENET_TDES0_TTSEN;
	dma_current_txdesc->status = nStatus;

	ptpframe_transmit(reinterpret_cast<uint8_t *>(pBuffer), nLength, false);
}

void emac_eth_send_timestamp(void *pBuffer, uint32_t nLength) {
	while (0 != (dma_current_txdesc->status & ENET_TDES0_DAV)) {
        __DMB();
	}

	auto nStatus = dma_current_txdesc->status;
	nStatus |= ENET_TDES0_TTSEN;
	dma_current_txdesc->status = nStatus;

	ptpframe_transmit(reinterpret_cast<uint8_t *>(pBuffer), nLength, true);
}
#else
void emac_eth_send(void *pBuffer, uint32_t nLength) {
	assert(nullptr != pBuffer);
	assert(nLength <= static_cast<int>(ENET_MAX_FRAME_SIZE));

	while (0 != (dma_current_txdesc->status & ENET_TDES0_DAV)) {
		__DMB();
	}

	auto *pDst = reinterpret_cast<uint8_t *>(dma_current_txdesc->buffer1_addr);
	net::memcpy(pDst, pBuffer, nLength);

	/* set the frame length */
	dma_current_txdesc->control_buffer_size = nLength;
	/* set the segment of frame, frame is transmitted in one descriptor */
	dma_current_txdesc->status |= ENET_TDES0_LSG | ENET_TDES0_FSG;
	/* enable the DMA transmission */
	dma_current_txdesc->status |= ENET_TDES0_DAV;

	/* check Tx buffer unavailable flag status */
	const auto dma_tbu_flag = (ENET_DMA_STAT & ENET_DMA_STAT_TBU);
	const auto dma_tu_flag = (ENET_DMA_STAT & ENET_DMA_STAT_TU);

	if ((0 != dma_tbu_flag) || (0 != dma_tu_flag)) {
		/* clear TBU and TU flag */
		ENET_DMA_STAT = (dma_tbu_flag | dma_tu_flag);
		/* resume DMA transmission by writing to the TPEN register*/
		ENET_DMA_TPEN = 0;
	}

	assert(0 != (dma_current_txdesc->status & ENET_TDES0_TCHM)); /* chained mode */

	/* update the current TxDMA descriptor pointer to the next descriptor in TxDMA descriptor table*/
	dma_current_txdesc = reinterpret_cast<enet_descriptors_struct *>(dma_current_txdesc->buffer2_next_desc_addr);
}
#endif
