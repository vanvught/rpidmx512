/**
 * @file rgbmapping.cpp
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

#include "rgbmapping.h"

using namespace rgbmapping;

constexpr char aMapping[static_cast<uint32_t>(Map::UNDEFINED)][4] = { "RGB", "RBG", "GRB", "GBR", "BRG", "BGR"};

Map RGBMapping::FromString(const char *pString) {
	assert(pString != nullptr);

	for (uint32_t nIndex = 0; nIndex < static_cast<uint32_t>(Map::UNDEFINED); nIndex++) {
		if (strncasecmp(aMapping[nIndex], pString, 3) == 0) {
			return static_cast<Map>(nIndex);
		}
	}

	return Map::UNDEFINED;
}

const char *RGBMapping::ToString(Map tRGBMapping) {
	if (tRGBMapping < Map::UNDEFINED) {
		return aMapping[static_cast<uint32_t>(tRGBMapping)];
	}

	return "Undefined";
}
