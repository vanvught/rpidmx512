/**
 * @file ws28xxmulti.cpp
 *
 */
/* Copyright (C) 2019-2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <cstdint>
#include <cstring>
#include <cassert>

#include "ws28xxmulti.h"
#include "pixeltype.h"

#include "h3_spi.h"

#include "debug.h"

using namespace pixel;

void WS28xxMulti::SetupBuffers() {
	DEBUG_ENTRY

	uint32_t nSize;

	m_pBuffer = const_cast<uint8_t*>(h3_spi_dma_tx_prepare(&nSize));
	assert(m_pBuffer != nullptr);

	const uint32_t nSizeHalf = nSize / 2;
	assert(m_nBufSize <= nSizeHalf);

	m_pBlackoutBuffer = m_pBuffer + (nSizeHalf & static_cast<uint32_t>(~3));

	if ((m_Type == Type::APA102) || (m_Type == Type::SK9822) || (m_Type == Type::P9813)) {
		DEBUG_PUTS("SPI");

		for (uint32_t nPortIndex = 0; nPortIndex < 8; nPortIndex++) {
			SetPixel(nPortIndex, 0, 0, 0, 0, 0);

			for (uint32_t nPixelIndex = 1; nPixelIndex <= m_nCount; nPixelIndex++) {
				SetPixel(nPortIndex, nPixelIndex, 0, 0xE0, 0, 0);
			}

			if ((m_Type == Type::APA102) || (m_Type == Type::SK9822)) {
				SetPixel(nPortIndex, 1U + m_nCount, 0xFF, 0xFF, 0xFF, 0xFF);
			} else {
				SetPixel(nPortIndex, 1U + m_nCount, 0, 0, 0, 0);
			}
		}
		memcpy(m_pBlackoutBuffer, m_pBuffer, m_nBufSize);
	} else {
		memset(m_pBuffer, 0, m_nBufSize);
		memset(m_pBlackoutBuffer, 0, m_nBufSize);
	}

	DEBUG_PRINTF("nSize=%x, m_pBuffer=%p, m_pBlackoutBuffer=%p", nSize, m_pBuffer, m_pBlackoutBuffer);
	DEBUG_EXIT
}

uint8_t WS28xxMulti::ReverseBits(uint8_t nBits) {
	const uint32_t input = nBits;
	uint32_t output;
	asm("rbit %0, %1" : "=r"(output) : "r"(input));
	return static_cast<uint8_t>((output >> 24));
}

void WS28xxMulti::Update() {
	assert(m_pBuffer != nullptr);
	assert(!h3_spi_dma_tx_is_active());

	h3_spi_dma_tx_start(m_pBuffer, m_nBufSize);

}

void WS28xxMulti::Blackout() {
	DEBUG_ENTRY

	assert(m_pBlackoutBuffer != nullptr);
	assert(!h3_spi_dma_tx_is_active());

	h3_spi_dma_tx_start(m_pBlackoutBuffer, m_nBufSize);

	// A blackout may not be interrupted.
	do {
		asm volatile ("isb" ::: "memory");
	} while (h3_spi_dma_tx_is_active());

	DEBUG_EXIT
}
