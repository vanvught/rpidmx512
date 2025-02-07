/**
 * @file h3_dma_memcpy32.cpp
 *
 * @brief DMA memory operations for Cortex-A profile using the Sunxi DMA controller.
 *
 * This namespace provides functions to initialize and perform 32-bit aligned memory copy
 * using the DMA hardware on the Allwinner H3 platform.
 *
 */
/* Copyright (C) 2024-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#if defined(__GNUC__) && !defined(__clang__)
# pragma GCC push_options
# pragma GCC optimize ("O2")
# pragma GCC diagnostic ignored "-Warray-bounds"
#endif

#include <cstdint>
#include <cassert>

#include "h3.h"
#include "h3_ccu.h"
#include "h3_dma.h"

#include "arm/gic.h"

#if defined (CONFIG_DMA_MEMCPY_IRQ)
static void DMA_IRQHandler() {
	H3_DMA->IRQ_PEND0 = DMA_IRQ_PEND0_DMA5_PKG_IRQ_EN;
}
#endif

namespace dma {
/**
 * @struct dma_memory
 * @brief Structure containing a single Sunxi DMA linked list item.
 *
 * This structure holds the configuration and control parameters for a single DMA transaction.
 */
struct dma_memory {
	struct sunxi_dma_lli lli; ///< Linked list item descriptor.
};

/**
 * @brief Pointer to DMA memory linked list item in SRAM A1.
 *
 * This pointer must be offset by 4 bytes from the base to avoid null pointer dereference.
 */
static struct dma_memory *p_dma = reinterpret_cast<struct dma_memory *>(H3_SRAM_A1_BASE + 4);

/**
 * @brief Initializes the DMA memcpy operation with predefined configuration.
 *
 * Configures the DMA channel for SRAM memory-to-memory transfers with
 * specific parameters such as source and destination linear mode,
 * data width, and burst size.
 *
 * @note The configuration is stored in the `lli` structure.
 */
void memcpy32_init() {
	assert(p_dma != nullptr);

	p_dma->lli.cfg = DMA_CHAN_CFG_SRC_LINEAR_MODE | DMA_CHAN_CFG_SRC_DRQ(DRQSRC_SRAM) | DMA_CHAN_CFG_SRC_WIDTH(0x10) | DMA_CHAN_CFG_SRC_BURST(3)
				   | DMA_CHAN_CFG_DST_LINEAR_MODE | DMA_CHAN_CFG_DST_DRQ(DRQDST_SDRAM) | DMA_CHAN_CFG_DST_WIDTH(0x10) | DMA_CHAN_CFG_DST_BURST(3);
	p_dma->lli.para = DMA_NORMAL_WAIT;
	p_dma->lli.p_lli_next = DMA_LLI_LAST_ITEM;

#ifndef NDEBUG
	h3_dma_dump_lli(&p_dma->lli);
#endif

#if defined (CONFIG_DMA_MEMCPY_IRQ)
	H3_DMA->IRQ_PEND0 = DMA_IRQ_PEND0_DMA5_PKG_IRQ_EN;
	H3_DMA->IRQ_EN0 = DMA_IRQ_EN0_DMA5_PKG_IRQ_EN;

	IRQ_SetHandler(H3_DMA_IRQn, DMA_IRQHandler);
	gic_irq_config(H3_DMA_IRQn, GIC_CORE0);
	__enable_irq();
#endif
}

/**
 * @brief Performs a 32-bit aligned memory copy using DMA.
 *
 * Copies a block of memory from the source address to the destination address
 * using the DMA controller. The number of bytes copied must be a multiple of 4.
 *
 * @param pDestination Pointer to the destination memory address (must be 32-bit aligned).
 * @param pSource Pointer to the source memory address (must be 32-bit aligned).
 * @param nBytesCount Number of bytes to copy (must be a multiple of 4).
 *
 * @note The function performs checks to ensure that the addresses and size are 32-bit aligned.
 */
void memcpy32(const void *pDestination, const void *pSource, const uint32_t nBytesCount) {
	assert((reinterpret_cast<uint32_t>(pDestination) & 0x3) == 0);
	assert((reinterpret_cast<uint32_t>(pSource) & 0x3) == 0);
	assert((nBytesCount & 0x3) == 0);

	p_dma->lli.src = reinterpret_cast<uint32_t>(pSource);
	p_dma->lli.dst = reinterpret_cast<uint32_t>(pDestination);
	p_dma->lli.len = nBytesCount;
	__DMB();

	H3_DMA_CHL5->DESC_ADDR = reinterpret_cast<uint32_t>(&p_dma->lli);
	H3_DMA_CHL5->EN = DMA_CHAN_ENABLE_START;
}
}  // namespace dma
