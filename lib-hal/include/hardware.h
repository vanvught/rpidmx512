/**
 * @file hardware.h
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

#ifndef HARDWARE_H_
#define HARDWARE_H_

#include <cstdint>
#include <cstring>
#include <uuid/uuid.h>

namespace hardware {
enum class BootDevice {
	UNKOWN,
	FEL,	// H3 Only
	MMC0,
	SPI,	// H3 Only
	HDD,
	FLASH,
	RAM
};
namespace ledblink {
enum class Mode {
	OFF_OFF, OFF_ON, NORMAL, DATA, FAST, REBOOT, UNKNOWN
};
}  // namespace ledblink
}  // namespace hardware

namespace hal {
#if !defined (CONFIG_HAL_TIMERS_COUNT)
# define CONFIG_HAL_TIMERS_COUNT 8
#endif

static constexpr uint32_t SOFTWARE_TIMERS_MAX = CONFIG_HAL_TIMERS_COUNT;

typedef void (*TimerCallback)();
}  // namespace hal

#if defined(__linux__) || defined (__APPLE__)
# if defined (CONFIG_HAL_USE_MINIMUM)
#  include "linux/minimum/hardware.h"
# else
#  include "linux/hardware.h"
# endif
#else
# if defined (H3)
#  include "h3/hardware.h"
# elif defined (GD32)
#  include "gd32/hardware.h"
# else
#  include "rpi/hardware.h"
# endif
#endif

#endif /* HARDWARE_H_ */
