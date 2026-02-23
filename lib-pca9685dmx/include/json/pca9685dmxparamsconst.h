/**
 * @file pca9685dmxparamsconst.h
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

#ifndef JSON_PCA9685DMXPARAMSCONST_H_
#define JSON_PCA9685DMXPARAMSCONST_H_

#include "common/utils/utils_hash.h"
#include "json/json_key.h"

namespace json
{
struct Pca9685DmxParamsConst
{
	static constexpr char kFileName[] = "pca9685.json";

	static constexpr json::SimpleKey kI2cAddres {
	    "i2c_address",
	    11,
	    Fnv1a32("i2c_address", 11)
	};
	
	static constexpr json::SimpleKey kDmxStartAddress {
	    "dmx_start_address",
	    17,
	    Fnv1a32("dmx_start_address", 17)
	};	
   
	static constexpr json::SimpleKey kMode {
	    "mode",
	    4,
	    Fnv1a32("mode", 4)
	};

	static constexpr json::SimpleKey kChannelCount {
	    "channel_count",
	    13,
	    Fnv1a32("channel_count", 13)
	};

	static constexpr json::SimpleKey kUse8Bit {
	    "use_8bit",
	    8,
	    Fnv1a32("use_8bit", 8)
	};
	
	static constexpr json::SimpleKey kLedPwmFrequency {
	    "led_pwm_frequency",
	    8,
	    Fnv1a32("led_pwm_frequency", 8)
	};
	
	static constexpr json::SimpleKey kLedOutputInvert {
	    "led_output_invert",
	    8,
	    Fnv1a32("led_output_invert", 8)
	};
	
	static constexpr json::SimpleKey kLedOutputOpendrain {
	    "led_output_opendrain",
	    8,
	    Fnv1a32("led_output_opendrain", 8)
	};
	
	static constexpr json::SimpleKey kServoLeftUs {
	    "servo_left_us",
	    8,
	    Fnv1a32("servo_left_us", 8)
	};
	
	static constexpr json::SimpleKey kServoCenterUs {
	    "servo_center_us",
	    8,
	    Fnv1a32("servo_center_us", 8)
	};
	
	static constexpr json::SimpleKey kServoRightUs {
	    "servo_right_us",
	    8,
	    Fnv1a32("servo_right_us", 8)
	};
};
} // namespace json

#endif  // JSON_PCA9685DMXPARAMSCONST_H_
