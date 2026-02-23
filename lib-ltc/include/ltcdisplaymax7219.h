/**
 * @file ltcdisplaymax7219.h
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

#ifndef LTCDISPLAYMAX7219_H_
#define LTCDISPLAYMAX7219_H_

#include <cstdint>
#include <cstdio>
#include <cassert>

#include "ltcdisplaymax7219set.h"
#include "ltcdisplaymax72197segment.h"
#include "ltcdisplaymax7219matrix.h"
#include "firmware/debug/debug_debug.h"

namespace ltc::display::max7219
{
enum class Types
{
    kMatrix,
    kSegment
};
} // namespace ltc::display::max7219

class LtcDisplayMax7219
{
   public:
    explicit LtcDisplayMax7219()
    {
        DEBUG_ENTRY();

        assert(s_this == nullptr);
        s_this = this;

        DEBUG_ENTRY();
    }

    void SetType(ltc::display::max7219::Types type) { type_ = type; }
    ltc::display::max7219::Types GetType() const { return type_; }

    void SetIntensity(uint8_t intensity)
    {
        intensity_ = intensity;
        if (max7219set_ != nullptr) max7219set_->SetIntensity(intensity);
    }
    uint8_t GetIntensity() const { return intensity_; }

    void Init()
    {
        DEBUG_ENTRY();

        if (max7219set_ != nullptr) delete max7219set_;

        if (type_ == ltc::display::max7219::Types::kSegment)
        {
            max7219set_ = new LtcDisplayMax72197Segment(intensity_);
        }
        else
        {
            max7219set_ = new LtcDisplayMax7219Matrix(intensity_);
        }

        assert(max7219set_ != nullptr);

        DEBUG_EXIT();
    }

    const char* GetTypeString() const
    {
        if (type_ == ltc::display::max7219::Types::kSegment) return "7segment";
        return "matrix";
    }

    ltc::display::max7219::Types GetType(const char* val, uint8_t len)
    {
        if (len == 8)
        {
            if (memcmp(val, "7segment", 8) == 0)
            {
                return ltc::display::max7219::Types::kSegment;
            }
        }

        return ltc::display::max7219::Types::kMatrix;
    }

    void Show(const char* timecode)
    {
        assert(max7219set_ != nullptr);
        max7219set_->Show(timecode);
    }
	
    void ShowSysTime(const char* systemtime)
    {
        assert(max7219set_ != nullptr);
        max7219set_->ShowSysTime(systemtime);
    }
	
    void WriteChar(uint8_t ch, uint8_t pos = 0)
    {
        assert(max7219set_ != nullptr);
        max7219set_->WriteChar(ch, pos);
    }

    void Print()
    {
        printf("MAX7219\n");
        printf(" %s [%d]\n", type_ == ltc::display::max7219::Types::kSegment ? "7-segment" : "matrix", intensity_);
    }

    static LtcDisplayMax7219* Get()
    {
        DEBUG_PRINTF("%p", s_this);
        assert(s_this != nullptr);
        return s_this;
    }

   private:
    LtcDisplayMax7219Set* max7219set_{nullptr};
    ltc::display::max7219::Types type_{ltc::display::max7219::Types::kMatrix};
    uint8_t intensity_{0x7f};

    inline static LtcDisplayMax7219* s_this;
};

#endif // LTCDISPLAYMAX7219_H_
