/**
 * @file ltcdisplayws28xx7segment.cpp
 */
/*
 * Copyright (C) 2019 by hippy mailto:dmxout@gmail.com
 * Copyright (C) 2019-2020 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#include <assert.h>

#include "ltcdisplayws28xx7segment.h"

#include "ws28xxdisplay7segment.h"

#include "debug.h"

LtcDisplayWS28xx7Segment::LtcDisplayWS28xx7Segment(void) {
	DEBUG1_ENTRY

	m_pWS28xxDisplay7Segment = new WS28xxDisplay7Segment;
	assert(m_pWS28xxDisplay7Segment != 0);

	DEBUG1_EXIT
}

LtcDisplayWS28xx7Segment::~LtcDisplayWS28xx7Segment(void) {
	DEBUG1_ENTRY

	DEBUG1_EXIT
}

void LtcDisplayWS28xx7Segment::Init(TWS28XXType tLedType) {
	DEBUG1_ENTRY

	m_pWS28xxDisplay7Segment->Init(tLedType);

	DEBUG1_EXIT
}

void LtcDisplayWS28xx7Segment::Show(const char *pTimecode, struct TLtcDisplayRgbColours &tColours, struct TLtcDisplayRgbColours &tColoursColons) {
	uint8_t nRed = tColours.nRed;
	uint8_t nGreen = tColours.nGreen;
	uint8_t nBlue = tColours.nBlue;

	const uint8_t aChars[] = { pTimecode[0], pTimecode[1], pTimecode[3], pTimecode[4], pTimecode[6], pTimecode[7], pTimecode[9], pTimecode[10] };
	assert(sizeof(aChars) <= WS28XX_NUM_OF_DIGITS);

	m_pWS28xxDisplay7Segment->WriteAll(aChars, nRed, nGreen, nBlue);

	nRed = tColoursColons.nRed;
	nGreen = tColoursColons.nGreen;
	nBlue = tColoursColons.nBlue;

	m_pWS28xxDisplay7Segment->WriteColon(pTimecode[2], 0, nRed, nGreen, nBlue);
	m_pWS28xxDisplay7Segment->WriteColon(pTimecode[5], 1, nRed, nGreen, nBlue);
	m_pWS28xxDisplay7Segment->WriteColon(pTimecode[8], 2, nRed, nGreen, nBlue);

	m_pWS28xxDisplay7Segment->Show();
}

void LtcDisplayWS28xx7Segment::ShowSysTime(const char *pSystemTime, struct TLtcDisplayRgbColours &tColours, struct TLtcDisplayRgbColours &tColoursColons) {
	uint8_t nRed = tColours.nRed;
	uint8_t nGreen = tColours.nGreen;
	uint8_t nBlue = tColours.nBlue;

	const uint8_t aChars[] = { ' ', ' ', pSystemTime[0], pSystemTime[1], pSystemTime[3], pSystemTime[4], pSystemTime[6], pSystemTime[7]};
	assert(sizeof(aChars) <= WS28XX_NUM_OF_DIGITS);

	m_pWS28xxDisplay7Segment->WriteAll(aChars, nRed, nGreen, nBlue);

	nRed = tColoursColons.nRed;
	nGreen = tColoursColons.nGreen;
	nBlue = tColoursColons.nBlue;

	m_pWS28xxDisplay7Segment->WriteColon(' ', 0, nRed, nGreen, nBlue);
	m_pWS28xxDisplay7Segment->WriteColon(':', 1, nRed, nGreen, nBlue);
	m_pWS28xxDisplay7Segment->WriteColon(':', 3, nRed, nGreen, nBlue);

	m_pWS28xxDisplay7Segment->Show();
}

void LtcDisplayWS28xx7Segment::ShowMessage(const char *pMessage, struct TLtcDisplayRgbColours &tColours) {
	assert(WS28XX_NUM_OF_DIGITS == LTCDISPLAY_MAX_MESSAGE_SIZE);

	const uint8_t nRed = tColours.nRed;
	const uint8_t nGreen = tColours.nGreen;
	const uint8_t nBlue = tColours.nBlue;

	m_pWS28xxDisplay7Segment->WriteAll((const uint8_t *)pMessage, nRed, nGreen, nBlue);

	m_pWS28xxDisplay7Segment->SetColonsOff();

	m_pWS28xxDisplay7Segment->Show();
}

void LtcDisplayWS28xx7Segment::WriteChar(uint8_t nChar, uint8_t nPos, struct TLtcDisplayRgbColours &tColours) {
	m_pWS28xxDisplay7Segment->WriteChar(nChar, nPos, tColours.nRed, tColours.nGreen, tColours.nBlue);
	m_pWS28xxDisplay7Segment->Show();
}

void LtcDisplayWS28xx7Segment::Print(void) {
	printf(" 7-Segment %d Digit(s), %d Colons, %d LEDs\n", WS28XX_NUM_OF_DIGITS, WS28XX_NUM_OF_COLONS, WS28XX_LED_COUNT);
}
