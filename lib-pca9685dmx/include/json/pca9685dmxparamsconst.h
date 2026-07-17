/**
 * @file pca9685dmxparamsconst.h
 *
 */
/* Copyright (C) 2025-2026 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef JSON_PCA9685DMXPARAMSCONST_H_
#define JSON_PCA9685DMXPARAMSCONST_H_

#include "json/json_key.h"

namespace json {
struct Pca9685DmxParamsConst {
    static constexpr char kFileName[] = "dmxpca9685.json";

    static constexpr auto kI2cAddres = json::MakeSimpleKey("i2c_address");
    static constexpr auto kDmxStartAddress = json::MakeSimpleKey("dmx_start_address");
    static constexpr auto kMode = json::MakeSimpleKey("mode");
    static constexpr auto kChannelCount = json::MakeSimpleKey("channel_count");
    static constexpr auto kUse8Bit = json::MakeSimpleKey("use_8bit");
    static constexpr auto kLedPwmFrequency = json::MakeSimpleKey("led_pwm_frequency");
    static constexpr auto kLedOutputInvert = json::MakeSimpleKey("led_output_invert");
    static constexpr auto kLedOutputOpendrain = json::MakeSimpleKey("led_output_opendrain");
    static constexpr auto kServoLeftUs = json::MakeSimpleKey("servo_left_us");
    static constexpr auto kServoCenterUs = json::MakeSimpleKey("servo_center_us");
    static constexpr auto kServoRightUs = json::MakeSimpleKey("servo_right_us");
};
} // namespace json

#endif // JSON_PCA9685DMXPARAMSCONST_H_
