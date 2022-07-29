/**
 * @file pixeltype.cpp
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
#include <cstring>
#include <cassert>

#include "pixeltype.h"

using namespace pixel;

const char PixelType::TYPES[static_cast<uint32_t>(Type::UNDEFINED)][TYPES_MAX_NAME_LENGTH] =
		{ "WS2801\0", 																// 1
		  "WS2811\0", "WS2812\0", "WS2812B", "WS2813\0", "WS2815\0",				// 5
		  "SK6812\0", "SK6812W",													// 2
		  "UCS1903", "UCS2903",														// 2
		  "CS8812",																	// 1
		  "APA102\0", "SK9822\0",													// 2
		  "P9813",																	// 1
		};																			// = 14

const char PixelType::MAPS[static_cast<uint32_t>(Map::UNDEFINED)][4] = { "RGB", "RBG", "GRB", "GBR", "BRG", "BGR"};

const char *PixelType::GetType(Type type) {
	if (type < Type::UNDEFINED) {
		return TYPES[static_cast<uint32_t>(type)];
	}

	return "Unknown";
}

Type PixelType::GetType(const char *pString) {
	assert(pString != nullptr);

	for (uint32_t i = 0; i < static_cast<uint32_t>(Type::UNDEFINED); i++) {
		if (strcasecmp(pString, TYPES[i]) == 0) {
			return static_cast<Type>(i);
		}
	}

	return Type::UNDEFINED;
}

Map PixelType::GetMap(const char *pString) {
	assert(pString != nullptr);

	for (uint32_t nIndex = 0; nIndex < static_cast<uint32_t>(Map::UNDEFINED); nIndex++) {
		if (strncasecmp(MAPS[nIndex], pString, 3) == 0) {
			return static_cast<Map>(nIndex);
		}
	}

	return Map::UNDEFINED;
}

const char *PixelType::GetMap(Map map) {
	if (map < Map::UNDEFINED) {
		return MAPS[static_cast<uint32_t>(map)];
	}

	return "Undefined";
}

Map PixelType::GetMap(Type type) {
	if ((type == Type::WS2811) || (type == Type::UCS2903)) {
		return Map::RGB;
	}

	if (type == Type::UCS1903) {
		return Map::BRG;
	}

	if (type == Type::CS8812) {
		return Map::BGR;
	}

	return Map::GRB;
}

static constexpr auto F_INTERVAL = 0.15625f;

float PixelType::ConvertTxH(uint8_t nCode) {
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

uint8_t PixelType::ConvertTxH(float fTxH) {
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
