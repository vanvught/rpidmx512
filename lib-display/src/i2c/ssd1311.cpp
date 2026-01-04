/**
 * @file ssd1311.cpp
 *
 */
/* Copyright (C) 2020-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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
#include <cstdio>
#include <cstring>
#include <cassert>
#include "displayset.h"

#include "i2c/ssd1311.h"

// Co – Continuation bit
// D/C# – Data / Command Selection bit
// A control byte mainly consists of Co and D/C# bits following by six “0”’s.

namespace ssd1311
{
static constexpr uint8_t kDefaultI2CAddress = 0x3C;
static constexpr uint32_t kMaxColumns = 20;
static constexpr uint32_t kMaxRows = 4;
static constexpr uint8_t kModeData = 0x40; // Co = 0, D/C# = 1
static constexpr uint8_t kModeCmd = 0x80;  // Co = 1, D/C# = 0
} // namespace ssd1311
enum class Rom : uint8_t
{
    kA,
    kB,
    kC
};
namespace cmd
{
static constexpr uint8_t kClearDisplay = 0x01;
static constexpr uint8_t kCgramAddress = 0x40;
static constexpr uint8_t kFunctionSelectionB = 0x72;
static constexpr uint8_t kDdramAddress = 0x80;
static constexpr uint8_t kContrast = 0x81;
} // namespace cmd

static uint8_t clear_buffer[1 + ssd1311::kMaxColumns] __attribute__((aligned(4)));
static uint8_t text_buffer[1 + ssd1311::kMaxColumns] __attribute__((aligned(4)));

Ssd1311::Ssd1311() : hal_i2_c_(ssd1311::kDefaultI2CAddress)
{
    assert(s_this == nullptr);
    s_this = this;

    rows_ = ssd1311::kMaxRows;
    cols_ = ssd1311::kMaxColumns;
}

bool Ssd1311::Start()
{
    if (!hal_i2_c_.IsConnected())
    {
        return false;
    }

    if (!CheckSSD1311())
    {
        return false;
    }

    for (uint32_t i = 0; i < sizeof(clear_buffer); i++)
    {
        clear_buffer[i] = ' ';
    }

    clear_buffer[0] = ssd1311::kModeData;
    text_buffer[0] = ssd1311::kModeData;

    SendCommand(0x3A);
    SendCommand(0x09);
    SendCommand(0x05);
    SendCommand(0x1C);
    SendCommand(0x3C);
    SendCommand(0x3A);
    SendCommand(0x72);
    SendData(0x00);
    SendCommand(0x3C);
    SendCommand(0x0C);
    SendCommand(0x01);

    SelectRamRom(0, static_cast<uint8_t>(Rom::kA));

    return true;
}

void Ssd1311::PrintInfo()
{
    printf("SSD1311 (%u,%u)\n", static_cast<unsigned int>(rows_), static_cast<unsigned int>(cols_));
}

void Ssd1311::Cls()
{
    SendCommand(cmd::kClearDisplay);
}

void Ssd1311::PutChar(int c)
{
    SendData(c & 0x7F);
}

void Ssd1311::PutString(const char* string)
{
    assert(string != nullptr);

    uint32_t n = ssd1311::kMaxColumns;
    auto* src = string;
    auto* dst = reinterpret_cast<char*>(&text_buffer[1]);

    while (n > 0 && *src != '\0')
    {
        *dst++ = *src++;
        --n;
    }

    if (clear_end_of_line_)
    {
        clear_end_of_line_ = false;

        for (auto i = static_cast<uint32_t>(src - string); i < ssd1311::kMaxColumns; i++)
        {
            *dst++ = ' ';
        }

        SendData(text_buffer, 1U + ssd1311::kMaxColumns);
        return;
    }

    SendData(text_buffer, 1U + ssd1311::kMaxColumns - n);
}

/**
 * line [1..4]
 */
void Ssd1311::ClearLine(uint32_t line)
{
    if (__builtin_expect((!((line > 0) && (line <= ssd1311::kMaxRows))), 0))
    {
        return;
    }

    Ssd1311::SetCursorPos(0, static_cast<uint8_t>(line - 1));
    SendData(clear_buffer, sizeof(clear_buffer));
    Ssd1311::SetCursorPos(0, static_cast<uint8_t>(line - 1));
}

void Ssd1311::TextLine(uint32_t line, const char* data, uint32_t length)
{
    if (__builtin_expect((!((line > 0) && (line <= ssd1311::kMaxRows))), 0))
    {
        return;
    }

    Ssd1311::SetCursorPos(0, static_cast<uint8_t>(line - 1));
    Text(data, length);
}

void Ssd1311::Text(const char* data, uint32_t length)
{
    if (length > ssd1311::kMaxColumns)
    {
        length = ssd1311::kMaxColumns;
    }

    memcpy(&text_buffer[1], data, length);

    if (clear_end_of_line_)
    {
        clear_end_of_line_ = false;

        memset(&text_buffer[length + 1], ' ', ssd1311::kMaxColumns - length);

        SendData(text_buffer, 1U + ssd1311::kMaxColumns);
        return;
    }

    SendData(text_buffer, 1U + length);
}

/**
 * (0,0)
 */
void Ssd1311::SetCursorPos(uint32_t col, uint32_t row)
{
    if (__builtin_expect((!((col < ssd1311::kMaxColumns) && (row < ssd1311::kMaxRows))), 0))
    {
        return;
    }

    // In 4-line display mode (N=1, NW = 1), DDRAM address is from “00H” – “13H” in the 1st line, from
    // “20H” to “33H” in the 2nd line, from “40H” – “53H” in the 3rd line and from “60H” – “73H” in the 4th line.

    SetDDRAM(static_cast<uint8_t>((col + row * 0x20)));
}

/**
 * 9.2.2 Function Selection B [72h]
 *
 * The character number of the Character Generator RAM and
 * the character ROM can be selected through this command
 *
 * It is recommended to turn off the display (cmd 08h) before setting no. of CGRAM and defining character ROM,
 * while clear display (cmd 01h) is recommended to sent afterwards
 */
void Ssd1311::SelectRamRom(uint32_t ram, uint32_t rom)
{
    // [IS=X,RE=1,SD=0]
    Ssd1311::SetSleep(true);

    SetRE(FunctionSet::kReOne);

    SendCommand(cmd::kFunctionSelectionB);
    SendCommand(static_cast<uint8_t>(((rom & 0x03) << 2) | (ram & 0x03)));

    SetRE(FunctionSet::kReZero);

    Ssd1311::SetSleep(false);
    Ssd1311::Cls();
}

void Ssd1311::SetDDRAM(uint8_t address)
{
    // [IS=X,RE=0,SD=0]
    SendCommand(cmd::kDdramAddress | (address & 0x7F));
}

void Ssd1311::SetCGRAM(uint8_t address)
{
    // [IS=0,RE=0,SD=0]
    SendCommand(cmd::kCgramAddress | (address & 0x3F));
}

void Ssd1311::SetRE(FunctionSet re)
{
    uint8_t cmd = 0x20;

    constexpr auto kN = 1;
    constexpr auto kDh = 0;

    if (re == FunctionSet::kReZero)
    {
        // 0 0 1 0 N DH RE IS
        // N - Numbers of display line
        // DH - Double height font control
        // RE register set to 0
        // IS register
        constexpr auto kIs = 0;
        cmd |= (kN << 3);
        cmd |= (kDh << 2);
        cmd |= (0 << 1);
        cmd |= (kIs << 0);
    }
    else
    {
        // 0 0 1 0 N BE RE REV
        // N - Numbers of display line
        // BE - CGRAM blink enable
        // RE register set to 1
        // REV - reverse display
        constexpr auto kBe = 0;
        constexpr auto kRev = 0;
        cmd |= (kN << 3);
        cmd |= (kBe << 2);
        cmd |= (1 << 1);
        cmd |= (kRev << 0);
    }

    SendCommand(cmd);
}

void Ssd1311::SetSD(CommandSet sd)
{
    SetRE(FunctionSet::kReOne);
    SendCommand(sd == CommandSet::kDisabled ? 0x78 : 0x79);
}

void Ssd1311::SendCommand(uint8_t command)
{
    hal_i2_c_.WriteRegister(ssd1311::kModeCmd, command);
}

void Ssd1311::SendData(uint8_t data)
{
    hal_i2_c_.WriteRegister(ssd1311::kModeData, data);
}

void Ssd1311::SendData(const uint8_t* data, uint32_t length)
{
    hal_i2_c_.Write(reinterpret_cast<const char*>(data), length);
}

bool Ssd1311::CheckSSD1311()
{
    SetCGRAM(0);

    const uint8_t kDataSend[] = {ssd1311::kModeData, 0xAA, 0x55, 0xAA, 0x55};

    SendData(kDataSend, sizeof(kDataSend));

    uint8_t data_receive[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    static_assert((1 + sizeof(kDataSend)) == sizeof(data_receive), "Mismatch buffers");

    SetCGRAM(0);
    hal_i2_c_.Write(ssd1311::kModeData);
    hal_i2_c_.Read(reinterpret_cast<char*>(data_receive), sizeof(data_receive));

    const auto kIsEqual = (memcmp(&kDataSend[1], &data_receive[1], sizeof(kDataSend) - 1) == 0);

#ifndef NDEBUG
    printf("CheckSSD1311 kIsEqual=%d\n", kIsEqual);
#endif

    return kIsEqual;
}

/**
 * 9.1.4 Display ON/OFF Control
 */

constexpr auto kDisplayOnOff = (1U << 2);
#if defined(CONFIG_DISPLAY_ENABLE_CURSOR_MODE)
constexpr auto kCursorOnOff = (1U << 1);
constexpr auto kCursorBlinkOnOff = (1U << 0);
#endif

void Ssd1311::SetSleep(bool sleep)
{
    if (sleep)
    {
        display_control_ &= static_cast<uint8_t>(~kDisplayOnOff);
    }
    else
    {
        display_control_ |= kDisplayOnOff;
    }

    SendCommand(display_control_);
}

void Ssd1311::SetContrast(uint8_t contrast)
{
    // [IS=X,RE=1,SD=1]
    SetRE(FunctionSet::kReOne);
    SetSD(CommandSet::kEnabled);

    // This is a two bytes command
    SendCommand(cmd::kContrast);
    SendCommand(contrast);

    SetSD(CommandSet::kDisabled);
    SetRE(FunctionSet::kReZero);
}

void Ssd1311::SetCursor([[maybe_unused]] uint32_t mode)
{
#if defined(CONFIG_DISPLAY_ENABLE_CURSOR_MODE)
    switch (static_cast<int>(mode))
    {
        case display::cursor::kOff:
            display_control_ &= static_cast<uint8_t>(~kCursorOnOff);
            break;
        case display::cursor::kOn:
            display_control_ |= kCursorOnOff;
            display_control_ &= static_cast<uint8_t>(~kCursorBlinkOnOff);
            break;
        case display::cursor::kOn | display::cursor::kBlinkOn:
            display_control_ |= kCursorOnOff;
            display_control_ |= kCursorBlinkOnOff;
            break;
        default:
            break;
    }

    SendCommand(display_control_);
#endif
}
