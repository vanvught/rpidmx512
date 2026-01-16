/**
 * @file ltcdisplayrgbset.h
 */
/* Copyright (C) 2019-2023 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef LTCDISPLAYRGBSET_H_
#define LTCDISPLAYRGBSET_H_

#include <cstdint>

#include "ltc.h"

namespace ltc::display::rgb
{
struct Colours
{
    uint8_t red;
    uint8_t green;
    uint8_t blue;
};
static constexpr auto kMaxMessageSize = 8;
} // namespace ltc::display::rgb

class LtcDisplayRgbSet
{
   public:
    virtual ~LtcDisplayRgbSet() {}

    virtual void Init() {}

    virtual void Show(const char* timecode, struct ltc::display::rgb::Colours& colours, struct ltc::display::rgb::Colours& colours_colons) = 0;
    virtual void ShowSysTime(const char* systemtime, struct ltc::display::rgb::Colours& colours, struct ltc::display::rgb::Colours& colours_colons) = 0;
    virtual void ShowMessage(const char* message, struct ltc::display::rgb::Colours& colours) = 0;

    virtual void WriteChar(uint8_t ch, uint8_t pos, struct ltc::display::rgb::Colours& colours) = 0;

    virtual void ShowFPS([[maybe_unused]] ltc::Type type, [[maybe_unused]] struct ltc::display::rgb::Colours& colours) {}
    virtual void ShowSource([[maybe_unused]] ltc::Source source, [[maybe_unused]] struct ltc::display::rgb::Colours& colours) {}
    virtual void ShowInfo([[maybe_unused]] const char* info, [[maybe_unused]] uint32_t length, [[maybe_unused]] struct ltc::display::rgb::Colours& colours) {}

    virtual void Print() = 0;
};

#endif  // LTCDISPLAYRGBSET_H_
