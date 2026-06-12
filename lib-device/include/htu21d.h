/**
 * @file htu21d.h
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

#ifndef HTU21D_H_
#define HTU21D_H_

#include <cstdint>

#include "i2c.h"

namespace sensor {
namespace htu21d {
inline constexpr uint8_t kI2CAddress = 0x40;
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
namespace reg {
// static constexpr uint8_t TRIGGER_TEMP_MEASURE_HOLD = 0xE3;
// static constexpr uint8_t TRIGGER_HUMD_MEASURE_HOLD = 0xE5;
inline constexpr uint8_t kTriggerTempMeasureNohold = 0xF3;
inline constexpr uint8_t kTriggerHumdMeasureNohold = 0xF5;
// static constexpr uint8_t WRITE_USER_REG = 0xE6;
// static constexpr uint8_t READ_USER_REG = 0xE7;
// static constexpr uint8_t SOFT_RESET = 0xFE;
} // namespace reg
} // namespace htu21d

class HTU21D: I2c {
   public:
    explicit HTU21D(uint8_t address = 0) : I2c(address == 0 ? sensor::htu21d::kI2CAddress : address) {
        initialized_ = IsConnected();
    }

    float GetTemperature() {
        const auto kTemp = static_cast<float>(ReadRaw(sensor::htu21d::reg::kTriggerTempMeasureNohold)) / 65536.0f;
        return -46.85f + (175.72f * kTemp);
    }

    float GetHumidity() {
        const auto kHumd = static_cast<float>(ReadRaw(sensor::htu21d::reg::kTriggerHumdMeasureNohold)) / 65536.0f;
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

#endif // HTU21D_H_
