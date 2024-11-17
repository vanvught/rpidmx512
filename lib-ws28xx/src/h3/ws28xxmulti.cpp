/**
 * @file ws28xxmulti.cpp
 *
 */
/* Copyright (C) 2019-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#if !defined(__clang__)	// Needed for compiling on MacOS
# pragma GCC push_options
# pragma GCC optimize ("O3")
# pragma GCC optimize ("-funroll-loops")
# pragma GCC optimize ("-fprefetch-loop-arrays")
#endif

#include <cstdint>
#include <cstring>
#include <cstdio>
#include <cassert>

#include "ws28xxmulti.h"
#include "pixelconfiguration.h"
#include "pixeltype.h"

#include "hal_gpio.h"
#include "hal_spi.h"

#include "jamstapl.h"

#include "debug.h"

using namespace pixel;

WS28xxMulti::WS28xxMulti() {
	DEBUG_ENTRY

	assert(s_pThis == nullptr);
	s_pThis = this;

	auto& pixelConfiguration = PixelConfiguration::Get();

	pixelConfiguration.Validate();

	const auto nCount = pixelConfiguration.GetCount();
	m_nBufSize = nCount * pixelConfiguration.GetLedsPerPixel();

	const auto type = pixelConfiguration.GetType();

	if ((type == Type::APA102) || (type == Type::SK9822) || (type == Type::P9813)) {
		m_nBufSize += nCount;
		m_nBufSize += 8;
	}

	m_nBufSize *= 8;

	DEBUG_PRINTF("m_nBufSize=%d", m_nBufSize);

	const auto nLowCode = pixelConfiguration.GetLowCode();
	const auto nHighCode = pixelConfiguration.GetHighCode();

	m_hasCPLD = SetupCPLD();

	SetupHC595(ReverseBits(nLowCode), ReverseBits(nHighCode));

	if (pixelConfiguration.IsRTZProtocol()) {
		SetupSPI(pixelConfiguration.GetClockSpeedHz());
	} else {
		if (m_hasCPLD) {
			SetupSPI(pixelConfiguration.GetClockSpeedHz() * 6);
		} else {
			SetupSPI(pixelConfiguration.GetClockSpeedHz() * 4);
		}
	}

	m_nBufSize++;

	SetupBuffers();

	printf("Board: %s\n", m_hasCPLD ? "CPLD" : "74-logic");
}

WS28xxMulti::~WS28xxMulti() {
	m_pBuffer1 = nullptr;
	m_pBuffer2 = nullptr;

	s_pThis = nullptr;
}

void WS28xxMulti::SetupBuffers() {
	DEBUG_ENTRY

	uint32_t nSize;

	m_pBuffer1 = const_cast<uint8_t*>(FUNC_PREFIX(spi_dma_tx_prepare(&nSize)));
	assert(m_pBuffer1 != nullptr);

	memset(m_pBuffer1, 0, nSize);

	const auto nSizeHalf = nSize / 2;
	assert(m_nBufSize <= nSizeHalf);

	m_pBuffer2 = m_pBuffer1 + (nSizeHalf & static_cast<uint32_t>(~3));

	DEBUG_PRINTF("nSize=%x, m_pBuffer1=%p, m_pBuffer2=%p", nSize, m_pBuffer1, m_pBuffer2);
	DEBUG_EXIT
}

#define SPI_CS1		GPIO_EXT_26

void WS28xxMulti::SetupHC595(uint8_t nT0H, uint8_t nT1H) {
	DEBUG_ENTRY

	nT0H = static_cast<uint8_t>(nT0H << 1);
	nT1H = static_cast<uint8_t>(nT1H << 1);

	DEBUG_PRINTF("nT0H=%.2x nT1H=%.2x", nT0H, nT1H);

	FUNC_PREFIX(gpio_fsel(SPI_CS1, GPIO_FSEL_OUTPUT));
	FUNC_PREFIX(gpio_set(SPI_CS1));

	FUNC_PREFIX(spi_begin());
	FUNC_PREFIX(spi_chipSelect(SPI_CS_NONE));
	FUNC_PREFIX(spi_set_speed_hz(1000000));

	FUNC_PREFIX(gpio_clr(SPI_CS1));
	FUNC_PREFIX(spi_write(static_cast<uint16_t>((nT1H << 8) | nT0H)));
	FUNC_PREFIX(gpio_set(SPI_CS1));

	DEBUG_EXIT
}

void WS28xxMulti::SetupSPI(uint32_t nSpeedHz) {
	DEBUG_ENTRY

	FUNC_PREFIX(spi_begin());
	FUNC_PREFIX(spi_chipSelect(SPI_CS0));
	FUNC_PREFIX(spi_set_speed_hz(nSpeedHz));

	DEBUG_PRINTF("nSpeedHz=%u", nSpeedHz);
	DEBUG_EXIT
}

extern uint32_t PIXEL8X4_PROGRAM;

extern "C" {
uint32_t getPIXEL8X4_SIZE();
}

bool WS28xxMulti::SetupCPLD() {
	DEBUG_ENTRY

	JamSTAPL jbc(reinterpret_cast<uint8_t*>(&PIXEL8X4_PROGRAM), getPIXEL8X4_SIZE(), true);
	jbc.SetJamSTAPLDisplay(m_pJamSTAPLDisplay);

	if (jbc.PrintInfo() == JBIC_SUCCESS) {
		if ((jbc.CheckCRC() == JBIC_SUCCESS) && (jbc.GetCRC() == 0x1D3C)) {
			jbc.CheckIdCode();
			if (jbc.GetExitCode() == 0) {
				jbc.ReadUsercode();
				if ((jbc.GetExitCode() == 0) && (jbc.GetExportIntegerInt() != 0x0018ad81)) {
					jbc.Program();
				}
				DEBUG_EXIT
				return true;
			}
		}
	}

	DEBUG_EXIT
	return false;
}

uint8_t WS28xxMulti::ReverseBits(uint8_t nBits) {
	const uint32_t input = nBits;
	uint32_t output;
	asm("rbit %0, %1" : "=r"(output) : "r"(input));
	return static_cast<uint8_t>((output >> 24));
}

void WS28xxMulti::Update() {
	assert(!FUNC_PREFIX(spi_dma_tx_is_active()));

	do { // https://github.com/vanvught/rpidmx512/issues/281
		asm volatile ("isb" ::: "memory");
	} while (FUNC_PREFIX(spi_dma_tx_is_active()));

	auto *pTmp = m_pBuffer1;
	m_pBuffer1 = m_pBuffer2;
	m_pBuffer2 = pTmp;
	__DMB();

	FUNC_PREFIX(spi_dma_tx_start(m_pBuffer2, m_nBufSize));
}

void WS28xxMulti::Blackout() {
	DEBUG_ENTRY

	auto& pixelConfiguration = PixelConfiguration::Get();

	const auto type = pixelConfiguration.GetType();
	const auto nCount = pixelConfiguration.GetCount();

	if ((type == Type::APA102) || (type == Type::SK9822) || (type == Type::P9813)) {
		for (uint32_t nPortIndex = 0; nPortIndex < 8; nPortIndex++) {
			SetPixel4Bytes(nPortIndex, 0, 0, 0, 0, 0);

			for (uint32_t nPixelIndex = 1; nPixelIndex <= nCount; nPixelIndex++) {
				SetPixel4Bytes(nPortIndex, nPixelIndex, 0, 0xE0, 0, 0);
			}

			if ((type == Type::APA102) || (type == Type::SK9822)) {
				SetPixel4Bytes(nPortIndex, 1U + nCount, 0xFF, 0xFF, 0xFF, 0xFF);
			} else {
				SetPixel4Bytes(nPortIndex, 1U + nCount, 0, 0, 0, 0);
			}
		}
	} else {
		memset(m_pBuffer1, 0, m_nBufSize);
	}

	// Can be called any time.
	do {
		asm volatile ("isb" ::: "memory");
	} while (FUNC_PREFIX(spi_dma_tx_is_active()));

	Update();

	// May not be interrupted.
	do {
		asm volatile ("isb" ::: "memory");
	} while (FUNC_PREFIX(spi_dma_tx_is_active()));

	DEBUG_EXIT
}

void WS28xxMulti::FullOn() {
	DEBUG_ENTRY

	auto& pixelConfiguration = PixelConfiguration::Get();

	const auto type = pixelConfiguration.GetType();
	const auto nCount = pixelConfiguration.GetCount();

	if ((type == Type::APA102) || (type == Type::SK9822) || (type == Type::P9813)) {
		for (uint32_t nPortIndex = 0; nPortIndex < 8; nPortIndex++) {
			SetPixel4Bytes(nPortIndex, 0, 0, 0, 0, 0);

			for (uint32_t nPixelIndex = 1; nPixelIndex <= nCount; nPixelIndex++) {
				SetPixel4Bytes(nPortIndex, nPixelIndex, 0xFF, 0xE0, 0xFF, 0xFF);
			}

			if ((type == Type::APA102) || (type == Type::SK9822)) {
				SetPixel4Bytes(nPortIndex, 1U + nCount, 0xFF, 0xFF, 0xFF, 0xFF);
			} else {
				SetPixel4Bytes(nPortIndex, 1U + nCount, 0, 0, 0, 0);
			}
		}
	} else {
		memset(m_pBuffer1, 0xFF, m_nBufSize);
	}

	// Can be called any time.
	do {
		asm volatile ("isb" ::: "memory");
	} while (FUNC_PREFIX(spi_dma_tx_is_active()));

	Update();

	// May not be interrupted.
	do {
		asm volatile ("isb" ::: "memory");
	} while (FUNC_PREFIX(spi_dma_tx_is_active()));

	DEBUG_EXIT
}
