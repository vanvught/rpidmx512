/**
 * @file display.cpp
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

#include <stdint.h>
#include <stdio.h>
#include <cassert>

#include "displayset.h"
#include "display.h"

#if defined(ENABLE_LCDBW)
# include "lcdbw.h"
#endif
#if defined(ENABLE_TC1602)
# include "tc1602.h"
#endif
#include "ssd1306.h"
#if defined(ENABLE_SSD1311)
# include "ssd1311.h"
#endif

#include "display7segment.h"

#include "hal_i2c.h"

#if !defined(NO_HAL)
# include "hardware.h"
#endif

#if defined (BARE_METAL)
# include "console.h"
#endif

Display *Display::s_pThis = nullptr;

Display::Display(uint32_t nCols, uint32_t nRows):
	m_tType(DisplayType::UNKNOWN),
#if !defined(NO_HAL)
	m_nMillis(Hardware::Get()->Millis()),
#endif
	m_nSleepTimeout(1000 * 60 * DISPLAY_SLEEP_TIMEOUT_DEFAULT)
{
	assert(s_pThis == nullptr);
	s_pThis = this;

	Detect(nCols, nRows);
}

Display::Display(DisplayType tDisplayType):
#if !defined(NO_HAL)
	m_nMillis(Hardware::Get()->Millis()),
#endif
	m_nSleepTimeout(1000 * 60 * DISPLAY_SLEEP_TIMEOUT_DEFAULT)
{
	assert(s_pThis == nullptr);
	s_pThis = this;

	m_tType = tDisplayType;

	switch (tDisplayType) {
#if defined(ENABLE_LCDBW)
		case DisplayType::BW_UI_1602:
			m_LcdDisplay = new LcdBw(BW_UI_DEFAULT_SLAVE_ADDRESS, 16, 2);
			break;
		case DisplayType::BW_LCD_1602:
			m_LcdDisplay = new LcdBw(BW_LCD_DEFAULT_SLAVE_ADDRESS, 16, 2);
			break;
#endif
#if defined(ENABLE_TC1602)
		case DisplayType::PCF8574T_1602:
			m_LcdDisplay = new Tc1602(16, 2);
			break;
		case DisplayType::PCF8574T_2004:
			m_LcdDisplay = new Tc1602(20, 4);
			break;
#endif
#if defined(ENABLE_SSD1311)
		case DisplayType::SSD1311:
			m_LcdDisplay = new Ssd1311;
			break;
#endif
		case DisplayType::SSD1306:
			m_LcdDisplay = new Ssd1306(OLED_PANEL_128x64_8ROWS);
			break;
		case DisplayType::UNKNOWN:
			m_tType = DisplayType::UNKNOWN;
			/* no break */
		default:
			break;
	}

	if (m_LcdDisplay != nullptr) {
		if (!m_LcdDisplay->Start()) {
			delete m_LcdDisplay;
			m_LcdDisplay = nullptr;
			m_tType = DisplayType::UNKNOWN;
		} else {
			m_LcdDisplay->Cls();
			m_nCols = m_LcdDisplay->GetColumns();
			m_nRows = m_LcdDisplay->GetRows();
		}
	}

	if (m_LcdDisplay == nullptr){
		m_nSleepTimeout = 0;
	}
}

void Display::Detect(uint32_t nCols, uint32_t nRows) {
	m_nCols = nCols;
	m_nRows = nRows;
	m_LcdDisplay = nullptr;
	m_tType = DisplayType::UNKNOWN;

	if (HAL_I2C::IsConnected(OLED_I2C_SLAVE_ADDRESS_DEFAULT)) {
		if (nRows <= 4) {
			m_LcdDisplay = new Ssd1306(OLED_PANEL_128x64_4ROWS);
		} else {
			m_LcdDisplay = new Ssd1306(OLED_PANEL_128x64_8ROWS);
		}

		if (m_LcdDisplay->Start()) {
			m_tType = DisplayType::SSD1306;
			Printf(1, "SSD1306");
		}
	}
#if defined(ENABLE_TC1602)
	else if (HAL_I2C::IsConnected(TC1602_I2C_DEFAULT_SLAVE_ADDRESS)) {
		m_LcdDisplay = new Tc1602(m_nCols, m_nRows);

		if (m_LcdDisplay->Start()) {
			m_tType = DisplayType::PCF8574T_1602;
			Printf(1, "TC1602_PCF8574T");
		}
	}
#endif
#if defined(ENABLE_LCDBW)
	else if (HAL_I2C::IsConnected(BW_LCD_DEFAULT_SLAVE_ADDRESS >> 1)) {
		m_LcdDisplay = new LcdBw(BW_LCD_DEFAULT_SLAVE_ADDRESS, m_nCols, m_nRows);

		if (m_LcdDisplay->Start()) {
			m_tType = DisplayType::BW_LCD_1602;
			Printf(1, "BW_LCD");
		}
	} else if (HAL_I2C::IsConnected(BW_UI_DEFAULT_SLAVE_ADDRESS >> 1)) {
		m_LcdDisplay = new LcdBw(BW_UI_DEFAULT_SLAVE_ADDRESS, m_nCols, m_nRows);

		if (m_LcdDisplay->Start()) {
			m_tType = DisplayType::BW_UI_1602;
			Printf(1, "BW_UI");
		}
	}
#endif
	else {
#ifndef BARE_METAL
		puts("Unknown or no display attached");
#endif
		return;
	}

	if (m_LcdDisplay != nullptr) {
		m_nCols = m_LcdDisplay->GetColumns();
		m_nRows = m_LcdDisplay->GetRows();
	} else {
		m_nSleepTimeout = 0;
	}
}

Display::~Display() {
	s_pThis = nullptr;
	delete m_LcdDisplay;
}

void Display::Cls() {
	if (m_LcdDisplay == nullptr) {
		return;
	}

	m_LcdDisplay->Cls();
}

void Display::TextLine(uint8_t nLine, const char *pText, uint8_t nLength) {
	if (m_LcdDisplay == nullptr) {
		return;
	}

	m_LcdDisplay->TextLine(nLine, pText, nLength);
}

int Display::Printf(uint8_t nLine, const char *format, ...) {
	if (m_LcdDisplay == nullptr) {
		return 0;
	}

	char buffer[32];

	va_list arp;

	va_start(arp, format);

	int i = vsnprintf(buffer, sizeof(buffer) / sizeof(buffer[0]), format, arp);

	va_end(arp);

	m_LcdDisplay->TextLine(nLine, buffer, i);

	return i;
}

int Display::Write(uint8_t nLine, const char *pText) {
	if (m_LcdDisplay == nullptr) {
		return 0;
	}

	const char *p = pText;
	int nCount = 0;

	while ((*p != 0) && (nCount++ < static_cast<int>(m_nCols))) {
		++p;
	}

	m_LcdDisplay->TextLine(nLine, pText, static_cast<uint8_t>(nCount));

	return nCount;
}

void Display::SetCursorPos(uint8_t nCol, uint8_t nRow) {
	if (m_LcdDisplay == nullptr) {
		return;
	}

	m_LcdDisplay->SetCursorPos(nCol, nRow);
}

void Display::PutChar(int c) {
	if (m_LcdDisplay == nullptr) {
		return;
	}

	m_LcdDisplay->PutChar(c);
}

void Display::PutString(const char *pText) {
	if (m_LcdDisplay == nullptr) {
		return;
	}

	m_LcdDisplay->PutString(pText);
}

void Display::ClearLine(uint8_t nLine) {
	if (m_LcdDisplay == nullptr) {
		return;
	}

	m_LcdDisplay->ClearLine(nLine);
}

#if defined(ENABLE_CURSOR_MODE)
# define UNUSED
#else
# define UNUSED __attribute__((unused))
#endif

void Display::SetCursor(UNUSED uint32_t nMode) {
#if defined(ENABLE_CURSOR_MODE)
	if (m_LcdDisplay == nullptr) {
		return;
	}

	m_LcdDisplay->SetCursor(nMode);
#endif
}

void Display::TextStatus(const char *pText) {
	if (m_LcdDisplay == nullptr) {
		return;
	}

	SetCursorPos(0, m_nRows - 1);

	for (uint32_t i = 0; i < m_nCols - 1; i++) {
		PutChar(' ');
	}

	SetCursorPos(0, m_nRows - 1);

	Write(m_nRows, pText);
}

void Display::TextStatus(const char *pText, Display7SegmentMessage n7SegmentData, __attribute__((unused)) uint32_t nConsoleColor) {
	TextStatus(pText);
	m_Display7Segment.Status(n7SegmentData);
#if defined (BARE_METAL)
	if (nConsoleColor == UINT32_MAX) {
		return;
	}
	console_status(nConsoleColor, pText);
#endif
}

void Display::TextStatus(const char *pText, uint8_t nValue7Segment, bool bHex) {
	TextStatus(pText);
	m_Display7Segment.Status(nValue7Segment, bHex);
}

#if !defined(NO_HAL)
void Display::SetSleep(bool bSleep) {
	if (m_LcdDisplay == nullptr) {
		return;
	}

	m_bIsSleep = bSleep;

	m_LcdDisplay->SetSleep(bSleep);

	if (!bSleep) {
		m_nMillis = Hardware::Get()->Millis();
	}
}

void Display::Run() {
	if (m_nSleepTimeout == 0) {
		return;
	}

	if (!m_bIsSleep) {
		if (__builtin_expect(((Hardware::Get()->Millis() - m_nMillis) > m_nSleepTimeout), 0)) {
			SetSleep(true);
		}
	}
}
#endif

void Display::PrintInfo() {
	if (m_LcdDisplay == nullptr) {
		puts("No display found");
		return;
	}

	m_LcdDisplay->PrintInfo();
}
