/**
 * @file bh1750.cpp
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

#include <cstdint>

#include "bh1750.h"
#include "i2c.h"

namespace sensor {
namespace bh1750 {
static constexpr uint8_t kI2CAddress = 0x23;
namespace reg {
// static constexpr uint8_t POWER_DOWN = 0x00;
static constexpr uint8_t kPowerOn = 0x01;
// static constexpr uint8_t RESET = 0x07;
static constexpr uint8_t kContinuousHighResMode = 0x10;
// static constexpr uint8_t CONTINUOUS_HIGH_RES_MODE_2 = 0x11;
// static constexpr uint8_t CONTINUOUS_LOW_RES_MODE = 0x13;
// static constexpr uint8_t ONE_TIME_HIGH_RES_MODE = 0x20;
// static constexpr uint8_t ONE_TIME_HIGH_RES_MODE_2 = 0x21;
// static constexpr uint8_t ONE_TIME_LOW_RES_MODE = 0x23;
} // namespace reg
} // namespace bh1750

BH1750::BH1750(uint8_t address) : address_(address == 0 ? sensor::bh1750::kI2CAddress : address) {
    i2c::Begin();
    initialized_ = i2c::IsConnected(address_);

    if (initialized_) {
        i2c::Write(sensor::bh1750::reg::kPowerOn);
        i2c::Write(sensor::bh1750::reg::kContinuousHighResMode);
    }
}

uint16_t BH1750::Get() {
    i2c::SetAddress(address_);
    i2c::SetBaudrate(i2c::kFullSpeed);
    const auto kLevel = static_cast<uint16_t>(static_cast<float>(i2c::Read16()) / 1.2f);
    return kLevel;
}

} // namespace sensor
