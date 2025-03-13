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

namespace hal {
enum class StatusLedMode {
	OFF_OFF, OFF_ON, NORMAL, DATA, FAST, REBOOT, UNKNOWN
};

namespace global {
extern StatusLedMode g_statusLedMode;
}  // namespace global

void statusled_set_mode_with_lock(const StatusLedMode mode, const bool doLock);
void statusled_set_mode(const StatusLedMode mode);
inline StatusLedMode statusled_get_mode() {
	return global::g_statusLedMode;
}
void statusled_set_frequency(const uint32_t nFrequencyHz);
void display_statusled(const StatusLedMode mode);
}  // namespace hal

#endif /* HAL_STATUSLED_H_ */
