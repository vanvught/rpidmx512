/**
 * @file pixelconfiguration.cpp
 *
 */
/* Copyright (C) 2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "debug.h"

using namespace pixel;

void PixelConfiguration::Validate(uint32_t& nLedsPerPixel) {
	DEBUG_ENTRY

	if (m_Type == Type::SK6812W) {
		m_nCount = m_nCount <= static_cast<uint16_t>(max::ledcount::RGBW) ? m_nCount : static_cast<uint16_t>(max::ledcount::RGBW);
		nLedsPerPixel = 4;
	} else {
		m_nCount = m_nCount <= static_cast<uint16_t>(max::ledcount::RGB) ? m_nCount : static_cast<uint16_t>(max::ledcount::RGB);
		nLedsPerPixel = 3;
	}

	if ((m_Type == Type::APA102) || (m_Type == Type::SK9822)){
		if (m_nGlobalBrightness > 0x1F) {
			m_nGlobalBrightness = 0xFF;
		} else {
			m_nGlobalBrightness = 0xE0 | (m_nGlobalBrightness & 0x1F);
		}
	}

	if ((m_Type == Type::WS2801) || (m_Type == Type::APA102) || (m_Type == Type::SK9822) || (m_Type == Type::P9813)) {
		m_bIsRTZProtocol = false;
		m_nLowCode = 0x00;
		m_nHighCode = 0xFF;

		if (m_tRGBMapping == Map::UNDEFINED) {
			m_tRGBMapping = Map::RGB;
		}

		if (m_Type == Type::P9813) {
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

		if (m_tRGBMapping == Map::UNDEFINED) {
			m_tRGBMapping = GetRgbMapping(m_Type);
		}

		if (m_nLowCode >= m_nHighCode) {
			m_nLowCode = 0;
			m_nHighCode = 0;
		}

		uint8_t nLowCode, nHighCode;

		GetTxH(m_Type, nLowCode, nHighCode);

		if (m_nLowCode == 0) {
			m_nLowCode = nLowCode;
		}

		if (m_nHighCode == 0) {
			m_nHighCode = nHighCode;
		}

		m_nClockSpeedHz = 6400000;	// 6.4MHz / 8 bits = 800Hz
	}

	DEBUG_EXIT
}

Map PixelConfiguration::GetRgbMapping(Type tType) {
	if ((tType == Type::WS2811) || (tType == Type::UCS2903)) {
		return Map::RGB;
	}

	if (tType == Type::UCS1903) {
		return Map::BRG;
	}

	if (tType == Type::CS8812) {
		return Map::BGR;
	}

	return Map::GRB;
}

void PixelConfiguration::GetTxH(Type tType, uint8_t &nLowCode, uint8_t &nHighCode) {
	nLowCode = 0xC0;
	nHighCode = (tType == Type::WS2812B ? 0xF8 :
		(((tType == Type::UCS1903) || (tType == Type::UCS2903) || (tType == Type::CS8812)) ? 0xFC : 0xF0));
}

void PixelConfiguration::Dump() {
#ifndef NDEBUG
	printf("Type=%s [%u], Count=%u\n", PixelType::GetType(m_Type), static_cast<uint32_t>(m_Type), m_nCount);
	printf(" [%.2X,%.2X], Mapping=%s [%u]\n", m_nLowCode, m_nHighCode, PixelType::GetMap(m_tRGBMapping), static_cast<uint32_t>(m_tRGBMapping));
	if ((m_Type == Type::APA102) || (m_Type == Type::SK9822)){
		printf(" GlobalBrightness=%u\n", m_nGlobalBrightness);
	}
	printf(" Clock=%u Hz\n", m_nClockSpeedHz);
#endif
}
