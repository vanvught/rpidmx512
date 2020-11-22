/**
 * @file ws28xx.cpp
 *
 */
/* Copyright (C) 2017-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <cassert>

#include "ws28xx.h"

#include "rgbmapping.h"

#include "hal_spi.h"

#include "debug.h"

WS28xx::WS28xx(TWS28XXType Type, uint16_t nLedCount, TRGBMapping tRGBMapping, uint8_t nT0H, uint8_t nT1H, uint32_t nClockSpeed) :
	m_tLEDType(Type),
	m_nLedCount(nLedCount),
	m_tRGBMapping(tRGBMapping),
	m_nClockSpeedHz(nClockSpeed),
	m_nLowCode(nT0H),
	m_nHighCode(nT1H)
{
	assert(m_nLedCount != 0);

	if ((m_tLEDType == SK6812W) || (m_tLEDType == APA102)) {
		m_nBufSize = m_nLedCount * 4U;
	} else {
		m_nBufSize = m_nLedCount * 3U;
	}

	// TODO Update when new chip is added
	if (m_tLEDType == WS28XX_UNDEFINED || m_tLEDType == WS2811
			|| m_tLEDType == WS2812 || m_tLEDType == WS2812B
			|| m_tLEDType == WS2813 || m_tLEDType == WS2815
			|| m_tLEDType == SK6812 || m_tLEDType == SK6812W
			|| m_tLEDType == UCS1903 || m_tLEDType == UCS2903
			|| m_tLEDType == CS8812) {
		m_nBufSize *= 8;
		m_bIsRTZProtocol = true;
	}

	if ((m_tLEDType == APA102) || (m_tLEDType == P9813)) {
		m_nBufSize += 8;
	}

#ifdef H3
	if (m_bIsRTZProtocol) {
		h3_spi_set_ws28xx_mode(true);
	}
#endif

	if (m_bIsRTZProtocol) {
		DEBUG_PRINTF("m_tWS28xxType=%d (%s), m_nLedCount=%d, m_nBufSize=%d", m_tLEDType, WS28xx::GetLedTypeString(m_tLEDType), m_nLedCount, m_nBufSize);
		DEBUG_PRINTF("m_tRGBMapping=%d (%s), m_nLowCode=0x%X, m_nHighCode=0x%X", static_cast<int>(m_tRGBMapping), RGBMapping::ToString(m_tRGBMapping), static_cast<int>(m_nLowCode), static_cast<int>(m_nHighCode));

		if (m_tRGBMapping == RGB_MAPPING_UNDEFINED) {
			m_tRGBMapping = WS28xx::GetRgbMapping(m_tLEDType);
		}

		uint8_t nLowCode;
		uint8_t nHighCode;

		WS28xx::GetTxH(m_tLEDType, nLowCode, nHighCode);

		if (m_nLowCode == 0) {
			m_nLowCode = nLowCode;
		}

		if (m_nHighCode == 0) {
			m_nHighCode = nHighCode;
		}

		DEBUG_PRINTF("m_tWS28xxType=%d (%s), m_nLedCount=%d, m_nBufSize=%d", m_tLEDType, WS28xx::GetLedTypeString(m_tLEDType), m_nLedCount, m_nBufSize);
		DEBUG_PRINTF("m_tRGBMapping=%d (%s), m_nLowCode=0x%X, m_nHighCode=0x%X", static_cast<int>(m_tRGBMapping), RGBMapping::ToString(m_tRGBMapping), static_cast<int>(m_nLowCode), static_cast<int>(m_nHighCode));
	}

	FUNC_PREFIX (spi_begin());

	if (m_bIsRTZProtocol) {
		m_nClockSpeedHz = 6400000;	// 6.4MHz / 8 bits = 800Hz
	} else {
		if (m_tLEDType == P9813) {
			if (nClockSpeed == 0) {
				m_nClockSpeedHz = spi::speed::p9813::default_hz;
			} else if (nClockSpeed > spi::speed::p9813::max_hz) {
				m_nClockSpeedHz = spi::speed::p9813::max_hz;
			}
		} else {
			if (nClockSpeed == 0) {
				m_nClockSpeedHz = spi::speed::ws2801::default_hz;
			} else if (nClockSpeed > spi::speed::ws2801::max_hz) {
				m_nClockSpeedHz = spi::speed::ws2801::max_hz;
			}
		}
	}

	FUNC_PREFIX(spi_set_speed_hz(m_nClockSpeedHz));

	DEBUG_PRINTF("m_bIsRTZProtocol=%d, m_nClockSpeedHz=%d", static_cast<int>(m_bIsRTZProtocol), m_nClockSpeedHz);
}

WS28xx::~WS28xx() {
	if (m_pBlackoutBuffer != nullptr) {
		delete [] m_pBlackoutBuffer;
		m_pBlackoutBuffer = nullptr;
	}

	if (m_pBuffer != nullptr) {
		delete [] m_pBuffer;
		m_pBuffer = nullptr;
	}
}

bool WS28xx::Initialize() {
	assert(m_pBuffer == nullptr);
	m_pBuffer = new uint8_t[m_nBufSize];
	assert(m_pBuffer != nullptr);

	if ((m_tLEDType == APA102) || (m_tLEDType == P9813)) {
		memset(m_pBuffer, 0, 4);

		for (uint32_t i = 0; i < m_nLedCount; i++) {
			SetLED(i, 0, 0, 0);
		}

		if (m_tLEDType == APA102) {
			memset(&m_pBuffer[m_nBufSize - 4], 0xFF, 4);
		} else
			memset(&m_pBuffer[m_nBufSize - 4], 0, 4);
		}
	else {
		memset(m_pBuffer, m_tLEDType == WS2801 ? 0 : m_nLowCode, m_nBufSize);
	}

	assert(m_pBlackoutBuffer == nullptr);
	m_pBlackoutBuffer = new uint8_t[m_nBufSize];
	assert(m_pBlackoutBuffer != nullptr);

	memcpy(m_pBlackoutBuffer, m_pBuffer, m_nBufSize);

	Blackout();

	return true;
}

void WS28xx::Update() {
	assert (m_pBuffer != nullptr);
	FUNC_PREFIX(spi_writenb(reinterpret_cast<char *>(m_pBuffer), m_nBufSize));
}

void WS28xx::Blackout() {
	assert (m_pBlackoutBuffer != nullptr);
	FUNC_PREFIX(spi_writenb(reinterpret_cast<char *>(m_pBlackoutBuffer), m_nBufSize));
}
