/**
 * @file hd44780.cpp
 *
 */
/* Copyright (C) 2017-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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
#include "displayset.h"
#include "hal_i2c.h"

namespace hd44780
{
static constexpr uint8_t kBitRs = (1U << 0); ///< Register select
// static constexpr uint8_t BIT_RW = (1U << 1);	///< Read/Write
static constexpr uint8_t kBitEn = (1U << 2); ///< Enable
static constexpr uint8_t kBitBl = (1U << 3); ///< Backlight
/// https://cdn-shop.adafruit.com/datasheets/TC1602A-01T.pdf
namespace cmd
{
static constexpr uint8_t kCls = (1U << 0); ///< Clear all the display data by writing "20H" (space code) to all DDRAM address.
// static constexpr uint8_t RETURNHOME = (1U << 1);///< Return cursor to its original site and return display to its original status, if shifted. Contents of
// DDRAM do not change.
static constexpr uint8_t kEntryMode = (1U << 2); ///< Set the moving direction of cursor and display.
static constexpr uint8_t kDisplay = (1U << 3);    ///< Set display(D), cursor(C), and blinking of cursor(B) on/off control bit
static constexpr uint8_t kFunc = (1U << 5);       ///< Set interface data length, numbers of display lines, display font type
static constexpr uint8_t kSetddramaddr = 0x80;
namespace entrymode
{
// static constexpr uint8_t SH = (1U << 0);		///< Shift of entire display
// static constexpr uint8_t DEC = 0;				///< cursor/blink moves to left and DDRAM address is decreased by 1.
static constexpr uint8_t kInc = (1U << 1); ///< cursor/blink moves to right and DDRAM address is increased by 1.
} // namespace entrymode
namespace display
{
static constexpr uint8_t kBlinkOff = 0; ///< Cursor blink is off.
static constexpr uint8_t kBlinkOn = (1U << 0); ///< Cursor blink is on, that performs alternate between all the high data and display character at the cursor position.
static constexpr uint8_t kCursorOff = 0;        ///< Cursor is disappeared in current display, but I/D register remains its data.
static constexpr uint8_t kCursorOn = (1U << 1); ///< Cursor is turned on.
// static constexpr uint8_t DISPLAY_OFF = 0;		///< The display is turned off, but display data is remained in DDRAM.
static constexpr uint8_t kOn = (1U << 2); ///< The entire display is turned on.
} // namespace display
namespace func
{
static constexpr uint8_t kF4Bit = 0; ///< 4-bit bus mode with MPU.
// static constexpr uint8_t F8BIT = (1U << 4);		///< 8-bit bus mode with MPU.
// static constexpr uint8_t F1LINE = 0;			///< 1-line display mode.
static constexpr uint8_t kF2Line = (1U << 3); ///< 2-line display mode is set.
static constexpr uint8_t kF5x8Dots = 0;       ///< 5 x 8 dots format display mode.
// static constexpr uint8_t F5x11DOTS = (1U << 2);	///< 5 x11 dots format display mode.
} // namespace func
} // namespace cmd
namespace exectime
{
static constexpr auto kCmd = 37U;   ///< 37us
static constexpr auto kReg = 43U;   ///< 43us
static constexpr auto kCls = 1520U; ///< 1.52ms
} // namespace exectime

namespace lcd
{
static constexpr auto kMinColumns = 16;
static constexpr auto kMinRows = 2;
static constexpr auto kMaxColumns = 20;
static constexpr auto kMaxRows = 4;
} // namespace lcd
} // namespace hd44780

Hd44780::Hd44780() : hal_i2c_(hd44780::pcf8574t::kDefaultAddress)
{
    cols_ = hd44780::lcd::kMinColumns;
    rows_ = hd44780::lcd::kMinRows;
}

Hd44780::Hd44780(uint8_t columns, uint8_t rows) : hal_i2c_(hd44780::pcf8574t::kDefaultAddress)
{
    cols_ = (columns < hd44780::lcd::kMaxColumns) ? ((columns < hd44780::lcd::kMinColumns) ? hd44780::lcd::kMinColumns : columns) : hd44780::lcd::kMaxColumns;
    rows_ = (rows < hd44780::lcd::kMaxRows) ? ((rows < hd44780::lcd::kMinRows) ? hd44780::lcd::kMinRows : rows) : hd44780::lcd::kMaxRows;
}

Hd44780::Hd44780(uint8_t address, uint8_t columns, uint8_t rows) : hal_i2c_(address == 0 ? hd44780::pcf8574t::kDefaultAddress : address)
{
    cols_ = (columns < hd44780::lcd::kMaxColumns) ? ((columns < hd44780::lcd::kMinColumns) ? hd44780::lcd::kMinColumns : columns) : hd44780::lcd::kMaxColumns;
    rows_ = (rows < hd44780::lcd::kMaxRows) ? ((rows < hd44780::lcd::kMinRows) ? hd44780::lcd::kMinRows : rows) : hd44780::lcd::kMaxRows;
}

bool Hd44780::Start()
{
    if (!hal_i2c_.IsConnected())
    {
        return false;
    }

    WriteCmd(0x33); ///< 110011 Initialize
    WriteCmd(0x32); ///< 110010 Initialize

    WriteCmd((hd44780::cmd::kFunc | hd44780::cmd::func::kF4Bit | hd44780::cmd::func::kF2Line |
              hd44780::cmd::func::kF5x8Dots)); ///< Data length, number of lines, font size
    WriteCmd((hd44780::cmd::kDisplay | hd44780::cmd::display::kOn | hd44780::cmd::display::kCursorOff |
              hd44780::cmd::display::kBlinkOff)); ///< Display On,Cursor Off, Blink Off

    WriteCmd(hd44780::cmd::kCls);
    udelay(hd44780::exectime::kCls - hd44780::exectime::kCmd);

    WriteCmd((hd44780::cmd::kEntryMode | hd44780::cmd::entrymode::kInc)); ///< Cursor move direction

    return true;
}

void Hd44780::Cls()
{
    WriteCmd(hd44780::cmd::kCls);
    udelay(hd44780::exectime::kCls - hd44780::exectime::kCmd);
}

void Hd44780::PutChar(int c)
{
    WriteReg(static_cast<uint8_t>(c));
}

void Hd44780::PutString(const char* string)
{
    const char* p = string;

    while (*p != '\0')
    {
        Hd44780::PutChar(static_cast<int>(*p));
        p++;
    }
}

void Hd44780::Text(const char* data, uint32_t length)
{
    if (length > cols_)
    {
        length = cols_;
    }

    for (uint32_t i = 0; i < length; i++)
    {
        WriteReg(static_cast<uint8_t>(data[i]));
    }
}

void Hd44780::TextLine(uint32_t line, const char* data, uint32_t length)
{
    if (line > rows_)
    {
        return;
    }

    Hd44780::SetCursorPos(0, static_cast<uint8_t>(line - 1));
    Hd44780::Text(data, length);
}

void Hd44780::ClearLine(uint32_t line)
{
    if (line > rows_)
    {
        return;
    }

    Hd44780::SetCursorPos(0, static_cast<uint8_t>(line - 1));

    for (uint32_t i = 0; i < cols_; i++)
    {
        WriteReg(' ');
    }

    Hd44780::SetCursorPos(0, static_cast<uint8_t>(line - 1));
}

void Hd44780::PrintInfo()
{
    printf("HD44780 [PCF8574T] (%u,%u)\n", static_cast<unsigned int>(rows_), static_cast<unsigned int>(cols_));
}

void Hd44780::SetCursorPos(uint32_t column, uint32_t row)
{
    assert(row <= 3);

    constexpr uint8_t kRowOffsets[] = {0x00, 0x40, 0x14, 0x54};

    WriteCmd(static_cast<uint8_t>(hd44780::cmd::kSetddramaddr | (column + kRowOffsets[row & 0x03])));
}

void Hd44780::Write4bits(uint8_t data)
{
    hal_i2c_.Write(static_cast<char>(data));
    hal_i2c_.Write(static_cast<char>(data | hd44780::kBitEn | hd44780::kBitBl));
    hal_i2c_.Write(static_cast<char>((data & ~hd44780::kBitEn) | hd44780::kBitBl));
}

void Hd44780::WriteCmd(uint8_t cmd)
{
    Write4bits(cmd & 0xF0);
    Write4bits(static_cast<uint8_t>((cmd << 4) & 0xF0));
    udelay(hd44780::exectime::kCmd);
}

void Hd44780::WriteReg(uint8_t reg)
{
    Write4bits(static_cast<uint8_t>(hd44780::kBitRs | (reg & 0xF0)));
    Write4bits(static_cast<uint8_t>(hd44780::kBitRs | ((reg << 4) & 0xF0)));
    udelay(hd44780::exectime::kReg);
}

void Hd44780::SetCursor([[maybe_unused]] uint32_t mode)
{
#if defined(CONFIG_DISPLAY_ENABLE_CURSOR_MODE)
    uint8_t cmd = hd44780::cmd::kDisplay | hd44780::cmd::display::kOn;

    if ((mode & display::cursor::kOn) == display::cursor::kOn)
    {
        cmd |= hd44780::cmd::display::kCursorOn;
    }

    if ((mode & display::cursor::kBlinkOn) == display::cursor::kBlinkOn)
    {
        cmd |= hd44780::cmd::display::kBlinkOn;
    }

    WriteCmd(cmd);
#endif
}
