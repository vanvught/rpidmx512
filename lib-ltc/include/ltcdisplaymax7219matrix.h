/**
 * @file ltcdisplaymax7219matrix.h
 *
 */
/* Copyright (C) 2019-2025	 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef LTCDISPLAYMAX7219MATRIX_H_
#define LTCDISPLAYMAX7219MATRIX_H_

#include <cstdint>

#include "ltcdisplaymax7219set.h"
#include "max7219matrix.h"
#include "ltc.h"

namespace ltc::display::max7219::maxtrix
{
static constexpr auto kSegments = 8;
} // namespace ltc::display::max7219::maxtrix

class LtcDisplayMax7219Matrix final : public LtcDisplayMax7219Set, public Max7219Matrix
{
    static constexpr uint8_t kCharsColon[][8] = {
        {0x3E, 0x7F, 0x71, 0x59, 0x4D, 0x7F, 0x3E, 0x66}, // 0:
        {0x40, 0x42, 0x7F, 0x7F, 0x40, 0x40, 0x00, 0x66}, // 1:
        {0x62, 0x73, 0x59, 0x49, 0x6F, 0x66, 0x00, 0x66}, // 2:
        {0x22, 0x63, 0x49, 0x49, 0x7F, 0x36, 0x00, 0x66}, // 3:
        {0x18, 0x1C, 0x16, 0x53, 0x7F, 0x7F, 0x50, 0x66}, // 4:
        {0x27, 0x67, 0x45, 0x45, 0x7D, 0x39, 0x00, 0x66}, // 5:
        {0x3C, 0x7E, 0x4B, 0x49, 0x79, 0x30, 0x00, 0x66}, // 6:
        {0x03, 0x03, 0x71, 0x79, 0x0F, 0x07, 0x00, 0x66}, // 7:
        {0x36, 0x7F, 0x49, 0x49, 0x7F, 0x36, 0x00, 0x66}, // 8:
        {0x06, 0x4F, 0x49, 0x69, 0x3F, 0x1E, 0x00, 0x66}  // 9:
    };

    static constexpr uint8_t kCharsBlinkSemiColon[][8] = {
        {0x3E, 0x7F, 0x71, 0x59, 0x4D, 0x7F, 0x3E | 0x80, 0x66}, // 0;
        {0x40, 0x42, 0x7F, 0x7F, 0x40, 0x40, 0x00 | 0x80, 0x66}, // 1;
        {0x62, 0x73, 0x59, 0x49, 0x6F, 0x66, 0x00 | 0x80, 0x66}, // 2;
        {0x22, 0x63, 0x49, 0x49, 0x7F, 0x36, 0x00 | 0x80, 0x66}, // 3;
        {0x18, 0x1C, 0x16, 0x53, 0x7F, 0x7F, 0x50 | 0x80, 0x66}, // 4;
        {0x27, 0x67, 0x45, 0x45, 0x7D, 0x39, 0x00 | 0x80, 0x66}, // 5;
        {0x3C, 0x7E, 0x4B, 0x49, 0x79, 0x30, 0x00 | 0x80, 0x66}, // 6;
        {0x03, 0x03, 0x71, 0x79, 0x0F, 0x07, 0x00 | 0x80, 0x66}, // 7;
        {0x36, 0x7F, 0x49, 0x49, 0x7F, 0x36, 0x00 | 0x80, 0x66}, // 8;
        {0x06, 0x4F, 0x49, 0x69, 0x3F, 0x1E, 0x00 | 0x80, 0x66}  // 9;
    };

    static constexpr uint8_t kCharsBlinkComma[][8] = {
        {0x3E, 0x7F, 0x71, 0x59, 0x4D, 0x7F, 0x3E | 0x80, 0x00}, // 0,
        {0x40, 0x42, 0x7F, 0x7F, 0x40, 0x40, 0x00 | 0x80, 0x60}, // 1,
        {0x62, 0x73, 0x59, 0x49, 0x6F, 0x66, 0x00 | 0x80, 0x00}, // 2,
        {0x22, 0x63, 0x49, 0x49, 0x7F, 0x36, 0x00 | 0x80, 0x60}, // 3,
        {0x18, 0x1C, 0x16, 0x53, 0x7F, 0x7F, 0x50 | 0x80, 0x00}, // 4,
        {0x27, 0x67, 0x45, 0x45, 0x7D, 0x39, 0x00 | 0x80, 0x60}, // 5,
        {0x3C, 0x7E, 0x4B, 0x49, 0x79, 0x30, 0x00 | 0x80, 0x00}, // 6,
        {0x03, 0x03, 0x71, 0x79, 0x0F, 0x07, 0x00 | 0x80, 0x60}, // 7,
        {0x36, 0x7F, 0x49, 0x49, 0x7F, 0x36, 0x00 | 0x80, 0x00}, // 8,
        {0x06, 0x4F, 0x49, 0x69, 0x3F, 0x1E, 0x00 | 0x80, 0x60}  // 9,
    };

   public:
    explicit LtcDisplayMax7219Matrix(uint8_t intensity)
    {
        assert(s_this == nullptr);
        s_this = this;

        for (uint32_t i = 0; i < sizeof(kCharsColon) / sizeof(kCharsColon[0]); i++)
        {
            Max7219Matrix::UpdateCharacter(i, kCharsColon[i]);
        }

        for (uint32_t i = 10; i < 10 + sizeof(kCharsBlinkSemiColon) / sizeof(kCharsBlinkSemiColon[0]); i++)
        {
            Max7219Matrix::UpdateCharacter(i, kCharsBlinkSemiColon[i - 10]);
        }

        for (uint32_t i = 20; i < 20 + sizeof(kCharsBlinkComma) / sizeof(kCharsBlinkComma[0]); i++)
        {
            Max7219Matrix::UpdateCharacter(i, kCharsBlinkComma[i - 20]);
        }

        Max7219Matrix::Init(ltc::display::max7219::maxtrix::kSegments, intensity);
        Max7219Matrix::Write("Waiting", 7);
    }

    ~LtcDisplayMax7219Matrix() override 
    {
		s_this = nullptr;
	}
  
    void SetIntensity(uint8_t intensity) override
    {
		Max7219Matrix::SetIntensity(intensity);
	}

    void Show(const char* timecode) override
    {
        assert(timecode != nullptr);

        const auto kSeconds = timecode[ltc::timecode::index::SECONDS_UNITS];

        buffer_[0] = timecode[0];
        buffer_[1] = static_cast<char>(Offset(timecode[ltc::timecode::index::COLON_1], kSeconds) + timecode[1]);
        buffer_[2] = timecode[3];
        buffer_[3] = static_cast<char>(Offset(timecode[ltc::timecode::index::COLON_2], kSeconds) + timecode[4]);
        buffer_[4] = timecode[6];
        buffer_[5] = static_cast<char>(Offset(timecode[ltc::timecode::index::COLON_3], kSeconds) + timecode[7]);
        buffer_[6] = timecode[9];
        buffer_[7] = timecode[10];

        Max7219Matrix::Write(buffer_, ltc::display::max7219::maxtrix::kSegments);
    }

    void ShowSysTime(const char* systemtime) override
    {
        assert(systemtime != nullptr);

        const auto kSeconds = systemtime[ltc::systemtime::index::SECONDS_UNITS];

        buffer_[0] = ' ';
        buffer_[1] = systemtime[0];
        buffer_[2] = static_cast<char>(Offset(systemtime[ltc::systemtime::index::COLON_1], kSeconds) + systemtime[1]);
        buffer_[3] = systemtime[3];
        buffer_[4] = static_cast<char>(Offset(systemtime[ltc::systemtime::index::COLON_2], kSeconds) + systemtime[4]);
        buffer_[5] = systemtime[6];
        buffer_[6] = systemtime[7];
        buffer_[7] = ' ';

        Max7219Matrix::Write(buffer_, ltc::display::max7219::maxtrix::kSegments);
    }

    void WriteChar([[maybe_unused]] uint8_t ch, [[maybe_unused]] uint8_t pos) override
    {
        // TODO(avv): Implement WriteChar
    }

    static LtcDisplayMax7219Matrix* Get() { return s_this; }

   private:
    int32_t Offset(char ch, char seconds)
    {
        const auto kEven = !((seconds & 0x01) == 0x01);

        if (kEven)
        {
            switch (ch)
            {
                case ':':
                    return 0 - '0';
                    break;
                case ';':
                    return 10 - '0';
                    break;
                case ',':
                    return 20 - '0';
                    break;
                default:
                    break;
            }
        }

        return 0;
    }
    char buffer_[ltc::display::max7219::maxtrix::kSegments];

    inline static LtcDisplayMax7219Matrix* s_this;
};

#endif  // LTCDISPLAYMAX7219MATRIX_H_
