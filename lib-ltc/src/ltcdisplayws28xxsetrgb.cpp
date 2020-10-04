/**
 * @file ltcdisplayws28xxsetrgb.cpp
 */
/*
 * Copyright (C) 2019-2020 by hippy mailto:dmxout@gmail.com
 * Copyright (C) 2019-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <ctype.h>
#include <ltcdisplayrgb.h>
#include <cassert>

#include "rgbmapping.h"

void LtcDisplayRgb::SetRGB(uint8_t nRed, uint8_t nGreen, uint8_t nBlue, TLtcDisplayWS28xxColourIndex tIndex) {
	switch (tIndex) {
	case LTCDISPLAYWS28XX_COLOUR_INDEX_DIGIT:
		m_tColours.nRed = nRed;
		m_tColours.nGreen = nGreen;
		m_tColours.nBlue = nBlue;
		break;
	case LTCDISPLAYWS28XX_COLOUR_INDEX_COLON:
		m_tColoursColons.nRed = nRed;
		m_tColoursColons.nGreen = nGreen;
		m_tColoursColons.nBlue = nBlue;
		break;
	case LTCDISPLAYWS28XX_COLOUR_INDEX_MESSAGE:
		m_tColoursMessage.nRed = nRed;
		m_tColoursMessage.nGreen = nGreen;
		m_tColoursMessage.nBlue = nBlue;
		break;
	default:
		break;
	}
}

void LtcDisplayRgb::SetRGB(uint32_t nRGB, TLtcDisplayWS28xxColourIndex tIndex) {
	const uint8_t nRed = ((nRGB & 0xFF0000) >> 16);
	const uint8_t nGreen = ((nRGB & 0xFF00) >> 8);
	const uint8_t nBlue = (nRGB & 0xFF);

	SetRGB(nRed, nGreen, nBlue, tIndex);
}

void LtcDisplayRgb::SetRGB(const char *pHexString) {
	if (!isdigit(pHexString[0])) {
		return;
	}

	const TLtcDisplayWS28xxColourIndex tIndex = static_cast<TLtcDisplayWS28xxColourIndex>((pHexString[0] - '0'));

	if (tIndex >= LTCDISPLAYWS28XX_COLOUR_INDEX_LAST) {
		return;
	}

	const uint32_t nRGB = hexadecimalToDecimal(pHexString + 1);

	SetRGB(nRGB, tIndex);
}

uint32_t LtcDisplayRgb::hexadecimalToDecimal(const char *pHexValue, uint32_t nLength) {
	char *src = const_cast<char*>(pHexValue);
	uint32_t ret = 0;
	uint8_t nibble;

	while (nLength-- > 0) {
		const char d = *src;

		if (isxdigit(d) == 0) {
			break;
		}

		nibble = d > '9' ? (d | 0x20) - 'a' + 10 : (d - '0');
		ret = (ret << 4) | nibble;
		src++;
	}

	return ret;
}
