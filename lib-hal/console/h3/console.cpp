/**
 * @file console.cpp
 *
 */
/* Copyright (C) 2019-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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
 
#pragma GCC push_options
#pragma GCC optimize("O2")
#pragma GCC optimize("no-tree-loop-distribute-patterns")
#pragma GCC optimize("-fprefetch-loop-arrays")

#include <cstdarg>
#include <cstdint>
#include <cstring>

#include "console.h"
#include "device/fb.h"
#include "arm/arm.h"

extern unsigned char font[] __attribute__((aligned(4)));

namespace console
{
static const uint32_t kFbCharW = 8;
static const uint32_t kFbCharH = 16;

static uint32_t current_x = 0;
static uint32_t current_y = 0;
static uint32_t saved_x = 0;
static uint32_t saved_y = 0;

static uint32_t top_row = 0;

static auto cur_fore = Colours::kConsoleWhite;
static auto cur_back = Colours::kConsoleBlack;
static auto saved_fore = Colours::kConsoleWhite;
static auto saved_back = Colours::kConsoleBlack;

#if defined(USE_UBOOT_HDMI)
#undef FB_WIDTH
#define FB_WIDTH fb_width

#undef FB_HEIGHT
#define FB_HEIGHT fb_height

#undef FB_PITCH
#define FB_PITCH fb_pitch

#undef FB_ADDRESS
#define FB_ADDRESS fb_addr
#endif

void Clear();
void ClearLine(uint32_t);

void __attribute__((cold)) Init()
{
    const int kR = fb_init();

    if (kR == FB_OK)
    {
        Clear();
    }
}

uint32_t GetLineWidth()
{
    return FB_WIDTH / kFbCharW;
}

void SetTopRow(uint32_t row)
{
    if (row > FB_HEIGHT / kFbCharH)
    {
        top_row = 0;
    }
    else
    {
        top_row = row;
    }

    current_x = 0;
    current_y = row;
}

static void ClearRow(uint32_t* address)
{
    uint32_t i;

    for (i = 0; i < (kFbCharH * FB_WIDTH); i++)
    {
        *address++ = static_cast<uint32_t>(cur_back);
    }
}

static void Newline()
{
    uint32_t i;
    uint32_t* address;
    uint32_t* to;
    uint32_t* from;

    current_y++;
    current_x = 0;

    if (current_y == FB_HEIGHT / kFbCharH)
    {
        if (top_row == 0)
        {
            /* Pointer to row = 0 */
            to = reinterpret_cast<uint32_t*>(FB_ADDRESS);
            /* Pointer to row = 1 */
            from = to + (kFbCharH * FB_WIDTH);
            /* Copy block from {row = 1, rows} to {row = 0, rows - 1} */
            i = ((FB_HEIGHT - kFbCharH) * FB_WIDTH);
        }
        else
        {
            to = reinterpret_cast<uint32_t*>(FB_ADDRESS) + ((kFbCharH * FB_WIDTH) * top_row);
            from = to + (kFbCharH * FB_WIDTH);
            i = ((FB_HEIGHT - kFbCharH) * FB_WIDTH - ((kFbCharH * FB_WIDTH) * top_row));
        }

        memcpy_blk(to, from, i / 8);

        /* Clear last row */
        address = reinterpret_cast<uint32_t*>(FB_ADDRESS) + ((FB_HEIGHT - kFbCharH) * FB_WIDTH);
        ClearRow(address);

        current_y--;
    }
}

static void DrawPixel(uint32_t x, uint32_t y, Colours colour)
{
    volatile auto* address = reinterpret_cast<volatile uint32_t*>(FB_ADDRESS + (x * FB_BYTES_PER_PIXEL) + (y * FB_WIDTH * FB_BYTES_PER_PIXEL));
    *address = static_cast<uint32_t>(colour);
}

static void DrawChar(int c, uint32_t x, uint32_t y, Colours fore, Colours back)
{
    uint32_t i, j;
    uint8_t line;
    unsigned char* p = font + (c * static_cast<int>(kFbCharH));

    for (i = 0; i < kFbCharH; i++)
    {
        line = *p++;
        for (j = x; j < (kFbCharW + x); j++)
        {
            if ((line & 0x1) != 0)
            {
                DrawPixel(j, y, fore);
            }
            else
            {
                DrawPixel(j, y, back);
            }
            line >>= 1;
        }
        y++;
    }
}

int ConsoleDrawChar(int ch, uint16_t x, uint16_t y, Colours fore, Colours back)
{
    DrawChar(ch, x * kFbCharW, y * kFbCharH, fore, back);
    return ch;
}

void Putc(int ch)
{
    if (ch == '\n')
    {
        Newline();
    }
    else if (ch == '\r')
    {
        current_x = 0;
    }
    else if (ch == '\t')
    {
        current_x += 4;
    }
    else
    {
        DrawChar(ch, current_x * kFbCharW, current_y * kFbCharH, cur_fore, cur_back);
        current_x++;
        if (current_x == FB_WIDTH / kFbCharW)
        {
            Newline();
        }
    }
}

void Puts(const char* s)
{
    char c;

    while ((c = *s++) != 0)
    {
        Putc(static_cast<int>(c));
    }

    Putc('\n');
}

void Write(const char* s, unsigned int n)
{
    char c;

    while (((c = *s++) != 0) && (n-- != 0))
    {
        Putc(static_cast<int>(c));
    }
}

void Error(const char* s)
{
    auto fore_current = cur_fore;
    auto back_current = cur_back;

    cur_fore = console::Colours::kConsoleRed;
    cur_back = console::Colours::kConsoleBlack;

	static constexpr char kPrefix[] = "Error <";

    Write(kPrefix, sizeof(kPrefix) - 1);
    Write(const_cast<char*>(s), strlen(s));
    Puts(">");

    cur_fore = fore_current;
    cur_back = back_current;
}

void Status(Colours colour, const char* s)
{
    const auto kForeCurrent = cur_fore;
    const auto kBackCurrent = cur_back;

    const uint32_t kY = current_y;
    const uint32_t kX = current_x;

    ClearLine(29);

    cur_fore = colour;
    cur_back = console::Colours::kConsoleBlack;

    Write(const_cast<char*>(s), strlen(s));

    current_y = kY;
    current_x = kX;

    cur_fore = kForeCurrent;
    cur_back = kBackCurrent;
}

#define TO_HEX(i) ((i) < 10) ? '0' + (i) : 'A' + ((i) - 10)

void ConsolePuthex(uint8_t data)
{
    Putc((TO_HEX(((data & 0xF0) >> 4))));
    Putc((TO_HEX(data & 0x0F)));
}

void PuthexFgBg(uint8_t data, Colours fore, Colours back)
{
    auto fore_current = cur_fore;
    auto back_current = cur_back;

    cur_fore = fore;
    cur_back = back;

    Putc((TO_HEX(((data & 0xF0) >> 4))));
    Putc((TO_HEX(data & 0x0F)));

    cur_fore = fore_current;
    cur_back = back_current;
}

void PutpctFgBg(uint8_t data, Colours fore, Colours back)
{
    auto fore_current = cur_fore;
    auto back_current = cur_back;

    cur_fore = fore;
    cur_back = back;

    if (data < 100)
    {
        Putc('0' + (data / 10U));
        Putc('0' + (data % 10U));
    }
    else
    {
        Write("%%", 2);
    }

    cur_fore = fore_current;
    cur_back = back_current;
}

void Put3decFgBg(uint8_t data, Colours fore, Colours back)
{
    auto fore_current = cur_fore;
    auto back_current = cur_back;

    cur_fore = fore;
    cur_back = back;

    const int kI = data / 100U;

    Putc('0' + kI);

    data = (data - static_cast<uint8_t>(kI * 100));

    Putc('0' + (data / 10U));
    Putc('0' + (data % 10U));

    cur_fore = fore_current;
    cur_back = back_current;
}

void ConsoleNewline()
{
    Newline();
}

void Clear()
{
    auto* address = reinterpret_cast<uint32_t*>(fb_addr);
    uint32_t i;

    for (i = 0; i < (FB_HEIGHT * FB_WIDTH); i++)
    {
        *address++ = static_cast<uint32_t>(cur_back);
    }

    current_x = 0;
    current_y = 0;
}

void SetCursor(uint32_t x, uint32_t y)
{
    if (x > FB_WIDTH / kFbCharW)
    {
        current_x = 0;
    }
    else
    {
        current_x = x;
    }

    if (y > FB_HEIGHT / kFbCharH)
    {
        current_y = 0;
    }
    else
    {
        current_y = y;
    }
}

void SaveCursor()
{
    saved_y = current_y;
    saved_x = current_x;
    saved_back = cur_back;
    saved_fore = cur_fore;
}

void RestoreCursor()
{
    current_y = saved_y;
    current_x = saved_x;
    cur_back = saved_back;
    cur_fore = saved_fore;
}

void ConsoleSaveColor()
{
    saved_back = cur_back;
    saved_fore = cur_fore;
}

void ConsoleRestoreColor()
{
    cur_back = saved_back;
    cur_fore = saved_fore;
}

void SetFgColour(Colours fore)
{
    cur_fore = fore;
}

void SetBgColour(Colours back)
{
    cur_back = back;
}

void SetFgBgColour(Colours fore, Colours back)
{
    cur_fore = fore;
    cur_back = back;
}

void ClearLine(uint32_t line)
{
    uint32_t* address;

    if (line > FB_HEIGHT / kFbCharH)
    {
        return;
    }
    else
    {
        current_y = line;
    }

    current_x = 0;

    address = reinterpret_cast<uint32_t*>(fb_addr) + (line * kFbCharH * FB_WIDTH);
    ClearRow(address);
}

void ClearTopRow()
{
    uint32_t line;
    uint32_t* address;

    for (line = top_row; line < (FB_HEIGHT / kFbCharH) - 1; line++)
    {
        address = reinterpret_cast<uint32_t*>(fb_addr) + (line * kFbCharH * FB_WIDTH);
        ClearRow(address);
    }

    current_x = 0;
    current_y = top_row;
}

void ConsolePutpixel(uint32_t x, uint32_t y, Colours colour)
{
    DrawPixel(x, y, colour);
}
} // namespace console

#pragma GCC pop_options