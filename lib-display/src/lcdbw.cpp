/**
 * @file lcdbw.cpp
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

#include <stdint.h>

extern "C" {
extern uint32_t micros(void);
#if defined(__linux__)
extern void bcm2835_delayMicroseconds (const uint64_t);
#define udelay bcm2835_delayMicroseconds
#else
extern void udelay(uint32_t);
#endif
}

#include "lcdbw.h"

#include "i2c.h"

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

LcdBw::LcdBw(void): m_nSlaveAddress(BW_LCD_DEFAULT_SLAVE_ADDRESS), m_nWriteMicros(0) {
}

LcdBw::LcdBw(const uint8_t nCols, const uint8_t nRows): m_nSlaveAddress(BW_LCD_DEFAULT_SLAVE_ADDRESS), m_nWriteMicros(0) {
	m_nCols = nCols;
	m_nRows = nRows;
}

LcdBw::LcdBw(const uint8_t nSlaveAddress, const uint8_t nCols, const uint8_t nRows): m_nWriteMicros(0) {
	if (nSlaveAddress == (uint8_t) 0) {
		m_nSlaveAddress = BW_LCD_DEFAULT_SLAVE_ADDRESS;
	} else {
		m_nSlaveAddress = nSlaveAddress;
	}

	m_nCols = nCols;
	m_nRows = nRows;
}

LcdBw::~LcdBw(void) {
}

bool LcdBw::Start(void) {
	i2c_begin();

	Setup();

	if (!i2c_is_connected(m_nSlaveAddress >> 1)) {
		return false;
	}

	return true;
}

void LcdBw::Cls(void) {
	char cmd[] = { (char) BW_PORT_WRITE_CLEAR_SCREEN, ' ' };

	Setup();
	Write(cmd, sizeof(cmd) / sizeof(cmd[0]));
}

void LcdBw::TextLine(const uint8_t nLine, const char *data, const uint8_t nLength) {
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
	data[0] = (char) BW_PORT_WRITE_DISPLAY_DATA;
	data[1] = (char) c;

	Setup();
	Write(data, 2);
}

void LcdBw::PutString(const char *s) {
	int i;
	char data[(4 * 20)];
	char *p = (char *) s;

	data[0] = (char) BW_PORT_WRITE_DISPLAY_DATA;

	for (i = 1; (i < m_nCols * m_nRows) && (*p != '\0'); i++) {
		data[i] = *p;
		p++;
	}

	Setup();
	Write(data, i + 1);
}

void LcdBw::Text(const char *pText, uint8_t nLength) {
	uint8_t i;
	char data[32];

	data[0] = (char) BW_PORT_WRITE_DISPLAY_DATA;

	if (nLength > m_nCols) {
		nLength = m_nCols;
	}

	for (i = 0; i < nLength; i++) {
		data[i + 1] = pText[i];
	}

	Setup();
	Write(data, nLength + 1);
}

void LcdBw::SetCursorPos(uint8_t col, uint8_t line) {
	char cmd[] = { (char) BW_PORT_WRITE_MOVE_CURSOR, (char) 0x00 };

	cmd[1] = (char) ((line & 0x03) << 5) | (col & 0x1f);

	Setup();
	Write(cmd, sizeof(cmd) / sizeof(cmd[0]));
}

void LcdBw::ClearLine(const uint8_t nLine) {
	char data[32];

	data[0] = (char) BW_PORT_WRITE_DISPLAY_DATA;

	for (int i = 1; i < m_nCols; i++) {
		data[i] = ' ';
	}

	Setup();
	Write(data, m_nCols + 1);
}

void LcdBw::Setup(void) {
	i2c_set_address(m_nSlaveAddress >> 1);
	i2c_set_clockdivider(I2C_CLOCK_DIVIDER_100kHz);
}

void LcdBw::SetCursor(const TCursorMode constEnumTCursorOnOff) {
}

void LcdBw::Write(const char *buffer, const uint32_t size) {
	const uint32_t elapsed = micros() - m_nWriteMicros;

	if (elapsed < BW_LCD_I2C_BYTE_WAIT_US) {
		udelay(BW_LCD_I2C_BYTE_WAIT_US - elapsed);
	}

	i2c_write_nb(buffer, size);

	m_nWriteMicros = micros();
}
