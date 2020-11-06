/**
 * @file tc1602.cpp
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
#include <cassert>

#include "tc1602.h"

#include "hal_i2c.h"

#define MIN_COLS	16
#define MIN_ROWS	2

#define MAX_COLS	20
#define MAX_ROWS	4

#define TC1602_RS 			(1<<0)	///< 0b00000001 # Register select bit
#define TC1602_RW 			(1<<1)	///< 0b00000010 # Read/Write bit
#define TC1602_EN			(1<<2)	///< 0b00000100 # Enable bit
#define TC1602_BACKLIGHT	(1<<3)	///< 0b00001000 # Backlight bit
	#define TC1602_NOBACKLIGHT	0x00

#define EXEC_TIME_CMD		37		///< 37us
#define EXEC_TIME_REG		43		///< 43us
#define EXEC_TIME_CLS		1520	///< 1.52ms

/// Instruction Code
/// http://www.oppod.com/upload/download/20111224105943_57765.pdf
/// 1. Clear Display
#define TC1602_IC_CLS			(1<<0)	///< Clear all the display data by writing "20H" (space code) to all DDRAM address.
/// 2. Return Home
#define TC1602_IC_RETURNHOME	(1<<1)	///< Return cursor to its original site and return display to its original status, if shifted. Contents of DDRAM do not change.
/// 3. Entry Mode Set
#define TC1602_IC_ENTRY_MODE	(1<<2)	///< Set the moving direction of cursor and display.
	#define TC1602_IC_ENTRY_MODE_SH		(1<<0)	///< Shift of entire display
	#define TC1602_IC_ENTRY_MODE_DEC		0	///< cursor/blink moves to left and DDRAM address is decreased by 1.
	#define TC1602_IC_ENTRY_MODE_INC	(1<<1)	///< cursor/blink moves to right and DDRAM address is increased by 1.
/// 4. Display ON/OFF Control
#define TC1602_IC_DISPLAY		(1<<3)	///< Set display(D), cursor(C), and blinking of cursor(B) on/off control bit
 	#define TC1602_IC_DISPLAY_BLINK_OFF		0	///< Cursor blink is off.
	#define TC1602_IC_DISPLAY_BLINK_ON	(1<<0)	///< Cursor blink is on, that performs alternate between all the high data and display character at the cursor position.
	#define TC1602_IC_DISPLAY_CURSOR_OFF	0	///< Cursor is disappeared in current display, but I/D register remains its data.
	#define TC1602_IC_DISPLAY_CURSOR_ON	(1<<1)	///< Cursor is turned on.
 	#define TC1602_IC_DISPLAY_OFF			0	///< The display is turned off, but display data is remained in DDRAM.
	#define TC1602_IC_DISPLAY_ON		(1<<2)	///< The entire display is turned on.
/// 6. Function Set
#define TC1602_IC_FUNC			(1<<5)	///< Set interface data length, numbers of display lines, display font type
	#define TC1602_IC_FUNC_4BIT			0	///< 4-bit bus mode with MPU.
	#define TC1602_IC_FUNC_8BIT		(1<<4)	///< 8-bit bus mode with MPU.
	#define TC1602_IC_FUNC_1LINE		0	///< 1-line display mode.
	#define TC1602_IC_FUNC_2LINE	(1<<3)	///< 2-line display mode is set.
	#define TC1602_IC_FUNC_5x8DOTS		0	///< 5 x 8 dots format display mode.
	#define TC1602_IC_FUNC_5x11DOTS	(1<<2)	///< 5 x11 dots format display mode.

#define LCD_SETDDRAMADDR 0x80

Tc1602::Tc1602(): m_I2C(TC1602_I2C_DEFAULT_SLAVE_ADDRESS) {
	m_nCols = 16;
	m_nRows = 2;
}

Tc1602::Tc1602(const uint8_t nCols, const uint8_t nRows): m_I2C(TC1602_I2C_DEFAULT_SLAVE_ADDRESS) {
	m_nCols = (nCols < MAX_COLS) ? ((nCols < MIN_COLS) ? MIN_COLS : nCols) : MAX_COLS;
	m_nRows = (nRows < MAX_ROWS) ? ((nRows < MIN_ROWS) ? MIN_ROWS : nRows) : MAX_ROWS;
}

Tc1602::Tc1602(const uint8_t nSlaveAddress, const uint8_t nCols, const uint8_t nRows): m_I2C(nSlaveAddress == 0 ? TC1602_I2C_DEFAULT_SLAVE_ADDRESS : nSlaveAddress) {
	m_nCols = (nCols < MAX_COLS) ? ((nCols < MIN_COLS) ? MIN_COLS : nCols) : MAX_COLS;
	m_nRows = (nRows < MAX_ROWS) ? ((nRows < MIN_ROWS) ? MIN_ROWS : nRows) : MAX_ROWS;
}

bool Tc1602::Start() {
	if (!m_I2C.IsConnected()) {
		return false;
	}

	WriteCmd(0x33);	///< 110011 Initialize
	WriteCmd(0x32);	///< 110010 Initialize

	WriteCmd((TC1602_IC_FUNC | TC1602_IC_FUNC_4BIT | TC1602_IC_FUNC_2LINE | TC1602_IC_FUNC_5x8DOTS)); ///< Data length, number of lines, font size
	WriteCmd((TC1602_IC_DISPLAY | TC1602_IC_DISPLAY_ON | TC1602_IC_DISPLAY_CURSOR_OFF | TC1602_IC_DISPLAY_BLINK_OFF));	///< Display On,Cursor Off, Blink Off
	WriteCmd(TC1602_IC_CLS);
	udelay(EXEC_TIME_CLS - EXEC_TIME_CMD);
	WriteCmd((TC1602_IC_ENTRY_MODE | TC1602_IC_ENTRY_MODE_INC));	///< Cursor move direction

	return true;
}

void Tc1602::Cls() {
	WriteCmd(TC1602_IC_CLS);
	udelay(EXEC_TIME_CLS - EXEC_TIME_CMD);
}

void Tc1602::PutChar(int c) {
	WriteReg(c);
}

void Tc1602::PutString(const char *pString) {
	const char *p = pString;

	for (uint32_t i = 0; *p != '\0'; i++) {
		Tc1602::PutChar(static_cast<int>(*p));
		p++;
	}
}

void Tc1602::Text(const char *data, uint8_t nLength) {
	if (nLength > m_nCols) {
		nLength = m_nCols;
	}

	for (uint32_t i = 0; i < nLength; i++) {
		WriteReg(data[i]);
	}
}

void Tc1602::TextLine(uint8_t nLine, const char *data, uint8_t nLength) {
	if (nLine > m_nRows) {
		return;
	}

	Tc1602::SetCursorPos(0, nLine - 1);
	Tc1602::Text(data, nLength);
}

void Tc1602::ClearLine(uint8_t nLine) {
	if (nLine > m_nRows) {
		return;
	}

	Tc1602::SetCursorPos(0, nLine - 1);

	for (uint32_t i = 0; i < m_nCols; i++) {
		WriteReg(' ');
	}

	Tc1602::SetCursorPos(0, nLine - 1);
}

void Tc1602::SetCursorPos(uint8_t col, uint8_t row) {
	assert(row <= 3);

	constexpr uint8_t row_offsets[] = { 0x00, 0x40, 0x14, 0x54 };

	WriteCmd(LCD_SETDDRAMADDR | (col + row_offsets[row & 0x03]));
}

void Tc1602::Write4bits(uint8_t data) {
	m_I2C.Write(data);
	m_I2C.Write(data | TC1602_EN | TC1602_BACKLIGHT);
	m_I2C.Write((data & ~TC1602_EN) | TC1602_BACKLIGHT);
}

void Tc1602::WriteCmd(uint8_t cmd) {
	Write4bits(cmd & 0xF0);
	Write4bits((cmd << 4) & 0xF0);
	udelay(EXEC_TIME_CMD);
}

void Tc1602::WriteReg(const uint8_t reg) {
	Write4bits(TC1602_RS | (reg & 0xF0));
	Write4bits(TC1602_RS | ((reg << 4) & 0xF0));
	udelay(EXEC_TIME_REG);
}

#if defined(ENABLE_CURSOR_MODE)
# define UNUSED
#else
# define UNUSED __attribute__((unused))
#endif

void Tc1602::SetCursor(UNUSED uint32_t nMode) {
#if defined(ENABLE_CURSOR_MODE)
	uint8_t mode = TC1602_IC_DISPLAY | TC1602_IC_DISPLAY_ON;

	if ((nMode & display::cursor::ON) == display::cursor::ON ){
		mode |= TC1602_IC_DISPLAY_CURSOR_ON;
	}

	if ((nMode & display::cursor::BLINK_ON) == display::cursor::BLINK_ON ){
		mode |= TC1602_IC_DISPLAY_BLINK_ON;
	}

	WriteCmd(mode);
#endif
}
