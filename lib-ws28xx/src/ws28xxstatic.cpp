/**
 * @file ws28xxstatic.cpp
 *
 */
/* Copyright (C) 2019-2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <stdint.h>
#include <string.h>
#include <cassert>

#include "ws28xx.h"
#include "ws28xxconst.h"

using namespace ws28xx;

const char *WS28xx::GetLedTypeString(Type tType) {
	if (tType < Type::UNDEFINED) {
		return WS28xxConst::TYPES[static_cast<uint32_t>(tType)];
	}

	return "Unknown";
}

Type WS28xx::GetLedTypeString(const char *pValue) {
	assert(pValue != nullptr);

	for (uint32_t i = 0; i < static_cast<uint32_t>(Type::UNDEFINED); i++) {
		if (strcasecmp(pValue, WS28xxConst::TYPES[i]) == 0) {
			return static_cast<Type>(i);
		}
	}

	return Type::UNDEFINED;
}

// TODO Update when a new chip is added
void WS28xx::GetTxH(Type tType, uint8_t &nLowCode, uint8_t &nHighCode) {
	nLowCode = 0xC0;
	nHighCode = (
			tType == Type::WS2812B ?
					0xF8 :
					(((tType == Type::UCS1903) || (tType == Type::UCS2903)
							|| (tType == Type::CS8812)) ? 0xFC : 0xF0));
}

// TODO Update when a new chip is added
rgbmapping::Map WS28xx::GetRgbMapping(Type tType) {
	if ((tType == Type::WS2811) || (tType == Type::UCS2903)) {
		return rgbmapping::Map::RGB;
	}

	if (tType == Type::UCS1903) {
		return rgbmapping::Map::BRG;
	}

	if (tType == Type::CS8812) {
		return rgbmapping::Map::BGR;
	}

	return rgbmapping::Map::GRB;
}

float WS28xx::ConvertTxH(uint8_t nCode) {

#define F_INTERVAL 0.15625f

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

	__builtin_unreachable();
}

uint8_t WS28xx::ConvertTxH(float fTxH) {
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
