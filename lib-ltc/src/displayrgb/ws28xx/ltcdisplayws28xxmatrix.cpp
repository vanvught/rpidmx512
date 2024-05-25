/**
 * @file ltcdisplayws28xxmatrix.cpp
 */
/* Copyright (C) 2019-2023 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include "ltcdisplayws28xxmatrix.h"

#include "ws28xxdisplaymatrix.h"

#include "pixeltype.h"

#include "debug.h"

LtcDisplayWS28xxMatrix::LtcDisplayWS28xxMatrix(pixel::Type type, pixel::Map map) {
	DEBUG_ENTRY

	m_pWS28xxDisplayMatrix = new WS28xxDisplayMatrix(64, 8, type, map);
	assert(m_pWS28xxDisplayMatrix != nullptr);

	DEBUG_EXIT
}

void LtcDisplayWS28xxMatrix::Show(const char *pTimecode, struct ltcdisplayrgb::Colours& colours, struct ltcdisplayrgb::Colours& coloursColons) {
	m_pWS28xxDisplayMatrix->SetColonsOff();
	m_pWS28xxDisplayMatrix->SetColon(pTimecode[ltc::timecode::index::COLON_1], 1, coloursColons.nRed, coloursColons.nGreen, coloursColons.nBlue);
	m_pWS28xxDisplayMatrix->SetColon(pTimecode[ltc::timecode::index::COLON_2], 3, coloursColons.nRed, coloursColons.nGreen, coloursColons.nBlue);
	m_pWS28xxDisplayMatrix->SetColon(pTimecode[ltc::timecode::index::COLON_3], 5, coloursColons.nRed, coloursColons.nGreen, coloursColons.nBlue);

	const char cLine[] = { pTimecode[0], pTimecode[1], pTimecode[3], pTimecode[4], pTimecode[6], pTimecode[7], pTimecode[9], pTimecode[10] };

	m_pWS28xxDisplayMatrix->TextLine(1, cLine, sizeof(cLine), colours.nRed, colours.nGreen, colours.nBlue);
	m_pWS28xxDisplayMatrix->Show();
}

void LtcDisplayWS28xxMatrix::ShowSysTime(const char *pSystemTime, struct ltcdisplayrgb::Colours& colours, struct ltcdisplayrgb::Colours& coloursColons) {
	m_pWS28xxDisplayMatrix->SetColonsOff();
	m_pWS28xxDisplayMatrix->SetColon(pSystemTime[ltc::systemtime::index::COLON_1], 2, coloursColons.nRed, coloursColons.nGreen, coloursColons.nBlue);
	m_pWS28xxDisplayMatrix->SetColon(pSystemTime[ltc::systemtime::index::COLON_2], 4, coloursColons.nRed, coloursColons.nGreen, coloursColons.nBlue);

	const char cLine[] = { ' ', pSystemTime[0], pSystemTime[1], pSystemTime[3], pSystemTime[4], pSystemTime[6], pSystemTime[7], ' '};

	m_pWS28xxDisplayMatrix->TextLine(1, cLine, sizeof(cLine), colours.nRed, colours.nGreen, colours.nBlue);
	m_pWS28xxDisplayMatrix->Show();
}

void LtcDisplayWS28xxMatrix::ShowMessage(const char *pMessage, struct ltcdisplayrgb::Colours& colours) {
	m_pWS28xxDisplayMatrix->SetColonsOff();
	m_pWS28xxDisplayMatrix->TextLine(1, pMessage, ltcdisplayrgb::MAX_MESSAGE_SIZE, colours.nRed, colours.nGreen, colours.nBlue);
	m_pWS28xxDisplayMatrix->Show();
}

void LtcDisplayWS28xxMatrix::WriteChar(uint8_t nChar, uint8_t nPos, struct ltcdisplayrgb::Colours& colours) {
	m_pWS28xxDisplayMatrix->SetCursorPos(nPos, 0);
	m_pWS28xxDisplayMatrix->PutChar(static_cast<char>(nChar), colours.nRed, colours.nGreen, colours.nBlue);
	m_pWS28xxDisplayMatrix->Show();
}

void LtcDisplayWS28xxMatrix::Print() {
	printf(" Matrix %dx%d\n", m_pWS28xxDisplayMatrix->GetMaxPosition(), m_pWS28xxDisplayMatrix->GetMaxLine());
}
