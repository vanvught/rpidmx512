/**
 * @file mcp9808.h
 *
 */
/* Copyright (C) 2018-2026 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef MCP9808_H_
#define MCP9808_H_

#include <cstdint>

#include "i2c.h"

namespace sensor {
namespace mcp9808 {
inline constexpr uint8_t kI2CAddress = 0x18;
namespace reg {
// inline constexpr uint8_t UPPER_TEMP = 0x02;
// inline constexpr uint8_t LOWER_TEMP = 0x03;
// inline constexpr uint8_t CRIT_TEMP = 0x04;
inline constexpr uint8_t kAmbientTemp = 0x05;
inline constexpr uint8_t kManufId = 0x06;
inline constexpr uint8_t kDeviceId = 0x07;
} // namespace reg
inline constexpr char kDescription[] = "Ambient Temperature";
inline constexpr int16_t kRangeMin = -20;
inline constexpr int16_t kRangeMax = 100;
} // namespace mcp9808

class MCP9808 : I2c {
   public:
    explicit MCP9808(uint8_t address) : I2c(address == 0 ? sensor::mcp9808::kI2CAddress : address) {
        initialized_ = IsConnected();

        if (initialized_) {
            initialized_ = (ReadRegister16(sensor::mcp9808::reg::kManufId, false) == 0x0054);
        }

        if (initialized_) {
            initialized_ = (ReadRegister16(sensor::mcp9808::reg::kDeviceId, false) == 0x0400);
        }
    }

    float Get() {
        const auto kValue = ReadRegister16(sensor::mcp9808::reg::kAmbientTemp, true);
        auto temperature = static_cast<float>(kValue & 0x0FFF);

        temperature /= 16.0f;

        if ((kValue & 0x1000) == 0x1000) {
            temperature -= 256.0f;
        }

        return temperature;
    }

    bool Initialize() { return initialized_; }

   private:
     bool initialized_{false};
};
} // namespace sensor

#endif // MCP9808_H_
