/**
 * @file console.cpp
 *
 */
/* Copyright (C) 2018-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include <cstring>

#include "console.h"

namespace uart0
{
void Init();
void Putc(int);
void Puts(const char*);
} // namespace uart0

namespace console
{
void SetFgColour(Colours fg);

void Putc(int c)
{
    uart0::Putc(c);
}

void Puts(const char* s)
{
    uart0::Puts(s);
}

void SetFgColour(Colours fg)
{
    switch (fg)
    {
        case console::Colours::kConsoleBlack:
            Write(AnsiColours::Fg::kBlack, sizeof(AnsiColours::Fg::kBlack) - 1);
            break;
        case console::Colours::kConsoleRed:
            Write(AnsiColours::Fg::kRed, sizeof(AnsiColours::Fg::kRed) - 1);
            break;
        case console::Colours::kConsoleGreen:
            Write(AnsiColours::Fg::kGreen, sizeof(AnsiColours::Fg::kGreen) - 1);
            break;
        case console::Colours::kConsoleYellow:
            Write(AnsiColours::Fg::kYellow, sizeof(AnsiColours::Fg::kYellow) - 1);
            break;
        case console::Colours::kConsoleWhite:
            Write(AnsiColours::Fg::kWhite, sizeof(AnsiColours::Fg::kWhite) - 1);
            break;
        default:
            Write(AnsiColours::Fg::kDefault, sizeof(AnsiColours::Fg::kDefault) - 1);
            break;
    }
}

void SetBgColour(Colours bg)
{
    switch (bg)
    {
        case console::Colours::kConsoleBlack:
            Write(AnsiColours::Bg::kBlack, sizeof(AnsiColours::Bg::kBlack) - 1);
            break;
        case console::Colours::kConsoleRed:
            Write(AnsiColours::Bg::kRed, sizeof(AnsiColours::Bg::kRed) - 1);
            break;
        case console::Colours::kConsoleWhite:
            Write(AnsiColours::Bg::kWhite, sizeof(AnsiColours::Bg::kWhite) - 1);
            ;
            break;
        default:
            Write(AnsiColours::Bg::kDefault, sizeof(AnsiColours::Bg::kDefault) - 1);
            break;
    }
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
    Write(AnsiColours::Fg::kRed, sizeof(AnsiColours::Fg::kRed) - 1);
    Write(s, strlen(s));
    Write(AnsiColours::Fg::kDefault, sizeof(AnsiColours::Fg::kDefault) - 1);
}

void Status(Colours colour, const char* s)
{
    SetFgColour(colour);
    Puts(s);
    SetFgColour(Colours::kConsoleDefault);
}

void __attribute__((cold)) Init()
{
    uart0::Init();

    SetFgColour(Colours::kConsoleWhite);
    SetBgColour(Colours::kConsoleBlack);
}
} // namespace console
