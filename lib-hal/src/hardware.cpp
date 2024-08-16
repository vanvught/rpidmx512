/**
 * @file hardware.cpp
 *
 */
/* Copyright (C) 2023-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include "hardware.h"
#include "debug.h"

namespace hardware::ledblink {
void __attribute__((weak)) display([[maybe_unused]] const uint32_t nState) {}
}  // namespace hardware::ledblink

enum class FreqMode {
	OFF_OFF = 0, NORMAL = 1, DATA = 3, FAST = 5, REBOOT = 8, OFF_ON = 255
};

#if !defined (CONFIG_HAL_USE_MINIMUM)
void Hardware::SetModeWithLock(hardware::ledblink::Mode mode, bool doLock) {
	m_doLock = false;
	SetMode(mode);
	m_doLock = doLock;
}

void Hardware::SetMode(hardware::ledblink::Mode mode) {
	if (m_doLock || (m_Mode == mode)) {
		return;
	}

	m_Mode = mode;

	switch (m_Mode) {
	case hardware::ledblink::Mode::OFF_OFF:
		SetFrequency(static_cast<uint32_t>(FreqMode::OFF_OFF));
		break;
	case hardware::ledblink::Mode::OFF_ON:
		SetFrequency(static_cast<uint32_t>(FreqMode::OFF_ON));
		break;
	case hardware::ledblink::Mode::NORMAL:
		SetFrequency(static_cast<uint32_t>(FreqMode::NORMAL));
		break;
	case hardware::ledblink::Mode::DATA:
		SetFrequency(static_cast<uint32_t>(FreqMode::DATA));
		break;
	case hardware::ledblink::Mode::FAST:
		SetFrequency(static_cast<uint32_t>(FreqMode::FAST));
		break;
	case hardware::ledblink::Mode::REBOOT:
		SetFrequency(static_cast<uint32_t>(FreqMode::REBOOT));
		break;
	default:
		SetFrequency(static_cast<uint32_t>(FreqMode::OFF_OFF));
		break;
	}

	hardware::ledblink::display(static_cast<uint32_t>(m_Mode));

	DEBUG_PRINTF("Mode=%d", static_cast<int>(m_Mode));
}
#endif
