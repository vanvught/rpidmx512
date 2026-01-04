/**
 * @file pixeltype.cpp
 *
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

#include <cstdint>
#include <cstdio>
#include <cassert>
#include <cstring>

#include "pixeltype.h"

namespace pixel
{
static constexpr char kTypes[static_cast<uint32_t>(pixel::Type::UNDEFINED)][pixel::kTypesMaxNameLength] = {
    "WS2801\0",                                                // 1
    "WS2811\0", 
    "WS2812\0", 
    "WS2812B", 
    "WS2813\0", 
    "WS2815\0", // 5
    "SK6812\0", 
    "SK6812W",                                     // 2
    "UCS1903",  
    "UCS2903",                                     // 2
    "CS8812",                                                  // 1
    "APA102\0",
	"SK9822\0",                                    // 2
    "P9813",                                                   // 1
};

static constexpr char kMaps[static_cast<uint32_t>(pixel::Map::UNDEFINED)][4] = 
{
	"RGB", 
	"RBG", 
	"GRB", 
	"GBR", 
	"BRG", 
	"BGR"
};

const char* GetType(Type type)
{
    if (type < Type::UNDEFINED)
    {
        return kTypes[static_cast<uint32_t>(type)];
    }

    return "Unknown";
}

Type GetType(const char* string)
{
    assert(string != nullptr);
    uint32_t index = 0;

    for (const char(&type)[kTypesMaxNameLength] : kTypes)
    {
        if (strcasecmp(type, string) == 0)
        {
            return static_cast<Type>(index);
        }
        ++index;
    }

    return Type::UNDEFINED;
}

Map GetMap(const char* string)
{
    assert(string != nullptr);
    uint32_t index = 0;

    for (const char(&map)[4] : kMaps)
    {
        if (strncasecmp(map, string, 3) == 0)
        {
            return static_cast<Map>(index);
        }
        ++index;
    }

    return Map::UNDEFINED;
}

const char* GetMap(Map map)
{
    if (map < Map::UNDEFINED)
    {
        return kMaps[static_cast<uint32_t>(map)];
    }

    return "Undefined";
}
} // namespace pixel

namespace remoteconfig::pixel
{
uint32_t JsonGetTypes(char* out_buffer, uint32_t out_buffer_size)
{
    const auto kBufferSize = out_buffer_size - 2U;
    auto length = static_cast<uint32_t>(snprintf(out_buffer, kBufferSize, "{\"types\":["));

    for (const char(&type)[::pixel::kTypesMaxNameLength] : ::pixel::kTypes)
    {
        length += static_cast<uint32_t>(snprintf(&out_buffer[length], kBufferSize - length, "\"%s\",", type));
    }

    length--;

    out_buffer[length++] = ']';
    out_buffer[length++] = '}';

    assert(length <= kBufferSize);
    return length;
}
} // namespace remoteconfig::pixel
