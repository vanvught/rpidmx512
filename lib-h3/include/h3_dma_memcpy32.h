/**
 * @file h3_dma_memcpy32.h
 *
 * @brief DMA memory operations using the Sunxi DMA controller.
 *
 * This namespace provides functions to initialize and perform 32-bit aligned memory copy
 * using the DMA hardware on the Allwinner H3 platform.
 *
 */
/* Copyright (C) 2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef H3_DMA_MEMCPY32_H_
#define H3_DMA_MEMCPY32_H_

#include <cstdint>
#include <cassert>

#include "h3.h"

namespace dma {
/**
 * @brief Initializes the DMA memcpy operation with predefined configuration.
 *
 * Configures the DMA channel for memory-to-memory transfers with
 * specific parameters such as source and destination linear mode,
 * data width, and burst size.
 */
void memcpy32_init() ;

/**
 * @brief Performs a 32-bit aligned memory copy using DMA.
 *
 * Copies a block of memory from the source address to the destination address
 * using the DMA controller. The number of bytes copied must be a multiple of 4.
 *
 * @param pDestination Pointer to the destination memory address (must be 32-bit aligned).
 * @param pSource Pointer to the source memory address (must be 32-bit aligned).
 * @param nBytesCount Number of bytes to copy (must be a multiple of 4).
 */
void memcpy32(const void *pDestination, const void *pSource, const uint32_t nBytesCount);

/**
 * @brief Checks if the DMA memcpy operation is still active.
 *
 * This function verifies if there are bytes left to transfer in the
 * current DMA operation by checking the `BCNT_LEFT` register.
 *
 * @return `true` if the DMA operation is active (bytes remaining),
 *         `false` otherwise.
 *
 * @note The `__DMB` (Data Memory Barrier) ensures memory operations
 *       are completed before accessing `BCNT_LEFT`.
 */
inline bool memcpy32_is_active() {
	const auto nBytesCountLeft = H3_DMA_CHL5->BCNT_LEFT;
	__DMB();
	return nBytesCountLeft != 0;
}
}  // namespace dma

#endif /* H3_DMA_MEMCPY32_H_ */
