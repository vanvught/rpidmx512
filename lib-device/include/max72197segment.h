/**
 * @file max72197segment.h
 *
 */
/* Copyright (C) 2020-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef MAX72197SEGMENT_H_
#define MAX72197SEGMENT_H_

#include "max7219.h"

class Max72197Segment : public MAX7219
{
   public:
    Max72197Segment() = default;
    ~Max72197Segment() = default;

    void Init(uint8_t intensity)
    {
        WriteRegister(max7219::reg::SHUTDOWN, max7219::reg::shutdown::NORMAL_OP);
        WriteRegister(max7219::reg::DISPLAY_TEST, 0, false);
        WriteRegister(max7219::reg::DECODE_MODE, max7219::reg::decode_mode::CODEB, false);
        WriteRegister(max7219::reg::SCAN_LIMIT, 7, false);

		SetIntensity(intensity);
        Cls();
    }
    
    void SetIntensity(uint8_t intensity)
    {
        WriteRegister(max7219::reg::INTENSITY, intensity & 0x0F, false);		
	}

    void Cls()
    {
        WriteRegister(8, max7219::digit::BLANK);

        uint32_t i = 7;

        do
        {
            WriteRegister(i, max7219::digit::BLANK, false);
        } while (--i > 0);
    }
};

#endif  // MAX72197SEGMENT_H_
