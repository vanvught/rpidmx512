/**
 * @file display.h
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

#ifndef DISPLAY_H_
#define DISPLAY_H_

#include <stdarg.h>
#include <cstdint>

#include "displayset.h"
#include "display7segment.h"

#if defined (BARE_METAL)
# include "console.h"
#endif

namespace display {
struct Defaults {
	static constexpr auto SEEP_TIMEOUT = 5;
};
}  // namespace display

enum class DisplayType {
	PCF8574T_1602,
	PCF8574T_2004,
	SSD1306,
	SSD1311,
	UNKNOWN
};

class Display {
public:
	Display();
	Display(uint8_t nCols, uint8_t nRows);
	Display(DisplayType tDisplayType);
	~Display() {
		s_pThis = nullptr;
		delete m_LcdDisplay;
	}

#if !defined(NO_HAL)
	void SetSleep(bool bSleep);
	bool isSleep() {
		return m_bIsSleep;
	}

	void Run();
#endif

	void Cls() {
		if (m_LcdDisplay == nullptr) {
			return;
		}

		m_LcdDisplay->Cls();
	}

	void ClearLine(uint8_t nLine) {
		if (m_LcdDisplay == nullptr) {
			return;
		}

		m_LcdDisplay->ClearLine(nLine);
	}

	void PutChar(int c) {
		if (m_LcdDisplay == nullptr) {
			return;
		}

		m_LcdDisplay->PutChar(c);
	}

	void PutString(const char *pText) {
		if (m_LcdDisplay == nullptr) {
			return;
		}

		m_LcdDisplay->PutString(pText);
	}

	int Write(uint8_t nLine, const char *);
	int Printf(uint8_t nLine, const char *, ...);

	void TextLine(uint8_t nLine, const char *pText, uint8_t nLength) {
		if (m_LcdDisplay == nullptr) {
			return;
		}

		m_LcdDisplay->TextLine(nLine, pText, nLength);
	}

	void TextStatus(const char *pText);
	void TextStatus(const char *pText, Display7SegmentMessage msg, uint32_t nConsoleColor = UINT32_MAX);
	void TextStatus(const char *pText, uint8_t nValue7Segment, bool bHex = false);

	bool isDetected() const {
		return m_LcdDisplay == nullptr ? false : true;
	}

	DisplayType GetDetectedType() const {
		return m_tType;
	}

	void SetCursor(uint32_t nMode);
	void SetCursorPos(uint8_t nCol, uint8_t nRow);

	void SetSleepTimeout(uint32_t nSleepTimeout = display::Defaults::SEEP_TIMEOUT) {
		m_nSleepTimeout = 1000 * 60 * nSleepTimeout;
	}
	uint32_t GetSleepTimeout() const {
		return m_nSleepTimeout / 1000 / 60;
	}

	void SetContrast(uint8_t nContrast) {
		if (m_LcdDisplay == nullptr) {
			return;
		}

		m_LcdDisplay->SetContrast(nContrast);
	}

	void DoFlipVertically() {
		if (m_LcdDisplay == nullptr) {
			return;
		}

		m_LcdDisplay->DoFlipVertically();
	}

	uint8_t getCols() const {
		return m_nCols;
	}

	uint8_t getRows() const {
		return m_nRows;
	}

	void PrintInfo();

	static Display* Get() {
		return s_pThis;
	}

private:
	void Detect(DisplayType tDisplayType);
	void Detect(uint8_t nCols, uint8_t nRows);

private:
	uint8_t m_nCols { 0 };
	uint8_t m_nRows { 0 };
	DisplayType m_tType { DisplayType::UNKNOWN };
	DisplaySet *m_LcdDisplay { nullptr };
	bool m_bIsSleep { false };
#if !defined(NO_HAL)
	uint32_t m_nMillis { 0 };
#endif
	uint32_t m_nSleepTimeout { 1000 * 60 * display::Defaults::SEEP_TIMEOUT };

	Display7Segment m_Display7Segment;

	static Display *s_pThis;
};

#endif /* DISPLAY_H_ */
