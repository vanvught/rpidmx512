/**
 * @file ws28xxmatrix.cpp
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
#include <string.h>
#include <cassert>

#include "ws28xxdisplaymatrix.h"

#if defined(USE_SPI_DMA)
# include "h3/ws28xxdma.h"
#else
# include "ws28xx.h"
#endif

#include "../lib-device/src/font_cp437.h"

#include "debug.h"

// FIXME Currently working for single row only

WS28xxDisplayMatrix::WS28xxDisplayMatrix(uint32_t nColumns, uint32_t nRows):
	m_nColumns(nColumns),
	m_nRows(nRows),
	m_nOffset((nRows - FONT_CP437_CHAR_H) * 2),
	m_nMaxLeds(nColumns * nRows),
	m_nMaxPosition(nColumns / FONT_CP437_CHAR_W),
	m_nMaxLine(nRows / FONT_CP437_CHAR_H)
{
	DEBUG2_ENTRY

	assert(nColumns % FONT_CP437_CHAR_W == 0);
	assert(nRows % FONT_CP437_CHAR_H == 0);

	m_ptColons = new struct TWS28xxDisplayMatrixColon[m_nMaxPosition];
	assert(m_ptColons != nullptr);

	SetColonsOff();

	DEBUG_PRINTF("m_nColumns=%u, m_nRows=%u, m_nOffset=%u, m_nMaxPosition=%u, m_nMaxLine=%u", m_nColumns, m_nRows, m_nOffset, m_nMaxPosition, m_nMaxLine);
	DEBUG2_EXIT
}

WS28xxDisplayMatrix::~WS28xxDisplayMatrix() {
	DEBUG2_ENTRY

	if (m_pWS28xx != nullptr) {
		Cls();

		delete m_pWS28xx;
		m_pWS28xx = nullptr;
	}

	delete[] m_ptColons;
	m_ptColons = nullptr;

	DEBUG2_EXIT
}

void WS28xxDisplayMatrix::Init(TWS28XXType tLedType, TRGBMapping tRGBMapping) {
	DEBUG2_ENTRY

	assert(m_pWS28xx == nullptr);
#if defined(USE_SPI_DMA)
	m_pWS28xx = new WS28xxDMA(tLedType, m_nMaxLeds, tRGBMapping);
#else
	m_pWS28xx = new WS28xx(tLedType, m_nMaxLeds, tRGBMapping);
#endif
	assert(m_pWS28xx != nullptr);

	m_pWS28xx->Initialize();

	DEBUG2_EXIT
}

void WS28xxDisplayMatrix::PutChar(char nChar, uint8_t nRed, uint8_t nGreen, uint8_t nBlue) {
	if (nChar >= cp437_font_size()) {
		nChar = ' ';
	}

	while (m_pWS28xx->IsUpdating()) {
		// wait for completion
	}

	uint32_t nOffset = (FONT_CP437_CHAR_W * FONT_CP437_CHAR_H) * m_nPosition;

	for (uint32_t nWidth = 0; nWidth < FONT_CP437_CHAR_W; nWidth++) {
		uint8_t nByte = cp437_font[static_cast<int>(nChar)][nWidth];

		//FIXME This can be optimized. See rgbpanel code

		if (nWidth == (FONT_CP437_CHAR_W - 1)) {
			if (m_ptColons[m_nPosition].nBits != 0) {
				nByte = m_ptColons[m_nPosition].nBits;
				nRed = m_ptColons[m_nPosition].nRed;
				nGreen = m_ptColons[m_nPosition].nGreen;
				nBlue = m_ptColons[m_nPosition].nBlue;
			}
		}

		if ((nWidth & 0x1) != 0) {
			nByte = ReverseBits(nByte);
		}

		for (uint32_t nHeight = 0; nHeight < FONT_CP437_CHAR_H; nHeight++) {
			if (nByte & (1 << nHeight)) {
				m_pWS28xx->SetLED(nOffset, nRed, nGreen, nBlue);
			} else {
				m_pWS28xx->SetLED(nOffset, 0x00, 0x00, 0x00);
			}

			nOffset++;
		}
	}

	m_nPosition++;

	if (m_nPosition == m_nMaxPosition ) {
		m_nPosition = 0;
		m_nLine++;

		if (m_nLine == m_nMaxLine) {
			m_nLine = 0;
		}
	}

	m_bUpdateNeeded = true;
}

void WS28xxDisplayMatrix::PutString(const char *pString, uint8_t nRed, uint8_t nGreen, uint8_t nBlue) {
	uint8_t nChar;

	while ((nChar = *pString++) != 0) {
		PutChar(nChar, nRed, nGreen, nBlue);
	}
}

void WS28xxDisplayMatrix::Text(const char *pText, uint8_t nLength, uint8_t nRed, uint8_t nGreen, uint8_t nBlue) {
	if (nLength > m_nMaxPosition) {
		nLength = m_nMaxPosition;
	}

	for (uint32_t i = 0; i < nLength; i++) {
		PutChar(pText[i], nRed, nGreen, nBlue);
	}
}

/*
 * 1 is top line
 */
void WS28xxDisplayMatrix::TextLine(uint8_t nLine, const char *pText, uint8_t nLength, uint8_t nRed, uint8_t nGreen, uint8_t nBlue) {
	if ((nLine == 0) || (nLine > m_nMaxLine)) {
		return;
	}

	SetCursorPos(0, nLine - 1);
	Text(pText, nLength, nRed, nGreen, nBlue);
}

/*
 * 1 is top line
 */
void WS28xxDisplayMatrix::ClearLine(uint8_t nLine) {
	if ((nLine == 0) || (nLine > m_nMaxLine)) {
		return;
	}

	while (m_pWS28xx->IsUpdating()) {
		// wait for completion
	}

	for (uint32_t i = 0; i < m_nMaxLeds; i++) {
		m_pWS28xx->SetLED(i, 0, 0, 0); // FIXME Currently working for single row only
	}

	SetCursorPos(0, nLine - 1);
}

/**
 * 0,0 is top left
 */
void WS28xxDisplayMatrix::SetCursorPos(uint8_t nCol, uint8_t nRow) {
	if ((nCol >= m_nMaxPosition) || (nRow >= m_nMaxLine)) {
		return;
	}

	m_nPosition = nCol;
	m_nLine = nRow;
}

void WS28xxDisplayMatrix::Cls() {
	while (m_pWS28xx->IsUpdating()) {
		// wait for completion
	}
	m_pWS28xx->Blackout();
}

void WS28xxDisplayMatrix::Show() {
	if (m_bUpdateNeeded) {
		m_bUpdateNeeded = false;
		m_pWS28xx->Update();
	}
}

void WS28xxDisplayMatrix::SetColon(char nChar, uint8_t nPos, uint8_t nRed, uint8_t nGreen, uint8_t nBlue) {
	if (nPos >= m_nMaxPosition) {
		return;
	}

	switch (nChar) {
		case ':':
			m_ptColons[nPos].nBits = 0x66;
			break;
		case '.':
			m_ptColons[nPos].nBits = 0x60;
			break;
		default:
			m_ptColons[nPos].nBits = 0;
			break;
	}

	m_ptColons[nPos].nRed = nRed;
	m_ptColons[nPos].nBlue = nBlue;
	m_ptColons[nPos].nGreen = nGreen;
}

void WS28xxDisplayMatrix::SetColonsOff() {
	for (uint32_t nPos = 0; nPos < m_nMaxPosition; nPos++) {
		m_ptColons[nPos].nBits = 0;
		m_ptColons[nPos].nRed = 0;
		m_ptColons[nPos].nGreen = 0;
		m_ptColons[nPos].nBlue = 0;
	}
}

uint8_t WS28xxDisplayMatrix::ReverseBits(uint8_t nBits) {
#if defined (H3)
	const auto input = static_cast<uint32_t>(nBits);
	uint32_t output;
	asm("rbit %0, %1" : "=r"(output) : "r"(input));
	return static_cast<uint8_t>((output >> 24));
#else
	// http://graphics.stanford.edu/~seander/bithacks.html#ReverseByteWith64Bits
	const uint8_t nResult = ((nBits * 0x80200802ULL) & 0x0884422110ULL) * 0x0101010101ULL >> 32;
	return nResult;
#endif
}

