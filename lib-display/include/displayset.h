/**
 * @file dislpayset.h
 *
 */
/* Copyright (C) 2017-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
static constexpr uint32_t OFF = 0;
static constexpr uint32_t ON = (1U << 0);
static constexpr uint32_t BLINK_OFF = 0;
static constexpr uint32_t BLINK_ON = (1U << 1);
}  // namespace cursor_mode
}  // namespace display

class DisplaySet {
public:
	virtual ~DisplaySet() = default;

	uint32_t GetColumns() const {
		return m_nCols;
	}

	uint32_t GetRows() const {
		return m_nRows;
	}

	void ClearEndOfLine() {
		m_bClearEndOfLine = true;
	}

	virtual bool Start()= 0;

	virtual void Cls()= 0;
	virtual void ClearLine(uint32_t nLine)= 0;

	virtual void PutChar(int)= 0;
	virtual void PutString(const char*)= 0;

	virtual void TextLine(uint32_t nLine, const char *pData, uint32_t nLength)= 0;

	virtual void SetCursorPos(uint32_t nCol, uint32_t nRow)= 0;
	virtual void SetCursor(uint32_t)= 0;

	virtual void SetSleep([[maybe_unused]] bool bSleep) {}
	virtual void SetContrast([[maybe_unused]] uint8_t nContrast) {}
	virtual void SetFlipVertically([[maybe_unused]] bool doFlipVertically) {}

	virtual void PrintInfo() {}

protected:
	uint32_t m_nCols;
	uint32_t m_nRows;
	bool m_bClearEndOfLine { false };
};

#endif /* DISPLAYSET_H_ */
