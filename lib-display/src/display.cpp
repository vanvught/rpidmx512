/**
 * @file display.cpp
 *
 */
/* Copyright (C) 2017-2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include "display7segment.h"

#include "i2c.h"

#include "hardware.h"

#define MCP23017_I2C_ADDRESS	0x20
#define SEGMENT7_I2C_ADDRESS	(MCP23017_I2C_ADDRESS + 1)	///< It must be different from base address
#define MCP23X17_IODIRA			0x00	///< I/O DIRECTION (IODIRA) REGISTER, 1 = Input (default), 0 = Output
#define MCP23X17_GPIOA			0x12	///< PORT (GPIOA) REGISTER, Value on the Port - Writing Sets Bits in the Output Latch

Display *Display::s_pThis = 0;

Display::Display(uint8_t nCols, uint8_t nRows):
	m_tType(DISPLAY_TYPE_UNKNOWN),
	m_LcdDisplay(0),
	m_bIsSleep(false),
	m_bHave7Segment(false),
	m_nMillis(Hardware::Get()->Millis())
{
	s_pThis = this;

	Detect(nCols, nRows);
	Init7Segment();
}

Display::Display(TDisplayTypes tDisplayType):
	m_nCols(0),
	m_nRows(0),
	m_LcdDisplay(0),
	m_bIsSleep(false),
	m_bHave7Segment(false),
	m_nMillis(Hardware::Get()->Millis())
{
	s_pThis = this;
	m_tType = tDisplayType;

	switch (tDisplayType) {
#if defined(ENABLE_LCDBW)
		case DISPLAY_BW_UI_1602:
			m_LcdDisplay = new LcdBw(BW_UI_DEFAULT_SLAVE_ADDRESS, 16, 2);
			break;
		case DISPLAY_BW_LCD_1602:
			m_LcdDisplay = new LcdBw(BW_LCD_DEFAULT_SLAVE_ADDRESS, 16, 2);
			break;
#endif
#if defined(ENABLE_TC1602)
		case DISPLAY_PCF8574T_1602:
			m_LcdDisplay = new Tc1602(16, 2);
			break;
		case DISPLAY_PCF8574T_2004:
			m_LcdDisplay = new Tc1602(20, 4);
			break;
#endif
		case DISPLAY_SSD1306:
			m_LcdDisplay = new Ssd1306(OLED_PANEL_128x64_8ROWS);
			break;
		case DISPLAY_TYPE_UNKNOWN:
			m_tType = DISPLAY_TYPE_UNKNOWN;
			/* no break */
		default:
			break;
	}

	if (m_LcdDisplay != 0) {
		if (!m_LcdDisplay->Start()) {
			m_LcdDisplay = 0;
			m_tType = DISPLAY_TYPE_UNKNOWN;
		} else {
			m_LcdDisplay->Cls();
			m_nCols = m_LcdDisplay->GetColumns();
			m_nRows = m_LcdDisplay->GetRows();
		}
	}

	Init7Segment();
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
		if (nRows <= 4) {
			m_LcdDisplay = new Ssd1306(OLED_PANEL_128x64_4ROWS);
		} else {
			m_LcdDisplay = new Ssd1306(OLED_PANEL_128x64_8ROWS);
		}
		if (m_LcdDisplay->Start()) {
			m_tType = DISPLAY_SSD1306;
			Printf(1, "SSD1306");
		}
	}
#if defined(ENABLE_TC1602)
	else if (i2c_is_connected(TC1602_I2C_DEFAULT_SLAVE_ADDRESS)) {
		m_LcdDisplay = new Tc1602(m_nCols, m_nRows);
		if (m_LcdDisplay->Start()) {
			m_tType = DISPLAY_PCF8574T_1602;
			Printf(1, "TC1602_PCF8574T");
		}
	}
#endif
#if defined(ENABLE_LCDBW)
	else if (i2c_is_connected(BW_LCD_DEFAULT_SLAVE_ADDRESS >> 1)) {
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
	}
#endif
	else {
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
	if (m_LcdDisplay == 0) {
		return;
	}
	m_LcdDisplay->Cls();
}

void Display::TextLine(uint8_t nLine, const char *pText, uint8_t nLength) {
	if (m_LcdDisplay == 0) {
		return;
	}
	m_LcdDisplay->TextLine(nLine, pText, nLength);
}

void Display::TextStatus(const char *pText) {
	ClearLine(m_nRows);
	Write(m_nRows, pText);
}

uint8_t Display::Printf(uint8_t nLine, const char *format, ...) {
	int i;
	char buffer[32];

	if (m_LcdDisplay == 0) {
		return 0;
	}

	va_list arp;

	va_start(arp, format);

	i = vsnprintf(buffer, sizeof(buffer) / sizeof(buffer[0]), format, arp);

	va_end(arp);

	m_LcdDisplay->TextLine(nLine, buffer, i);

	return i;
}

uint8_t Display::Write(uint8_t nLine, const char *pText) {
	const char *p = pText;

	if (m_LcdDisplay == 0) {
		return 0;
	}

	while (*p != (char) 0) {
		++p;
	}

	const size_t nLength =  (size_t) (p - pText);

	m_LcdDisplay->TextLine(nLine, pText, nLength);

	return nLength;
}

void Display::SetCursor(TCursorMode constEnumTCursorOnOff) {
	if (m_LcdDisplay == 0) {
		return;
	}
	m_LcdDisplay->SetCursor(constEnumTCursorOnOff);
}

void Display::SetCursorPos(uint8_t nCol, uint8_t nRow) {
	if (m_LcdDisplay == 0) {
		return;
	}
	m_LcdDisplay->SetCursorPos(nCol, nRow);
}

void Display::PutChar(int c) {
	if (m_LcdDisplay == 0) {
		return;
	}
	m_LcdDisplay->PutChar(c);
}

void Display::PutString(const char *pText) {
	if (m_LcdDisplay == 0) {
		return;
	}
	m_LcdDisplay->PutString(pText);
}

void Display::ClearLine(uint8_t nLine) {
	if (m_LcdDisplay == 0) {
		return;
	}
	m_LcdDisplay->ClearLine(nLine);
}

void Display::SetSleep(bool bSleep) {
	if (m_LcdDisplay == 0) {
		return;
	}
	m_bIsSleep = bSleep;
	m_LcdDisplay->SetSleep(bSleep);

	if(!bSleep) {
		m_nMillis = Hardware::Get()->Millis();
	}
}

// Support for 2 digits 7-segment based on MCP23017

void Display::Init7Segment(void) {
	m_bHave7Segment = i2c_is_connected(SEGMENT7_I2C_ADDRESS);

	if (m_bHave7Segment) {
		i2c_set_address(SEGMENT7_I2C_ADDRESS);
		i2c_write_reg_uint16(MCP23X17_IODIRA, 0x0000); // All output
		Status(DISPLAY_7SEGMENT_MSG_INFO_STARTUP);
	}
}

void Display::Status(TDisplay7SegmentMessages nStatus) {
	if (m_bHave7Segment) {
		i2c_set_address(SEGMENT7_I2C_ADDRESS);
		i2c_write_reg_uint16(MCP23X17_GPIOA, (uint16_t)~nStatus);
	}
}

void Display::TextStatus(const char* pText, TDisplay7SegmentMessages nStatus) {
	TextStatus(pText);
	Status(nStatus);
}

void Display::Run(void) {
	if (!m_bIsSleep) {
		if ((Hardware::Get()->Millis() - m_nMillis) > (5 * 60 * 1000)) {
			SetSleep(true);
		}
	}
}
