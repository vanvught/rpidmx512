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
/**
 * PoC
 */

#include <cassert>
#include <algorithm>
#include <stdint.h>
#include <stdio.h>

#include "rgbpanel.h"


#include "../../lib-device/src/font_cp437.h"
#include "../../lib-device/src/font_5x8.h"

#include "debug.h"

using namespace rgbpanel;

RgbPanel::RgbPanel(uint32_t nColumns, uint32_t nRows, uint32_t nChain, RgbPanelTypes tType):
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

	m_ptColons = new struct TColon[m_nMaxPosition * m_nMaxLine];  // TODO: Support smaller font 
	assert(m_ptColons != nullptr);
	SetColonsOff();

}

/**
 * Text
 */
void RgbPanel::PutChar(char nChar, uint8_t nRed, uint8_t nGreen, uint8_t nBlue) {

	uint32_t font_size = 0;
	uint32_t font_width = 0;
	uint32_t font_height = 8;
	uint32_t font_space = 0;

	if (m_TFontID == rgbpanel::TFontID::FONT_8x8){
		font_size = cp437_font_size();
		font_width = FONT_CP437_CHAR_W;
	} else if (m_TFontID == rgbpanel::TFontID::FONT_5x8) {
		font_size = font_5x8_size();
		font_width = FONT_5x8_CHAR_W;
		font_space = 1;		
	}

	if (__builtin_expect((static_cast<uint32_t>(nChar) >= font_size), 0)) {
		nChar = ' ';
	}

	auto nStartColumn = m_nPosition * (font_width + font_space);
	auto nRow = m_nLine * font_height; //* m_nMaxPosition;
	const auto nColonIndex = m_nPosition + nRow;
	const bool bShowColon = (m_ptColons[nColonIndex].nBits != 0);

	for (uint32_t i = 0; i < font_height; i++) {
		uint32_t nWidth = font_space;

		for (uint32_t nColumn = nStartColumn; nColumn < ((font_width) + nStartColumn); nColumn++) {
			
			if ((m_TFontID == rgbpanel::TFontID::FONT_8x8) && (bShowColon) && (nColumn == (nStartColumn + font_width - 1))) {
				const uint8_t nByte = m_ptColons[nColonIndex].nBits >> i;

				if ((nByte & 0x1) != 0) {
					SetPixel(nColumn, nRow, m_ptColons[nColonIndex].nRed, m_ptColons[nColonIndex].nGreen, m_ptColons[nColonIndex].nBlue);
				} else {
					SetPixel(nColumn, nRow, 0, 0, 0);
				}

				continue;
			}

			uint8_t nByte = 0;
			if (m_TFontID == rgbpanel::TFontID::FONT_8x8){
				nByte = cp437_font[static_cast<int>(nChar)][nWidth++] >> i;
			} else if (m_TFontID == rgbpanel::TFontID::FONT_5x8) {
				nByte = font_5x8[static_cast<int>(nChar)-127][(nWidth++)] >> i;																					
			}
			 
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
	
	// TODO: Optimize away, they are all currently 8 high,  only width varies. 
	switch (m_LineFont[nLine-1])
		{
		case rgbpanel::TFontID::FONT_8x8:
			m_nMaxPosition = m_nColumns / FONT_CP437_CHAR_W;
			m_nMaxLine = m_nRows / FONT_CP437_CHAR_H;		
			break;
		
		case rgbpanel::TFontID::FONT_5x8:
			m_nMaxPosition = m_nColumns / FONT_5x8_CHAR_W;
			m_nMaxLine = m_nRows / FONT_5x8_CHAR_H;
			break;
		
		default:
			m_nMaxPosition = m_nColumns / FONT_CP437_CHAR_W;
			m_nMaxLine = m_nRows / FONT_CP437_CHAR_H;
			break;
		}
	m_TFontID = m_LineFont[nLine - 1];
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

	uint32_t font_height = 8;

	for (uint32_t nRow = nStartRow; nRow < (nStartRow + font_height); nRow++) {
		for (uint32_t nColumn = 0; nColumn < m_nColumns; nColumn++) {
			SetPixel(nColumn, nRow, 0, 0, 0);
		}		
	}

	SetCursorPos(0, nLine - 1);
}

/**
 * 0 is 8x8 with 0 column spacing 
 * 1 is 5x8 with 1 column spacing
 */	
void RgbPanel::SetFont(uint8_t nLine, rgbpanel::TFontID font_id) {
	if (__builtin_expect((nLine >= m_nMaxPosition), 0)) {
		return;
	}	
	m_LineFont[nLine] = font_id;	

	// TODO: Optimize away, they are all currently 8 high,  only width varies. 
	switch (m_LineFont[nLine-1])
		{
		case rgbpanel::TFontID::FONT_8x8:
			m_nMaxPosition = m_nColumns / FONT_CP437_CHAR_W;
			m_nMaxLine = m_nRows / FONT_CP437_CHAR_H;		
			break;
		
		case rgbpanel::TFontID::FONT_5x8:
			m_nMaxPosition = m_nColumns / FONT_5x8_CHAR_W;
			m_nMaxLine = m_nRows / FONT_5x8_CHAR_H;
			break;
		
		default:
			m_nMaxPosition = m_nColumns / FONT_CP437_CHAR_W;
			m_nMaxLine = m_nRows / FONT_CP437_CHAR_H;
			break;
		}	
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

void RgbPanel::SetColon(uint8_t nChar, uint8_t nCol, uint8_t nRow, uint8_t nRed, uint8_t nGreen, uint8_t nBlue) {
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

