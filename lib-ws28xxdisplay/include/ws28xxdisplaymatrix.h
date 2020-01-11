/**
 * @file ws28xxdisplaymatrix.h
 *
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

#ifndef WS28XXDISPLAYMATRIX_H_
#define WS28XXDISPLAYMATRIX_H_

#include <stdint.h>
#include <stdbool.h>

#if defined(USE_SPI_DMA)
 #include "h3/ws28xxdma.h"
#endif
#include "ws28xx.h"

struct TWS28xxDisplayMatrixColon {
	uint8_t nBits;
	uint8_t nRed;
	uint8_t nGreen;
	uint8_t nBlue;
};

class WS28xxDisplayMatrix {
public:
	WS28xxDisplayMatrix(uint32_t nColumns, uint32_t nRows);
	~WS28xxDisplayMatrix(void);

	void Init(TWS28XXType tLedType = WS2812B);

	void PutChar(uint8_t nChar, uint8_t nRed = 0x10, uint8_t nGreen = 0x10, uint8_t nBlue = 0x10);
	void PutString(const char *pString, uint8_t nRed = 0x10, uint8_t nGreen = 0x10, uint8_t nBlue = 0x10);

	void Text(const char *pText, uint8_t nLength, uint8_t nRed = 0x10, uint8_t nGreen = 0x10, uint8_t nBlue = 0x10);
	void TextLine(uint8_t nLine, const char *pText, uint8_t nLength, uint8_t nRed = 0x10, uint8_t nGreen = 0x10, uint8_t nBlue = 0x10);

	void SetColon(uint8_t nChar, uint8_t nPos, uint8_t nRed = 0x10, uint8_t nGreen = 0x10, uint8_t nBlue = 0x10);
	void SetColonsOff(void);

	void ClearLine(uint8_t nLine);

	void SetCursorPos(uint8_t nCol, uint8_t nRow);

	void Cls(void);
	void Show(void);

	uint32_t GetMaxPosition(void) {
		return m_nMaxPosition;
	}

	uint32_t GetMaxLine(void) {
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
#if defined(USE_SPI_DMA)
	WS28xxDMA *m_pWS28xx;
#else
	WS28xx *m_pWS28xx;
#endif
	bool m_bUpdateNeeded;
	uint32_t m_nPosition;
	uint32_t m_nLine;
	struct TWS28xxDisplayMatrixColon *m_ptColons;
};

#endif /* WS28XXDISPLAYMATRIX_H_ */
