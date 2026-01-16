/**
 * @file console_null.h
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

#ifndef CONSOLE_CONSOLE_NULL_H_
#define CONSOLE_CONSOLE_NULL_H_

#if !defined(CONSOLE_NULL)
#error File should not be included
#endif

namespace console
{
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

inline void Init() {}
inline void Putc([[maybe_unused]] int i) {}
inline void Puts([[maybe_unused]] const char* p) {}
inline void Write([[maybe_unused]] const char* p, [[maybe_unused]] unsigned int i) {}
inline void Status([[maybe_unused]] Colours c, [[maybe_unused]] const char* p) {}
inline void Error([[maybe_unused]] const char* p) {}
} // namespace console

#endif  // CONSOLE_CONSOLE_NULL_H_
