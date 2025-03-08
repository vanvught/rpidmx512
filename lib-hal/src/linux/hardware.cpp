#if !defined (CONFIG_HAL_USE_MINIMUM)
/**
 * @file hardware.cpp
 *
 */
/* Copyright (C) 2020-2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <cstdio>
#include <cctype>
#include <cstring>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/reboot.h>
#include <sys/utsname.h>
#include <cassert>

#include "hardware.h"

#include "exec_cmd.h"

#include "debug.h"

namespace global {
int32_t *gp_nUtcOffset;
}  // namespace global

void linux_init();

#if defined (__linux__)
static constexpr char RASPBIAN_LED_INIT[] = "echo gpio | sudo tee /sys/class/leds/led0/trigger";
static constexpr char RASPBIAN_LED_OFF[] = "echo 0 | sudo tee /sys/class/leds/led0/brightness";
static constexpr char RASPBIAN_LED_ON[] = "echo 1 | sudo tee /sys/class/leds/led0/brightness";
static constexpr char RASPBIAN_LED_HB[] = "echo heartbeat | sudo tee /sys/class/leds/led0/trigger";
static constexpr char RASPBIAN_LED_FLASH[] = "echo timer | sudo tee /sys/class/leds/led0/trigger";
#endif

Hardware::Hardware() {
	assert(s_pThis == nullptr);
	s_pThis = this;

	linux_init();
}

#if defined (__linux__)
 static hardware::LedStatus s_ledStatus;
#endif

void Hardware::SetLed([[maybe_unused]] hardware::LedStatus ledStatus) {
#if defined (__linux__)
	if (linux_board_type() == Board::TYPE_RASPBIAN) {
		if (s_ledStatus == ledStatus) {
			return;
		}
		s_ledStatus = ledStatus;
		char *p = 0;

		switch (ledStatus) {
		case hardware::LedStatus::OFF:
			p = const_cast<char*>(RASPBIAN_LED_OFF);
			break;
		case hardware::LedStatus::ON:
			p = const_cast<char*>(RASPBIAN_LED_ON);
			break;
		case hardware::LedStatus::HEARTBEAT:
			p = const_cast<char*>(RASPBIAN_LED_HB);
			break;
		case hardware::LedStatus::FLASH:
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
	}
#endif
}
#endif
