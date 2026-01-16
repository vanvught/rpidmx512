/**
 * @file console_uart0.h
 *
 */
/* Copyright (C) 2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef CONSOLE_CONSOLE_UART0_H_
#define CONSOLE_CONSOLE_UART0_H_

#if defined(CONSOLE_FB) || defined(CONSOLE_NULL) || defined(CONSOLE_I2C)
#error File should not be included
#endif

#include <cstdint>

namespace console
{
// https://github.com/shiena/ansicolor/blob/master/README.md
struct AnsiColours
{
    struct Fg
    {
        static constexpr char kBlack[] = "\x1b[30m";
        static constexpr char kRed[] = "\x1b[31m";
        static constexpr char kGreen[] = "\x1b[32m";
        static constexpr char kYellow[] = "\x1b[33m";
        static constexpr char kWhite[] = "\x1b[37m";
        static constexpr char kDefault[] = "\x1b[39m";
    };

    struct Bg
    {
        static constexpr char kBlack[] = "\x1b[40m";
        static constexpr char kRed[] = "\x1b[41m";
        static constexpr char kGreen[] = "\x1b[42m";
        static constexpr char kYellow[] = "\x1b[43m";
        static constexpr char kWhite[] = "\x1b[47m";
        static constexpr char kDefault[] = "\x1b[49m";
    };
};

enum class Colours
{
    kConsoleBlack,
    kConsoleRed,
    kConsoleGreen,
    kConsoleYellow,
    kConsoleBlue,
    kConsoleMagenta,
    kConsoleCyan,
    kConsoleWhite,
    kConsoleDefault
};

void Init();
void Putc(int);
void Puts(const char*);
void Write(const char*, unsigned int);
void Status(Colours, const char*);
void SetFgColour(uint32_t);
void SetBgColour(uint32_t);
void Error(const char*);

} // namespace console

#endif  // CONSOLE_CONSOLE_UART0_H_
