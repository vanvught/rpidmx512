/**
 * @file ws28xxmulti.cpp
 *
 */
/* Copyright (C) 2019-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <cassert>

#include "ws28xxmulti.h"

#include "h3/ws28xxdma.h"

#include "debug.h"

uint8_t WS28xxMulti::ReverseBits(uint8_t nBits) {
	const uint32_t input = nBits;
	uint32_t output;
	asm("rbit %0, %1" : "=r"(output) : "r"(input));
	return static_cast<uint8_t>((output >> 24));
}

void WS28xxMulti::Update() {
	if (m_tBoard == WS28XXMULTI_BOARD_8X) {
		assert(m_pBuffer8x != nullptr);
		assert(!h3_spi_dma_tx_is_active());

		h3_spi_dma_tx_start(m_pBuffer8x, m_nBufSize);
	} else {
		assert(m_pBuffer4x != nullptr);
		Generate800kHz(m_pBuffer4x);
	}
}

void WS28xxMulti::Blackout() {
	DEBUG_ENTRY

	if (m_tBoard == WS28XXMULTI_BOARD_8X) {
		assert(m_pBlackoutBuffer8x != nullptr);
		assert(!h3_spi_dma_tx_is_active());

		h3_spi_dma_tx_start(m_pBlackoutBuffer8x, m_nBufSize);
	} else {
		Generate800kHz(m_pBlackoutBuffer4x);
	}

	DEBUG_EXIT
}
