/**
 * @file pca9685pwmled.h
 *
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

#ifndef PCA9685PWMLED_H_
#define PCA9685PWMLED_H_

#include <cstdint>

#include "pca9685.h"

namespace pca9685::pwmled
{
inline constexpr uint32_t kDefaultFrequency = 120;
} // namespace pca9685::pwmled

class PCA9685PWMLed : public PCA9685
{
   public:
    explicit PCA9685PWMLed(uint8_t address) : PCA9685(address) { SetFrequency(pca9685::pwmled::kDefaultFrequency); }

    void Set(uint32_t channel, uint16_t data)
    {
        if (data >= 0xFFF)
        {
            SetFullOn(channel, true);
        }
        else if (data == 0)
        {
            SetFullOff(channel, true);
        }
        else
        {
            Write(channel, data);
        }
    }

    void Set(uint32_t channel, uint8_t data)
    {
        if (data == 0xFF)
        {
            SetFullOn(channel, true);
        }
        else if (data == 0)
        {
            SetFullOff(channel, true);
        }
        else
        {
            const auto kValue = static_cast<uint16_t>((data << 4) | (data >> 4));
            Write(channel, kValue);
        }
    }
};

#endif  // PCA9685PWMLED_H_
