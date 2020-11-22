/**
 * @file display.h
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

#ifndef DISPLAY_H_
#define DISPLAY_H_

#include <stdarg.h>
#include <stdint.h>

#include "displayset.h"

#include "display7segment.h"

#if defined (BARE_METAL)
# include "console.h"
#endif

static constexpr uint32_t DISPLAY_SLEEP_TIMEOUT_DEFAULT	= 5;

enum class DisplayType {
	BW_UI_1602,
	BW_LCD_1602,
	PCF8574T_1602,
	PCF8574T_2004,
	SSD1306,
	SSD1311,
	UNKNOWN
};

class Display {
public:
	Display(uint32_t nCols, uint32_t nRows);
	Display(DisplayType tDisplayType = DisplayType::SSD1306);
	~Display();

#if !defined(NO_HAL)
	void SetSleep(bool bSleep);
	bool isSleep() {
		return m_bIsSleep;
	}

	void Run();
#endif

	void Cls();
	void ClearLine(uint8_t nLine);

	void PutChar(int c);
	void PutString(const char *pText);

	int Write(uint8_t, const char *);
	int Printf(uint8_t, const char *, ...);

	void TextLine(uint8_t, const char *, uint8_t);

	void TextStatus(const char *pText);
	void TextStatus(const char *pText, Display7SegmentMessage msg, uint32_t nConsoleColor = UINT32_MAX);
	void TextStatus(const char *pText, uint8_t nValue7Segment, bool bHex = false);

	bool isDetected() {
		return m_LcdDisplay == nullptr ? false : true;
	}

	DisplayType GetDetectedType() {
		return m_tType;
	}

	void SetCursor(uint32_t nMode);
	void SetCursorPos(uint8_t nCol, uint8_t nRow);

	void SetSleepTimeout(uint32_t nSleepTimeout = DISPLAY_SLEEP_TIMEOUT_DEFAULT) {
		m_nSleepTimeout = 1000 * 60 * nSleepTimeout;
	}
	uint32_t GetSleepTimeout() {
		return m_nSleepTimeout / 1000 / 60;
	}

	uint32_t getCols() {
		return m_nCols;
	}

	uint32_t getRows() {
		return m_nRows;
	}

	void PrintInfo();

	static Display* Get() {
		return s_pThis;
	}

private:
	void Detect(uint32_t nCols, uint32_t nRows);

private:
	uint32_t m_nCols{0};
	uint32_t m_nRows{0};
	DisplayType m_tType;
	DisplaySet *m_LcdDisplay{nullptr};
	bool m_bIsSleep{false};
#if !defined(NO_HAL)
	uint32_t m_nMillis;
#endif
	uint32_t m_nSleepTimeout;

	Display7Segment m_Display7Segment;

	static Display *s_pThis;
};

#endif /* DISPLAY_H_ */
