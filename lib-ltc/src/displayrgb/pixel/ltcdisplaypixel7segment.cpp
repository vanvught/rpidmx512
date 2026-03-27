/**
 * @file ltcdisplaypixel7segment.cpp
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

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC push_options
#pragma GCC optimize("O3")
#pragma GCC optimize("no-tree-loop-distribute-patterns")
#pragma GCC optimize("-fprefetch-loop-arrays")
#pragma GCC optimize("-funroll-loops")
#endif

#include <cstdint>
#include <cstdio>
#include <cassert>

#include "ltcdisplaypixel7segment.h"
#include "pixeldisplay7segment.h"
#include "pixeltype.h"
#include "firmware/debug/debug_debug.h"

LtcDisplayPixel7Segment::LtcDisplayPixel7Segment(pixel::LedType led_type, pixel::LedMap led_map)
{
    DEBUG_ENTRY();

    pixel_7segment_ = new WS28xxDisplay7Segment(led_type, led_map);
    assert(pixel_7segment_ != nullptr);

    DEBUG_EXIT();
}

void LtcDisplayPixel7Segment::Show(const char* timecode, struct ltc::display::rgb::Colours& colours, struct ltc::display::rgb::Colours& colours_colons)
{
    auto red = colours.red;
    auto green = colours.green;
    auto blue = colours.blue;

    const char kChars[] = {timecode[0], timecode[1], timecode[3], timecode[4], timecode[6], timecode[7], timecode[9], timecode[10]};
    assert(sizeof(kChars) <= WS28xxDisplay7SegmentConfig::NUM_OF_DIGITS);

    pixel_7segment_->WriteAll(kChars, red, green, blue);

    red = colours_colons.red;
    green = colours_colons.green;
    blue = colours_colons.blue;

    pixel_7segment_->WriteColon(timecode[ltc::timecode::index::COLON_1], 0, red, green, blue);
    pixel_7segment_->WriteColon(timecode[ltc::timecode::index::COLON_2], 1, red, green, blue);
    pixel_7segment_->WriteColon(timecode[ltc::timecode::index::COLON_3], 2, red, green, blue);
    pixel_7segment_->Show();
}

void LtcDisplayPixel7Segment::ShowSysTime(const char* system_time, struct ltc::display::rgb::Colours& colours,
                                          struct ltc::display::rgb::Colours& colours_colons)
{
    auto red = colours.red;
    auto green = colours.green;
    auto blue = colours.blue;

    const char kChars[] = {' ', ' ', system_time[0], system_time[1], system_time[3], system_time[4], system_time[6], system_time[7]};
    assert(sizeof(kChars) <= WS28xxDisplay7SegmentConfig::NUM_OF_DIGITS);

    pixel_7segment_->WriteAll(kChars, red, green, blue);

    red = colours_colons.red;
    green = colours_colons.green;
    blue = colours_colons.blue;

    pixel_7segment_->WriteColon(' ', 0, red, green, blue);
    pixel_7segment_->WriteColon(system_time[ltc::systemtime::index::COLON_1], 1, red, green, blue);
    pixel_7segment_->WriteColon(system_time[ltc::systemtime::index::COLON_2], 2, red, green, blue);
    pixel_7segment_->Show();
}

void LtcDisplayPixel7Segment::ShowMessage(const char* message, struct ltc::display::rgb::Colours& colours)
{
    assert(WS28xxDisplay7SegmentConfig::NUM_OF_DIGITS == ltc::display::rgb::kMaxMessageSize);

    const auto kRed = colours.red;
    const auto kGreen = colours.green;
    const auto kBlue = colours.blue;

    pixel_7segment_->WriteAll(message, kRed, kGreen, kBlue);
    pixel_7segment_->SetColonsOff();
    pixel_7segment_->Show();
}

void LtcDisplayPixel7Segment::WriteChar(uint8_t ch, uint8_t pos, struct ltc::display::rgb::Colours& colours)
{
    pixel_7segment_->WriteChar(static_cast<char>(ch), pos, colours.red, colours.green, colours.blue);
    pixel_7segment_->Show();
}

void LtcDisplayPixel7Segment::Print()
{
    printf(" 7-Segment %d Digit(s), %d Colons, %d LEDs\n", WS28xxDisplay7SegmentConfig::NUM_OF_DIGITS, WS28xxDisplay7SegmentConfig::NUM_OF_COLONS,
           WS28xxDisplay7SegmentConfig::LED_COUNT);
}
