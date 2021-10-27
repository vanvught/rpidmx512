/**
 * @file ws28xx.cpp
 *
 */
/* Copyright (C) 2017-2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <cstdio>
#include <cassert>

#include "ws28xx.h"

#include "pixelconfiguration.h"

#include "hal_spi.h"

#include "debug.h"

using namespace pixel;

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

#if defined( USE_SPI_DMA )
	FUNC_PREFIX(spi_dma_begin());
	FUNC_PREFIX(spi_dma_set_speed_hz(pixelConfiguration.GetClockSpeedHz()));
#else
	FUNC_PREFIX(spi_begin());
	FUNC_PREFIX(spi_set_speed_hz(pixelConfiguration.GetClockSpeedHz()));
#endif

	DEBUG_EXIT
}

WS28xx::~WS28xx() {
#if defined( USE_SPI_DMA )
	m_pBlackoutBuffer = nullptr;
	m_pBuffer = nullptr;
#else
	if (m_pBlackoutBuffer != nullptr) {
		delete [] m_pBlackoutBuffer;
		m_pBlackoutBuffer = nullptr;
	}

	if (m_pBuffer != nullptr) {
		delete [] m_pBuffer;
		m_pBuffer = nullptr;
	}
#endif

	s_pThis = nullptr;
}

void WS28xx::SetupBuffers() {
	DEBUG_ENTRY
#if defined( USE_SPI_DMA )
	uint32_t nSize;

	m_pBuffer = const_cast<uint8_t*>(FUNC_PREFIX (spi_dma_tx_prepare(&nSize)));
	assert(m_pBuffer != nullptr);

	const auto nSizeHalf = nSize / 2;
	assert(m_nBufSize <= nSizeHalf);

	m_pBlackoutBuffer = m_pBuffer + (nSizeHalf & static_cast<uint32_t>(~3));
#else
	assert(m_pBuffer == nullptr);
	m_pBuffer = new uint8_t[m_nBufSize];
	assert(m_pBuffer != nullptr);

	assert(m_pBlackoutBuffer == nullptr);
	m_pBlackoutBuffer = new uint8_t[m_nBufSize];
	assert(m_pBlackoutBuffer != nullptr);
#endif

	if ((m_Type == Type::APA102) || (m_Type == Type::SK9822) || (m_Type == Type::P9813)) {
		memset(m_pBuffer, 0, 4);

		for (uint32_t i = 0; i < m_nCount; i++) {
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

#if defined ( GD32 )
	auto tmp = m_nBufSize;
	m_nBufSize = (m_nBufSize + 3) & ~3;

	for (auto i = tmp; i < m_nBufSize; i++) {
		m_pBuffer[i] = 0x00;
	}

	DEBUG_PRINTF("m_nBufSize=%u -> %d", m_nBufSize, m_nBufSize - tmp);
#endif

	memcpy(m_pBlackoutBuffer, m_pBuffer, m_nBufSize);

	DEBUG_EXIT
}

void WS28xx::Update() {
	assert (m_pBuffer != nullptr);
#if defined( USE_SPI_DMA )
	assert(!IsUpdating());

	FUNC_PREFIX(spi_dma_tx_start(m_pBuffer, m_nBufSize));
#else
	FUNC_PREFIX(spi_writenb(reinterpret_cast<char *>(m_pBuffer), m_nBufSize));
#endif
}

void WS28xx::Blackout() {
	assert (m_pBlackoutBuffer != nullptr);
#if defined( USE_SPI_DMA )
	assert(!IsUpdating());

	FUNC_PREFIX(spi_dma_tx_start(m_pBlackoutBuffer, m_nBufSize));

	// A blackout may not be interrupted.
	do {
		asm volatile ("isb" ::: "memory");
	} while (FUNC_PREFIX(spi_dma_tx_is_active()));
#else
	FUNC_PREFIX(spi_writenb(reinterpret_cast<char *>(m_pBlackoutBuffer), m_nBufSize));
#endif
}

void WS28xx::Print() {
	printf("Pixel parameters\n");
	printf(" Type    : %s [%d]\n", PixelType::GetType(m_Type), static_cast<int>(m_Type));
	printf(" Count   : %d\n", m_nCount);
	if (m_bIsRTZProtocol) {
		printf(" Mapping : %s [%d]\n", PixelType::GetMap(m_Map), static_cast<int>(m_Map));
		printf(" T0H     : %.2f [0x%X]\n", PixelType::ConvertTxH(m_nLowCode), m_nLowCode);
		printf(" T1H     : %.2f [0x%X]\n", PixelType::ConvertTxH(m_nHighCode), m_nHighCode);
	} else {

	}
}

void WS28xx::SetPixel(uint32_t nPixelIndex, uint8_t nRed, uint8_t nGreen, uint8_t nBlue) {
	assert(m_pBuffer != nullptr);
	assert(nPixelIndex < m_nCount);

	if (m_bIsRTZProtocol) {
		auto nOffset = nPixelIndex * 3U;
		nOffset *= 8U;

		switch (m_Map) {
		case Map::RGB:
			SetColorWS28xx(nOffset, nRed);
			SetColorWS28xx(nOffset + 8, nGreen);
			SetColorWS28xx(nOffset + 16, nBlue);
			break;
		case Map::RBG:
			SetColorWS28xx(nOffset, nRed);
			SetColorWS28xx(nOffset + 8, nBlue);
			SetColorWS28xx(nOffset + 16, nGreen);
			break;
		case Map::GRB:
			SetColorWS28xx(nOffset, nGreen);
			SetColorWS28xx(nOffset + 8, nRed);
			SetColorWS28xx(nOffset + 16, nBlue);
			break;
		case Map::GBR:
			SetColorWS28xx(nOffset, nGreen);
			SetColorWS28xx(nOffset + 8, nBlue);
			SetColorWS28xx(nOffset + 16, nRed);
			break;
		case Map::BRG:
			SetColorWS28xx(nOffset, nBlue);
			SetColorWS28xx(nOffset + 8, nRed);
			SetColorWS28xx(nOffset + 16, nGreen);
			break;
		case Map::BGR:
			SetColorWS28xx(nOffset, nBlue);
			SetColorWS28xx(nOffset + 8, nGreen);
			SetColorWS28xx(nOffset + 16, nRed);
			break;
		default:  // RGB
			SetColorWS28xx(nOffset, nRed);
			SetColorWS28xx(nOffset + 8, nGreen);
			SetColorWS28xx(nOffset + 16, nBlue);
			break;
		}

		return;
	}

	if ((m_Type == Type::APA102) || (m_Type == Type::SK9822)) {
		auto nOffset = 4U + (nPixelIndex * 4U);
		assert(nOffset + 3U < m_nBufSize);

		m_pBuffer[nOffset] = m_nGlobalBrightness;
		m_pBuffer[nOffset + 1] = nRed;
		m_pBuffer[nOffset + 2] = nGreen;
		m_pBuffer[nOffset + 3] = nBlue;

		return;
	}

	if (m_Type == Type::WS2801) {
		auto nOffset = nPixelIndex * 3U;
		assert(nOffset + 2U < m_nBufSize);

		m_pBuffer[nOffset] = nRed;
		m_pBuffer[nOffset + 1] = nGreen;
		m_pBuffer[nOffset + 2] = nBlue;

		return;
	}

	if (m_Type == Type::P9813) {
		auto nOffset = 4U + (nPixelIndex * 4U);
		assert(nOffset + 3 < m_nBufSize);

		const auto nFlag = static_cast<uint8_t>(0xC0 | ((~nBlue & 0xC0) >> 2) | ((~nGreen & 0xC0) >> 4) | ((~nRed & 0xC0) >> 6));

		m_pBuffer[nOffset] = nFlag;
		m_pBuffer[nOffset + 1] = nBlue;
		m_pBuffer[nOffset + 2] = nGreen;
		m_pBuffer[nOffset + 3] = nRed;

		return;
	}

	assert(0);
	__builtin_unreachable();
}

void WS28xx::SetPixel(uint32_t nPixelIndex, uint8_t nRed, uint8_t nGreen, uint8_t nBlue, uint8_t nWhite) {
	assert(nPixelIndex < m_nCount);
	assert(m_Type == Type::SK6812W);

	auto nOffset = nPixelIndex * 4U;

	if (m_Type == Type::SK6812W) {
		nOffset *= 8;

		SetColorWS28xx(nOffset, nGreen);
		SetColorWS28xx(nOffset + 8, nRed);
		SetColorWS28xx(nOffset + 16, nBlue);
		SetColorWS28xx(nOffset + 24, nWhite);
	}
}

void WS28xx::SetColorWS28xx(uint32_t nOffset, uint8_t nValue) {
	assert(m_pBuffer != nullptr);
	assert(m_Type != Type::WS2801);
	assert(nOffset + 7 < m_nBufSize);

	nOffset += 1;

	for (uint8_t mask = 0x80; mask != 0; mask = static_cast<uint8_t>(mask >> 1)) {
		if (nValue & mask) {
			m_pBuffer[nOffset] = m_nHighCode;
		} else {
			m_pBuffer[nOffset] = m_nLowCode;
		}
		nOffset++;
	}
}
