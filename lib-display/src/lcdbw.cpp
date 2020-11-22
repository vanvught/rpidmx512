/**
 * @file lcdbw.cpp
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

#include "lcdbw.h"

#include "hal_i2c.h"

#define BW_PORT_WRITE_DISPLAY_DATA			0x00	///< display data
#define BW_PORT_WRITE_COMMAND				0x01	///< write data as command
#define BW_PORT_WRITE_STARTUPMESSAGE_LINE1	0x08	///< Set startup message line 1
#define BW_PORT_WRITE_STARTUPMESSAGE_LINE2	0x09	///< Set startup message line 2
#define BW_PORT_WRITE_STARTUPMESSAGE_LINE3	0x0a	///< Set startup message line 3
#define BW_PORT_WRITE_STARTUPMESSAGE_LINE4	0x0b	///< Set startup message line 4
#define BW_PORT_WRITE_CLEAR_SCREEN			0x10	///< any data clears the screen
#define BW_PORT_WRITE_MOVE_CURSOR			0x11
#define BW_PORT_WRITE_SET_CONTRAST			0x12
#define BW_PORT_WRITE_SET_BACKLIGHT_TEMP	0x13
#define BW_PORT_WRITE_REINIT_LCD			0x14
#define BW_PORT_WRITE_SET_BACKLIGHT			0x17
#define BW_PORT_WRITE_CHANGE_SLAVE_ADDRESS	0xf0

#define BW_LCD_I2C_BYTE_WAIT_US	37

LcdBw::LcdBw(): m_I2C(BW_LCD_DEFAULT_SLAVE_ADDRESS, hal::i2c::NORMAL_SPEED) {
}

LcdBw::LcdBw(uint8_t nCols, uint8_t nRows): m_I2C(BW_LCD_DEFAULT_SLAVE_ADDRESS, hal::i2c::NORMAL_SPEED) {
	m_nCols = nCols;
	m_nRows = nRows;
}

LcdBw::LcdBw(uint8_t nSlaveAddress, uint8_t nCols, uint8_t nRows): m_I2C((nSlaveAddress == 0 ? BW_LCD_DEFAULT_SLAVE_ADDRESS : nSlaveAddress >> 1), hal::i2c::NORMAL_SPEED) {
	m_nCols = nCols;
	m_nRows = nRows;
}

bool LcdBw::Start() {
	if (!m_I2C.IsConnected()) {
		return false;
	}

	return true;
}

void LcdBw::Cls() {
	char cmd[] = { BW_PORT_WRITE_CLEAR_SCREEN, ' ' };
	Write(cmd, sizeof(cmd) / sizeof(cmd[0]));
}

void LcdBw::TextLine(uint8_t nLine, const char *data, uint8_t nLength) {
	if (nLine > m_nRows) {
		return;
	}

	switch (nLine) {
		case 1:
			SetCursorPos(0, 0);
			break;
		case 2:
			SetCursorPos(1, 0);
			break;
		default:
			return;
			break;
	}

	Text(data, nLength);
}

void LcdBw::PutChar(int c) {
	char data[2];
	data[0] = BW_PORT_WRITE_DISPLAY_DATA;
	data[1] = c;

	Write(data, 2);
}

void LcdBw::PutString(const char *s) {
	uint32_t i;
	char data[(4 * 20)];
	const char *p = s;

	data[0] = BW_PORT_WRITE_DISPLAY_DATA;

	for (i = 1; (i < static_cast<uint32_t>(m_nCols * m_nRows)) && (*p != '\0'); i++) {
		data[i] = *p;
		p++;
	}

	Write(data, i + 1);
}

void LcdBw::Text(const char *pText, uint8_t nLength) {
	uint8_t i;
	char data[32];

	data[0] = BW_PORT_WRITE_DISPLAY_DATA;

	if (nLength > m_nCols) {
		nLength = m_nCols;
	}

	for (i = 0; i < nLength; i++) {
		data[i + 1] = pText[i];
	}

	Write(data, static_cast<uint32_t>(nLength + 1));
}

void LcdBw::SetCursorPos(uint8_t col, uint8_t line) {
	char cmd[] = { BW_PORT_WRITE_MOVE_CURSOR, 0x00 };

	cmd[1] = ((line & 0x03) << 5) | (col & 0x1f);

	Write(cmd, sizeof(cmd) / sizeof(cmd[0]));
}

void LcdBw::ClearLine(__attribute__((unused))uint8_t nLine) {
	char data[32];

	data[0] = BW_PORT_WRITE_DISPLAY_DATA;

	for (int i = 1; i < m_nCols; i++) {
		data[i] = ' ';
	}

	Write(data, static_cast<uint32_t>(m_nCols + 1));
}


void LcdBw::SetCursor(__attribute__((unused))const uint32_t constEnumTCursorOnOff) {
#if defined(ENABLE_CURSOR_MODE)
#endif
}

void LcdBw::Write(const char *buffer, uint32_t size) {
	const uint32_t elapsed = micros() - m_nWriteMicros;

	if (elapsed < BW_LCD_I2C_BYTE_WAIT_US) {
		udelay(BW_LCD_I2C_BYTE_WAIT_US - elapsed);
	}

	m_I2C.Write(buffer, size);

	m_nWriteMicros = micros();
}
