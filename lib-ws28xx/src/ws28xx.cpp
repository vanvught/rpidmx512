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

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <cassert>

#include "ws28xx.h"

#include "pixelconfiguration.h"

#include "hal_spi.h"

#include "debug.h"

using namespace pixel;

WS28xx *WS28xx::s_pThis = nullptr;

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

	if ((m_Type == Type::SK6812W) || (m_Type == Type::APA102)) {
		m_nBufSize = m_nCount * 4U;
	} else {
		m_nBufSize = m_nCount * 3U;
	}

	if (m_bIsRTZProtocol) {
		m_nBufSize *= 8;
#if defined( H3 )
		h3_spi_set_ws28xx_mode(true);
#endif
	}

	if ((m_Type == Type::APA102) || (m_Type == Type::P9813)) {
		m_nBufSize += 8;
	}

	FUNC_PREFIX (spi_begin());
	FUNC_PREFIX(spi_set_speed_hz(pixelConfiguration.GetClockSpeedHz()));

	SetupBuffers();

	DEBUG_EXIT
}

WS28xx::~WS28xx() {
#if defined( H3 )
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
#if defined( H3 )
	uint32_t nSize;

	m_pBuffer = const_cast<uint8_t*>(h3_spi_dma_tx_prepare(&nSize));
	assert(m_pBuffer != nullptr);

	const uint32_t nSizeHalf = nSize / 2;
	assert(m_nBufSize <= nSizeHalf);

	m_pBlackoutBuffer = m_pBuffer + (nSizeHalf & static_cast<uint32_t>(~3));

	if (m_Type == Type::APA102) {
		memset(m_pBuffer, 0, 4);
		for (uint32_t i = 0; i < m_nCount; i++) {
			SetPixel(i, 0, 0, 0);
		}
		memset(&m_pBuffer[m_nBufSize - 4], 0xFF, 4);
	} else {
		memset(m_pBuffer, m_Type == Type::WS2801 ? 0 : m_nLowCode, m_nBufSize);
	}
#else
	assert(m_pBuffer == nullptr);
	m_pBuffer = new uint8_t[m_nBufSize];
	assert(m_pBuffer != nullptr);

	if ((m_Type == Type::APA102) || (m_Type == Type::P9813)) {
		memset(m_pBuffer, 0, 4);

		for (uint32_t i = 0; i < m_nCount; i++) {
			SetPixel(i, 0, 0, 0);
		}

		if (m_Type == Type::APA102) {
			memset(&m_pBuffer[m_nBufSize - 4], 0xFF, 4);
		} else
			memset(&m_pBuffer[m_nBufSize - 4], 0, 4);
		}
	else {
		memset(m_pBuffer, m_Type == Type::WS2801 ? 0 : m_nLowCode, m_nBufSize);
	}

	assert(m_pBlackoutBuffer == nullptr);
	m_pBlackoutBuffer = new uint8_t[m_nBufSize];
	assert(m_pBlackoutBuffer != nullptr);
#endif

	memcpy(m_pBlackoutBuffer, m_pBuffer, m_nBufSize);
	DEBUG_EXIT
}

void WS28xx::Update() {
	assert (m_pBuffer != nullptr);
#if defined( H3 )
	assert(!IsUpdating());

	h3_spi_dma_tx_start(m_pBuffer, m_nBufSize);
#else
	FUNC_PREFIX(spi_writenb(reinterpret_cast<char *>(m_pBuffer), m_nBufSize));
#endif
}

void WS28xx::Blackout() {
	assert (m_pBlackoutBuffer != nullptr);
#if defined( H3 )
	assert(!IsUpdating());

	h3_spi_dma_tx_start(m_pBlackoutBuffer, m_nBufSize);
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
