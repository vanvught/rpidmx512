/**
 * @file ltcdisplaypixel7segment.h
 */
/*
 * Copyright (C) 2019-2020 by hippy mailto:dmxout@gmail.com
 * Copyright (C) 2019-2026 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef LTCDISPLAYPIXEL7SEGMENT_H_
#define LTCDISPLAYPIXEL7SEGMENT_H_

#include <cstdint>

#include "pixeldisplay7segment.h"
#include "ltcdisplayrgbset.h"

class LtcDisplayPixel7Segment final : public LtcDisplayRgbSet
{
   public:
    LtcDisplayPixel7Segment(pixel::LedType led_type, pixel::LedMap led_map);

    void Show(const char* timecode, struct ltc::display::rgb::Colours& colours, struct ltc::display::rgb::Colours& colours_colons) override;
    void ShowSysTime(const char* systemtime, struct ltc::display::rgb::Colours& colours, struct ltc::display::rgb::Colours& colours_colons) override;
    void ShowMessage(const char* message, struct ltc::display::rgb::Colours& colours) override;
    void WriteChar(uint8_t ch, uint8_t pos, struct ltc::display::rgb::Colours& colours) override;
    void Print() override;

   private:
    WS28xxDisplay7Segment* pixel_7segment_;
};

#endif // LTCDISPLAYPIXEL7SEGMENT_H_
