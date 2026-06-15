/**
 * @file hal_statusled.cpp
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

#if defined(DEBUG_HAL)
#undef NDEBUG
#endif

#include <cstdint>
#include <cstdlib>

#include "hal_statusled.h"
#include "firmware/debug/debug_debug.h"

enum class LedStatus { OFF, ON, HEARTBEAT, FLASH };

#if defined(RASPPI)
static constexpr char RASPBIAN_LED_INIT[] = "echo gpio | sudo tee /sys/class/leds/led0/trigger";
static constexpr char RASPBIAN_LED_OFF[] = "echo 0 | sudo tee /sys/class/leds/led0/brightness";
static constexpr char RASPBIAN_LED_ON[] = "echo 1 | sudo tee /sys/class/leds/led0/brightness";
static constexpr char RASPBIAN_LED_HB[] = "echo heartbeat | sudo tee /sys/class/leds/led0/trigger";
static constexpr char RASPBIAN_LED_FLASH[] = "echo timer | sudo tee /sys/class/leds/led0/trigger";

static LedStatus s_ledStatus;
#endif

static void SetLed([[maybe_unused]] LedStatus led_status) {
#if defined(RASPPI)
    if (s_ledStatus == led_status) {
        return;
    }
    s_ledStatus = led_status;
    char* p = 0;

    switch (led_status) {
        case LedStatus::OFF:
            p = const_cast<char*>(RASPBIAN_LED_OFF);
            break;
        case LedStatus::ON:
            p = const_cast<char*>(RASPBIAN_LED_ON);
            break;
        case LedStatus::HEARTBEAT:
            p = const_cast<char*>(RASPBIAN_LED_HB);
            break;
        case LedStatus::FLASH:
            p = const_cast<char*>(RASPBIAN_LED_FLASH);
            break;
        default:
            break;
    }

    if (p != 0) {
        if (system(p) == 0) {
            // Just make the compile happy
        }
    }
#endif
}

namespace hal::statusled {
void SetFrequency(uint32_t frequency_hz) {
    if (frequency_hz == 0) {
        SetLed(LedStatus::OFF);
    } else if (frequency_hz > 20) {
        SetLed(LedStatus::ON);
    } else {
        if (frequency_hz > 1) {
            SetLed(LedStatus::HEARTBEAT);
        } else {
            SetLed(LedStatus::FLASH);
        }
    }
}
} // namespace hal::statusled
