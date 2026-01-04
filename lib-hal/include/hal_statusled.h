/**
 * @file hal_statusled.h
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

#ifndef HAL_STATUSLED_H_
#define HAL_STATUSLED_H_

#include <cstdint>

namespace hal::statusled
{
enum class Mode
{
    OFF_OFF,
    OFF_ON,
    NORMAL,
    DATA,
    FAST,
    REBOOT,
    UNKNOWN
};
namespace global
{
extern Mode g_status_led_mode;
} // namespace global
void SetModeWithLock(Mode mode, bool do_lock);
void SetMode(Mode mode);
inline Mode GetMode()
{
    return global::g_status_led_mode;
}
void SetFrequency(uint32_t frequency_hz);
void Event(Mode mode);
}  // namespace hal::statusled

#endif  // HAL_STATUSLED_H_
