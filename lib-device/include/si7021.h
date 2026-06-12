/**
 * @file si7021.h
 *
 */
/* Copyright (C) 2020-2026 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include "i2c.h"

namespace sensor {
namespace si7021 {
inline constexpr uint8_t kI2CAddress = 0x40;
namespace reg {
inline constexpr uint8_t kTriggerTempMeasureNohold = 0xF3;
inline constexpr uint8_t kTriggerHumdMeasureNohold = 0xF5;
} // namespace reg
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

class SI7021 : I2c {
   public:
    explicit SI7021(uint8_t address) : I2c(address == 0 ? sensor::si7021::kI2CAddress : address) { 
		initialized_ = IsConnected(); 
	}

    float GetTemperature() {
        const auto kTemp = static_cast<float>(ReadRaw(sensor::si7021::reg::kTriggerTempMeasureNohold)) / 65536.0f;
        return -46.85f + (175.72f * kTemp);
    }

    float GetHumidity() {
        const auto kHumd = static_cast<float>(ReadRaw(sensor::si7021::reg::kTriggerHumdMeasureNohold)) / 65536.0f;
        return -6.0f + (125.0f * kHumd);
    }

    bool Initialize() { return initialized_; }

   private:
    uint16_t ReadRaw(uint8_t cmd) {
        Write(cmd, true);

        char buf[3] = {0};

        for (uint32_t i = 0; i < 8; ++i) {
            timing::DelayUs(10000);
            Read(buf, 3, false);

            if ((buf[0] & 0x3) == 2) {
                break;
            }
        }

        const auto kRawValue = static_cast<uint16_t>((buf[0] << 8) | buf[1]);

        return kRawValue & 0xFFFC;
    }

   private:
    bool initialized_{false};
};
} // namespace sensor

#endif // SI7021_H_
