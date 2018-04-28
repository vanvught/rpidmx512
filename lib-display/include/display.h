/**
 * @file lcd.h
 *
 */
/* Copyright (C) 2017 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#ifndef LCD_H_
#define LCD_H_

#include <stdbool.h>
#include <stdarg.h>
#include <stdint.h>

#include "displayset.h"

enum TDisplayTypes {
	DISPLAY_BW_UI_1602 = 0,
	DISPLAY_BW_LCD_1602,
	DISPLAY_PCF8574T_1602,
	DISPLAY_PCF8574T_2004,
	DISPLAY_SSD1306,
	DISPLAY_TYPE_UNKNOWN = -1
};

#define DISPLAY_CONNECTED(b,f)	\
do {						\
	if(b) {					\
		(void) f;			\
	}						\
} while (0);

class Display {
public:
	Display(void);
	Display(uint8_t, uint8_t);
	Display(TDisplayTypes);
	~Display(void);

	void Cls(void);
	void ClearLine(uint8_t);

	void PutChar(int);
	void PutString(const char *);

	void TextLine(uint8_t, const char *, uint8_t);
	void TextStatus(const char *);

	uint8_t Write(uint8_t, const char *);
	uint8_t Printf(uint8_t, const char *, ...);

	bool isDetected(void) const;
	TDisplayTypes GetDetectedType(void) const;

	void SetCursor(TCursorMode);
	void SetCursorPos(uint8_t, uint8_t);

	static Display *Get (void);

	inline uint8_t getNCols(void)  {
		return m_nCols;
	}

	inline uint8_t getNRows(void) {
		return m_nRows;
	}

protected:
	uint8_t m_nCols;
	uint8_t m_nRows;

private:
	void Detect(uint8_t, uint8_t);

private:
	TDisplayTypes m_tType;
	DisplaySet *m_LcdDisplay;

	static Display *s_pThis;
};

#endif /* LCD_H_ */
