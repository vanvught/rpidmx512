/**
 * @file pca9685dmx.cpp
 *
 */
/* Copyright (C) 2023-2026 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include <cassert>

#include "pca9685dmx.h"
#include "pca9685.h"
#include "pca9685dmxled.h"
#include "pca9685dmxservo.h"
#include "pca9685dmxset.h"
#include "firmware/debug/debug_debug.h"
#include "pca9685pwmled.h"

Pca9685Dmx::Pca9685Dmx() {
    DEBUG_EXIT();
    assert(s_this == nullptr);
    s_this = this;

    configuration_.mode = 0;
    configuration_.address = pca9685::kI2CAddressDefault;
    configuration_.channel_count = pca9685::kPwmChannels;
    configuration_.dmx_start_address = dmxnode::kStartAddressDefault;
    configuration_.use8bit = false;

    configuration_.led.led_pwm_frequency = pca9685::pwmled::kDefaultFrequency;
    configuration_.led.invert = pca9685::Invert::kOutputNotInverted;
    configuration_.led.output = pca9685::Output::kDriverTotempole;

    configuration_.servo.left_us = pca9685::servo::kLeftDefaultUs;
    configuration_.servo.right_us = pca9685::servo::kRightDefaultUs;

    DEBUG_EXIT();
}

Pca9685Dmx::~Pca9685Dmx() {
    DEBUG_ENTRY();

    if (dmx_set_ == nullptr) {
        DEBUG_EXIT();
        return;
    }

    dmx_set_->Stop(0);
    delete dmx_set_;
    dmx_set_ = nullptr;

    DEBUG_EXIT();
}

void Pca9685Dmx::Start() {
    DEBUG_EXIT();

    assert(m_pPCA9685DmxSet == nullptr);

    if ((!configuration_.use8bit) && (configuration_.channel_count > dmxnode::kUniverseSize / 2)) {
        configuration_.channel_count = dmxnode::kUniverseSize / 2;
    }

    if (configuration_.mode != 0) { // Servo
        dmx_set_ = new PCA9685DmxServo(configuration_);
    } else { // Led
        dmx_set_ = new PCA9685DmxLed(configuration_);
    }

    assert(m_pPCA9685DmxSet != nullptr);
    dmx_set_->Start(0);

    DEBUG_EXIT();
}
