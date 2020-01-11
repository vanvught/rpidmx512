/**
 * @file ltcdisplayws28xxmatrix.cpp
 */
/* Copyright (C) 2019-2020 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include "ltcdisplayws28xxmatrix.h"

#include "ws28xxdisplaymatrix.h"

#include "debug.h"

LtcDisplayWS28xxMatrix::LtcDisplayWS28xxMatrix(void) {
	DEBUG1_ENTRY

	m_pWS28xxDisplayMatrix = new WS28xxDisplayMatrix(64, 8);
	assert(m_pWS28xxDisplayMatrix != 0);

	DEBUG1_EXIT
}

LtcDisplayWS28xxMatrix::~LtcDisplayWS28xxMatrix(void) {
	DEBUG1_ENTRY

	delete m_pWS28xxDisplayMatrix;
	m_pWS28xxDisplayMatrix = 0;

	DEBUG1_EXIT
}

void LtcDisplayWS28xxMatrix::Init(TWS28XXType tLedType) {
	DEBUG1_ENTRY

	m_pWS28xxDisplayMatrix->Init(tLedType);

	DEBUG1_EXIT
}

void LtcDisplayWS28xxMatrix::Show(const char *pTimecode, struct TLtcDisplayRgbColours &tColours, struct TLtcDisplayRgbColours &tColoursColons) {
	m_pWS28xxDisplayMatrix->SetColonsOff();
	m_pWS28xxDisplayMatrix->SetColon(':', 1, tColoursColons.nRed, tColoursColons.nGreen, tColoursColons.nBlue);
	m_pWS28xxDisplayMatrix->SetColon(':', 3, tColoursColons.nRed, tColoursColons.nGreen, tColoursColons.nBlue);
	m_pWS28xxDisplayMatrix->SetColon('.', 5, tColoursColons.nRed, tColoursColons.nGreen, tColoursColons.nBlue);

	const char cLine[] = { pTimecode[0], pTimecode[1], pTimecode[3], pTimecode[4], pTimecode[6], pTimecode[7], pTimecode[9], pTimecode[10] };

	m_pWS28xxDisplayMatrix->TextLine(1, cLine, sizeof(cLine), tColours.nRed, tColours.nGreen, tColours.nBlue);
	m_pWS28xxDisplayMatrix->Show();
}

void LtcDisplayWS28xxMatrix::ShowSysTime(const char *pSystemTime, struct TLtcDisplayRgbColours &tColours, struct TLtcDisplayRgbColours &tColoursColons) {
	m_pWS28xxDisplayMatrix->SetColonsOff();
	m_pWS28xxDisplayMatrix->SetColon(':', 2, tColoursColons.nRed, tColoursColons.nGreen, tColoursColons.nBlue);
	m_pWS28xxDisplayMatrix->SetColon(':', 4, tColoursColons.nRed, tColoursColons.nGreen, tColoursColons.nBlue);

	const char cLine[] = { ' ', pSystemTime[0], pSystemTime[1], pSystemTime[3], pSystemTime[4], pSystemTime[6], pSystemTime[7], ' '};

	m_pWS28xxDisplayMatrix->TextLine(1, cLine, sizeof(cLine), tColours.nRed, tColours.nGreen, tColours.nBlue);
	m_pWS28xxDisplayMatrix->Show();
}

void LtcDisplayWS28xxMatrix::ShowMessage(const char *pMessage, struct TLtcDisplayRgbColours &tColours) {
	m_pWS28xxDisplayMatrix->SetColonsOff();
	m_pWS28xxDisplayMatrix->TextLine(1, pMessage, LTCDISPLAY_MAX_MESSAGE_SIZE, tColours.nRed, tColours.nGreen, tColours.nBlue);
	m_pWS28xxDisplayMatrix->Show();
}

void LtcDisplayWS28xxMatrix::WriteChar(uint8_t nChar, uint8_t nPos, struct TLtcDisplayRgbColours &tColours) {
	m_pWS28xxDisplayMatrix->SetCursorPos(nPos, 0);
	m_pWS28xxDisplayMatrix->PutChar(nChar, tColours.nRed, tColours.nGreen, tColours.nBlue);
	m_pWS28xxDisplayMatrix->Show();
}

void LtcDisplayWS28xxMatrix::Print(void) {
	printf(" Matrix %dx%d\n", m_pWS28xxDisplayMatrix->GetMaxPosition(), m_pWS28xxDisplayMatrix->GetMaxLine());
}
