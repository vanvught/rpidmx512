/**
 * @file hd44780.cpp
 *
 */
/* Copyright (C) 2017-2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <cstdint>
#include <stdio.h>
#include <cassert>

#include "i2c/hd44780.h"

#include "hal_i2c.h"

namespace hd44780 {
static constexpr uint8_t BIT_RS = (1U << 0);	///< Register select
// static constexpr uint8_t BIT_RW = (1U << 1);	///< Read/Write
static constexpr uint8_t BIT_EN = (1U << 2);	///< Enable
static constexpr uint8_t BIT_BL = (1U << 3);	///< Backlight
/// https://cdn-shop.adafruit.com/datasheets/TC1602A-01T.pdf
namespace cmd {
static constexpr uint8_t CLS = (1U << 0);		///< Clear all the display data by writing "20H" (space code) to all DDRAM address.
// static constexpr uint8_t RETURNHOME = (1U << 1);///< Return cursor to its original site and return display to its original status, if shifted. Contents of DDRAM do not change.
static constexpr uint8_t ENTRY_MODE = (1U << 2);///< Set the moving direction of cursor and display.
static constexpr uint8_t DISPLAY = (1U << 3);	///< Set display(D), cursor(C), and blinking of cursor(B) on/off control bit
static constexpr uint8_t FUNC = (1U << 5);		///< Set interface data length, numbers of display lines, display font type
static constexpr uint8_t SETDDRAMADDR = 0x80;
namespace entrymode {
// static constexpr uint8_t SH = (1U << 0);		///< Shift of entire display
// static constexpr uint8_t DEC = 0;				///< cursor/blink moves to left and DDRAM address is decreased by 1.
static constexpr uint8_t INC = (1U << 1);		///< cursor/blink moves to right and DDRAM address is increased by 1.
}  // namespace entrymode
namespace display {
static constexpr uint8_t BLINK_OFF = 0;			///< Cursor blink is off.
static constexpr uint8_t BLINK_ON = (1U << 0);	///< Cursor blink is on, that performs alternate between all the high data and display character at the cursor position.
static constexpr uint8_t CURSOR_OFF = 0;		///< Cursor is disappeared in current display, but I/D register remains its data.
static constexpr uint8_t CURSOR_ON = (1U << 1);	///< Cursor is turned on.
// static constexpr uint8_t DISPLAY_OFF = 0;		///< The display is turned off, but display data is remained in DDRAM.
static constexpr uint8_t ON = (1U << 2);		///< The entire display is turned on.
}  // namespace display
namespace func {
static constexpr uint8_t F4BIT = 0;				///< 4-bit bus mode with MPU.
// static constexpr uint8_t F8BIT = (1U << 4);		///< 8-bit bus mode with MPU.
// static constexpr uint8_t F1LINE = 0;			///< 1-line display mode.
static constexpr uint8_t F2LINE = (1U << 3);	///< 2-line display mode is set.
static constexpr uint8_t F5x8DOTS = 0;			///< 5 x 8 dots format display mode.
// static constexpr uint8_t F5x11DOTS = (1U << 2);	///< 5 x11 dots format display mode.
}  // namespace func
}  // namespace cmd
namespace exectime {
static constexpr auto CMD = 37U;				///< 37us
static constexpr auto REG = 43U;				///< 43us
static constexpr auto CLS = 1520U;				///< 1.52ms
}  // namespace exectime

namespace lcd {
static constexpr auto MIN_COLS = 16;
static constexpr auto MIN_ROWS = 2;
static constexpr auto MAX_COLS = 20;
static constexpr auto MAX_ROWS = 4;
}  // namespace lcd
}  // namespace hd44780

using namespace hd44780;

Hd44780::Hd44780(): m_I2C(pcf8574t::DEFAULT_ADDRESS) {
	m_nCols = lcd::MIN_COLS;
	m_nRows = lcd::MIN_ROWS;
}

Hd44780::Hd44780(uint8_t nCols, uint8_t nRows): m_I2C(pcf8574t::DEFAULT_ADDRESS) {
	m_nCols = (nCols < lcd::MAX_COLS) ? ((nCols < lcd::MIN_COLS) ? lcd::MIN_COLS : nCols) : lcd::MAX_COLS;
	m_nRows = (nRows < lcd::MAX_ROWS) ? ((nRows < lcd::MIN_ROWS) ? lcd::MIN_ROWS : nRows) : lcd::MAX_ROWS;
}

Hd44780::Hd44780(uint8_t nSlaveAddress, uint8_t nCols, uint8_t nRows): m_I2C(nSlaveAddress == 0 ? pcf8574t::DEFAULT_ADDRESS : nSlaveAddress) {
	m_nCols = (nCols < lcd::MAX_COLS) ? ((nCols < lcd::MIN_COLS) ? lcd::MIN_COLS : nCols) : lcd::MAX_COLS;
	m_nRows = (nRows < lcd::MAX_ROWS) ? ((nRows < lcd::MIN_ROWS) ? lcd::MIN_ROWS : nRows) : lcd::MAX_ROWS;
}

bool Hd44780::Start() {
	if (!m_I2C.IsConnected()) {
		return false;
	}

	WriteCmd(0x33);	///< 110011 Initialize
	WriteCmd(0x32);	///< 110010 Initialize

	WriteCmd((cmd::FUNC | cmd::func::F4BIT | cmd::func::F2LINE | cmd::func::F5x8DOTS));					///< Data length, number of lines, font size
	WriteCmd((cmd::DISPLAY | cmd::display::ON | cmd::display::CURSOR_OFF | cmd::display::BLINK_OFF));	///< Display On,Cursor Off, Blink Off

	WriteCmd(cmd::CLS);
	udelay(exectime::CLS - exectime::CMD);

	WriteCmd((cmd::ENTRY_MODE | cmd::entrymode::INC));	///< Cursor move direction

	return true;
}

void Hd44780::Cls() {
	WriteCmd(cmd::CLS);
	udelay(exectime::CLS - exectime::CMD);
}

void Hd44780::PutChar(int c) {
	WriteReg(static_cast<uint8_t>(c));
}

void Hd44780::PutString(const char *pString) {
	const char *p = pString;

	while (*p != '\0') {
		Hd44780::PutChar(static_cast<int>(*p));
		p++;
	}
}

void Hd44780::Text(const char *pData, uint32_t nLength) {
	if (nLength > m_nCols) {
		nLength = m_nCols;
	}

	for (uint32_t i = 0; i < nLength; i++) {
		WriteReg(static_cast<uint8_t>(pData[i]));
	}
}

void Hd44780::TextLine(uint32_t nLine, const char *pData, uint32_t nLength) {
	if (nLine > m_nRows) {
		return;
	}

	Hd44780::SetCursorPos(0, static_cast<uint8_t>(nLine - 1));
	Hd44780::Text(pData, nLength);
}

void Hd44780::ClearLine(uint32_t nLine) {
	if (nLine > m_nRows) {
		return;
	}

	Hd44780::SetCursorPos(0, static_cast<uint8_t>(nLine - 1));

	for (uint32_t i = 0; i < m_nCols; i++) {
		WriteReg(' ');
	}

	Hd44780::SetCursorPos(0, static_cast<uint8_t>(nLine - 1));
}

void Hd44780::PrintInfo() {
	printf("HD44780 [PCF8574T] (%u,%u)\n", static_cast<unsigned int>(m_nRows), static_cast<unsigned int>(m_nCols));
}

void Hd44780::SetCursorPos(uint32_t nCol, uint32_t nRow) {
	assert(nRow <= 3);

	constexpr uint8_t rowOffsets[] = { 0x00, 0x40, 0x14, 0x54 };

	WriteCmd(static_cast<uint8_t>(cmd::SETDDRAMADDR | (nCol + rowOffsets[nRow & 0x03])));
}

void Hd44780::Write4bits(const uint8_t nData) {
	m_I2C.Write(static_cast<char>(nData));
	m_I2C.Write(static_cast<char>(nData | BIT_EN | BIT_BL));
	m_I2C.Write(static_cast<char>((nData & ~BIT_EN) | BIT_BL));
}

void Hd44780::WriteCmd(const uint8_t nCmd) {
	Write4bits(nCmd & 0xF0);
	Write4bits(static_cast<uint8_t>((nCmd << 4) & 0xF0));
	udelay(exectime::CMD);
}

void Hd44780::WriteReg(const uint8_t nReg) {
	Write4bits(static_cast<uint8_t>(BIT_RS | (nReg & 0xF0)));
	Write4bits(static_cast<uint8_t>(BIT_RS | ((nReg << 4) & 0xF0)));
	udelay(exectime::REG);
}

#if defined(CONFIG_DISPLAY_ENABLE_CURSOR_MODE)
# define UNUSED
#else
# define UNUSED [[maybe_unused]]
#endif

void Hd44780::SetCursor(UNUSED uint32_t nMode) {
#if defined(CONFIG_DISPLAY_ENABLE_CURSOR_MODE)
	uint8_t nCmd = cmd::DISPLAY | cmd::display::ON;

	if ((nMode & display::cursor::ON) == display::cursor::ON ){
		nCmd |= cmd::display::CURSOR_ON;
	}

	if ((nMode & display::cursor::BLINK_ON) == display::cursor::BLINK_ON ){
		nCmd |= cmd::display::BLINK_ON;
	}

	WriteCmd(nCmd);
#endif
}
