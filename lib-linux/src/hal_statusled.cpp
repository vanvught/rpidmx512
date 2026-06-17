/**
 * @file hal_statusled.cpp
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

#include <cstdint>
#include <cstdlib>

#include "hal_statusled.h"

enum class LedStatus { kOff, kOn, kHeartbeat, kFlash };

#if defined(RASPPI)
namespace rasppi {
static constexpr char kRaspbianLedInit[] = "echo gpio | sudo tee /sys/class/leds/ACT/trigger";
static constexpr char kRaspbianLedOff[] = "echo 0 | sudo tee /sys/class/leds/ACT/brightness";
static constexpr char kRaspbianLedOn[] = "echo 1 | sudo tee /sys/class/leds/ACT/brightness";
static constexpr char kRaspbianLedHb[] = "echo heartbeat | sudo tee /sys/class/leds/ACT/trigger";
static constexpr char kRaspbianLedFlash[] = "echo timer | sudo tee /sys/class/leds/ACT/trigger";
} // namespace rasppi
#endif
static LedStatus s_led_status;

static void SetLed([[maybe_unused]] LedStatus led_status) {
    if (s_led_status == led_status) {
        return;
    }
    s_led_status = led_status;
#if defined(RASPPI)
    char* p = nullptr;

    switch (led_status) {
        case LedStatus::kOff:
            p = const_cast<char*>(rasppi::kRaspbianLedOff);
            break;
        case LedStatus::kOn:
            p = const_cast<char*>(rasppi::kRaspbianLedOn);
            break;
        case LedStatus::kHeartbeat:
            p = const_cast<char*>(rasppi::kRaspbianLedHb);
            break;
        case LedStatus::kFlash:
            p = const_cast<char*>(rasppi::kRaspbianLedFlash);
            break;
        default:
            break;
    }

    if (p != nullptr) {
        if (system(p) == 0) {
            // Just make the compile happy
        }
    }
#endif
}

namespace hal::statusled {
void SetFrequency(uint32_t frequency_hz) {
    if (frequency_hz == 0) {
        SetLed(LedStatus::kOff);
    } else if (frequency_hz > 20) {
        SetLed(LedStatus::kOn);
    } else {
        if (frequency_hz > 1) {
            SetLed(LedStatus::kHeartbeat);
        } else {
            SetLed(LedStatus::kFlash);
        }
    }
}
} // namespace hal::statusled
