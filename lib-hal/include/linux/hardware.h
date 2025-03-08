/**
 * @file hardware.h
 *
 */
/* Copyright (C) 2020-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef LINUX_HARDWARE_H_
#define LINUX_HARDWARE_H_

#if defined(__linux__) || defined (__APPLE__)
#else
# error
#endif

#include <cstdint>

#include "superloop/softwaretimers.h"

namespace hardware {
enum class LedStatus {
	OFF, ON, HEARTBEAT, FLASH
};
}  // namespace hardware

class Hardware {
public:
	Hardware();

	void SetLed(hardware::LedStatus tLedStatus);

	void SetModeWithLock(hardware::ledblink::Mode mode, bool doLock);

	void SetMode(hardware::ledblink::Mode mode);

	hardware::ledblink::Mode GetMode() {
		return m_Mode;
	}

	 static Hardware *Get() {
		return s_pThis;
	}

private:
	void SetFrequency(uint32_t nFreqHz) {
		if (nFreqHz == 0) {
			SetLed(hardware::LedStatus::OFF);
		} else if (nFreqHz > 20) {
			SetLed(hardware::LedStatus::ON);
		} else {
			if (nFreqHz > 1) {
				SetLed(hardware::LedStatus::HEARTBEAT);
			} else {
				SetLed(hardware::LedStatus::FLASH);
			}
		}
	}

private:
	hardware::ledblink::Mode m_Mode { hardware::ledblink::Mode::UNKNOWN };
	bool m_doLock { false };

	static inline Hardware *s_pThis;
};

#endif /* LINUX_HARDWARE_H_ */
