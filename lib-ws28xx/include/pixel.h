/**
 * @file pixel.h
 *
 */
/* Copyright (C) 2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef PIXEL_H_
#define PIXEL_H_

#pragma GCC push_options
#pragma GCC optimize ("O3")
#pragma GCC optimize ("no-tree-loop-distribute-patterns")
#pragma GCC optimize ("-fprefetch-loop-arrays")

#include <cstdint>
#include <cassert>

#if defined (PIXELPATTERNS_MULTI)
# include "ws28xxmulti.h"
  using OutputType = WS28xxMulti;
#else
# include "ws28xx.h"
  using OutputType = WS28xx;
#endif

namespace pixel {
inline uint32_t get_colour(const uint8_t nRed, const uint8_t nGreen, const uint8_t nBlue) {
	return static_cast<uint32_t>(nRed << 16) | static_cast<uint32_t>(nGreen << 8) | nBlue;
}

inline uint32_t get_colour(const uint8_t nRed, const uint8_t nGreen, const uint8_t nBlue, const uint8_t nWhite) {
	return static_cast<uint32_t>(nWhite << 24) | static_cast<uint32_t>(nRed << 16) | static_cast<uint32_t>(nGreen << 8) | nBlue;
}

inline uint8_t get_white(const uint32_t nColour) {
	return (nColour >> 24) & 0xFF;
}

inline uint8_t get_red(const uint32_t nColour) {
	return (nColour >> 16) & 0xFF;
}

inline uint8_t get_green(const uint32_t nColour) {
	return (nColour >> 8) & 0xFF;
}

inline uint8_t get_blue(const uint32_t nColour) {
	return nColour & 0xFF;
}

inline void set_pixel_colour([[maybe_unused]] uint32_t nPortIndex, const uint32_t nPixelIndex, const uint32_t nColour) {
	auto *pOutputType = OutputType::Get();
	assert(pOutputType != nullptr);

	const auto nRed = pixel::get_red(nColour);
	const auto nGreen = pixel::get_green(nColour);
	const auto nBlue = pixel::get_blue(nColour);

#if defined (PIXELPATTERNS_MULTI)
	switch (PixelConfiguration::Get().GetType()) {
	case pixel::Type::WS2801:
		pOutputType->SetColourWS2801(nPortIndex, nPixelIndex, nRed, nGreen, nBlue);
		break;
	case pixel::Type::APA102:
	case pixel::Type::SK9822:
		pOutputType->SetPixel4Bytes(nPortIndex, nPixelIndex, 0xFF, nRed, nGreen, nBlue);
		break;
	case pixel::Type::P9813: {
		const auto nFlag = static_cast<uint8_t>(0xC0 | ((~nBlue & 0xC0) >> 2) | ((~nRed & 0xC0) >> 4) | ((~nRed & 0xC0) >> 6));
		pOutputType->SetPixel4Bytes(nPortIndex, nPixelIndex, nFlag, nBlue, nGreen, nRed);
	}
		break;
	case pixel::Type::SK6812W: {
		const auto nWhite = pixel::get_white(nColour);
		pOutputType->SetColourRTZ(nPortIndex, nPixelIndex, nRed, nGreen, nBlue, nWhite);
	}
		break;
	default:
		pOutputType->SetColourRTZ(nPortIndex, nPixelIndex, nRed, nGreen, nBlue);
		break;
	}
#else
	auto& pixelConfiguration = PixelConfiguration::Get();
	const auto type = pixelConfiguration.GetType();

	if (type != pixel::Type::SK6812W) {
		pOutputType->SetPixel(nPixelIndex, nRed, nGreen, nBlue);
	} else {
		if ((nRed == nGreen) && (nGreen == nBlue)) {
			pOutputType->SetPixel(nPixelIndex, 0x00, 0x00, 0x00, nRed);
		} else {
			pOutputType->SetPixel(nPixelIndex, nRed, nGreen, nBlue, 0x00);
		}
	}
#endif
}

inline void set_pixel_colour(const uint32_t nPortIndex, const uint32_t nColour) {
	const auto nCount = PixelConfiguration::Get().GetCount();

	for (uint32_t i = 0; i < nCount; i++) {
		set_pixel_colour(nPortIndex, i, nColour);
	}
}

inline bool is_updating() {
	auto *pOutputType = OutputType::Get();
	assert(pOutputType != nullptr);
	return pOutputType->IsUpdating();
}

inline void update() {
	auto *pOutputType = OutputType::Get();
	assert(pOutputType != nullptr);
	return pOutputType->Update();
}
}  // namespace pixel

#pragma GCC pop_options
#endif /* PIXEL_H_ */
