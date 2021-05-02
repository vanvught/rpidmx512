/**
 * @file ltcdisplaymax7219matrix.cpp
 *
 */
/* Copyright (C) 2019-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <stdio.h>
#include <time.h>
#include <cassert>

#include "ltcdisplaymax7219matrix.h"
#include "ltc.h"

#include "max7219matrix.h"

static constexpr uint8_t nCharsColon[][8] = {
{ 0x3E, 0x7F, 0x71, 0x59, 0x4D, 0x7F, 0x3E, 0x66 }, 		// 0:
{ 0x40, 0x42, 0x7F, 0x7F, 0x40, 0x40, 0x00, 0x66 }, 		// 1:
{ 0x62, 0x73, 0x59, 0x49, 0x6F, 0x66, 0x00, 0x66 }, 		// 2:
{ 0x22, 0x63, 0x49, 0x49, 0x7F, 0x36, 0x00, 0x66 }, 		// 3:
{ 0x18, 0x1C, 0x16, 0x53, 0x7F, 0x7F, 0x50, 0x66 }, 		// 4:
{ 0x27, 0x67, 0x45, 0x45, 0x7D, 0x39, 0x00, 0x66 }, 		// 5:
{ 0x3C, 0x7E, 0x4B, 0x49, 0x79, 0x30, 0x00, 0x66 }, 		// 6:
{ 0x03, 0x03, 0x71, 0x79, 0x0F, 0x07, 0x00, 0x66 }, 		// 7:
{ 0x36, 0x7F, 0x49, 0x49, 0x7F, 0x36, 0x00, 0x66 }, 		// 8:
{ 0x06, 0x4F, 0x49, 0x69, 0x3F, 0x1E, 0x00, 0x66 }  		// 9:
};

static constexpr uint8_t nCharsBlinkSemiColon[][8] = {
{ 0x3E, 0x7F, 0x71, 0x59, 0x4D, 0x7F, 0x3E | 0x80, 0x66 },	// 0;
{ 0x40, 0x42, 0x7F, 0x7F, 0x40, 0x40, 0x00 | 0x80, 0x66 },	// 1;
{ 0x62, 0x73, 0x59, 0x49, 0x6F, 0x66, 0x00 | 0x80, 0x66 },	// 2;
{ 0x22, 0x63, 0x49, 0x49, 0x7F, 0x36, 0x00 | 0x80, 0x66 },	// 3;
{ 0x18, 0x1C, 0x16, 0x53, 0x7F, 0x7F, 0x50 | 0x80, 0x66 },	// 4;
{ 0x27, 0x67, 0x45, 0x45, 0x7D, 0x39, 0x00 | 0x80, 0x66 },	// 5;
{ 0x3C, 0x7E, 0x4B, 0x49, 0x79, 0x30, 0x00 | 0x80, 0x66 },	// 6;
{ 0x03, 0x03, 0x71, 0x79, 0x0F, 0x07, 0x00 | 0x80, 0x66 },	// 7;
{ 0x36, 0x7F, 0x49, 0x49, 0x7F, 0x36, 0x00 | 0x80, 0x66 },	// 8;
{ 0x06, 0x4F, 0x49, 0x69, 0x3F, 0x1E, 0x00 | 0x80, 0x66 }  	// 9;
};

static constexpr uint8_t nCharsBlinkComma[][8] = {
{ 0x3E, 0x7F, 0x71, 0x59, 0x4D, 0x7F, 0x3E | 0x80, 0x00 }, 	// 0,
{ 0x40, 0x42, 0x7F, 0x7F, 0x40, 0x40, 0x00 | 0x80, 0x60 }, 	// 1,
{ 0x62, 0x73, 0x59, 0x49, 0x6F, 0x66, 0x00 | 0x80, 0x00 }, 	// 2,
{ 0x22, 0x63, 0x49, 0x49, 0x7F, 0x36, 0x00 | 0x80, 0x60 }, 	// 3,
{ 0x18, 0x1C, 0x16, 0x53, 0x7F, 0x7F, 0x50 | 0x80, 0x00 }, 	// 4,
{ 0x27, 0x67, 0x45, 0x45, 0x7D, 0x39, 0x00 | 0x80, 0x60 }, 	// 5,
{ 0x3C, 0x7E, 0x4B, 0x49, 0x79, 0x30, 0x00 | 0x80, 0x00 }, 	// 6,
{ 0x03, 0x03, 0x71, 0x79, 0x0F, 0x07, 0x00 | 0x80, 0x60 }, 	// 7,
{ 0x36, 0x7F, 0x49, 0x49, 0x7F, 0x36, 0x00 | 0x80, 0x00 }, 	// 8,
{ 0x06, 0x4F, 0x49, 0x69, 0x3F, 0x1E, 0x00 | 0x80, 0x60 }  	// 9,
};

LtcDisplayMax7219Matrix *LtcDisplayMax7219Matrix::s_pThis = nullptr;

LtcDisplayMax7219Matrix::LtcDisplayMax7219Matrix() {
	assert(s_pThis == nullptr);
	s_pThis = this;
}

void LtcDisplayMax7219Matrix::Init(uint8_t nIntensity) {
	for (uint32_t i = 0; i < sizeof(nCharsColon)/sizeof(nCharsColon[0]); i++) {
		Max7219Matrix::UpdateCharacter(i, nCharsColon[i]);
	}

	for (uint32_t i = 10; i < 10 + sizeof(nCharsBlinkSemiColon)/sizeof(nCharsBlinkSemiColon[0]); i++) {
		Max7219Matrix::UpdateCharacter(i, nCharsBlinkSemiColon[i - 10]);
	}

	for (uint32_t i = 20; i < 20 + sizeof(nCharsBlinkComma)/sizeof(nCharsBlinkComma[0]); i++) {
		Max7219Matrix::UpdateCharacter(i, nCharsBlinkComma[i - 20]);
	}

	Max7219Matrix::Init(SEGMENTS, nIntensity);
	Max7219Matrix::Write("Waiting", 7);
}

int32_t LtcDisplayMax7219Matrix::Offset(const char nChar, const char nSeconds) {
	const auto bEven = !((nSeconds & 0x01) == 0x01);

	if (bEven) {
		switch (nChar) {
		case ':':
			return 0 - '0';
			break;
		case ';':
			return 10 - '0';
			break;
		case ',':
			return 20 - '0';
			break;
		default:
			break;
		}
	}

	return 0;
}

void LtcDisplayMax7219Matrix::Show(const char *pTimecode) {
	assert(pTimecode != nullptr);

	const auto nSeconds = pTimecode[LTC_TC_INDEX_SECONDS_UNITS];

	m_aBuffer[0] = pTimecode[0];
	m_aBuffer[1] = Offset(pTimecode[LTC_TC_INDEX_COLON_1], nSeconds) + pTimecode[1];
	m_aBuffer[2] = pTimecode[3];
	m_aBuffer[3] = Offset(pTimecode[LTC_TC_INDEX_COLON_2], nSeconds) + pTimecode[4];
	m_aBuffer[4] = pTimecode[6];
	m_aBuffer[5] = Offset(pTimecode[LTC_TC_INDEX_COLON_3], nSeconds) + pTimecode[7];
	m_aBuffer[6] = pTimecode[9];
	m_aBuffer[7] = pTimecode[10];

	Max7219Matrix::Write(m_aBuffer, SEGMENTS);
}

void LtcDisplayMax7219Matrix::ShowSysTime(const char *pSystemTime) {
	assert(pSystemTime != nullptr);

	const auto nSeconds = pSystemTime[LTC_ST_INDEX_SECONDS_UNITS];

	m_aBuffer[0] = ' ';
	m_aBuffer[1] = pSystemTime[0];
	m_aBuffer[2] = Offset(pSystemTime[LTC_ST_INDEX_COLON_1], nSeconds) + pSystemTime[1];
	m_aBuffer[3] = pSystemTime[3];
	m_aBuffer[4] = Offset(pSystemTime[LTC_ST_INDEX_COLON_2], nSeconds) + pSystemTime[4];
	m_aBuffer[5] = pSystemTime[6];
	m_aBuffer[6] = pSystemTime[7];
	m_aBuffer[7] = ' ';

	Max7219Matrix::Write(m_aBuffer, SEGMENTS);
}

void LtcDisplayMax7219Matrix::WriteChar(__attribute__((unused)) uint8_t nChar, __attribute__((unused)) uint8_t nPos) {
	// TODO Implement WriteChar
}
