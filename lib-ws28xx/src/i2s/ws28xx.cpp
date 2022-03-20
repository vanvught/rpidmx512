/**
 * @file ws28xx.cpp
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

#include <cstdint>
#include <cstring>
#include <cassert>

#include "ws28xx.h"
#include "pixelconfiguration.h"

#include "hal_spi.h"

#include "debug.h"

using namespace pixel;

static uint32_t s_tmp;

WS28xx *WS28xx::s_pThis;

WS28xx::WS28xx(PixelConfiguration& pixelConfiguration) {
	DEBUG_ENTRY

	assert(s_pThis == nullptr);
	s_pThis = this;

	uint32_t nLedsPerPixel;
	pixelConfiguration.Validate(nLedsPerPixel);
	pixelConfiguration.Dump();

	m_Type = pixelConfiguration.GetType();
	m_nCount = pixelConfiguration.GetCount();
	m_Map = pixelConfiguration.GetMap();
	m_nLowCode = pixelConfiguration.GetLowCode();
	m_nHighCode = pixelConfiguration.GetHighCode();
	m_bIsRTZProtocol = pixelConfiguration.IsRTZProtocol();
	m_nGlobalBrightness = pixelConfiguration.GetGlobalBrightness();
	m_nBufSize = m_nCount * nLedsPerPixel;

	if (m_bIsRTZProtocol) {
		m_nBufSize *= 8;
		m_nBufSize += 1;
	}

	if ((m_Type == Type::APA102) || (m_Type == Type::SK9822) || (m_Type == Type::P9813)) {
		m_nBufSize += m_nCount;
		m_nBufSize += 8;
	}

	SetupBuffers();

	FUNC_PREFIX(spi_dma_begin());
	FUNC_PREFIX(spi_dma_set_speed_hz(pixelConfiguration.GetClockSpeedHz()));

	DEBUG_EXIT
}

WS28xx::~WS28xx() {
	m_pBlackoutBuffer = nullptr;
	m_pBuffer = nullptr;
	s_pThis = nullptr;
}

void WS28xx::SetupBuffers() {
	DEBUG_ENTRY

	uint32_t nSize;

	m_pBuffer = const_cast<uint8_t*>(FUNC_PREFIX (spi_dma_tx_prepare(&nSize)));
	assert(m_pBuffer != nullptr);

	const auto nSizeHalf = nSize / 2;
	assert(m_nBufSize <= nSizeHalf);

	m_pBlackoutBuffer = m_pBuffer + (nSizeHalf & static_cast<uint32_t>(~3));

	DEBUG_PRINTF("m_nBufSize=%u, m_pBuffer=%p, m_pBlackoutBuffer=%p", m_nBufSize, m_pBuffer, m_pBlackoutBuffer);

	s_tmp = m_nBufSize;
	m_nBufSize = (m_nBufSize + 3) & static_cast<uint32_t>(~3);

	DEBUG_PRINTF("m_nBufSize=%u -> %d", m_nBufSize, m_nBufSize - s_tmp);
	DEBUG_EXIT
}

void WS28xx::Update() {
	assert(!IsUpdating());

	for (auto i = s_tmp; i < m_nBufSize; i++) {
		m_pBuffer[i] = 0x00;
	}

	const auto *pSrc = reinterpret_cast<uint16_t *>(m_pBuffer);
	auto *pDst = reinterpret_cast<uint16_t *>(m_pBlackoutBuffer);

	for (auto i = 0; i < m_nBufSize / 2; i++) {
		pDst[i] = __builtin_bswap16(pSrc[i]);
	}

	FUNC_PREFIX(spi_dma_tx_start(m_pBlackoutBuffer, m_nBufSize));
}

void WS28xx::Blackout() {
	DEBUG_ENTRY

	auto *pBuffer = m_pBuffer;
	m_pBuffer = m_pBlackoutBuffer;

	// A blackout can be called any time. Make sure the previous transmit is ended.

	do {
		__ISB();
	} while (FUNC_PREFIX(spi_dma_tx_is_active()));

	if ((m_Type == Type::APA102) || (m_Type == Type::SK9822) || (m_Type == Type::P9813)) {
		memset(m_pBuffer, 0, 4);

		for (auto i = 0; i < m_nCount; i++) {
			SetPixel(i, 0, 0, 0);
		}

		if ((m_Type == Type::APA102) || (m_Type == Type::SK9822)) {
			memset(&m_pBuffer[m_nBufSize - 4], 0xFF, 4);
		} else {
			memset(&m_pBuffer[m_nBufSize - 4], 0, 4);
		}
	} else {
		m_pBuffer[0] = 0x00;
		memset(&m_pBuffer[1], m_Type == Type::WS2801 ? 0 : m_nLowCode, m_nBufSize);
	}

	Update();

	// A blackout may not be interrupted.
	do {
		__ISB();
	} while (FUNC_PREFIX(spi_dma_tx_is_active()));

	m_pBuffer = pBuffer;

	DEBUG_EXIT
}
