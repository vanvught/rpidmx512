/**
 * @file pixeldisplaymatrix.h
 */
/* Copyright (C) 2019-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include "pixeloutput.h"
#include "pixeltype.h"

struct TWS28xxDisplayMatrixColon {
	uint8_t bits;
	uint8_t red;
	uint8_t green;
	uint8_t blue;
};

class WS28xxDisplayMatrix {
public:
	WS28xxDisplayMatrix(uint8_t columns, uint8_t rows, pixel::LedType led_type, pixel::LedMap led_map);
	~WS28xxDisplayMatrix();

	void PutChar(char ch, uint8_t red, uint8_t green, uint8_t blue);
	void PutString(const char *string, uint8_t red, uint8_t green, uint8_t blue);

	void Text(const char *text, uint32_t length, uint8_t red, uint8_t green, uint8_t blue);
	void TextLine(uint8_t line, const char *text, uint32_t length, uint8_t red, uint8_t green, uint8_t blue);

	void SetColon(char ch, uint32_t pos, uint8_t red, uint8_t green, uint8_t blue);
	void SetColonsOff();

	void ClearLine(uint8_t line);

	void SetCursorPos(uint8_t col, uint8_t row);

	void Cls();
	void Show();

	uint32_t GetMaxPosition() const  {
		return max_position_;
	}

	uint32_t GetMaxLine() const {
		return max_line_;
	}

private:
	uint8_t ReverseBits(uint8_t bits);

private:
	uint8_t columns_;
	uint8_t rows_;
	uint32_t m_nOffset;
	uint16_t m_nMaxLeds;
	uint16_t max_position_;
	uint8_t max_line_;
	PixelOutput *m_pPixelOutput { nullptr };
	bool m_bUpdateNeeded { false };
	uint32_t position_ { 0 };
	uint8_t line_ { 0 };
	struct TWS28xxDisplayMatrixColon *colons_ { nullptr };
};

#endif /* WS28XXDISPLAYMATRIX_H_ */
