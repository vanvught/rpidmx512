/**
 * @file ltcdisplayws28xx7segment.cpp
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
#include <stdio.h>
#include <cassert>

#include "ltcdisplayws28xx7segment.h"

#include "ws28xxdisplay7segment.h"

#include "rgbmapping.h"

#include "debug.h"

using namespace ltcdisplayrgb;

LtcDisplayWS28xx7Segment::LtcDisplayWS28xx7Segment() {
	DEBUG1_ENTRY

	m_pWS28xxDisplay7Segment = new WS28xxDisplay7Segment;
	assert(m_pWS28xxDisplay7Segment != nullptr);

	DEBUG1_EXIT
}

void LtcDisplayWS28xx7Segment::Init(TWS28XXType tLedType, TRGBMapping tRGBMapping) {
	DEBUG1_ENTRY

	m_pWS28xxDisplay7Segment->Init(tLedType, tRGBMapping);

	DEBUG1_EXIT
}

void LtcDisplayWS28xx7Segment::Show(const char *pTimecode, struct Colours &tColours, struct Colours &tColoursColons) {
	auto nRed = tColours.nRed;
	auto nGreen = tColours.nGreen;
	auto nBlue = tColours.nBlue;

	const char aChars[] = { pTimecode[0], pTimecode[1], pTimecode[3], pTimecode[4], pTimecode[6], pTimecode[7], pTimecode[9], pTimecode[10] };
	assert(sizeof(aChars) <= WS28xxDisplay7SegmentConfig::NUM_OF_DIGITS);

	m_pWS28xxDisplay7Segment->WriteAll(aChars, nRed, nGreen, nBlue);

	nRed = tColoursColons.nRed;
	nGreen = tColoursColons.nGreen;
	nBlue = tColoursColons.nBlue;

	m_pWS28xxDisplay7Segment->WriteColon(pTimecode[LTC_TC_INDEX_COLON_1], 0, nRed, nGreen, nBlue);
	m_pWS28xxDisplay7Segment->WriteColon(pTimecode[LTC_TC_INDEX_COLON_2], 1, nRed, nGreen, nBlue);
	m_pWS28xxDisplay7Segment->WriteColon(pTimecode[LTC_TC_INDEX_COLON_3], 2, nRed, nGreen, nBlue);
	m_pWS28xxDisplay7Segment->Show();
}

void LtcDisplayWS28xx7Segment::ShowSysTime(const char *pSystemTime, struct Colours &tColours, struct Colours &tColoursColons) {
	auto nRed = tColours.nRed;
	auto nGreen = tColours.nGreen;
	auto nBlue = tColours.nBlue;

	const char aChars[] = { ' ', ' ', pSystemTime[0], pSystemTime[1], pSystemTime[3], pSystemTime[4], pSystemTime[6], pSystemTime[7]};
	assert(sizeof(aChars) <= WS28xxDisplay7SegmentConfig::NUM_OF_DIGITS);

	m_pWS28xxDisplay7Segment->WriteAll(aChars, nRed, nGreen, nBlue);

	nRed = tColoursColons.nRed;
	nGreen = tColoursColons.nGreen;
	nBlue = tColoursColons.nBlue;

	m_pWS28xxDisplay7Segment->WriteColon(' ', 0, nRed, nGreen, nBlue);
	m_pWS28xxDisplay7Segment->WriteColon(pSystemTime[LTC_ST_INDEX_COLON_1], 1, nRed, nGreen, nBlue);
	m_pWS28xxDisplay7Segment->WriteColon(pSystemTime[LTC_ST_INDEX_COLON_2], 2, nRed, nGreen, nBlue);
	m_pWS28xxDisplay7Segment->Show();
}

void LtcDisplayWS28xx7Segment::ShowMessage(const char *pMessage, struct Colours &tColours) {
	assert(WS28xxDisplay7SegmentConfig::NUM_OF_DIGITS == MAX_MESSAGE_SIZE);

	const auto nRed = tColours.nRed;
	const auto nGreen = tColours.nGreen;
	const auto nBlue = tColours.nBlue;

	m_pWS28xxDisplay7Segment->WriteAll(pMessage, nRed, nGreen, nBlue);
	m_pWS28xxDisplay7Segment->SetColonsOff();
	m_pWS28xxDisplay7Segment->Show();
}

void LtcDisplayWS28xx7Segment::WriteChar(uint8_t nChar, uint8_t nPos, struct Colours &tColours) {
	m_pWS28xxDisplay7Segment->WriteChar(static_cast<char>(nChar), nPos, tColours.nRed, tColours.nGreen, tColours.nBlue);
	m_pWS28xxDisplay7Segment->Show();
}

void LtcDisplayWS28xx7Segment::Print() {
	printf(" 7-Segment %d Digit(s), %d Colons, %d LEDs\n", WS28xxDisplay7SegmentConfig::NUM_OF_DIGITS, WS28xxDisplay7SegmentConfig::NUM_OF_COLONS, WS28xxDisplay7SegmentConfig::LED_COUNT);
}
