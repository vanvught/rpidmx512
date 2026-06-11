/**
 * @file si7021.h
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

#ifndef SI7021_H_
#define SI7021_H_

#include <cstdint>

namespace sensor {
namespace si7021 {
namespace temperature {
inline constexpr char kDescription[] = "Ambient Temperature";
inline constexpr int16_t kRangeMin = -40;
inline constexpr int16_t kRangeMax = 125;
} // namespace temperature
namespace humidity {
inline constexpr char kDescription[] = "Relative Humidity";
inline constexpr int16_t kRangeMin = 0;
inline constexpr int16_t kRangeMax = 100;
} // namespace humidity
} // namespace si7021

class SI7021 {
   public:
    explicit SI7021(uint8_t address = 0);

    bool Initialize() { return initialized_; }

    float GetTemperature();
    float GetHumidity();

   private:
    uint16_t ReadRaw(uint8_t cmd);

   private:
    uint8_t address_{0};
    bool initialized_{false};
};
} // namespace sensor

#endif // SI7021_H_
