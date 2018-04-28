/**
 * @file lcd.cpp
 *
 */
/* Copyright (C) 2017-2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include <stdint.h>
#include <stdio.h>

#include "displayset.h"
#include "display.h"

#include "lcdbw.h"
#include "tc1602.h"
#include "ssd1306.h"

#include "i2c.h"

Display *Display::s_pThis = 0;

Display::Display(void): m_nCols(0), m_nRows(0), m_tType(DISPLAY_TYPE_UNKNOWN), m_LcdDisplay(0) {
	s_pThis = this;
	Detect(16,2);
}

Display::Display(const uint8_t nCols, const uint8_t nRows) {
	s_pThis = this;
	Detect(nCols, nRows);
}

Display::Display(TDisplayTypes tDisplayType): m_nCols(0), m_nRows(0), m_LcdDisplay(0) {
	s_pThis = this;
	m_tType = tDisplayType;

	switch (tDisplayType) {
		case DISPLAY_BW_UI_1602:
			m_LcdDisplay = new LcdBw(BW_UI_DEFAULT_SLAVE_ADDRESS, 16, 2);
			break;
		case DISPLAY_BW_LCD_1602:
			m_LcdDisplay = new LcdBw(BW_LCD_DEFAULT_SLAVE_ADDRESS, 16, 2);
			break;
		case DISPLAY_PCF8574T_1602:
			m_LcdDisplay = new Tc1602(16, 2);
			break;
		case DISPLAY_PCF8574T_2004:
			m_LcdDisplay = new Tc1602(20, 4);
			break;
		case DISPLAY_SSD1306:
			m_LcdDisplay = new Ssd1306(OLED_PANEL_128x64_8ROWS);
			break;
		case DISPLAY_TYPE_UNKNOWN:
			/* no break */
		default:
			break;
	}

	if (m_LcdDisplay != 0) {
		if (!m_LcdDisplay->Start()) {
			m_LcdDisplay = 0;
		}
	}
}

void Display::Detect(uint8_t nCols, uint8_t nRows) {
	m_nCols = nCols;
	m_nRows = nRows;
	m_LcdDisplay = 0;
	m_tType = DISPLAY_TYPE_UNKNOWN;

	if(!i2c_begin()) {
		return;
	}

	if (i2c_is_connected(OLED_I2C_SLAVE_ADDRESS_DEFAULT)) {
		switch (nRows) {
		case 4:
			m_LcdDisplay = new Ssd1306(OLED_PANEL_128x64_4ROWS);
			break;
		default:
			m_LcdDisplay = new Ssd1306(OLED_PANEL_128x64_8ROWS);
			break;
		}
		if (m_LcdDisplay->Start()) {
			m_tType = DISPLAY_SSD1306;
			Printf(1, "SSD1306");
		}
	} else if (i2c_is_connected(TC1602_I2C_DEFAULT_SLAVE_ADDRESS)) {
		m_LcdDisplay = new Tc1602(m_nCols, m_nRows);
		if (m_LcdDisplay->Start()) {
			m_tType = DISPLAY_PCF8574T_1602;
			Printf(1, "TC1602_PCF8574T");
		}
	} else if (i2c_is_connected(BW_LCD_DEFAULT_SLAVE_ADDRESS >> 1)) {
		m_LcdDisplay = new LcdBw(BW_LCD_DEFAULT_SLAVE_ADDRESS, m_nCols, m_nRows);
		if (m_LcdDisplay->Start()) {
			m_tType = DISPLAY_BW_LCD_1602;
			Printf(1, "BW_LCD");
		}
	} else if (i2c_is_connected(BW_UI_DEFAULT_SLAVE_ADDRESS >> 1)) {
		m_LcdDisplay = new LcdBw(BW_UI_DEFAULT_SLAVE_ADDRESS, m_nCols, m_nRows);
		if (m_LcdDisplay->Start()) {
			m_tType = DISPLAY_BW_UI_1602;
			Printf(1, "BW_UI");
		}
	} else {
#ifndef BARE_METAL
		puts("Unknown or no display attached");
#endif
		return;
	}
}

Display::~Display(void) {
	s_pThis = 0;
	delete m_LcdDisplay;
}

void Display::Cls(void) {
	if (m_LcdDisplay == 0) return;
	m_LcdDisplay->Cls();
}

void Display::TextLine(const uint8_t nLine, const char *pText, uint8_t nLength) {
	if (m_LcdDisplay == 0) return;
	m_LcdDisplay->TextLine(nLine, pText, nLength);
}

void Display::TextStatus(const char *pText) {
	ClearLine(m_nRows);
	Write(m_nRows, pText);
}

uint8_t Display::Printf(const uint8_t nLine, const char *format, ...) {
	int i;
	char buffer[32];

	if (m_LcdDisplay == 0) return 0;

	va_list arp;

	va_start(arp, format);

	i = vsnprintf(buffer, sizeof(buffer) / sizeof(buffer[0]), format, arp);

	va_end(arp);

	m_LcdDisplay->TextLine(nLine, buffer, i);

	return i;
}

bool Display::isDetected(void) const {
	return m_LcdDisplay == 0 ? false : true;
}

TDisplayTypes Display::GetDetectedType(void) const {
	return m_tType;
}

uint8_t Display::Write(uint8_t nLine, const char *pText) {
	const char *p = pText;

	if (m_LcdDisplay == 0) return 0;

	while (*p != (char)0) {
		++p;
	}

	const size_t nLength =  (size_t) (p - pText);

	m_LcdDisplay->TextLine(nLine, pText, nLength);

	return (uint8_t) nLength;
}

void Display::SetCursor(TCursorMode constEnumTCursorOnOff) {
	if (m_LcdDisplay == 0) return;
	m_LcdDisplay->SetCursor(constEnumTCursorOnOff);
}

void Display::SetCursorPos(uint8_t col, uint8_t row) {
	if (m_LcdDisplay == 0) return;
	m_LcdDisplay->SetCursorPos(col, row);
}

void Display::PutChar(int c) {
	if (m_LcdDisplay == 0) return;
	m_LcdDisplay->PutChar(c);
}

void Display::PutString(const char *pText) {
	if (m_LcdDisplay == 0) return;
	m_LcdDisplay->PutString(pText);
}

void Display::ClearLine(uint8_t nLine) {
	if (m_LcdDisplay == 0) return;
	m_LcdDisplay->ClearLine(nLine);
}

Display* Display::Get(void) {
	return s_pThis;
}
