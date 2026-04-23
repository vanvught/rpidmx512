/**
 * @file pca9685dmx.h
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

#ifndef PCA9685DMX_H_
#define PCA9685DMX_H_

#include <cstdint>
#include <cassert>

#include "pca9685.h"
#include "pca9685pwmled.h"
#include "pca9685servo.h"
#include "pca9685dmxset.h"

namespace pca9685dmx {
inline constexpr uint8_t kBoardInstancesDefault = 1;
inline constexpr uint8_t kBoardInstancesMax = 32;
inline constexpr auto kDmxFootprintDefault = pca9685::kPwmChannels;

struct Mode {
    static constexpr char kLed[] = "led";
    static constexpr char kServo[] = "servo";
};

struct Configuration {
    uint8_t mode;
    uint8_t address;
    uint16_t channel_count;
    uint16_t dmx_start_address;
    bool use8bit;

    struct {
        uint16_t led_pwm_frequency;
        pca9685::Invert invert;
        pca9685::Output output;
    } led;

    struct {
        uint16_t left_us;
        uint16_t center_us;
        uint16_t right_us;
    } servo;
};

inline const char* GetMode(uint32_t mode) {
    return mode != 0 ? Mode::kServo : Mode::kLed;
}

inline uint32_t GetMode(const char* mode) {
    return (strcasecmp(mode, Mode::kServo) == 0);
}

} // namespace pca9685dmx

class Pca9685Dmx {
   public:
    Pca9685Dmx();
    Pca9685Dmx(const Pca9685Dmx&) = delete;
    Pca9685Dmx& operator=(const Pca9685Dmx&) = delete;

    ~Pca9685Dmx();

    void SetMode(uint32_t mode) { configuration_.mode = (mode != 0) ? 1 : 0; }
    uint8_t GetMode() const { return (configuration_.mode != 0); }

    void SetAddress(uint8_t address) {
        if ((address >= 0x03) && (address <= 0x77)) {
            configuration_.address = address;
        } else {
            configuration_.address = pca9685::kI2CAddressDefault;
        }
    }

    uint8_t GetAddress() const { return configuration_.address; }

    void SetChannelCount(uint16_t channel_count) {
        if ((channel_count != 0) && (channel_count <= dmxnode::kUniverseSize)) {
            configuration_.channel_count = channel_count;
        } else {
            configuration_.channel_count = pca9685::kPwmChannels;
        }
    }

    uint16_t GetChannelCount() const { return configuration_.channel_count; }

    void SetDmxStartAddress(uint16_t dmx_start_address) {
        if ((dmx_start_address != 0) && (dmx_start_address <= dmxnode::kUniverseSize)) {
            configuration_.dmx_start_address = dmx_start_address;
        } else {
            configuration_.dmx_start_address = dmxnode::kStartAddressDefault;
        }
    }

    uint16_t GetDmxStartAddress() const { return configuration_.dmx_start_address; }

    void SetUse8Bit(bool use8_bit) { configuration_.use8bit = use8_bit; }
    bool IsUse8Bit() const { return configuration_.use8bit; }

    void SetLedPwmFrequency(uint16_t led_pwm_frequency) {
        if ((led_pwm_frequency >= pca9685::Frequency::kRangeMin) && (led_pwm_frequency <= pca9685::Frequency::kRangeMax)) {
            configuration_.led.led_pwm_frequency = led_pwm_frequency;
        } else {
            configuration_.led.led_pwm_frequency = pca9685::pwmled::kDefaultFrequency;
        }
    }

    uint16_t GetLedPwmFrequency() const { return configuration_.led.led_pwm_frequency; }

    void SetLedOutputInvert(pca9685::Invert invert) { configuration_.led.invert = invert; }
    pca9685::Invert GetLedOutputInvert() const { return configuration_.led.invert; }

    void SetLedOutputDriver(pca9685::Output output) { configuration_.led.output = output; }
    pca9685::Output GetLedOutputDriver() const { return configuration_.led.output; }

    void SetServoLeftUs(uint16_t left_us) { configuration_.servo.left_us = (left_us == 0 ? pca9685::servo::kLeftDefaultUs : left_us); }
    uint16_t GetServoLeftUs() const { return configuration_.servo.left_us; }

    void SetServoCenterUs(uint16_t center_us) { configuration_.servo.center_us = (center_us == 0 ? pca9685::servo::kCenterDefaultUs : center_us); }
    uint16_t GetServoCenterUs() const { return configuration_.servo.center_us; }

    void SetServoRightUs(uint16_t right_us) {
        configuration_.servo.right_us = (right_us == 0 ? pca9685::servo::kRightDefaultUs : right_us);
        ;
    }

    uint16_t GetServoRightUs() const { return configuration_.servo.right_us; }

    PCA9685DmxSet* GetPCA9685DmxSet() {
        if (dmx_set_ == nullptr) {
            Start();
        }
        return dmx_set_;
    }

    void Print() {
        if (dmx_set_ != nullptr) {
            dmx_set_->Print();
        } else {
            assert(0);
        }
    }

    static Pca9685Dmx& Instance() {
        assert(s_this != nullptr); // Ensure that s_this is valid
        return *s_this;
    }

    void Start();

   private:
    pca9685dmx::Configuration configuration_;

    PCA9685DmxSet* dmx_set_{nullptr};

    static inline Pca9685Dmx* s_this;
};

#endif // PCA9685DMX_H_
