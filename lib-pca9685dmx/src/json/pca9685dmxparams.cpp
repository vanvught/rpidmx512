/**
 * @file pca9685dmxparams.cpp
 *
 */
/* Copyright (C) 2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include "json/pca9685dmxparamsconst.h"
#include "pca9685dmx.h"
#include "json/pca9685dmxparams.h"
#include "json/json_parser.h"
#include "common/utils/utils_flags.h"
#include "configstore.h"

using common::store::dmxpwm::Flags;

namespace json
{
Pca9685DmxParams::Pca9685DmxParams()
{
    ConfigStore::Instance().Copy(&store_dmxpwm, &ConfigurationStore::dmx_pwm);
}

void Pca9685DmxParams::SetMode(const char* val, [[maybe_unused]] uint32_t len)
{
    const auto kV = static_cast<uint8_t>(pca9685dmx::GetMode(val));

    store_dmxpwm.flags = common::SetFlagValue(store_dmxpwm.flags, Flags::Flag::kModeServo, kV != 0);
}

void Pca9685DmxParams::Store(const char* buffer, uint32_t buffer_size)
{
    ParseJsonWithTable(buffer, buffer_size, kPca9685DmxKeys);
    ConfigStore::Instance().Store(&store_dmxpwm, &ConfigurationStore::dmx_pwm);
}

void Pca9685DmxParams::Set()
{
#ifndef NDEBUG
    Dump();
#endif
}

void Pca9685DmxParams::Dump()
{
    printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, json::Pca9685DmxParamsConst::kFileName);
    // Led
    printf(" %s=%d Hz\n", Pca9685DmxParamsConst::kLedPwmFrequency.name, store_dmxpwm.led_pwm_frequency);
    printf(" %s=%u\n", Pca9685DmxParamsConst::kUse8Bit.name, common::IsFlagSet(store_dmxpwm.flags, Flags::Flag::kUse8Bit));
    printf(" %s=%u\n", Pca9685DmxParamsConst::kLedOutputInvert.name, common::IsFlagSet(store_dmxpwm.flags, Flags::Flag::kLedOutputInvert));
    printf(" %s=%u\n", Pca9685DmxParamsConst::kLedOutputOpendrain.name, common::IsFlagSet(store_dmxpwm.flags, Flags::Flag::kLedOutputOpendrain));
    // Servo
    printf(" %s=%d\n", Pca9685DmxParamsConst::kServoLeftUs.name, store_dmxpwm.servo_left_us);
    printf(" %s=%d\n", Pca9685DmxParamsConst::kServoCenterUs.name, store_dmxpwm.servo_center_us);
    printf(" %s=%d\n", Pca9685DmxParamsConst::kServoRightUs.name, store_dmxpwm.servo_right_us);
}
} // namespace json