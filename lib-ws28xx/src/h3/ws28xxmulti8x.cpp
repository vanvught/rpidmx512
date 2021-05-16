/**
 * @file ws28xxmulti8x.cpp
 *
 */
/* Copyright (C) 2020-2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#pragma GCC optimize ("Os")

#include <cstdint>
#include <cstring>
#include <cassert>

#include "ws28xxmulti.h"
#include "pixeltype.h"

#include "h3_spi.h"

#include "debug.h"

using namespace pixel;

void WS28xxMulti::SetupBuffers8x() {
	DEBUG_ENTRY

	uint32_t nSize;

	m_pBuffer8x = const_cast<uint8_t*>(h3_spi_dma_tx_prepare(&nSize));
	assert(m_pBuffer8x != nullptr);

	const uint32_t nSizeHalf = nSize / 2;
	assert(m_nBufSize <= nSizeHalf);

	m_pBlackoutBuffer8x = m_pBuffer8x + (nSizeHalf & static_cast<uint32_t>(~3));

	if ((m_Type == Type::APA102) || (m_Type == Type::SK9822) || (m_Type == Type::P9813)) {
		DEBUG_PUTS("SPI");

		for (uint32_t nPortIndex = 0; nPortIndex < 8; nPortIndex++) {
			SetPixel8x(nPortIndex, 0, 0, 0, 0, 0);

			for (uint32_t nPixelIndex = 1; nPixelIndex <= m_nCount; nPixelIndex++) {
				SetPixel8x(nPortIndex, nPixelIndex, 0, 0xE0, 0, 0);
			}

			if ((m_Type == Type::APA102) || (m_Type == Type::SK9822)) {
				SetPixel8x(nPortIndex, 1 + m_nCount, 0xFF, 0xFF, 0xFF, 0xFF);
			} else {
				SetPixel8x(nPortIndex, 1 + m_nCount, 0, 0, 0, 0);
			}
		}
		memcpy(m_pBlackoutBuffer8x, m_pBuffer8x, m_nBufSize);
	} else {
		memset(m_pBuffer8x, 0, m_nBufSize);
		memset(m_pBlackoutBuffer8x, 0, m_nBufSize);
	}

	DEBUG_PRINTF("nSize=%x, m_pBuffer=%p, m_pBlackoutBuffer=%p", nSize, m_pBuffer8x, m_pBlackoutBuffer8x);
	DEBUG_EXIT
}
