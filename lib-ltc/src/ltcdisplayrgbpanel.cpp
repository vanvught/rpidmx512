/**
 * @file ltcdisplayrgbpanel.cpp
 */
/*
 * Copyright (C) 2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifdef NDEBUG
#undef NDEBUG
#endif

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter" //TODO Remove

#include <cassert>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "ltcdisplayrgbpanel.h"
#include "ltc.h"

#include "rgbpanel.h"

#include "debug.h"

static constexpr char aTypes[5][8 + 1] =
	{ "Film  24", "EBU   25", "DF 29.97", "SMPTE 30", "----- --" };

LtcDisplayRgbPanel::LtcDisplayRgbPanel() {
	DEBUG_ENTRY

	m_pRgbPanel = new RgbPanel(64, 32);
	assert(m_pRgbPanel != nullptr);

	DEBUG_EXIT
}

LtcDisplayRgbPanel::~LtcDisplayRgbPanel() {
	DEBUG_ENTRY

	delete m_pRgbPanel;
	m_pRgbPanel = nullptr;

	DEBUG_EXIT
}

void LtcDisplayRgbPanel::Init() {
	DEBUG_ENTRY

	for (uint32_t i = 0; i < 4; i++) {
		memset(m_Line[i], ' ', 8);
		m_LineColours[i].nRed = 0x00;
		m_LineColours[i].nGreen = 0x00;
		m_LineColours[i].nBlue = 0x00;
	}

	m_pRgbPanel->Start();

	DEBUG_EXIT
}

void LtcDisplayRgbPanel::Print() {
	printf("RGB Panel\n");
}

void LtcDisplayRgbPanel::Show(const char *pTimecode, struct TLtcDisplayRgbColours &tColours, struct TLtcDisplayRgbColours &tColoursColons) {
	m_pRgbPanel->SetColonsOff();
	m_pRgbPanel->SetColon(':', 1, 0, tColoursColons.nRed, tColoursColons.nGreen, tColoursColons.nBlue);
	m_pRgbPanel->SetColon(':', 3, 0, tColoursColons.nRed, tColoursColons.nGreen, tColoursColons.nBlue);
	m_pRgbPanel->SetColon('.', 5, 0, tColoursColons.nRed, tColoursColons.nGreen, tColoursColons.nBlue);

	const char cLine[] = { pTimecode[0], pTimecode[1], pTimecode[3], pTimecode[4], pTimecode[6], pTimecode[7], pTimecode[9], pTimecode[10] };

	m_pRgbPanel->TextLine(1, cLine, sizeof(cLine), tColours.nRed, tColours.nGreen, tColours.nBlue);

	for (uint32_t i = 1; i < 4; i++) {
		m_pRgbPanel->TextLine(1 + i, m_Line[i], 8, m_LineColours[i].nRed, m_LineColours[i].nGreen, m_LineColours[i].nBlue);
	}

	m_pRgbPanel->Show();
}

void LtcDisplayRgbPanel::ShowSysTime(const char *pSystemTime, struct TLtcDisplayRgbColours &tColours, struct TLtcDisplayRgbColours &tColoursColons) {
	m_pRgbPanel->SetColonsOff();
	m_pRgbPanel->SetColon(':', 2, 0, tColoursColons.nRed, tColoursColons.nGreen, tColoursColons.nBlue);
	m_pRgbPanel->SetColon(':', 4, 0, tColoursColons.nRed, tColoursColons.nGreen, tColoursColons.nBlue);

	const char cLine[] = { ' ', pSystemTime[0], pSystemTime[1], pSystemTime[3], pSystemTime[4], pSystemTime[6], pSystemTime[7], ' '};

	m_pRgbPanel->ClearLine(1);
	m_pRgbPanel->TextLine(2, cLine, sizeof(cLine), tColours.nRed, tColours.nGreen, tColours.nBlue);
	m_pRgbPanel->ClearLine(3);
	m_pRgbPanel->TextLine(4, m_Line[3], 8, m_LineColours[3].nRed, m_LineColours[3].nGreen, m_LineColours[3].nBlue);

	m_pRgbPanel->Show();
}

void LtcDisplayRgbPanel::ShowMessage(const char *pMessage, struct TLtcDisplayRgbColours &tColours) {
	m_pRgbPanel->SetColonsOff();
	m_pRgbPanel->TextLine(1, pMessage, LTCDISPLAY_MAX_MESSAGE_SIZE, tColours.nRed, tColours.nGreen, tColours.nBlue);
	m_pRgbPanel->Show();
}

void LtcDisplayRgbPanel::ShowFPS(ltc::type tTimeCodeType, struct TLtcDisplayRgbColours &tColours) {
	memcpy(m_Line[1], aTypes[tTimeCodeType], 8);

	m_LineColours[1].nRed = tColours.nRed;
	m_LineColours[1].nGreen = tColours.nGreen;
	m_LineColours[1].nBlue = tColours.nBlue;
}

void LtcDisplayRgbPanel::ShowSource(const char *pSource, uint32_t nLength, struct TLtcDisplayRgbColours &tColours) {
	memcpy(m_Line[3], pSource, 8);

	m_LineColours[3].nRed = tColours.nRed;
	m_LineColours[3].nGreen = tColours.nGreen;
	m_LineColours[3].nBlue = tColours.nBlue;
}

void LtcDisplayRgbPanel::WriteChar(uint8_t nChar, uint8_t nPos, struct TLtcDisplayRgbColours &tColours) {
	DEBUG_ENTRY

	DEBUG_EXIT
}
