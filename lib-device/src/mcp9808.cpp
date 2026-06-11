/**
 * @file mcp9808.cpp
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

#include <cstdint>

#include "mcp9808.h"
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
} // namespace mcp9808

MCP9808::MCP9808(uint8_t address) : address_(address == 0 ? sensor::mcp9808::kI2CAddress : address) {
    initialized_ = i2c::IsConnected(address_);

    if (initialized_) {
        initialized_ = (i2c::ReadRegister16(sensor::mcp9808::reg::kManufId) == 0x0054);
    }

    if (initialized_) {
        initialized_ = (i2c::ReadRegister16(sensor::mcp9808::reg::kDeviceId) == 0x0400);
    }
}

float MCP9808::Get() {
    i2c::SetAddress(address_);
    i2c::SetBaudrate(i2c::kFullSpeed);
	
    const auto kValue = i2c::ReadRegister16(sensor::mcp9808::reg::kAmbientTemp);
    auto temperature = static_cast<float>(kValue & 0x0FFF);

    temperature /= 16.0f;

    if ((kValue & 0x1000) == 0x1000) {
        temperature -= 256.0f;
    }

    return temperature;
}
} // namespace sensor
