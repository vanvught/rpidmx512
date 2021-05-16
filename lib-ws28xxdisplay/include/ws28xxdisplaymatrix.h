/**
 * @file ws28xxdisplaymatrix.h
 *
 */
/* Copyright (C) 2019-2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef WS28XXDISPLAYMATRIX_H_
#define WS28XXDISPLAYMATRIX_H_

#include <cstdint>

#include "ws28xx.h"

#include "pixeltype.h"

struct TWS28xxDisplayMatrixColon {
	uint8_t nBits;
	uint8_t nRed;
	uint8_t nGreen;
	uint8_t nBlue;
};

class WS28xxDisplayMatrix {
public:
	WS28xxDisplayMatrix(uint32_t nColumns, uint32_t nRows, pixel::Type tLedType, pixel::Map tRGBMapping);
	~WS28xxDisplayMatrix();

	void PutChar(char nChar, uint8_t nRed, uint8_t nGreen, uint8_t nBlue);
	void PutString(const char *pString, uint8_t nRed, uint8_t nGreen, uint8_t nBlue);

	void Text(const char *pText, uint32_t nLength, uint8_t nRed, uint8_t nGreen, uint8_t nBlue);
	void TextLine(uint32_t nLine, const char *pText, uint32_t nLength, uint8_t nRed, uint8_t nGreen, uint8_t nBlue);

	void SetColon(char nChar, uint32_t nPos, uint8_t nRed, uint8_t nGreen, uint8_t nBlue);
	void SetColonsOff();

	void ClearLine(uint32_t nLine);

	void SetCursorPos(uint32_t nCol, uint32_t nRow);

	void Cls();
	void Show();

	uint32_t GetMaxPosition() {
		return m_nMaxPosition;
	}

	uint32_t GetMaxLine() {
		return m_nMaxLine;
	}

private:
	uint8_t ReverseBits(uint8_t nBits);

private:
	uint32_t m_nColumns;
	uint32_t m_nRows;
	uint32_t m_nOffset;
	uint32_t m_nMaxLeds;
	uint32_t m_nMaxPosition;
	uint32_t m_nMaxLine;
	WS28xx *m_pWS28xx { nullptr };
	bool m_bUpdateNeeded { false };
	uint32_t m_nPosition { 0 };
	uint32_t m_nLine { 0 };
	struct TWS28xxDisplayMatrixColon *m_ptColons { nullptr };
};

#endif /* WS28XXDISPLAYMATRIX_H_ */
