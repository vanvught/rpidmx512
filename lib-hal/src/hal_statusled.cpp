/*
 * hal_statusled.cpp
 */
/* Copyright (C) 2025 by Arjan van Vught mailto:infogd32-dmx.org
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

#include "hal_statusled.h"

 #include "firmware/debug/debug_debug.h"

namespace hal::statusled
{
namespace global
{
Mode g_status_led_mode;
} // namespace global

static bool s_do_lock;

enum class ModeToFrequency
{
    kOffOff = 0,
    kNormal = 1,
    kData = 3,
    kFast = 5,
    kReboot = 8,
    kOffOn = 255
};

#if !defined(CONFIG_HAL_USE_MINIMUM)

void __attribute__((weak)) Event([[maybe_unused]] Mode mode)
{
    DEBUG_PRINTF("mode=%u", static_cast<uint32_t>(mode));
}

void SetModeWithLock(Mode mode, bool do_lock)
{
    s_do_lock = false;
    SetMode(mode);
    s_do_lock = do_lock;
}

void SetMode(hal::statusled::Mode mode)
{
    if (s_do_lock || (global::g_status_led_mode == mode))
    {
        return;
    }

    global::g_status_led_mode = mode;

    switch (global::g_status_led_mode)
    {
        case hal::statusled::Mode::OFF_OFF:
            SetFrequency(static_cast<uint32_t>(ModeToFrequency::kOffOff));
            break;
        case hal::statusled::Mode::OFF_ON:
            SetFrequency(static_cast<uint32_t>(ModeToFrequency::kOffOn));
            break;
        case hal::statusled::Mode::NORMAL:
            SetFrequency(static_cast<uint32_t>(ModeToFrequency::kNormal));
            break;
        case hal::statusled::Mode::DATA:
            SetFrequency(static_cast<uint32_t>(ModeToFrequency::kData));
            break;
        case hal::statusled::Mode::FAST:
            SetFrequency(static_cast<uint32_t>(ModeToFrequency::kFast));
            break;
        case hal::statusled::Mode::REBOOT:
            SetFrequency(static_cast<uint32_t>(ModeToFrequency::kReboot));
            break;
        default:
            SetFrequency(static_cast<uint32_t>(ModeToFrequency::kOffOff));
            break;
    }

    hal::statusled::Event(global::g_status_led_mode);

    DEBUG_PRINTF("global::g_status_led_mode=%u", static_cast<uint32_t>(global::g_status_led_mode));
}
#endif
} // namespace hal::statusled
