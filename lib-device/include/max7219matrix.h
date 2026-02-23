/**
 * @file max7219matrix.h
 *
 */
/* Copyright (C) 2020 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef MAX7219MATRIX_H_
#define MAX7219MATRIX_H_

#include <cstdint>

#include "max7219.h"

class Max7219Matrix : public MAX7219
{
   public:
    Max7219Matrix();
    ~Max7219Matrix();

    void SetIntensity(uint8_t intensity) { WriteAll(max7219::reg::INTENSITY, intensity & 0x0F); }

    void Init(uint16_t count, uint8_t intensity);

    void Cls()
    {
        WriteAll(max7219::reg::DIGIT0, 0);
        WriteAll(max7219::reg::DIGIT1, 0);
        WriteAll(max7219::reg::DIGIT2, 0);
        WriteAll(max7219::reg::DIGIT3, 0);
        WriteAll(max7219::reg::DIGIT4, 0);
        WriteAll(max7219::reg::DIGIT5, 0);
        WriteAll(max7219::reg::DIGIT6, 0);
        WriteAll(max7219::reg::DIGIT7, 0);
    }

    void Write(const char* buffer, uint16_t count);

    void UpdateCharacter(uint32_t c, const uint8_t bytes[8]);

   private:
    uint8_t Rotate(uint32_t r, uint32_t x);
    void WriteAll(uint8_t reg, uint8_t data);

   private:
    uint32_t font_size_;
    uint8_t* font_;
    uint16_t count_{4};
};

#endif  // MAX7219MATRIX_H_
