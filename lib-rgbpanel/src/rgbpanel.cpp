/**
 * @file rgbpanel.cpp
 *
 */
/* Copyright (C) 2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <cassert>
#include <algorithm>
#include <stdint.h>
#include <stdio.h>

#include "rgbpanel.h"

#include "../../lib-device/src/font_cp437.h"

#include "debug.h"

using namespace rgbpanel;

RgbPanel::RgbPanel(uint32_t nColumns, uint32_t nRows, uint32_t nChain, Types tType):
	m_nColumns(nColumns),
	m_nRows(nRows),
	m_nChain(nChain != 0 ? nChain : 1),
	m_tType(tType),
	// Text
	m_nMaxPosition(nColumns / FONT_CP437_CHAR_W),
	m_nMaxLine(nRows / FONT_CP437_CHAR_H)
{
	PlatformInit();

	// Text
	assert(nColumns % FONT_CP437_CHAR_W == 0);
	assert(nRows % FONT_CP437_CHAR_H == 0);

	m_ptColons = new struct TColon[m_nMaxPosition * m_nMaxLine];
	assert(m_ptColons != nullptr);
	SetColonsOff();
}

/**
 * Text
 */
void RgbPanel::PutChar(char nChar, uint8_t nRed, uint8_t nGreen, uint8_t nBlue) {
	if (__builtin_expect((static_cast<uint32_t>(nChar) >= cp437_font_size()), 0)) {
		nChar = ' ';
	}

	const auto nStartColumn = m_nPosition * FONT_CP437_CHAR_W;
	auto nRow = m_nLine * m_nMaxPosition;
	const auto nColonIndex = m_nPosition + nRow;
	const bool bShowColon = (m_ptColons[nColonIndex].nBits != 0);

	for (uint32_t i = 0; i < FONT_CP437_CHAR_H; i++) {
		uint32_t nWidth = 0;

		for (uint32_t nColumn = nStartColumn; nColumn < (FONT_CP437_CHAR_W + nStartColumn); nColumn++) {

			if ((bShowColon) && (nColumn == (nStartColumn + FONT_CP437_CHAR_W - 1))) {
				const uint8_t nByte = m_ptColons[nColonIndex].nBits >> i;

				if ((nByte & 0x1) != 0) {
					SetPixel(nColumn, nRow, m_ptColons[nColonIndex].nRed, m_ptColons[nColonIndex].nGreen, m_ptColons[nColonIndex].nBlue);
				} else {
					SetPixel(nColumn, nRow, 0, 0, 0);
				}

				continue;
			}

			const auto nByte = cp437_font[static_cast<int>(nChar)][nWidth++] >> i;

			if ((nByte & 0x1) != 0) {
				SetPixel(nColumn, nRow, nRed, nGreen, nBlue);
			} else {
				SetPixel(nColumn, nRow, 0, 0, 0);
			}

		}

		nRow++;
	}

	m_nPosition++;

	if (m_nPosition == m_nMaxPosition ) {
		m_nPosition = 0;
		m_nLine++;

		if (m_nLine == m_nMaxLine) {
			m_nLine = 0;
		}
	}
}

void RgbPanel::PutString(const char *pString, uint8_t nRed, uint8_t nGreen, uint8_t nBlue) {
	char nChar;

	while ((nChar = *pString++) != 0) {
		PutChar(nChar, nRed, nGreen, nBlue);
	}
}

void RgbPanel::Text(const char *pText, uint8_t nLength, uint8_t nRed, uint8_t nGreen, uint8_t nBlue) {
	if (__builtin_expect((nLength > m_nMaxPosition), 0)) {
		nLength = m_nMaxPosition;
	}

	for (uint32_t i = 0; i < nLength; i++) {
		PutChar(pText[i], nRed, nGreen, nBlue);
	}
}

/**
 * 1 is top line
 */
void RgbPanel::TextLine(uint8_t nLine, const char *pText, uint8_t nLength, uint8_t nRed, uint8_t nGreen, uint8_t nBlue) {
	if (__builtin_expect(((nLine == 0) || (nLine > m_nMaxLine)), 0)) {
		return;
	}

	SetCursorPos(0, nLine - 1);
	Text(pText, nLength, nRed, nGreen, nBlue);
}

/**
 * 1 is top line
 */
void RgbPanel::ClearLine(uint8_t nLine) {
	if (__builtin_expect(((nLine == 0) || (nLine > m_nMaxLine)), 0)) {
		return;
	}

	const auto nStartRow = (nLine - 1U) * m_nMaxPosition;

	for (uint32_t nRow = nStartRow; nRow < (nStartRow + FONT_CP437_CHAR_H); nRow++) {
		for (uint32_t nColumn = 0; nColumn < m_nColumns; nColumn++) {
			SetPixel(nColumn, nRow, 0, 0, 0);
		}
	}

	SetCursorPos(0, nLine - 1);
}

/**
 * 0,0 is top left
 */
void RgbPanel::SetCursorPos(uint8_t nCol, uint8_t nRow) {
	if (__builtin_expect(((nCol >= m_nMaxPosition) || (nRow >= m_nMaxLine)), 0)) {
		return;
	}

	m_nPosition = nCol;
	m_nLine = nRow;
}

void RgbPanel::SetColon(char nChar, uint8_t nCol, uint8_t nRow, uint8_t nRed, uint8_t nGreen, uint8_t nBlue) {
	if (__builtin_expect(((nCol >= m_nMaxPosition) || (nRow >= m_nMaxLine)), 0)) {
		return;
	}

	const auto nIndex = nCol + (nRow * m_nMaxPosition);

	switch (nChar) {
	case ':':
	case ';':
		m_ptColons[nIndex].nBits = 0x66;
		break;
	case '.':
	case ',':
		m_ptColons[nIndex].nBits = 0x60;
		break;
	default:
		m_ptColons[nIndex].nBits = 0;
		break;
	}

	m_ptColons[nIndex].nRed = nRed;
	m_ptColons[nIndex].nBlue = nBlue;
	m_ptColons[nIndex].nGreen = nGreen;
}

void RgbPanel::SetColonsOff() {
	for (uint32_t nIndex = 0; nIndex < m_nMaxPosition * m_nMaxLine; nIndex++) {
		m_ptColons[nIndex].nBits = 0;
		m_ptColons[nIndex].nRed = 0;
		m_ptColons[nIndex].nGreen = 0;
		m_ptColons[nIndex].nBlue = 0;
	}
}

/**
 * void RgbPanel::Start() is implemented as platform specific
 */

void RgbPanel::Stop() {
	if (!m_bIsStarted) {
		return;
	}

	m_bIsStarted = false;

	Cls();
	Show();
}

void RgbPanel::Print() {
	printf("RGB led panel\n");
	printf(" %ux%ux%u\n", m_nColumns, m_nRows, m_nChain);
}
