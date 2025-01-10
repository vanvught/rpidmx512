/**
 * @file ws28xx.cpp
 *
 */
/* Copyright (C) 2017-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#if defined (DEBUG_PIXEL)
# undef NDEBUG
#endif

#if defined(__GNUC__) && !defined(__clang__)	
# pragma GCC push_options
# pragma GCC optimize ("O3")
# pragma GCC optimize ("-funroll-loops")
# pragma GCC optimize ("-fprefetch-loop-arrays")
#endif

#include <cstdint>
#include <cstring>
#include <cassert>

#include "ws28xx.h"
#include "pixelconfiguration.h"

#include "hal_spi.h"

#include "debug.h"

WS28xx::WS28xx()  {
	DEBUG_ENTRY

	assert(s_pThis == nullptr);
	s_pThis = this;

	auto& pixelConfiguration = PixelConfiguration::Get();

	pixelConfiguration.Validate();

	const auto nCount = pixelConfiguration.GetCount();

	m_nBufSize = nCount * pixelConfiguration.GetLedsPerPixel();

	if (pixelConfiguration.IsRTZProtocol()) {
		m_nBufSize *= 8;
		m_nBufSize += 1;
	}

	const auto type = pixelConfiguration.GetType();

	if ((type == pixel::Type::APA102) || (type == pixel::Type::SK9822) || (type == pixel::Type::P9813)) {
		m_nBufSize += nCount;
		m_nBufSize += 8;
	}

	SetupBuffers();

	FUNC_PREFIX(spi_begin());
	FUNC_PREFIX(spi_set_speed_hz(pixelConfiguration.GetClockSpeedHz()));

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

	DEBUG_PRINTF("m_nBufSize=%u, m_pBuffer=%p, m_pBlackoutBuffer=%p", m_nBufSize, m_pBuffer, m_pBlackoutBuffer);

	auto& pixelConfiguration = PixelConfiguration::Get();

	const auto type = pixelConfiguration.GetType();
	const auto nCount = pixelConfiguration.GetCount();

	if ((type == pixel::Type::APA102) || (type == pixel::Type::SK9822) || (type == pixel::Type::P9813)) {
		memset(m_pBuffer, 0, 4);

		for (uint32_t nPixelIndex = 0; nPixelIndex < nCount; nPixelIndex++) {
			SetPixel(nPixelIndex, 0, 0, 0);
		}

		if ((type == pixel::Type::APA102) || (type == pixel::Type::SK9822)) {
			memset(&m_pBuffer[m_nBufSize - 4], 0xFF, 4);
		} else {
			memset(&m_pBuffer[m_nBufSize - 4], 0, 4);
		}
	} else {
		m_pBuffer[0] = 0x00;
		memset(&m_pBuffer[1], type == pixel::Type::WS2801 ? 0 : pixelConfiguration.GetLowCode(), m_nBufSize);
	}

	memcpy(m_pBlackoutBuffer, m_pBuffer, m_nBufSize);

	DEBUG_EXIT
}

void WS28xx::Update() {
#if defined( USE_SPI_DMA )
	assert(!IsUpdating());
	FUNC_PREFIX(spi_dma_tx_start(m_pBuffer, m_nBufSize));
#else
	FUNC_PREFIX(spi_writenb(reinterpret_cast<char *>(m_pBuffer), m_nBufSize));
#endif
}

void WS28xx::Blackout() {
	DEBUG_ENTRY

#if defined( USE_SPI_DMA )
	// Can be called any time.
	do {
		asm volatile ("isb" ::: "memory");
	} while (FUNC_PREFIX(spi_dma_tx_is_active()));
#endif

	auto *pBuffer = m_pBuffer;
	m_pBuffer = m_pBlackoutBuffer;

	auto& pixelConfiguration = PixelConfiguration::Get();

	const auto type = pixelConfiguration.GetType();
	const auto nCount = pixelConfiguration.GetCount();

	if ((type == pixel::Type::APA102) || (type == pixel::Type::SK9822) || (type == pixel::Type::P9813)) {
		memset(m_pBuffer, 0, 4);

		for (uint32_t nPixelIndex = 0; nPixelIndex < nCount; nPixelIndex++) {
			SetPixel(nPixelIndex, 0, 0, 0);
		}

		if ((type == pixel::Type::APA102) || (type == pixel::Type::SK9822)) {
			memset(&m_pBuffer[m_nBufSize - 4], 0xFF, 4);
		} else {
			memset(&m_pBuffer[m_nBufSize - 4], 0, 4);
		}
	} else {
		m_pBuffer[0] = 0x00;
		memset(&m_pBuffer[1], type == pixel::Type::WS2801 ? 0 : pixelConfiguration.GetLowCode(), m_nBufSize);
	}

	Update();

#if defined( USE_SPI_DMA )
	// A blackout may not be interrupted.
	do {
		asm volatile ("isb" ::: "memory");
	} while (FUNC_PREFIX(spi_dma_tx_is_active()));
#endif

	m_pBuffer = pBuffer;

	DEBUG_EXIT
}

void WS28xx::FullOn() {
	DEBUG_ENTRY

#if defined( USE_SPI_DMA )
	// Can be called any time.
	do {
		asm volatile ("isb" ::: "memory");
	} while (FUNC_PREFIX(spi_dma_tx_is_active()));
#endif

	auto& pixelConfiguration = PixelConfiguration::Get();

	const auto type = pixelConfiguration.GetType();
	const auto nCount = pixelConfiguration.GetCount();

	if ((type == pixel::Type::APA102) || (type == pixel::Type::SK9822) || (type == pixel::Type::P9813)) {
		memset(m_pBuffer, 0xFF, 4);

		for (uint32_t nPixelIndex = 0; nPixelIndex < nCount; nPixelIndex++) {
			SetPixel(nPixelIndex, 0xFF, 0xFF, 0xFF);
		}

		if ((type == pixel::Type::APA102) || (type == pixel::Type::SK9822)) {
			memset(&m_pBuffer[m_nBufSize - 4], 0xFF, 4);
		} else {
			memset(&m_pBuffer[m_nBufSize - 4], 0, 4);
		}
	} else {
		m_pBuffer[0] = 0x00;
		memset(&m_pBuffer[1], type == pixel::Type::WS2801 ? 0xFF : pixelConfiguration.GetHighCode(), m_nBufSize);
	}

	Update();

#if defined( USE_SPI_DMA )
	// May not be interrupted.
	do {
		asm volatile ("isb" ::: "memory");
	} while (FUNC_PREFIX(spi_dma_tx_is_active()));
#endif

	DEBUG_EXIT
}
