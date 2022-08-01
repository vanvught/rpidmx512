/**
 * @file pixelconfiguration.cpp
 *
 */
/* Copyright (C) 2021-2022 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#if !defined(__clang__)	// Needed for compiling on MacOS
# pragma GCC push_options
# pragma GCC optimize ("Os")
#endif

#include <cstdint>

#include "pixelconfiguration.h"
#include "pixeltype.h"
#include "gamma/gamma_tables.h"

#include "debug.h"

using namespace pixel;

void PixelConfiguration::Validate(uint32_t& nLedsPerPixel) {
	DEBUG_ENTRY

	if (m_type == Type::SK6812W) {
		m_nCount = m_nCount <= static_cast<uint16_t>(max::ledcount::RGBW) ? m_nCount : static_cast<uint16_t>(max::ledcount::RGBW);
		nLedsPerPixel = 4;
	} else {
		m_nCount = m_nCount <= static_cast<uint16_t>(max::ledcount::RGB) ? m_nCount : static_cast<uint16_t>(max::ledcount::RGB);
		nLedsPerPixel = 3;
	}

	if ((m_type == Type::APA102) || (m_type == Type::SK9822)){
		if (m_nGlobalBrightness > 0x1F) {
			m_nGlobalBrightness = 0xFF;
		} else {
			m_nGlobalBrightness = 0xE0 | (m_nGlobalBrightness & 0x1F);
		}
	}

	if ((m_type == Type::WS2801) || (m_type == Type::APA102) || (m_type == Type::SK9822) || (m_type == Type::P9813)) {
		m_bIsRTZProtocol = false;

		if (m_map == Map::UNDEFINED) {
			m_map = Map::RGB;
		}

		if (m_type == Type::P9813) {
			if (m_nClockSpeedHz == 0) {
				m_nClockSpeedHz = spi::speed::p9813::default_hz;
			} else if (m_nClockSpeedHz > spi::speed::p9813::max_hz) {
				m_nClockSpeedHz = spi::speed::p9813::max_hz;
			}
		} else {
			if (m_nClockSpeedHz == 0) {
				m_nClockSpeedHz = spi::speed::ws2801::default_hz;
			} else if (m_nClockSpeedHz > spi::speed::ws2801::max_hz) {
				m_nClockSpeedHz = spi::speed::ws2801::max_hz;
			}
		}
	} else {
		m_bIsRTZProtocol = true;

		if (m_map == Map::UNDEFINED) {
			m_map = PixelType::GetMap(m_type);
		}

		if (m_nLowCode >= m_nHighCode) {
			m_nLowCode = 0;
			m_nHighCode = 0;
		}

		uint8_t nLowCode, nHighCode;

		GetTxH(m_type, nLowCode, nHighCode);

		if (m_nLowCode == 0) {
			m_nLowCode = nLowCode;
		}

		if (m_nHighCode == 0) {
			m_nHighCode = nHighCode;
		}

		m_nClockSpeedHz = 6400000;	// 6.4MHz / 8 bits = 800Hz
	}

	if (m_bEnableGammaCorrection) {
		if (m_nGammaValue == 0) {
			m_pGammaTable = gamma::get_table_default(m_type);
		} else {
			m_pGammaTable = gamma::get_table(m_nGammaValue);
		}
	} else {
		m_pGammaTable = gamma10_0;
	}

	DEBUG_EXIT
}

void PixelConfiguration::GetTxH(Type type, uint8_t &nLowCode, uint8_t &nHighCode) {
	nLowCode = 0xC0;
	nHighCode = (type == Type::WS2812B ? 0xF8 :
		(((type == Type::UCS1903) || (type == Type::UCS2903) || (type == Type::CS8812)) ? 0xFC : 0xF0));
}

#include <cstdio>

void PixelConfiguration::Print() {
	printf("Pixel configuration\n");
	printf(" Type    : %s [%d]\n", PixelType::GetType(m_type), static_cast<int>(m_type));
	printf(" Count   : %d\n", m_nCount);

	if (m_bIsRTZProtocol) {
		printf(" Mapping : %s [%d]\n", PixelType::GetMap(m_map), static_cast<int>(m_map));
		printf(" T0H     : %.2f [0x%X]\n", PixelType::ConvertTxH(m_nLowCode), m_nLowCode);
		printf(" T1H     : %.2f [0x%X]\n", PixelType::ConvertTxH(m_nHighCode), m_nHighCode);
	} else {
		if ((m_type == Type::APA102) || (m_type == Type::SK9822)){
			printf(" GlobalBrightness: %u\n", m_nGlobalBrightness);
		}
	}

	printf(" Gamma correction %s\n", m_bEnableGammaCorrection ? "Yes" :  "No");
	printf(" Clock: %u Hz\n", m_nClockSpeedHz);
}
