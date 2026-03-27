/**
 * @file pixeltype.h
 */
/* Copyright (C) 2021-2026 by Arjan van Vught mailto:info@gd32-dmx.org
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
#include <cstring>
#include <cassert>

#include "common/utils/utils_enum.h"

namespace pixel
{
inline constexpr uint8_t kRtzLowCode = 0xC0;
inline constexpr uint32_t kNoSpeedHz = 0;
inline constexpr uint8_t kNoCode = 0;

enum class LedType : uint8_t
{
    kWS2801,
    kWS2811,
    kWS2812,
    kWS2812B,
    kWS2813,
    kWS2815,
    kSK6812,
    kSK6812W,
    kUCS1903,
    kUCS2903,
    kCS8812,
    kAPA102,
    kSK9822,
    kP9813,
    kUndefined
};

enum class LedMap : uint8_t
{
    kRGB,
    kRBG,
    kGRB,
    kGBR,
    kBRG,
    kBGR,
	kRGBW,
    kUndefined
};

inline constexpr char kMaps[static_cast<uint32_t>(pixel::LedMap::kUndefined)][5] = 
{
	"RGB", 
	"RBG", 
	"GRB", 
	"GBR", 
	"BRG", 
	"BGR",
	"RGBW"
};

constexpr uint32_t kMapsCount = static_cast<uint32_t>(sizeof(kMaps) / sizeof(kMaps[0]));
static_assert(kMapsCount == static_cast<uint32_t>(pixel::LedMap::kUndefined), "LedMap must match kMaps");

enum class ProtocolType : uint8_t
{
    kRtz,
    kSpi,
};

enum class LedCount : uint8_t
{
    k3 = 3,
    k4 = 4,
};

struct TypeInfo
{
    const char* name;           // 4 bytes on Cortex-M32
    uint32_t default_hz;        // SPI only, 0 for RTZ
    uint32_t max_hz;            // SPI only, 0 for RTZ
    ProtocolType protocol_type; // 1 byte
    LedCount led_count;         // 1 byte
    uint8_t low_code;           // RTZ only, 0 for SPI
    uint8_t high_code;          // RTZ only, 0 for SPI
	LedMap led_map;

    constexpr bool IsRtz() const { return protocol_type == ProtocolType::kRtz; }
    constexpr bool IsSpi() const { return protocol_type == ProtocolType::kSpi; }
};

static_assert(sizeof(TypeInfo) == 20, "TypeInfo must remain compact");
static_assert(alignof(TypeInfo) == 4, "Unexpected TypeInfo alignment");

constexpr TypeInfo MakeSpiTypeInfo(const char* name, LedCount led_count, uint32_t default_hz, uint32_t max_hz)
{
    return TypeInfo{
        .name = name,
        .default_hz = default_hz,
        .max_hz = max_hz,
        .protocol_type = ProtocolType::kSpi,
        .led_count = led_count,
        .low_code = kNoCode,
        .high_code = kNoCode,
		.led_map = LedMap::kRGB
    };
}

constexpr TypeInfo MakeRtzTypeInfo(const char* name, LedMap led_map, LedCount led_count, uint8_t high_code)
{
    return TypeInfo{
        .name = name,
        .default_hz = kNoSpeedHz,
        .max_hz = kNoSpeedHz,
        .protocol_type = ProtocolType::kRtz,
        .led_count = led_count,
        .low_code = kRtzLowCode,
        .high_code = high_code,
		.led_map = led_map
    };
}

inline constexpr TypeInfo kTypeInfo[] = {
    MakeSpiTypeInfo("WS2801", LedCount::k3, 4000000, 25000000),
    MakeRtzTypeInfo("WS2811", LedMap::kRGB, LedCount::k3, 0xF0),
    MakeRtzTypeInfo("WS2812", LedMap::kGRB, LedCount::k3, 0xF0),  
    MakeRtzTypeInfo("WS2812B", LedMap::kGRB, LedCount::k3, 0xF8),
    MakeRtzTypeInfo("WS2813", LedMap::kGRB, LedCount::k3, 0xF0),  
    MakeRtzTypeInfo("WS2815", LedMap::kGRB, LedCount::k3, 0xF0),
    MakeRtzTypeInfo("SK6812", LedMap::kGRB, LedCount::k3, 0xF0),  
    MakeRtzTypeInfo("SK6812W", LedMap::kRGBW, LedCount::k4, 0xF0),
    MakeRtzTypeInfo("UCS1903", LedMap::kBRG, LedCount::k3, 0xFC), 
    MakeRtzTypeInfo("UCS2903", LedMap::kRGB, LedCount::k3, 0xFC),
    MakeRtzTypeInfo("CS8812", LedMap::kBGR, LedCount::k3, 0xFC),  
    MakeSpiTypeInfo("APA102", LedCount::k3, 4000000, 25000000),
    MakeSpiTypeInfo("SK9822", LedCount::k3, 4000000, 25000000),   
    MakeSpiTypeInfo("P9813", LedCount::k3, 4000000, 25000000),
};

constexpr uint32_t kTypeInfoCount = static_cast<uint32_t>(sizeof(kTypeInfo) / sizeof(kTypeInfo[0]));
static_assert(kTypeInfoCount == static_cast<uint32_t>(LedType::kUndefined), "kTypeInfo must match LedType");

constexpr uint32_t ConstStrLen(const char* s)
{
    uint32_t len = 0;
    while (s[len] != '\0')
    {
        ++len;
    }
    return len;
}

constexpr uint32_t GetMaxTypeNameLength()
{
    uint32_t max_len = 0;

    for (uint32_t i = 0; i < kTypeInfoCount; ++i)
    {
        const uint32_t kLen = ConstStrLen(kTypeInfo[i].name);
        if (kLen > max_len)
        {
            max_len = kLen;
        }
    }

    return max_len;
}

constexpr auto kTypesMaxNameLength = GetMaxTypeNameLength();

constexpr uint32_t GetMaxLedMapNameLength()
{
    uint32_t max_len = 0;

    for (uint32_t i = 0; i < kMapsCount; ++i)
    {
        const uint32_t kLen = ConstStrLen(kMaps[i]);
        if (kLen > max_len)
        {
            max_len = kLen;
        }
    }

    return max_len;
}

constexpr auto kLedMapMaxNameLength = GetMaxLedMapNameLength();

constexpr inline const TypeInfo& GetTypeInfo(LedType type)
{
    return kTypeInfo[static_cast<uint32_t>(type)];
}

inline void GetTxH(LedType type, uint8_t& low_code, uint8_t& high_code)
{
    const auto& info = GetTypeInfo(type);

    low_code = info.low_code;
    high_code = info.high_code;
}

constexpr inline const char* GetTypeName(LedType type)
{
    const auto kIndex = static_cast<uint32_t>(type);

    if (kIndex < kTypeInfoCount) {
        return kTypeInfo[kIndex].name;
    }

    return "Unknown";
}

inline LedType GetTypeByName(const char* string)
{
    assert(string != nullptr);

    using U = std::underlying_type_t<LedType>;

    for (size_t i = 0; i < kTypeInfoCount; ++i)
    {
        if (strcasecmp(kTypeInfo[i].name, string) == 0)
        {
            return common::FromValue<LedType>(static_cast<U>(i));
        }
    }

    return LedType::kUndefined;
}

inline const char* GetMapName(LedMap map)
{
	const auto kIndex = static_cast<uint32_t>(map);

	if (kIndex < kMapsCount) {
	    return kMaps[kIndex];
	}

	return "Unknown";
}

inline LedMap GetMapByName(const char* string)
{
    assert(string != nullptr);

    using U = std::underlying_type_t<LedMap>;

    for (size_t i = 0; i < kMapsCount; ++i)
    {
        if (strcasecmp(kMaps[i], string) == 0)
        {
            return common::FromValue<LedMap>(static_cast<U>(i));
        }
    }

    return LedMap::kUndefined;
}

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

namespace defaults
{
inline constexpr auto kType = LedType::kWS2812B;
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
} // namespace pixel

#endif // PIXELTYPE_H_
