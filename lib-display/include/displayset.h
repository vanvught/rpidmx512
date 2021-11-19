/**
 * @file dislpayset.h
 *
 */
/* Copyright (C) 2017-2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef DISPLAYSET_H_
#define DISPLAYSET_H_

#include <cstdint>

namespace display {
namespace cursor {
static constexpr auto OFF = 0;
static constexpr auto ON = (1 << 0);
static constexpr auto BLINK_OFF = 0;
static constexpr auto BLINK_ON = (1 << 1);
}  // namespace cursor_mode
}  // namespace display

class DisplaySet {
public:
	virtual ~DisplaySet() {}

	uint8_t GetColumns() const {
		return m_nCols;
	}

	uint8_t GetRows() const {
		return m_nRows;
	}

	uint8_t GetContrast() const {
		return m_nContrast;
	}

	bool GetFlipVertically() const {
		return m_bIsFlippedVertically;
	}

	virtual bool Start()= 0;

	virtual void Cls()= 0;
	virtual void ClearLine(uint8_t nLine)= 0;

	virtual void PutChar(int)= 0;
	virtual void PutString(const char*)= 0;

	virtual void TextLine(uint8_t nLine, const char *pData, uint32_t nLength)= 0;

	virtual void SetCursorPos(uint8_t nCol, uint8_t nRow)= 0;
	virtual void SetCursor(uint32_t)= 0;

	virtual void SetSleep(__attribute__((unused)) bool bSleep) {}
	virtual void SetContrast(__attribute__((unused)) uint8_t nContrast) {}
	virtual void SetFlipVertically(__attribute__((unused)) bool doFlipVertically) {}

	virtual void PrintInfo() {}

protected:
	uint8_t m_nCols;
	uint8_t m_nRows;
	uint8_t m_nContrast { 0x7F };
	bool m_bIsFlippedVertically { false };
};

#endif /* DISPLAYSET_H_ */
