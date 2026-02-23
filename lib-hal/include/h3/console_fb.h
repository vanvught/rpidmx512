/**
 * @file console_fb.h
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

#ifndef H3_CONSOLE_FB_H_
#define H3_CONSOLE_FB_H_

#if !defined ORANGE_PI_ONE
#error Support for Orange Pi One only
#endif

#include <cstdint>

namespace console
{
// some RGB color definitions
enum class Colours: uint32_t
{
    kConsoleBlack = 0x00000000,  ///<   0,   0,   0
    kConsoleBlue = 0x000000FF,   ///<   0,   0, 255
    kConsoleGreen = 0x0000FF00,  ///<   0, 255,   0
    kConsoleCyan = 0x0000FFFF,   ///<   0, 255, 255
    kConsoleRed = 0x00FF0000,    ///< 255,   0,   0
    kConsoleYellow = 0x00FFFF00, ///< 255, 255,   0
    kConsoleWhite = 0x00FFFFFF   ///< 255, 255, 255
};

void Init();
uint32_t GetLineWidth();
void Clear();
void SetTopRow(uint32_t);
void ClearTopRow();
void ClearLine(uint32_t);
void SetCursor(uint32_t, uint32_t);
void SaveCursor();
void RestoreCursor();
void SaveColour();
void RestoreColour();
void Putc(int);
void ConsolePuthex(uint8_t);
void SetFgColour(Colours);
void SetBgColour(Colours);
void SetFgBgColour(Colours, Colours);
void PuthexFgBg(uint8_t, Colours, Colours);
void PutpctFgBg(uint8_t, Colours, Colours);
void Put3decFgBg(uint8_t, Colours, Colours);
void Write(const char*, unsigned int);
void Puts(const char*);
void Status(Colours, const char*);
void Error(const char*);
} // namespace console

#endif  // H3_CONSOLE_FB_H_
