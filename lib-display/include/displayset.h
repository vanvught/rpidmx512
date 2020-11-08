/**
 * @file dislpayset.h
 *
 */
/* Copyright (C) 2017-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <stdint.h>

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
	virtual ~DisplaySet() {
	}

	uint8_t GetColumns() {
		return m_nCols;
	}

	uint8_t GetRows() {
		return m_nRows;
	}

	virtual bool Start()= 0;
	virtual void Cls()= 0;
	virtual void PutChar(int)= 0;
	virtual void PutString(const char*)= 0;
	virtual void TextLine(uint8_t, const char*, uint8_t)= 0;
	virtual void ClearLine(uint8_t)= 0;
	virtual void SetSleep(__attribute__((unused)) bool bSleep) {
	}

	virtual void SetCursorPos(uint8_t, uint8_t)= 0;
	virtual void SetCursor(uint32_t)= 0;

	virtual void PrintInfo() {
	}

protected:
	uint8_t m_nCols;
	uint8_t m_nRows;
};

#endif /* DISPLAYSET_H_ */
