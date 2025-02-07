/**
 * @file emac.cpp
 * @brief Ethernet MAC driver implementation for GD32 devices.
 *
 * This file provides functions for managing Ethernet communication, including
 * packet transmission and reception. It also supports Precision Time Protocol
 * (PTP) functionality if enabled.
 */
/* Copyright (C) 2022-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#if defined (DEBUG_EMAC)
# undef NDEBUG	///< Disable NDEBUG for debugging
#endif

#if !defined (CONFIG_REMOTECONFIG_MINIMUM)
# pragma GCC push_options
# pragma GCC optimize ("O2")
# pragma GCC optimize ("no-tree-loop-distribute-patterns")
#endif

#include <cstdint>
#include <cassert>

#include "gd32.h"
#include "gd32_enet.h"
#include "../src/net/net_memcpy.h"

#include "debug.h"

#if defined (CONFIG_NET_ENABLE_PTP)
#include "gd32_ptp.h"

/// Current PTP receive descriptor
extern enet_descriptors_struct *dma_current_ptp_rxdesc;
/// Current PTP transmit descriptor
extern enet_descriptors_struct *dma_current_ptp_txdesc;

namespace net {
namespace globals {
extern uint32_t ptpTimestamp[2];
}  // namespace globals
}  // namespace net
#endif

/// Current receive descriptor
extern enet_descriptors_struct *dma_current_rxdesc;
/// Current transmit descriptor
extern enet_descriptors_struct *dma_current_txdesc;

/**
 * @brief Receives an Ethernet packet.
 *
 * @param[out] ppPacket Pointer to the received packet buffer.
 * @return Length of the received packet in bytes, or 0 if no packet is available.
 */
uint32_t emac_eth_recv(uint8_t **ppPacket) {
	const auto nLength = gd32_enet_desc_information_get<RXDESC_FRAME_LENGTH>(dma_current_rxdesc);

	if (nLength > 0) {
#if defined (CONFIG_NET_ENABLE_PTP)
		*ppPacket = reinterpret_cast<uint8_t *>(dma_current_ptp_rxdesc->buffer1_addr);
#else
		*ppPacket = reinterpret_cast<uint8_t *>(dma_current_rxdesc->buffer1_addr);
#endif
		return nLength;
	}

	return 0;
}

#if defined (CONFIG_NET_ENABLE_PTP)
/**
 * @brief Handles reception of a PTP frame in normal mode.
 */
static void ptpframe_receive_normal_mode() {
	net::globals::ptpTimestamp[0] = dma_current_rxdesc->buffer1_addr;
	net::globals::ptpTimestamp[1] = dma_current_rxdesc->buffer2_next_desc_addr;

	dma_current_rxdesc->buffer1_addr = dma_current_ptp_rxdesc->buffer1_addr;
	dma_current_rxdesc->buffer2_next_desc_addr = dma_current_ptp_rxdesc->buffer2_next_desc_addr;
	dma_current_rxdesc->status = ENET_RDES0_DAV;

#if defined (GD32H7XX)
    __DMB();
#endif

	gd32_enet_handle_rx_buffer_unavailable();

	assert(0 != (dma_current_rxdesc->control_buffer_size & ENET_RDES1_RCHM));	/// chained mode

	/// update the current RxDMA descriptor pointer to the next descriptor in RxDMA descriptor table
	dma_current_rxdesc = reinterpret_cast<enet_descriptors_struct *>(dma_current_ptp_rxdesc->buffer2_next_desc_addr);
	/// if it is the last ptp descriptor */
	if (0 != dma_current_ptp_rxdesc->status) {
		/// pointer back to the first ptp descriptor address in the desc_ptptab list address
		dma_current_ptp_rxdesc = reinterpret_cast<enet_descriptors_struct *>(dma_current_ptp_rxdesc->status);
	} else {
		/// Set pointer to the next ptp descriptor
		dma_current_ptp_rxdesc++;
	}
}
#else
/**
 * @brief Handles reception of a standard Ethernet frame.
 */
static void frame_receive() {
	dma_current_rxdesc->status = ENET_RDES0_DAV;

	gd32_enet_handle_rx_buffer_unavailable();

	assert(0 != (dma_current_rxdesc->control_buffer_size & ENET_RDES1_RCHM));

	/// Update Rx descriptor pointer
	dma_current_rxdesc = reinterpret_cast<enet_descriptors_struct *>(dma_current_rxdesc->buffer2_next_desc_addr);
}
#endif

/**
 * @brief Frees the current packet from the DMA buffer.
 */
void emac_free_pkt() {
    while(0 != (dma_current_rxdesc->status & ENET_RDES0_DAV)) {
        __DMB();
    }

#if defined (CONFIG_NET_ENABLE_PTP)
    ptpframe_receive_normal_mode();
#else
	frame_receive();
#endif
}

#if defined (CONFIG_NET_ENABLE_PTP)
/**
 * @brief Retrieves the DMA buffer for Ethernet transmission with PTP.
 *
 * @return Pointer to the DMA buffer for transmission.
 */
uint8_t *emac_eth_send_get_dma_buffer() {
	while (0 != (dma_current_txdesc->status & ENET_TDES0_DAV)) {
		__DMB();	///< Wait until descriptor is available
	}

	return reinterpret_cast<uint8_t *>(dma_current_ptp_txdesc->buffer1_addr);
}

/**
 * @brief Transmits a PTP frame.
 *
 * @tparam T Whether timestamping is enabled.
 * @param nLength Length of the frame to transmit.
 */
template <bool T>
static void ptpframe_transmit(const uint32_t nLength) {
    dma_current_txdesc->control_buffer_size = nLength;	///< Set the frame length
    dma_current_txdesc->status |= ENET_TDES0_LSG | ENET_TDES0_FSG; 	///< Set the segment of frame, frame is transmitted in one descriptor
    dma_current_txdesc->status |= ENET_TDES0_DAV;	///< Enable DMA transmission

#if defined (GD32H7XX)
    __DMB();
#endif

    gd32_enet_clear_dma_tx_flags_and_resume();	///< Handle transmission flags

    uint32_t timeout = 0;
    uint32_t tdes0_ttmss_flag;

	if constexpr (T) {	// Handle timestamping
		do {
			tdes0_ttmss_flag = (dma_current_txdesc->status & ENET_TDES0_TTMSS);
			__DMB();
			timeout++;
		} while ((0 == tdes0_ttmss_flag) && (timeout < UINT32_MAX));

		DEBUG_PRINTF("timeout=%x %d", timeout, (dma_current_txdesc->status & ENET_TDES0_TTMSS));

		dma_current_txdesc->status &= ~ENET_TDES0_TTMSS;	///< Clear timestamp flag

		net::globals::ptpTimestamp[0] = dma_current_txdesc->buffer1_addr;
		net::globals::ptpTimestamp[1] = dma_current_txdesc->buffer2_next_desc_addr;
	}

	dma_current_txdesc->buffer1_addr = dma_current_ptp_txdesc->buffer1_addr;
	dma_current_txdesc->buffer2_next_desc_addr = dma_current_ptp_txdesc->buffer2_next_desc_addr;

    assert(0 != (dma_current_txdesc->status & ENET_TDES0_TCHM)); /// Chained mode

    /// Update the current TxDMA descriptor pointer to the next descriptor in TxDMA descriptor table
    dma_current_txdesc = reinterpret_cast<enet_descriptors_struct *>(dma_current_ptp_txdesc->buffer2_next_desc_addr);
    /// if it is the last ptp descriptor */
    if(0 != dma_current_ptp_txdesc->status) {
    	/// pointer back to the first ptp descriptor address in the desc_ptptab list address
    	dma_current_ptp_txdesc = reinterpret_cast<enet_descriptors_struct *>(dma_current_ptp_txdesc->status);
    } else {
    	/// Set pointer to the next ptp descriptor
    	dma_current_ptp_txdesc++;
    }
}

/**
 * @brief Sends a PTP Ethernet frame.
 *
 * @tparam T Whether timestamping is enabled.
 * @param pBuffer Pointer to the frame buffer.
 * @param nLength Length of the frame in bytes.
 */
template <bool T>
static void ptpframe_transmit(const void *pBuffer, const uint32_t nLength) {
	assert (nullptr != pBuffer);
	assert(nLength <= ENET_MAX_FRAME_SIZE);

	auto *pDst = reinterpret_cast<uint8_t *>(dma_current_ptp_txdesc->buffer1_addr);
	net::memcpy(pDst, pBuffer, nLength);	///< Copy frame to DMA buffer

	ptpframe_transmit<T>(nLength);
}

/**
 * @brief Transmits an Ethernet frame without timestamping.
 *
 * @param nLength Length of the frame to transmit.
 */
void emac_eth_send(const uint32_t nLength) {
	assert(nLength <= ENET_MAX_FRAME_SIZE);

	auto nStatus = dma_current_txdesc->status;
	nStatus &= ~ENET_TDES0_TTSEN;
	dma_current_txdesc->status = nStatus;

#if defined (GD32H7XX)
	__DMB();
#endif

	ptpframe_transmit<false>(nLength);
}

/**
 * @brief Transmits an Ethernet frame with data copying.
 *
 * @param pBuffer Pointer to the frame buffer.
 * @param nLength Length of the frame in bytes.
 */
void emac_eth_send(void *pBuffer, const uint32_t nLength) {
	assert(nullptr != pBuffer);
	assert(nLength <= ENET_MAX_FRAME_SIZE);

	while (0 != (dma_current_txdesc->status & ENET_TDES0_DAV)) {
		__DMB(); ///< Wait until descriptor is available
	}

	auto nStatus = dma_current_txdesc->status;
	nStatus &= ~ENET_TDES0_TTSEN;	 ///< Disable timestamping
	dma_current_txdesc->status = nStatus;

#if defined (GD32H7XX)
	__DMB();
#endif

	ptpframe_transmit<false>(pBuffer, nLength);
}

/**
 * @brief Transmits a timestamped Ethernet frame.
 *
 * @param nLength Length of the frame in bytes.
 */
void emac_eth_send_timestamp(const uint32_t nLength) {
	assert(nLength <= ENET_MAX_FRAME_SIZE);

	auto nStatus = dma_current_txdesc->status;
	nStatus |= ENET_TDES0_TTSEN; ///< Enable timestamping
	dma_current_txdesc->status = nStatus;

#if defined (GD32H7XX)
	__DMB();
#endif

	ptpframe_transmit<true>(nLength);
}

/**
 * @brief Transmits a timestamped Ethernet frame with data copying.
 *
 * @param pBuffer Pointer to the frame buffer.
 * @param nLength Length of the frame in bytes.
 */
void emac_eth_send_timestamp(void *pBuffer, const uint32_t nLength) {
	assert(nullptr != pBuffer);
	assert(nLength <= ENET_MAX_FRAME_SIZE);

	while (0 != (dma_current_txdesc->status & ENET_TDES0_DAV)) {
		__DMB();
	}

	auto nStatus = dma_current_txdesc->status;
	nStatus |= ENET_TDES0_TTSEN; ///< Enable timestamping
	dma_current_txdesc->status = nStatus;

#if defined (GD32H7XX)
	__DMB();
#endif

	ptpframe_transmit<true>(pBuffer, nLength);
}
#else
/**
 * @brief Retrieves the DMA buffer for Ethernet transmission.
 *
 * @return Pointer to the DMA buffer for transmission.
 */
uint8_t *emac_eth_send_get_dma_buffer() {
	while (0 != (dma_current_txdesc->status & ENET_TDES0_DAV)) {
		__DMB(); ///< Wait until descriptor is available
	}

	return reinterpret_cast<uint8_t *>(dma_current_txdesc->buffer1_addr);
}

/**
 * @brief Transmits an Ethernet frame.
 *
 * @param nLength Length of the frame to transmit.
 */
void emac_eth_send(const uint32_t nLength) {
	debug_dump(reinterpret_cast<uint8_t *>(dma_current_txdesc->buffer1_addr), nLength);

	dma_current_txdesc->control_buffer_size = nLength;	///< Set the frame length
	dma_current_txdesc->status |= ENET_TDES0_LSG | ENET_TDES0_FSG; ///< Set the segment of frame, frame is transmitted in one descriptor
	dma_current_txdesc->status |= ENET_TDES0_DAV;	///< Enable DMA transmission

#if defined (GD32H7XX)
	__DMB();
#endif

	gd32_enet_clear_dma_tx_flags_and_resume();	///< Handle transmission flags

	assert(0 != (dma_current_txdesc->status & ENET_TDES0_TCHM)); /// Chained mode

	/// Update the current TxDMA descriptor pointer to the next descriptor in TxDMA descriptor table
	dma_current_txdesc = reinterpret_cast<enet_descriptors_struct *>(dma_current_txdesc->buffer2_next_desc_addr);
}

/**
 * @brief Transmits an Ethernet frame with data copying.
 *
 * @param pBuffer Pointer to the frame buffer.
 * @param nLength Length of the frame in bytes.
 */
void emac_eth_send(void *pBuffer, const uint32_t nLength) {
	DEBUG_PRINTF("%p -> %u", pBuffer, nLength);

	assert(nullptr != pBuffer);
	assert(nLength <= ENET_MAX_FRAME_SIZE);

	auto *pDst = emac_eth_send_get_dma_buffer();
	net::memcpy(pDst, pBuffer, nLength);	///< Copy frame to DMA buffer

	emac_eth_send(nLength);
}
#endif
