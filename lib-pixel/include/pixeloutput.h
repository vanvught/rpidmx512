/**
 * @file pixeloutput.h
 */
/* Copyright (C) 2017-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef PIXELOUTPUT_H_
#define PIXELOUTPUT_H_

#include <cstdint>

#if defined(GD32)
#include "gd32_spi.h"
#elif defined(H3)
#include "h3_spi.h"
#endif

class PixelOutput
{
   public:
    PixelOutput();
    ~PixelOutput();

    void ApplyConfiguration();

    void SetPixel(uint32_t index, uint8_t red, uint8_t green, uint8_t blue);
    void SetPixel(uint32_t index, uint8_t red, uint8_t green, uint8_t blue, uint8_t white);

    bool IsUpdating()
    {
#if defined(GD32)
        return i2s::Gd32SpiDmaTxIsActive();
#elif defined(H3)
        return H3SpiDmaTxIsActive();
#else
        return false;
#endif
    }

    void Update();
    void Blackout();
    void FullOn();

    uint32_t GetUserData()
    { // TODO(a): implement GetUserData
        return 0;
    }

    static PixelOutput* Get() { return s_this; }

   private:
    void SetupBuffers();
    void SetColorWS28xx(uint32_t offset, uint8_t value);

   private:
    uint32_t buf_size_;
    uint8_t* buffer_{nullptr};
    uint8_t* blackout_buffer_{nullptr};

    static inline PixelOutput* s_this;
};

using PixelOutputType = PixelOutput;

#endif  // PIXELOUTPUT_H_
