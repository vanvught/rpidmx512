/**
 * @file pixeltype.h
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

#ifndef PIXELTYPE_H_
#define PIXELTYPE_H_

#include <stdint.h>

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
}  // namespace pixel

class PixelType {
public:
	static const char *GetType(pixel::Type tType);
	static pixel::Type GetType(const char *pString);

	static const char *GetMap(pixel::Map tMap);
	static pixel::Map GetMap(const char *pString);

	static float ConvertTxH(uint8_t nCode);
	static uint8_t ConvertTxH(float fTxH);

private:
	static const char TYPES[static_cast<uint32_t>(pixel::Type::UNDEFINED)][pixel::TYPES_MAX_NAME_LENGTH];
	static const char MAPS[static_cast<uint32_t>(pixel::Map::UNDEFINED)][4];
};

#endif /* PIXELTYPE_H_ */
