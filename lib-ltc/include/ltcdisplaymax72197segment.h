/**
 * @file ltcdisplaymax72197segment.h
 *
 */
/* Copyright (C) 2019-2020 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef LTCDISPLAYMAX72197SEGMENT_H_
#define LTCDISPLAYMAX72197SEGMENT_H_

#include <cstdint>
#include <cassert>

#include "ltcdisplaymax7219set.h"
#include "max72197segment.h"

class LtcDisplayMax72197Segment final : public LtcDisplayMax7219Set, public Max72197Segment
{
   public:
    explicit LtcDisplayMax72197Segment(uint8_t intensity)
    {
        assert(s_this == nullptr);
        s_this = this;

        Max72197Segment::Init(intensity);

        WriteRegister(max7219::reg::DIGIT6, 0x80);
        WriteRegister(max7219::reg::DIGIT4, 0x80, false);
        WriteRegister(max7219::reg::DIGIT2, 0x80, false);
    }

    ~LtcDisplayMax72197Segment() override 
    {
		s_this = nullptr;
	}

    void SetIntensity(uint8_t intensity) override { Max72197Segment::SetIntensity(intensity); }

    void Show(const char* timecode) override
    {
        WriteRegister(max7219::reg::DIGIT7, static_cast<uint32_t>(timecode[0] - '0'));
        WriteRegister(max7219::reg::DIGIT6, static_cast<uint32_t>((timecode[1] - '0') | 0x80), false);
        WriteRegister(max7219::reg::DIGIT5, static_cast<uint32_t>(timecode[3] - '0'), false);
        WriteRegister(max7219::reg::DIGIT4, static_cast<uint32_t>((timecode[4] - '0') | 0x80), false);
        WriteRegister(max7219::reg::DIGIT3, static_cast<uint32_t>(timecode[6] - '0'), false);
        WriteRegister(max7219::reg::DIGIT2, static_cast<uint32_t>((timecode[7] - '0') | 0x80), false);
        WriteRegister(max7219::reg::DIGIT1, static_cast<uint32_t>(timecode[9] - '0'), false);
        WriteRegister(max7219::reg::DIGIT0, static_cast<uint32_t>(timecode[10] - '0'), false);
    }

    void ShowSysTime(const char* system_time) override
    {
        WriteRegister(max7219::reg::DIGIT7, max7219::digit::BLANK);
        WriteRegister(max7219::reg::DIGIT6, static_cast<uint32_t>(system_time[0] - '0'), false);
        WriteRegister(max7219::reg::DIGIT5, static_cast<uint32_t>((system_time[1] - '0') | 0x80), false);
        WriteRegister(max7219::reg::DIGIT4, static_cast<uint32_t>(system_time[3] - '0'), false);
        WriteRegister(max7219::reg::DIGIT3, static_cast<uint32_t>((system_time[4] - '0') | 0x80), false);
        WriteRegister(max7219::reg::DIGIT2, static_cast<uint32_t>(system_time[6] - '0'), false);
        WriteRegister(max7219::reg::DIGIT1, static_cast<uint32_t>(system_time[7] - '0'), false);
        WriteRegister(max7219::reg::DIGIT0, max7219::digit::BLANK, false);
    }

    void WriteChar(uint8_t ch, uint8_t pos) override
    {
        if (pos < 8)
        {
            WriteRegister(static_cast<uint32_t>(max7219::reg::DIGIT0 + pos), ch);
        }
    }

    static LtcDisplayMax72197Segment* Get() { return s_this; }

   private:
    inline static LtcDisplayMax72197Segment* s_this;
};

#endif  // LTCDISPLAYMAX72197SEGMENT_H_
