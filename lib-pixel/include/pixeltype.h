/**
 * @file pixeltype.h
 */
/* Copyright (C) 2021-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

namespace pixel
{
enum class Type: uint8_t
{
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

enum class Map: uint8_t
{
    RGB,
    RBG,
    GRB,
    GBR,
    BRG,
    BGR,
    UNDEFINED
};

inline constexpr auto kTypesMaxNameLength = 8;

namespace max::ledcount
{
inline constexpr uint32_t RGB = (4 * 170);
inline constexpr uint32_t RGBW = (4 * 128);
} // namespace max::ledcount

namespace single
{
inline constexpr uint32_t RGB = 24;
inline constexpr uint32_t RGBW = 32;
} // namespace single

namespace spi::speed
{
namespace ws2801
{
inline constexpr uint32_t kMaxHz = 25000000;    ///< 25 MHz
inline constexpr uint32_t kDefaultHz = 4000000; ///< 4 MHz
} // namespace ws2801
namespace p9813
{
inline constexpr uint32_t kMaxHz = 15000000;    ///< 15 MHz
inline constexpr uint32_t kDefaultHz = 4000000; ///< 4 MHz
} // namespace p9813
} // namespace spi::speed

namespace defaults
{
inline constexpr auto kType = Type::WS2812B;
inline constexpr uint32_t kCount = 170;
inline constexpr uint32_t kOutputPorts = 1;
} // namespace defaults

inline constexpr auto kFInterval = 0.15625f;

inline float ConvertTxH(uint8_t code)
{
    switch (code)
    {
        case 0x80:
            return kFInterval * 1;
            break;
        case 0xC0:
            return kFInterval * 2;
            break;
        case 0xE0:
            return kFInterval * 3;
            break;
        case 0xF0:
            return kFInterval * 4;
            break;
        case 0xF8:
            return kFInterval * 5;
            break;
        case 0xFC:
            return kFInterval * 6;
            break;
        case 0xFE:
            return kFInterval * 7;
            break;
        default:
            return 0;
            break;
    }

    assert(0);
    __builtin_unreachable();
}

inline uint8_t ConvertTxH(float tx_h)
{
    if (tx_h < 0.5f * kFInterval)
    {
        return 0x00;
    }

    if (tx_h < 1.5f * kFInterval)
    {
        return 0x80;
    }

    if (tx_h < 2.5f * kFInterval)
    {
        return 0xC0;
    }

    if (tx_h < 3.5f * kFInterval)
    {
        return 0xE0;
    }

    if (tx_h < 4.5f * kFInterval)
    {
        return 0xF0;
    }

    if (tx_h < 5.5f * kFInterval)
    {
        return 0xF8;
    }

    if (tx_h < 6.5f * kFInterval)
    {
        return 0xFC;
    }

    if (tx_h < 7.5f * kFInterval)
    {
        return 0xFE;
    }
	
    return 0x00;
}

const char* GetType(Type);
Type GetType(const char*);

const char* GetMap(Map);
Map GetMap(const char*);

inline Map GetMap(Type type)
{
    if ((type == Type::WS2811) || (type == Type::UCS2903))
    {
        return Map::RGB;
    }

    if (type == Type::UCS1903)
    {
        return Map::BRG;
    }

    if (type == Type::CS8812)
    {
        return Map::BGR;
    }

    return Map::GRB;
}
} // namespace pixel

#endif  // PIXELTYPE_H_
