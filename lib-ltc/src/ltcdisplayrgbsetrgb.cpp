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
#include <cassert>

#include "ltcdisplayrgb.h"
#include "rgbmapping.h"

using namespace ltcdisplayrgb;

void LtcDisplayRgb::SetRGB(uint8_t nRed, uint8_t nGreen, uint8_t nBlue, ColourIndex tIndex) {
	switch (tIndex) {
	case ColourIndex::TIME:
		m_tColoursTime.nRed = nRed;
		m_tColoursTime.nGreen = nGreen;
		m_tColoursTime.nBlue = nBlue;
		break;
	case ColourIndex::COLON:
		m_tColoursColons.nRed = nRed;
		m_tColoursColons.nGreen = nGreen;
		m_tColoursColons.nBlue = nBlue;
		break;
	case ColourIndex::MESSAGE:
		m_tColoursMessage.nRed = nRed;
		m_tColoursMessage.nGreen = nGreen;
		m_tColoursMessage.nBlue = nBlue;
		break;
	case ColourIndex::FPS:
		m_tColoursFPS.nRed = nRed;
		m_tColoursFPS.nGreen = nGreen;
		m_tColoursFPS.nBlue = nBlue;
		break;
	case ColourIndex::INFO:
		m_tColoursInfo.nRed = nRed;
		m_tColoursInfo.nGreen = nGreen;
		m_tColoursInfo.nBlue = nBlue;
		break;
	case ColourIndex::SOURCE:
		m_tColoursSource.nRed = nRed;
		m_tColoursSource.nGreen = nGreen;
		m_tColoursSource.nBlue = nBlue;
		break;
	default:
		break;
	}
}

void LtcDisplayRgb::SetRGB(uint32_t nRGB, ColourIndex tIndex) {
	const auto nRed = ((nRGB & 0xFF0000) >> 16);
	const auto nGreen = ((nRGB & 0xFF00) >> 8);
	const auto nBlue = (nRGB & 0xFF);

	SetRGB(nRed, nGreen, nBlue, tIndex);
}

void LtcDisplayRgb::SetRGB(const char *pHexString) {
	if (!isdigit(pHexString[0])) {
		return;
	}

	const auto tIndex = static_cast<ColourIndex>((pHexString[0] - '0'));

	if (tIndex >= ColourIndex::LAST) {
		return;
	}

	const auto nRGB = hexadecimalToDecimal(pHexString + 1);

	SetRGB(nRGB, tIndex);
}

uint32_t LtcDisplayRgb::hexadecimalToDecimal(const char *pHexValue, uint32_t nLength) {
	auto *pSrc = const_cast<char*>(pHexValue);
	uint32_t nReturn = 0;

	while (nLength-- > 0) {
		const auto c = *pSrc;

		if (isxdigit(c) == 0) {
			break;
		}

		const uint8_t nNibble = c > '9' ? (c | 0x20) - 'a' + 10 : (c - '0');
		nReturn = (nReturn << 4) | nNibble;
		pSrc++;
	}

	return nReturn;
}
