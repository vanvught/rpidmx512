/**
 * @file pixeltype.cpp
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

#if defined(__GNUC__) && !defined(__clang__) ///< Needed for macOS
# pragma GCC push_options
# pragma GCC optimize ("Os")
#endif

#include <cstdint>
#include <cstring>
#include <cstdio>
#include <cassert>

#include "pixeltype.h"

namespace pixel {
const char TYPES[static_cast<uint32_t>(pixel::Type::UNDEFINED)][pixel::TYPES_MAX_NAME_LENGTH] =
		{ "WS2801\0", 																// 1
		  "WS2811\0", "WS2812\0", "WS2812B", "WS2813\0", "WS2815\0",				// 5
		  "SK6812\0", "SK6812W",													// 2
		  "UCS1903", "UCS2903",														// 2
		  "CS8812",																	// 1
		  "APA102\0", "SK9822\0",													// 2
		  "P9813",																	// 1
		};																			// = 14

const char MAPS[static_cast<uint32_t>(pixel::Map::UNDEFINED)][4] = { "RGB", "RBG", "GRB", "GBR", "BRG", "BGR"};

const char *pixel_get_type(pixel::Type type) {
	if (type < pixel::Type::UNDEFINED) {
		return TYPES[static_cast<uint32_t>(type)];
	}

	return "Unknown";
}

pixel::Type pixel_get_type(const char *pString) {
	assert(pString != nullptr);
	uint32_t index = 0;

	for (const char (&type)[pixel::TYPES_MAX_NAME_LENGTH] : TYPES) {
		if (strcasecmp(pString, type) == 0) {
			return static_cast<pixel::Type>(index);
		}
		++index;
	}

	return pixel::Type::UNDEFINED;
}

pixel::Map pixel_get_map(const char *pString) {
	assert(pString != nullptr);
	uint32_t index = 0;

	for (const char (&map)[4] : MAPS) {
		if (strncasecmp(map, pString, 3) == 0) {
			return static_cast<pixel::Map>(index);
		}
		++index;
	}

	return pixel::Map::UNDEFINED;
}

const char *pixel_get_map(pixel::Map map) {
	if (map < pixel::Map::UNDEFINED) {
		return MAPS[static_cast<uint32_t>(map)];
	}

	return "Undefined";
}
}  // namespace pixel

namespace remoteconfig::pixel {
uint32_t json_get_types(char *pOutBuffer, const uint32_t nOutBufferSize) {
	const auto nBufferSize = nOutBufferSize - 2U;
	auto nLength = static_cast<uint32_t>(snprintf(pOutBuffer, nBufferSize, "{\"types\":[" ));

	for (const char (&type)[::pixel::TYPES_MAX_NAME_LENGTH] : ::pixel::TYPES) {
		nLength += static_cast<uint32_t>(snprintf(&pOutBuffer[nLength], nBufferSize - nLength, "\"%s\",", type));
	}

	nLength--;

	pOutBuffer[nLength++] = ']';
	pOutBuffer[nLength++] = '}';

	assert(nLength <= nOutBufferSize);
	return nLength;
}
}  // namespace remoteconfig::pixel
