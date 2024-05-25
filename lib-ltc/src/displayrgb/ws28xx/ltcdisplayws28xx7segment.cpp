/**
 * @file ltcdisplayws28xx7segment.cpp
 */
/*
 * Copyright (C) 2019-2020 by hippy mailto:dmxout@gmail.com
 * Copyright (C) 2019-2023 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include "ltcdisplayws28xx7segment.h"

#include "ws28xxdisplay7segment.h"

#include "pixeltype.h"

#include "debug.h"

LtcDisplayWS28xx7Segment::LtcDisplayWS28xx7Segment(pixel::Type type, pixel::Map map) {
	DEBUG_ENTRY

	m_pWS28xxDisplay7Segment = new WS28xxDisplay7Segment(type, map);
	assert(m_pWS28xxDisplay7Segment != nullptr);

	DEBUG_EXIT
}

void LtcDisplayWS28xx7Segment::Show(const char *pTimecode, struct ltcdisplayrgb::Colours& colours, struct ltcdisplayrgb::Colours& coloursColons) {
	auto nRed = colours.nRed;
	auto nGreen = colours.nGreen;
	auto nBlue = colours.nBlue;

	const char aChars[] = { pTimecode[0], pTimecode[1], pTimecode[3], pTimecode[4], pTimecode[6], pTimecode[7], pTimecode[9], pTimecode[10] };
	assert(sizeof(aChars) <= WS28xxDisplay7SegmentConfig::NUM_OF_DIGITS);

	m_pWS28xxDisplay7Segment->WriteAll(aChars, nRed, nGreen, nBlue);

	nRed = coloursColons.nRed;
	nGreen = coloursColons.nGreen;
	nBlue = coloursColons.nBlue;

	m_pWS28xxDisplay7Segment->WriteColon(pTimecode[ltc::timecode::index::COLON_1], 0, nRed, nGreen, nBlue);
	m_pWS28xxDisplay7Segment->WriteColon(pTimecode[ltc::timecode::index::COLON_2], 1, nRed, nGreen, nBlue);
	m_pWS28xxDisplay7Segment->WriteColon(pTimecode[ltc::timecode::index::COLON_3], 2, nRed, nGreen, nBlue);
	m_pWS28xxDisplay7Segment->Show();
}

void LtcDisplayWS28xx7Segment::ShowSysTime(const char *pSystemTime, struct ltcdisplayrgb::Colours& colours, struct ltcdisplayrgb::Colours& coloursColons) {
	auto nRed = colours.nRed;
	auto nGreen = colours.nGreen;
	auto nBlue = colours.nBlue;

	const char aChars[] = { ' ', ' ', pSystemTime[0], pSystemTime[1], pSystemTime[3], pSystemTime[4], pSystemTime[6], pSystemTime[7]};
	assert(sizeof(aChars) <= WS28xxDisplay7SegmentConfig::NUM_OF_DIGITS);

	m_pWS28xxDisplay7Segment->WriteAll(aChars, nRed, nGreen, nBlue);

	nRed = coloursColons.nRed;
	nGreen = coloursColons.nGreen;
	nBlue = coloursColons.nBlue;

	m_pWS28xxDisplay7Segment->WriteColon(' ', 0, nRed, nGreen, nBlue);
	m_pWS28xxDisplay7Segment->WriteColon(pSystemTime[ltc::systemtime::index::COLON_1], 1, nRed, nGreen, nBlue);
	m_pWS28xxDisplay7Segment->WriteColon(pSystemTime[ltc::systemtime::index::COLON_2], 2, nRed, nGreen, nBlue);
	m_pWS28xxDisplay7Segment->Show();
}

void LtcDisplayWS28xx7Segment::ShowMessage(const char *pMessage, struct ltcdisplayrgb::Colours& colours) {
	assert(WS28xxDisplay7SegmentConfig::NUM_OF_DIGITS == ltcdisplayrgb::MAX_MESSAGE_SIZE);

	const auto nRed = colours.nRed;
	const auto nGreen = colours.nGreen;
	const auto nBlue = colours.nBlue;

	m_pWS28xxDisplay7Segment->WriteAll(pMessage, nRed, nGreen, nBlue);
	m_pWS28xxDisplay7Segment->SetColonsOff();
	m_pWS28xxDisplay7Segment->Show();
}

void LtcDisplayWS28xx7Segment::WriteChar(uint8_t nChar, uint8_t nPos, struct ltcdisplayrgb::Colours& colours) {
	m_pWS28xxDisplay7Segment->WriteChar(static_cast<char>(nChar), nPos, colours.nRed, colours.nGreen, colours.nBlue);
	m_pWS28xxDisplay7Segment->Show();
}

void LtcDisplayWS28xx7Segment::Print() {
	printf(" 7-Segment %d Digit(s), %d Colons, %d LEDs\n", WS28xxDisplay7SegmentConfig::NUM_OF_DIGITS, WS28xxDisplay7SegmentConfig::NUM_OF_COLONS, WS28xxDisplay7SegmentConfig::LED_COUNT);
}
