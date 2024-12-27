/**
 * @file ws28xxmulti.h
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

#ifndef GD32_GPIO_WS28XXMULTI_H_
#define GD32_GPIO_WS28XXMULTI_H_

#if defined (DEBUG_PIXEL)
# if defined (NDEBUG)
#  undef NDEBUG
#  define _NDEBUG
# endif
#endif

#pragma GCC push_options
#pragma GCC optimize ("O3")
#pragma GCC optimize ("no-tree-loop-distribute-patterns")
#pragma GCC optimize ("-fprefetch-loop-arrays")

#include <cstdint>

#include "pixelconfiguration.h"
#include "gd32/gpio/pixelmulti_config.h"

#include "gd32.h"

#include "debug.h"

class WS28xxMulti {
public:
	WS28xxMulti();
	~WS28xxMulti() {
		DEBUG_ENTRY
		DEBUG_EXIT
	}

#define BIT_SET(Addr, Bit) {																						\
		*(volatile uint32_t *) (BITBAND_SRAM_BASE + (((uint32_t)&Addr) - SRAM_BASE) * 32U + (Bit & 0xFF) * 4U) = 0x1;	\
}

#define BIT_CLEAR(Addr, Bit) {																						\
		*(volatile uint32_t *) (BITBAND_SRAM_BASE + (((uint32_t)&(Addr)) - SRAM_BASE) * 32U + (Bit & 0xFF) * 4U) = 0x0;	\
}

	void SetColourRTZ(const uint32_t nPortIndex, const uint32_t nPixelIndex, const uint8_t nColour1, const uint8_t nColour2, const uint8_t nColour3) {
		assert(nPortIndex < pixel::PORT_COUNT);

		uint32_t j = 0;
		const auto k = nPixelIndex * pixel::single::RGB;
		const auto nBit = nPortIndex + GPIO_PIN_OFFSET;
		auto *p = &s_pPixelBufferData[k];

		for (uint8_t mask = 0x80; mask != 0; mask = static_cast<uint8_t>(mask >> 1)) {
			auto &p1 = p[j];
			auto &p2 = p[8 + j];
			auto &p3 = p[16 + j];

			if (!(mask & nColour1)) {
				BIT_SET(p1, nBit);
			} else {
				BIT_CLEAR(p1, nBit);
			}
			if (!(mask & nColour2)) {
				BIT_SET(p2, nBit);
			} else {
				BIT_CLEAR(p2, nBit);
			}
			if (!(mask & nColour3)) {
				BIT_SET(p3, nBit);
			} else {
				BIT_CLEAR(p3, nBit);
			}

			j++;
		}
	}

	void SetColourRTZ(const uint32_t nPortIndex, const uint32_t nPixelIndex, const uint8_t nRed, const uint8_t nGreen, const uint8_t nBlue, const uint8_t nWhite) {
		assert(nPortIndex < pixel::PORT_COUNT);

		const auto k = nPixelIndex * pixel::single::RGBW;
		const auto nBit = nPortIndex + GPIO_PIN_OFFSET;

		auto *p = &s_pPixelBufferData[k];
		uint32_t j = 0;

		for (uint8_t mask = 0x80; mask != 0; mask = static_cast<uint8_t>(mask >> 1)) {
			auto& p1 = p[j];
			auto& p2 = p[8 + j];
			auto& p3 = p[16 + j];
			auto& p4 = p[24 + j];

			// GRBW
			if (!(mask & nGreen)) {
				BIT_SET(p1, nBit);
			} else {
				BIT_CLEAR(p1, nBit);
			}

			if (!(mask & nRed)) {
				BIT_SET(p2, nBit);
			} else {
				BIT_CLEAR(p2, nBit);
			}

			if (!(mask & nBlue)) {
				BIT_SET(p3, nBit);
			} else {
				BIT_CLEAR(p3, nBit);
			}

			if (!(mask & nWhite)) {
				BIT_SET(p4, nBit);
			} else {
				BIT_CLEAR(p4, nBit);
			}

			j++;
		}
	}

	void SetColourWS2801(const uint32_t nPortIndex, const uint32_t nPixelIndex, const uint8_t nColour1, const uint8_t nColour2, const uint8_t nColour3) {
		assert(nPortIndex < pixel::PORT_COUNT);

		uint32_t j = 0;
		const auto k = nPixelIndex * pixel::single::RGB;
		const auto nBit = nPortIndex + GPIO_PIN_OFFSET;
		auto *p = &s_pPixelBufferData[k];

		for (uint8_t mask = 0x80; mask != 0; mask = static_cast<uint8_t>(mask >> 1)) {
			auto& p1 = p[j];
			auto& p2 = p[8 + j];
			auto& p3 = p[16 + j];

			if (mask & nColour1) {
				BIT_SET(p1, nBit);
			} else {
				BIT_CLEAR(p1, nBit);
			}
			if (mask & nColour2) {
				BIT_SET(p2, nBit);
			} else {
				BIT_CLEAR(p2, nBit);
			}
			if (mask & nColour3) {
				BIT_SET(p3, nBit);
			} else {
				BIT_CLEAR(p3, nBit);
			}

			j++;
		}
	}

	void SetPixel4Bytes(const uint32_t nPortIndex, const uint32_t nPixelIndex, const uint8_t nCtrl, const uint8_t nColour1, const uint8_t nColour2, const uint8_t nColour3) {
		assert(nPortIndex < pixel::PORT_COUNT);

		uint32_t j = 0;
		const auto k = nPixelIndex * pixel::single::RGBW;
		const auto nBit = nPortIndex + GPIO_PIN_OFFSET;
		auto *p = &s_pPixelBufferData[k];

		for (uint8_t mask = 0x80; mask != 0; mask = static_cast<uint8_t>(mask >> 1)) {
			if (mask & nCtrl) {
				BIT_SET(p[j], nBit);
			} else {
				BIT_CLEAR(p[j], nBit);
			}

			if (mask & nColour1) {
				BIT_SET(p[8 + j], nBit);
			} else {
				BIT_CLEAR(p[8 + j], nBit);
			}

			if (mask & nColour2) {
				BIT_SET(p[16 + j], nBit);
			} else {
				BIT_CLEAR(p[16 + j], nBit);
			}

			if (mask & nColour3) {
				BIT_SET(p[24 + j], nBit);
			} else {
				BIT_CLEAR(p[24 + j], nBit);
			}

			j++;
		}
	}

	bool IsUpdating();

	void Update();
	void Blackout();
	void FullOn();

	uint32_t GetUserData();

	static WS28xxMulti *Get() {
		return s_pThis;
	}

private:
	void Setup(uint8_t nLowCode, uint8_t nHighCode);
	void Setup(uint32_t nFrequency);

private:
	uint32_t m_nBufSize { 0 };

	/**
	 * https://www.gd32-dmx.org/memory.html
	 */
#if defined (GD32F20X) || defined (GD32F4XX)
# define SECTION_PIXEL	__attribute__ ((section (".pixel")))
#else
# define SECTION_PIXEL
#endif

	static inline uint16_t PixelBuffer[2 * 1024 * 16] __attribute__ ((aligned (4))) SECTION_PIXEL;
	static inline constexpr auto PIXEL_BUFFER_SIZE = sizeof(PixelBuffer) / sizeof(PixelBuffer[0]);
	static inline auto *s_pPixelBufferData = reinterpret_cast<uint16_t *>(PixelBuffer);
	static inline auto *s_PixelBufferDma = reinterpret_cast<uint16_t *>(PixelBuffer + PIXEL_BUFFER_SIZE / 2);
	static inline constexpr auto MAX_APA102 = ((PIXEL_BUFFER_SIZE / 8) - 8 ) / 4;

	static inline WS28xxMulti *s_pThis;
};

#pragma GCC pop_options
#if defined (_NDEBUG)
# undef _NDEBUG
# define NDEBUG
#endif
#endif /* GD32_GPIO_WS28XXMULTI_H_ */
