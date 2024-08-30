/**
 * @file pixelconfiguration.h
 *
 */
/* Copyright (C) 2021-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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
/**
 * Static Local Variables:
 * Since C++11, the initialization of function-local static variables, is guaranteed to be thread-safe.
 * This means that even if multiple threads attempt to access Get() simultaneously,
 * the C++ runtime ensures that the instance is initialized only once.
 */

#ifndef PIXELCONFIGURATION_H_
#define PIXELCONFIGURATION_H_

#include <cstdint>
#include <cstdio>
#include <cassert>

#include "pixeltype.h"

#if defined (CONFIG_PIXELDMX_ENABLE_GAMMATABLE)
# include "gamma/gamma_tables.h"
#endif

#include "debug.h"

class PixelConfiguration {
public:
    PixelConfiguration() {
    	DEBUG_ENTRY

    	assert(s_pThis == nullptr);
    	s_pThis = this;

    	DEBUG_EXIT
    }

	void SetType(const pixel::Type Type) {
		m_type = Type;
	}

	pixel::Type GetType() const {
		return m_type;
	}

	void SetCount(const uint32_t nCount) {
		m_nCount = (nCount == 0 ? pixel::defaults::COUNT : nCount);
	}

	uint32_t GetCount() const {
		return m_nCount;
	}

	void SetMap(const pixel::Map tMap) {
		m_map = tMap;
	}

	pixel::Map GetMap() const {
		return m_map;
	}

	void SetLowCode(const uint8_t nLowCode) {
		m_nLowCode = nLowCode;
	}

	uint8_t GetLowCode() const {
		return m_nLowCode;
	}

	void SetHighCode(const uint8_t nHighCode) {
		m_nHighCode = nHighCode;
	}

	uint8_t GetHighCode() const {
		return m_nHighCode;
	}

	void SetClockSpeedHz(const uint32_t nClockSpeedHz)  {
		m_nClockSpeedHz = nClockSpeedHz;
	}

	uint32_t GetClockSpeedHz() const {
		return m_nClockSpeedHz;
	}

	void SetGlobalBrightness(const uint8_t nGlobalBrightness) {
		m_nGlobalBrightness = nGlobalBrightness;
	}

	uint8_t GetGlobalBrightness() const {
		return m_nGlobalBrightness;
	}

	bool IsRTZProtocol() const {
		return m_bIsRTZProtocol;
	}

	uint32_t GetLedsPerPixel() const {
		return m_nLedsPerPixel;
	}

	uint32_t GetRefreshRate() const {
		return m_nRefreshRate;
	}

#if defined (CONFIG_PIXELDMX_ENABLE_GAMMATABLE)
	void SetEnableGammaCorrection(const bool doEnable) {
		m_bEnableGammaCorrection = doEnable;
	}

	bool IsEnableGammaCorrection() const {
		return m_bEnableGammaCorrection;
	}

	void SetGammaTable(const uint32_t nValue) {
		m_nGammaValue = static_cast<uint8_t>(nValue);
	}

	const uint8_t *GetGammaTable() const {
		return m_pGammaTable;
	}
#endif

	void GetTxH(const pixel::Type type, uint8_t &nLowCode, uint8_t &nHighCode) {
		nLowCode = 0xC0;
		nHighCode = (type == pixel::Type::WS2812B ? 0xF8 :
				(((type == pixel::Type::UCS1903) || (type == pixel::Type::UCS2903) || (type == pixel::Type::CS8812)) ? 0xFC : 0xF0));
	}

	void Validate() {
		DEBUG_ENTRY

		if (m_type == pixel::Type::SK6812W) {
			m_nCount = m_nCount <= static_cast<uint16_t>(pixel::max::ledcount::RGBW) ? m_nCount : static_cast<uint16_t>(pixel::max::ledcount::RGBW);
			m_nLedsPerPixel = 4;
		} else {
			m_nCount = m_nCount <= static_cast<uint16_t>(pixel::max::ledcount::RGB) ? m_nCount : static_cast<uint16_t>(pixel::max::ledcount::RGB);
			m_nLedsPerPixel = 3;
		}

		if ((m_type == pixel::Type::APA102) || (m_type == pixel::Type::SK9822)){
			if (m_nGlobalBrightness > 0x1F) {
				m_nGlobalBrightness = 0xFF;
			} else {
				m_nGlobalBrightness = 0xE0 | (m_nGlobalBrightness & 0x1F);
			}
		}

		if ((m_type == pixel::Type::WS2801) || (m_type == pixel::Type::APA102) || (m_type == pixel::Type::SK9822) || (m_type == pixel::Type::P9813)) {
			m_bIsRTZProtocol = false;

			if (m_map == pixel::Map::UNDEFINED) {
				m_map = pixel::Map::RGB;
			}

			if (m_type == pixel::Type::P9813) {
				if (m_nClockSpeedHz == 0) {
					m_nClockSpeedHz = pixel::spi::speed::p9813::default_hz;
				} else if (m_nClockSpeedHz > pixel::spi::speed::p9813::max_hz) {
					m_nClockSpeedHz = pixel::spi::speed::p9813::max_hz;
				}
			} else {
				if (m_nClockSpeedHz == 0) {
					m_nClockSpeedHz = pixel::spi::speed::ws2801::default_hz;
				} else if (m_nClockSpeedHz > pixel::spi::speed::ws2801::max_hz) {
					m_nClockSpeedHz = pixel::spi::speed::ws2801::max_hz;
				}
			}

			const auto nLedTime = (8U * 1000000U) / m_nClockSpeedHz;
			const auto nLedsTime = nLedTime * m_nCount * m_nLedsPerPixel;
			m_nRefreshRate = 1000000U / nLedsTime;
		} else {
			m_bIsRTZProtocol = true;

			if (m_type == pixel::Type::UNDEFINED) {
				m_type = pixel::Type::WS2812B;
			}

			if (m_map == pixel::Map::UNDEFINED) {
				m_map = pixel::pixel_get_map(m_type);
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

			//                  8 * 1000.000
			// led time (us) =  ------------ * 8 = 10 us
			//                   6.400.000
			const auto nLedsTime = 10U * m_nCount * m_nLedsPerPixel;
			m_nRefreshRate = 1000000U / nLedsTime;
		}

#if defined (CONFIG_PIXELDMX_ENABLE_GAMMATABLE)
		if (m_bEnableGammaCorrection) {
			if (m_nGammaValue == 0) {
				m_pGammaTable = gamma::get_table_default(m_type);
			} else {
				m_pGammaTable = gamma::get_table(m_nGammaValue);
			}
		} else {
			m_pGammaTable = gamma10_0;
		}
#endif

		DEBUG_EXIT
	}

	void Print() {
		puts("Pixel configuration");
		printf(" Type    : %s [%d] <%d leds/pixel>\n", pixel::pixel_get_type(m_type), static_cast<int>(m_type), static_cast<int>(m_nLedsPerPixel));
		printf(" Count   : %d\n", m_nCount);

		if (m_bIsRTZProtocol) {
			printf(" Mapping : %s [%d]\n", pixel::pixel_get_map(m_map), static_cast<int>(m_map));
			printf(" T0H     : %.2f [0x%X]\n", pixel::pixel_convert_TxH(m_nLowCode), m_nLowCode);
			printf(" T1H     : %.2f [0x%X]\n", pixel::pixel_convert_TxH(m_nHighCode), m_nHighCode);
		} else {
			if ((m_type == pixel::Type::APA102) || (m_type == pixel::Type::SK9822)){
				printf(" GlobalBrightness: %u\n", m_nGlobalBrightness);
			}
		}

		printf(" Clock   : %u Hz\n", static_cast<unsigned int>(m_nClockSpeedHz));
		printf(" Refresh : %u Hz\n", static_cast<unsigned int>(m_nRefreshRate));

#if defined (CONFIG_PIXELDMX_ENABLE_GAMMATABLE)
		printf(" Gamma correction %s\n", m_bEnableGammaCorrection ? "Yes" :  "No");
#endif
	}

	static PixelConfiguration& Get() {
		assert(s_pThis != nullptr); // Ensure that s_pThis is valid
		return *s_pThis;
	}

private:
	uint32_t m_nCount { pixel::defaults::COUNT };
	uint32_t m_nClockSpeedHz { 0 };
	uint32_t m_nLedsPerPixel { 3 };
	pixel::Type m_type { pixel::defaults::TYPE };
	pixel::Map m_map { pixel::Map::UNDEFINED };
	bool m_bIsRTZProtocol { true };
	uint8_t m_nLowCode { 0 };
	uint8_t m_nHighCode { 0 };
	uint8_t m_nGlobalBrightness { 0xFF };
	uint32_t m_nRefreshRate { 0 };
#if defined (CONFIG_PIXELDMX_ENABLE_GAMMATABLE)
	uint8_t m_nGammaValue { 0 };
	bool m_bEnableGammaCorrection { false };
	const uint8_t *m_pGammaTable { gamma10_0 };
#endif

	static inline PixelConfiguration *s_pThis { nullptr };
};

#endif /* PIXELCONFIGURATION_H_ */
