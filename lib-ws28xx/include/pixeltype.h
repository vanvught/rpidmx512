/**
 * @file pixeltype.h
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

#ifndef PIXELTYPE_H_
#define PIXELTYPE_H_

#include <stdint.h>
#include <cassert>

namespace pixel {
enum class Type {
	WS2801,
	WS2811,
	WS2812,
	WS2812B,
	WS2813,
	WS2815,
	SK6812,
	SK6812W,
	UCS1903,
	UCS2903,
	CS8812,
	APA102,
	SK9822,
	P9813,
	UNDEFINED
};
enum class Map {
	RGB, RBG, GRB, GBR, BRG, BGR, UNDEFINED
};
static constexpr auto TYPES_MAX_NAME_LENGTH  = 8;
namespace max {
namespace ledcount {
static constexpr auto RGB = (4 * 170);
static constexpr auto RGBW = (4 * 128);
}  // namespace ledcount
}  // namespace max
namespace single {
static constexpr auto RGB = 24;
static constexpr auto RGBW = 32;
}  // namespace single
namespace spi {
namespace speed {
namespace ws2801 {
static constexpr uint32_t max_hz = 25000000;	///< 25 MHz
static constexpr uint32_t default_hz = 4000000;	///< 4 MHz
}  // namespace ws2801
namespace p9813 {
static constexpr uint32_t max_hz = 15000000;	///< 15 MHz
static constexpr uint32_t default_hz = 4000000;	///< 4 MHz
}  // namespace p9813
}  // namespace speed
}  // namespace spi
namespace defaults {
static constexpr auto TYPE = Type::WS2812B;
static constexpr auto COUNT = 170;
static constexpr auto OUTPUT_PORTS = 1;
}  // namespace defaults

inline Map pixel_get_map(const pixel::Type type) {
	if ((type == pixel::Type::WS2811) || (type == pixel::Type::UCS2903)) {
		return pixel::Map::RGB;
	}

	if (type == pixel::Type::UCS1903) {
		return pixel::Map::BRG;
	}

	if (type == pixel::Type::CS8812) {
		return pixel::Map::BGR;
	}

	return pixel::Map::GRB;
}

static constexpr auto F_INTERVAL = 0.15625f;

inline float pixel_convert_TxH(const uint8_t nCode) {
	switch (nCode) {
	case 0x80:
		return F_INTERVAL * 1;
		break;
	case 0xC0:
		return F_INTERVAL * 2;
		break;
	case 0xE0:
		return F_INTERVAL * 3;
		break;
	case 0xF0:
		return F_INTERVAL * 4;
		break;
	case 0xF8:
		return F_INTERVAL * 5;
		break;
	case 0xFC:
		return F_INTERVAL * 6;
		break;
	case 0xFE:
		return F_INTERVAL * 7;
		break;
	default:
		return 0;
		break;
	}

	assert(0);
	__builtin_unreachable();
}

inline uint8_t pixel_convert_TxH(const float fTxH) {
	if (fTxH < 0.5f * F_INTERVAL) {
		return 0x00;
	}

	if (fTxH < 1.5f * F_INTERVAL) {
		return 0x80;
	}

	if (fTxH < 2.5f * F_INTERVAL) {
		return 0xC0;
	}

	if (fTxH < 3.5f * F_INTERVAL) {
		return 0xE0;
	}

	if (fTxH < 4.5f * F_INTERVAL) {
		return 0xF0;
	}

	if (fTxH < 5.5f * F_INTERVAL) {
		return 0xF8;
	}

	if (fTxH < 6.5f * F_INTERVAL) {
		return 0xFC;
	}

	if (fTxH < 7.5f * F_INTERVAL) {
		return 0xFE;
	}

	return 0x00;
}

const char* pixel_get_type(pixel::Type);
pixel::Type pixel_get_type(const char *);

const char* pixel_get_map(pixel::Map);
pixel::Map pixel_get_map(const char *);
}  // namespace pixel

#endif /* PIXELTYPE_H_ */
