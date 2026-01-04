/**
 * @file ssd1306.cpp
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
#include <cstring>
#include <cstdio>
#include <cassert>

#include "i2c/ssd1306.h"
#include "displayset.h"
#include "hal_i2c.h"

namespace ssd1306
{
static constexpr auto kLcdWidth = 128;
namespace mode
{
static constexpr auto kCommand = 0x00;
static constexpr auto kData = 0x40;
} // namespace mode
namespace cmd
{
static constexpr auto kSetLowcolumn = 0x00;
static constexpr auto kSetHighcolumn = 0x10;
static constexpr auto kSetMemorymode = 0x20;
static constexpr auto kSetStartline = 0x40;
static constexpr auto kSetContrast = 0x81;
static constexpr auto kSetChargepump = 0x8D;
static constexpr auto kSegremap = 0xA0;
static constexpr auto kOuputRam = 0xA4;
static constexpr auto kDisplayNormal = 0xA6;
static constexpr auto kSetMultiplex = 0xA8;
static constexpr auto kDisplayOff = 0xAE;
static constexpr auto kDisplayOn = 0xAF;
static constexpr auto kSetStartpage = 0xB0;
static constexpr auto kComscanInc = 0xC0;
static constexpr auto kComscanDec = 0xC8;
static constexpr auto kSetDisplayoffset = 0xD3;
static constexpr auto kSetDisplayclockdiv = 0xD5;
static constexpr auto kSetPrecharge = 0xD9;
static constexpr auto kSetCompins = 0xDA;
static constexpr auto kSetVcomdetect = 0xDB;
} // namespace cmd

namespace oled::font8x6
{
static constexpr auto kCharH = 8;
static constexpr auto kCharW = 6;
static constexpr auto kCols = (kLcdWidth / kCharW);
} // namespace oled::font8x6

} // namespace ssd1306

static const uint8_t kOledFont8x6[] __attribute__((aligned(4))) = {
	0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x40, 0x00, 0x00, 0x5F, 0x00, 0x00, 0x00,
	0x40, 0x00, 0x07, 0x00, 0x07, 0x00, 0x00,
	0x40, 0x14, 0x7F, 0x14, 0x7F, 0x14, 0x00,
	0x40, 0x24, 0x2A, 0x7F, 0x2A, 0x12, 0x00,
	0x40, 0x23, 0x13, 0x08, 0x64, 0x62, 0x00,
	0x40, 0x36, 0x49, 0x56, 0x20, 0x50, 0x00,
	0x40, 0x00, 0x08, 0x07, 0x03, 0x00, 0x00,
	0x40, 0x00, 0x1C, 0x22, 0x41, 0x00, 0x00,
	0x40, 0x00, 0x41, 0x22, 0x1C, 0x00, 0x00,
	0x40, 0x2A, 0x1C, 0x7F, 0x1C, 0x2A, 0x00,
	0x40, 0x08, 0x08, 0x3E, 0x08, 0x08, 0x00,
	0x40, 0x00, 0x80, 0x70, 0x30, 0x00, 0x00,
	0x40, 0x08, 0x08, 0x08, 0x08, 0x08, 0x00,
	0x40, 0x00, 0x00, 0x60, 0x60, 0x00, 0x00,
	0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x00,
	0x40, 0x3E, 0x51, 0x49, 0x45, 0x3E, 0x00,
	0x40, 0x00, 0x42, 0x7F, 0x40, 0x00, 0x00,
	0x40, 0x72, 0x49, 0x49, 0x49, 0x46, 0x00,
	0x40, 0x21, 0x41, 0x49, 0x4D, 0x33, 0x00,
	0x40, 0x18, 0x14, 0x12, 0x7F, 0x10, 0x00,
	0x40, 0x27, 0x45, 0x45, 0x45, 0x39, 0x00,
	0x40, 0x3C, 0x4A, 0x49, 0x49, 0x31, 0x00,
	0x40, 0x41, 0x21, 0x11, 0x09, 0x07, 0x00,
	0x40, 0x36, 0x49, 0x49, 0x49, 0x36, 0x00,
	0x40, 0x46, 0x49, 0x49, 0x29, 0x1E, 0x00,
	0x40, 0x00, 0x00, 0x14, 0x00, 0x00, 0x00,
	0x40, 0x00, 0x40, 0x34, 0x00, 0x00, 0x00,
	0x40, 0x00, 0x08, 0x14, 0x22, 0x41, 0x00,
	0x40, 0x14, 0x14, 0x14, 0x14, 0x14, 0x00,
	0x40, 0x00, 0x41, 0x22, 0x14, 0x08, 0x00,
	0x40, 0x02, 0x01, 0x59, 0x09, 0x06, 0x00,
	0x40, 0x3E, 0x41, 0x5D, 0x59, 0x4E, 0x00,
	0x40, 0x7C, 0x12, 0x11, 0x12, 0x7C, 0x00,
	0x40, 0x7F, 0x49, 0x49, 0x49, 0x36, 0x00,
	0x40, 0x3E, 0x41, 0x41, 0x41, 0x22, 0x00,
	0x40, 0x7F, 0x41, 0x41, 0x41, 0x3E, 0x00,
	0x40, 0x7F, 0x49, 0x49, 0x49, 0x41, 0x00,
	0x40, 0x7F, 0x09, 0x09, 0x09, 0x01, 0x00,
	0x40, 0x3E, 0x41, 0x41, 0x51, 0x73, 0x00,
	0x40, 0x7F, 0x08, 0x08, 0x08, 0x7F, 0x00,
	0x40, 0x00, 0x41, 0x7F, 0x41, 0x00, 0x00,
	0x40, 0x20, 0x40, 0x41, 0x3F, 0x01, 0x00,
	0x40, 0x7F, 0x08, 0x14, 0x22, 0x41, 0x00,
	0x40, 0x7F, 0x40, 0x40, 0x40, 0x40, 0x00,
	0x40, 0x7F, 0x02, 0x1C, 0x02, 0x7F, 0x00,
	0x40, 0x7F, 0x04, 0x08, 0x10, 0x7F, 0x00,
	0x40, 0x3E, 0x41, 0x41, 0x41, 0x3E, 0x00,
	0x40, 0x7F, 0x09, 0x09, 0x09, 0x06, 0x00,
	0x40, 0x3E, 0x41, 0x51, 0x21, 0x5E, 0x00,
	0x40, 0x7F, 0x09, 0x19, 0x29, 0x46, 0x00,
	0x40, 0x26, 0x49, 0x49, 0x49, 0x32, 0x00,
	0x40, 0x03, 0x01, 0x7F, 0x01, 0x03, 0x00,
	0x40, 0x3F, 0x40, 0x40, 0x40, 0x3F, 0x00,
	0x40, 0x1F, 0x20, 0x40, 0x20, 0x1F, 0x00,
	0x40, 0x3F, 0x40, 0x38, 0x40, 0x3F, 0x00,
	0x40, 0x63, 0x14, 0x08, 0x14, 0x63, 0x00,
	0x40, 0x03, 0x04, 0x78, 0x04, 0x03, 0x00,
	0x40, 0x61, 0x59, 0x49, 0x4D, 0x43, 0x00,
	0x40, 0x00, 0x7F, 0x41, 0x41, 0x41, 0x00,
	0x40, 0x02, 0x04, 0x08, 0x10, 0x20, 0x00,
	0x40, 0x00, 0x41, 0x41, 0x41, 0x7F, 0x00,
	0x40, 0x04, 0x02, 0x01, 0x02, 0x04, 0x00,
	0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x00,
	0x40, 0x00, 0x03, 0x07, 0x08, 0x00, 0x00,
	0x40, 0x20, 0x54, 0x54, 0x78, 0x40, 0x00,
	0x40, 0x7F, 0x28, 0x44, 0x44, 0x38, 0x00,
	0x40, 0x38, 0x44, 0x44, 0x44, 0x28, 0x00,
	0x40, 0x38, 0x44, 0x44, 0x28, 0x7F, 0x00,
	0x40, 0x38, 0x54, 0x54, 0x54, 0x18, 0x00,
	0x40, 0x00, 0x08, 0x7E, 0x09, 0x02, 0x00,
	0x40, 0x18, 0xA4, 0xA4, 0x9C, 0x78, 0x00,
	0x40, 0x7F, 0x08, 0x04, 0x04, 0x78, 0x00,
	0x40, 0x00, 0x44, 0x7D, 0x40, 0x00, 0x00,
	0x40, 0x20, 0x40, 0x40, 0x3D, 0x00, 0x00,
	0x40, 0x7F, 0x10, 0x28, 0x44, 0x00, 0x00,
	0x40, 0x00, 0x41, 0x7F, 0x40, 0x00, 0x00,
	0x40, 0x7C, 0x04, 0x78, 0x04, 0x78, 0x00,
	0x40, 0x7C, 0x08, 0x04, 0x04, 0x78, 0x00,
	0x40, 0x38, 0x44, 0x44, 0x44, 0x38, 0x00,
	0x40, 0xFC, 0x18, 0x24, 0x24, 0x18, 0x00,
	0x40, 0x18, 0x24, 0x24, 0x18, 0xFC, 0x00,
	0x40, 0x7C, 0x08, 0x04, 0x04, 0x08, 0x00,
	0x40, 0x48, 0x54, 0x54, 0x54, 0x24, 0x00,
	0x40, 0x04, 0x04, 0x3F, 0x44, 0x24, 0x00,
	0x40, 0x3C, 0x40, 0x40, 0x20, 0x7C, 0x00,
	0x40, 0x1C, 0x20, 0x40, 0x20, 0x1C, 0x00,
	0x40, 0x3C, 0x40, 0x30, 0x40, 0x3C, 0x00,
	0x40, 0x44, 0x28, 0x10, 0x28, 0x44, 0x00,
	0x40, 0x4C, 0x90, 0x90, 0x90, 0x7C, 0x00,
	0x40, 0x44, 0x64, 0x54, 0x4C, 0x44, 0x00,
	0x40, 0x00, 0x08, 0x36, 0x41, 0x00, 0x00,
	0x40, 0x00, 0x00, 0x77, 0x00, 0x00, 0x00,
	0x40, 0x00, 0x41, 0x36, 0x08, 0x00, 0x00,
	0x40, 0x02, 0x01, 0x02, 0x04, 0x02, 0x00,
	0x40, 0x3C, 0x26, 0x23, 0x26, 0x3C, 0x00
};

static constexpr uint8_t kOled128x64Init[] __attribute__((aligned(4))) = {
		ssd1306::cmd::kDisplayOff,
		ssd1306::cmd::kSetDisplayclockdiv, 0x80, 	// The suggested value
		ssd1306::cmd::kSetMultiplex, 0x3F,			// 1/64
		ssd1306::cmd::kSetDisplayoffset, 0x00,		// No offset
		ssd1306::cmd::kSetStartline | 0x00,			// line #0
		ssd1306::cmd::kSetChargepump, 0x14,
		ssd1306::cmd::kSetMemorymode, 0x00,			// Horizontal addressing
		ssd1306::cmd::kSegremap | 0x01,				// Flip horizontally
		ssd1306::cmd::kComscanDec,					// Flip vertically
		ssd1306::cmd::kSetCompins, 0x12,
		ssd1306::cmd::kSetContrast, 0x7F,			// 0x00 to 0xFF
		ssd1306::cmd::kSetPrecharge, 0xF1,
		ssd1306::cmd::kSetVcomdetect, 0x40,
		ssd1306::cmd::kOuputRam,
		ssd1306::cmd::kDisplayNormal };

static constexpr uint8_t kOled128x32Init[] __attribute__((aligned(4))) = {
		ssd1306::cmd::kDisplayOff,
		ssd1306::cmd::kSetDisplayclockdiv, 0x80, 	// The suggested value
		ssd1306::cmd::kSetMultiplex, 0x1F,			// 1/32
		ssd1306::cmd::kSetDisplayoffset, 0x00,		// No offset
		ssd1306::cmd::kSetStartline | 0x00,			// line #0
		ssd1306::cmd::kSetChargepump, 0x14,
		ssd1306::cmd::kSetMemorymode, 0x00,			// Horizontal addressing
		ssd1306::cmd::kSegremap | 0x01,				// Flip horizontally
		ssd1306::cmd::kComscanDec,					// Flip vertically
		ssd1306::cmd::kSetCompins, 0x02,
		ssd1306::cmd::kSetContrast, 0x7F,			// 0x00 to 0xFF
		ssd1306::cmd::kSetPrecharge, 0xF1,
		ssd1306::cmd::kSetVcomdetect, 0x40,
		ssd1306::cmd::kOuputRam,
		ssd1306::cmd::kDisplayNormal };

static uint8_t s_clear_buffer[133 + 1] __attribute__((aligned(4)));

Ssd1306::Ssd1306() : hal_i2c_(OLED_I2C_ADDRESS_DEFAULT)
{
    assert(s_this == nullptr);
    s_this = this;

    InitMembers();
}

Ssd1306::Ssd1306(OledPanel oled_panel) : hal_i2c_(OLED_I2C_ADDRESS_DEFAULT), oled_panel_(oled_panel)
{
    assert(s_this == nullptr);
    s_this = this;

    InitMembers();
}

Ssd1306::Ssd1306(uint8_t address, OledPanel oled_panel) : hal_i2c_(address == 0 ? OLED_I2C_ADDRESS_DEFAULT : address), oled_panel_(oled_panel)
{
    assert(s_this == nullptr);
    s_this = this;

    InitMembers();
}

void Ssd1306::PrintInfo()
{
    printf("%s (%u,%u)\n", have_sh1106_ ? "SH1106" : "SSD1306", static_cast<unsigned int>(rows_), static_cast<unsigned int>(cols_));
}

void Ssd1306::CheckSH1106()
{
    // Check for columns 128-133
    SendCommand(ssd1306::cmd::kSetLowcolumn | (128 & 0XF));
    SendCommand(ssd1306::cmd::kSetHighcolumn | (128));
    SendCommand(ssd1306::cmd::kSetStartpage);

    constexpr uint8_t kATestBytes[5] = {ssd1306::mode::kData, 0xAA, 0xEE, 0xAA, 0xEE};
    SendData(kATestBytes, sizeof(kATestBytes));

    // Check for columns 128-133
    SendCommand(ssd1306::cmd::kSetLowcolumn | (128 & 0XF));
    SendCommand(ssd1306::cmd::kSetHighcolumn | (128));
    SendCommand(ssd1306::cmd::kSetStartpage);

    char result_bytes[5] = {0};

    hal_i2c_.Write(0x40);
    hal_i2c_.Read(result_bytes, sizeof(result_bytes));

#ifndef NDEBUG
    printf("%.2x %.2x %.2x %.2x %.2x\n", result_bytes[0], result_bytes[1], result_bytes[2], result_bytes[3], result_bytes[4]);
#endif

    have_sh1106_ = (memcmp(&kATestBytes[1], &result_bytes[1], 4) == 0);

#ifndef NDEBUG
    printf("have_sh1106_=%d\n", have_sh1106_);
#endif
}

bool Ssd1306::Start()
{
    if (!hal_i2c_.IsConnected())
    {
        return false;
    }

    switch (oled_panel_)
    {
        case OledPanel::k128x648Rows:
            for (size_t i = 0; i < sizeof(kOled128x64Init); i++)
            {
                SendCommand(kOled128x64Init[i]);
            }
            break;
        case OledPanel::k128x644Rows:
            /* no break */
        case OledPanel::k128x324Rows:
            for (size_t i = 0; i < sizeof(kOled128x32Init); i++)
            {
                SendCommand(kOled128x32Init[i]);
            }
            break;
        default:
            return false;
    }

    for (size_t i = 0; i < sizeof(s_clear_buffer); i++)
    {
        s_clear_buffer[i] = 0x00;
    }

    s_clear_buffer[0] = 0x40;

    CheckSH1106();

    Ssd1306::Cls();

    SendCommand(ssd1306::cmd::kDisplayOn);
    return true;
}

void Ssd1306::Cls()
{
    uint32_t column_add = 0;

    if (have_sh1106_)
    {
        column_add = 4;
    }

    for (uint32_t page = 0; page < pages_; page++)
    {
        SendCommand(ssd1306::cmd::kSetLowcolumn | (column_add & 0XF));
        SendCommand(static_cast<uint8_t>(ssd1306::cmd::kSetHighcolumn | (column_add)));
        SendCommand(static_cast<uint8_t>(ssd1306::cmd::kSetStartpage | page));
        SendData(reinterpret_cast<const uint8_t*>(&s_clear_buffer), (column_add + ssd1306::kLcdWidth + 1));
    }

    SendCommand(ssd1306::cmd::kSetLowcolumn | (column_add & 0XF));
    SendCommand(static_cast<uint8_t>(ssd1306::cmd::kSetHighcolumn | (column_add)));
    SendCommand(ssd1306::cmd::kSetStartpage);

#if defined(CONFIG_DISPLAY_ENABLE_CURSOR_MODE) || defined(CONFIG_DISPLAY_FIX_FLIP_VERTICALLY)
    shadow_ram_index_ = 0;
    memset(shadow_ram_, ' ', ssd1306::oled::font8x6::kCols * rows_);
#endif
}

void Ssd1306::PutChar(int c)
{
    int i;

    if (c < 32 || c > 127)
    {
#if defined(CONFIG_DISPLAY_ENABLE_CURSOR_MODE) || defined(CONFIG_DISPLAY_FIX_FLIP_VERTICALLY)
        c = 32;
#endif
        i = 0;
    }
    else
    {
        i = c - 32;
    }

#if defined(CONFIG_DISPLAY_ENABLE_CURSOR_MODE) || defined(CONFIG_DISPLAY_FIX_FLIP_VERTICALLY)
    shadow_ram_[shadow_ram_index_++] = static_cast<char>(c);
#endif
    const uint8_t* base = kOledFont8x6 + (ssd1306::oled::font8x6::kCharW + 1) * i;
    SendData(base, ssd1306::oled::font8x6::kCharW + 1);
}

void Ssd1306::PutString(const char* string)
{
    const char* p = string;

    while (*p != '\0')
    {
        Ssd1306::PutChar(static_cast<int>(*p));
        p++;
    }

    if (clear_end_of_line_)
    {
        clear_end_of_line_ = false;
        for (auto i = static_cast<uint32_t>(p - string); i < cols_; i++)
        {
            Ssd1306::PutChar(' ');
        }
    }
}

/**
 * line [1..4]
 */
void Ssd1306::ClearLine(uint32_t line)
{
    if (__builtin_expect((!(line <= rows_)), 0))
    {
        return;
    }

    Ssd1306::SetCursorPos(0, static_cast<uint8_t>(line - 1));
    SendData(reinterpret_cast<const uint8_t*>(&s_clear_buffer), ssd1306::kLcdWidth + 1);
    Ssd1306::SetCursorPos(0, static_cast<uint8_t>(line - 1));

#if defined(CONFIG_DISPLAY_ENABLE_CURSOR_MODE) || defined(CONFIG_DISPLAY_FIX_FLIP_VERTICALLY)
    memset(&shadow_ram_[shadow_ram_index_], ' ', ssd1306::oled::font8x6::kCols);
#endif
}

void Ssd1306::TextLine(uint32_t line, const char* data, uint32_t length)
{
    if (__builtin_expect((!(line <= rows_)), 0))
    {
        return;
    }

    Ssd1306::SetCursorPos(0, static_cast<uint8_t>(line - 1));
    Text(data, length);
}

void Ssd1306::Text(const char* data, uint32_t length)
{
    if (length > cols_)
    {
        length = cols_;
    }

    uint32_t i;

    for (i = 0; i < length; i++)
    {
        Ssd1306::PutChar(data[i]);
    }

    if (clear_end_of_line_)
    {
        clear_end_of_line_ = false;
        for (; i < cols_; i++)
        {
            Ssd1306::PutChar(' ');
        }
    }
}

/**
 * (0,0)
 */
void Ssd1306::SetCursorPos(uint32_t column, uint32_t row)
{
    if (__builtin_expect((!((column < ssd1306::oled::font8x6::kCols) && (row < rows_))), 0))
    {
        return;
    }

    column = static_cast<uint8_t>(column * ssd1306::oled::font8x6::kCharW);

    if (have_sh1106_)
    {
        column = static_cast<uint8_t>(column + 4);
    }

    SendCommand(ssd1306::cmd::kSetLowcolumn | (column & 0XF));
    SendCommand(static_cast<uint8_t>(ssd1306::cmd::kSetHighcolumn | (column >> 4)));
    SendCommand(static_cast<uint8_t>(ssd1306::cmd::kSetStartpage | row));

#if defined(CONFIG_DISPLAY_ENABLE_CURSOR_MODE) || defined(CONFIG_DISPLAY_FIX_FLIP_VERTICALLY)
    shadow_ram_index_ = static_cast<uint16_t>((row * ssd1306::oled::font8x6::kCols) + (column / ssd1306::oled::font8x6::kCharW));
#endif
#if defined(CONFIG_DISPLAY_ENABLE_CURSOR_MODE)
    if (cursor_mode_ == display::cursor::kOn)
    {
        SetCursorOff();
        SetCursorOn();
    }
    else if (cursor_mode_ == (display::cursor::kOn | display::cursor::kBlinkOn))
    {
        SetCursorOff();
        SetCursorBlinkOn();
    }
#endif
}

void Ssd1306::SetSleep(bool sleep)
{
    if (sleep)
    {
        SendCommand(ssd1306::cmd::kDisplayOff);
    }
    else
    {
        SendCommand(ssd1306::cmd::kDisplayOn);
    }
}

void Ssd1306::SetContrast(uint8_t contrast)
{
    SendCommand(ssd1306::cmd::kSetContrast);
    SendCommand(contrast);
}

void Ssd1306::SetFlipVertically(bool do_flip_vertically)
{
    if (do_flip_vertically)
    {
        SendCommand(ssd1306::cmd::kSegremap); ///< Data already stored in GDDRAM will have no changes.
        SendCommand(ssd1306::cmd::kComscanInc);
    }
    else
    {
        SendCommand(ssd1306::cmd::kSegremap | 0x01); ///< Data already stored in GDDRAM will have no changes.
        SendCommand(ssd1306::cmd::kComscanDec);
    }

#if defined(CONFIG_DISPLAY_FIX_FLIP_VERTICALLY)
    for (uint32_t i = 0; i < rows_; i++)
    {
        Ssd1306::SetCursorPos(0, static_cast<uint8_t>(i));
        for (uint32_t j = 0; j < ssd1306::oled::font8x6::kCols; j++)
        {
            const auto kN = shadow_ram_[i * ssd1306::oled::font8x6::kCols + j] - 32;
            const uint8_t* base = kOledFont8x6 + (ssd1306::oled::font8x6::kCharW + 1) * kN;
            SendData(base, ssd1306::oled::font8x6::kCharW + 1);
        }
    }
#endif
}

void Ssd1306::InitMembers()
{
    cols_ = ssd1306::oled::font8x6::kCols;

    switch (oled_panel_)
    {
        case OledPanel::k128x648Rows:
            rows_ = 64 / ssd1306::oled::font8x6::kCharH;
            break;
        case OledPanel::k128x644Rows: // Trick : 128x32
                                      /* no break */
        case OledPanel::k128x324Rows:
            rows_ = 32 / ssd1306::oled::font8x6::kCharH;
            break;
        default:
            rows_ = 64 / ssd1306::oled::font8x6::kCharH;
            break;
    }

    pages_ = (oled_panel_ == OledPanel::k128x648Rows ? 8 : 4);

#if defined(CONFIG_DISPLAY_ENABLE_CURSOR_MODE) || defined(CONFIG_DISPLAY_FIX_FLIP_VERTICALLY)
    shadow_ram_ = new char[ssd1306::oled::font8x6::kCols * rows_];
    assert(shadow_ram_ != nullptr);
    memset(shadow_ram_, ' ', ssd1306::oled::font8x6::kCols * rows_);
#endif
}

void Ssd1306::SendCommand(uint8_t cmd)
{
    hal_i2c_.WriteRegister(ssd1306::mode::kCommand, cmd);
}

void Ssd1306::SendData(const uint8_t* data, uint32_t length)
{
    hal_i2c_.Write(reinterpret_cast<const char*>(data), length);
}

/**
 *  Cursor mode support
 */

void Ssd1306::SetCursor([[maybe_unused]] uint32_t cursor_mode)
{
#if defined(CONFIG_DISPLAY_ENABLE_CURSOR_MODE)
    if (cursor_mode == cursor_mode_)
    {
        return;
    }

    cursor_mode_ = cursor_mode;

    switch (cursor_mode)
    {
        case display::cursor::kOff:
            SetCursorOff();
            break;
        case display::cursor::kOn:
            SetCursorOn();
            break;
        case display::cursor::kOn | display::cursor::kBlinkOn:
            SetCursorBlinkOn();
            break;
        default:
            break;
    }
#endif
}

void Ssd1306::SetCursorOn()
{
#if defined(CONFIG_DISPLAY_ENABLE_CURSOR_MODE)
    cursor_on_column_ = static_cast<uint8_t>(shadow_ram_index_ % ssd1306::oled::font8x6::kCols);
    cursor_on_row_ = static_cast<uint8_t>(shadow_ram_index_ / ssd1306::oled::font8x6::kCols);
    cursor_on_char_ = static_cast<uint8_t>(shadow_ram_[shadow_ram_index_] - 32);

    const auto* base = const_cast<uint8_t*>(kOledFont8x6) + 1 + (ssd1306::oled::font8x6::kCharW + 1) * cursor_on_char_;

    uint8_t data[ssd1306::oled::font8x6::kCharW + 1];
    data[0] = 0x40;

    for (uint32_t i = 1; i <= ssd1306::oled::font8x6::kCharW; i++)
    {
        data[i] = *base | 0x80;
        base++;
    }

    SendData(data, ssd1306::oled::font8x6::kCharW + 1);
    SetColumnRow(cursor_on_column_, cursor_on_row_);
#endif
}

void Ssd1306::SetCursorBlinkOn()
{
#if defined(CONFIG_DISPLAY_ENABLE_CURSOR_MODE)
    cursor_on_column_ = static_cast<uint8_t>(shadow_ram_index_ % ssd1306::oled::font8x6::kCols);
    cursor_on_row_ = static_cast<uint8_t>(shadow_ram_index_ / ssd1306::oled::font8x6::kCols);
    cursor_on_char_ = static_cast<uint8_t>(shadow_ram_[shadow_ram_index_] - 32);

    const uint8_t* base = const_cast<uint8_t*>(kOledFont8x6) + 1 + (ssd1306::oled::font8x6::kCharW + 1) * cursor_on_char_;

    uint8_t data[ssd1306::oled::font8x6::kCharW + 1];
    data[0] = 0x40;

    for (uint32_t i = 1; i <= ssd1306::oled::font8x6::kCharW; i++)
    {
        data[i] = static_cast<uint8_t>(~*base);
        base++;
    }

    SendData(data, static_cast<uint32_t>(ssd1306::oled::font8x6::kCharW + 1));
    SetColumnRow(cursor_on_column_, cursor_on_row_);
#endif
}

void Ssd1306::SetCursorOff()
{
#if defined(CONFIG_DISPLAY_ENABLE_CURSOR_MODE)
    const auto kCol = static_cast<uint8_t>(shadow_ram_index_ % ssd1306::oled::font8x6::kCols);
    const auto kRow = static_cast<uint8_t>(shadow_ram_index_ / ssd1306::oled::font8x6::kCols);

    SetColumnRow(cursor_on_column_, cursor_on_row_);

    const uint8_t* base = kOledFont8x6 + (ssd1306::oled::font8x6::kCharW + 1) * cursor_on_char_;

    SendData(base, (ssd1306::oled::font8x6::kCharW + 1));
    SetColumnRow(kCol, kRow);
#endif
}

void Ssd1306::SetColumnRow([[maybe_unused]] uint8_t column, [[maybe_unused]] uint8_t row)
{
#if defined(CONFIG_DISPLAY_ENABLE_CURSOR_MODE)
    auto column_add = static_cast<uint8_t>(column * ssd1306::oled::font8x6::kCharW);

    if (have_sh1106_)
    {
        column_add = static_cast<uint8_t>(column_add + 4);
    }

    SendCommand(ssd1306::cmd::kSetLowcolumn | (column_add & 0xF));
    SendCommand(ssd1306::cmd::kSetHighcolumn | static_cast<uint8_t>(column_add >> 4));
    SendCommand(ssd1306::cmd::kSetStartpage | row);
#endif
}

void Ssd1306::DumpShadowRam()
{
#if defined(CONFIG_DISPLAY_ENABLE_CURSOR_MODE) || defined(CONFIG_DISPLAY_FIX_FLIP_VERTICALLY)
#ifndef NDEBUG
    for (uint32_t i = 0; i < rows_; i++)
    {
        printf("%d: [%.*s]\n", i, ssd1306::oled::font8x6::kCols, &shadow_ram_[i * ssd1306::oled::font8x6::kCols]);
    }
#endif
#endif
}
