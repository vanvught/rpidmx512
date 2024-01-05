/**
 * @file pca9685dmxparamsconst.cpp
 *
 */
/* Copyright (C) 2023-2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "pca9685dmxparamsconst.h"

const char PCA9685DmxParamsConst::FILE_NAME[]= "pca9685.txt";

const char PCA9685DmxParamsConst::I2C_ADDRESS[] = "i2c_address";

const char PCA9685DmxParamsConst::MODE[] = "mode";
const char PCA9685DmxParamsConst::CHANNEL_COUNT[] = "channel_count";
const char PCA9685DmxParamsConst::USE_8BIT[] = "use_8bit";

const char PCA9685DmxParamsConst::LED_PWM_FREQUENCY[] = "led_pwm_frequency";
const char PCA9685DmxParamsConst::LED_OUTPUT_INVERT[] = "led_output_invert";
const char PCA9685DmxParamsConst::LED_OUTPUT_OPENDRAIN[] = "led_output_opendrain";

const char PCA9685DmxParamsConst::SERVO_LEFT_US[] = "servo_left_us";
const char PCA9685DmxParamsConst::SERVO_CENTER_US[] = "servo_center_us";
const char PCA9685DmxParamsConst::SERVO_RIGHT_US[] = "servo_right_us";
