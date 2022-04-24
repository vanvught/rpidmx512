/**
 * @file paint.h
 *
 */
/* Copyright (C) 2022 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef PAINT_H
#define PAINT_H

#include "spi/lcd_font.h"
#include "spi/config.h"

#include "debug.h"

class Paint {
public:
	Paint();
	virtual ~Paint();

	uint16_t GetWidth() const {
		return m_nWidth;
	}

	uint16_t GetHeight() const {
		return m_nHeight;
	}

	void FillColour(uint16_t nColor);
	void Fill(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t nColour);

	void DrawPixel(uint16_t x, uint16_t y, uint16_t nColour);
	void DrawChar(uint16_t x0, uint16_t y0, const char c, sFONT* pFont, uint16_t nColourBackground, uint16_t nColourForeground);
	void DrawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t nColour);

private:
	virtual void SetAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)=0;

private:
	void SetCursor(uint16_t x, uint16_t y) {
		SetAddressWindow(x, y, x, y);
	}

protected:
	uint16_t m_nWidth;
	uint16_t m_nHeight;
	uint16_t m_nRotate { 0 };
};

#endif /* PAINT_H */
