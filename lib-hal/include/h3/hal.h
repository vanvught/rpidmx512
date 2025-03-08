/**
 * @file hal.h
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

#ifndef H3_HAL_H_
#define H3_HAL_H_

#include <cstdint>
#include <cassert>

#include "h3.h"
#include "h3_board.h"
#include "h3_thermal.h"
#include "h3_watchdog.h"
#include "superloop/softwaretimers.h"

#if !defined(DISABLE_RTC)
# include "hwclock.h"
#endif

#if defined (DEBUG_STACK)
 void stack_debug_run();
#endif

uint32_t h3_uptime();

namespace hal {
extern bool g_bWatchdog;

#if defined(ORANGE_PI)
 static constexpr uint32_t BOARD_ID = 0;
#elif defined(ORANGE_PI_ONE)
 static constexpr uint32_t BOARD_ID = 1;
#else
# error Platform not supported
#endif
static constexpr uint32_t RELEASE_ID = 0;
static constexpr const char WEBSITE[] = "www.orangepi-dmx.org";

inline const char *board_name(uint8_t &nLength) {
	nLength = sizeof(H3_BOARD_NAME)  - 1;
	return H3_BOARD_NAME;
}

inline const char *soc_name(uint8_t &nLength) {
#if defined (ORANGE_PI)
	static constexpr const char SOC_NAME[] = "H2+";
	static constexpr auto SOC_NAME_LENGTH = sizeof(SOC_NAME) - 1;
#elif defined(ORANGE_PI_ONE)
	static constexpr char SOC_NAME[] = "H3";
	static constexpr auto SOC_NAME_LENGTH = sizeof(SOC_NAME) - 1;
#endif
	nLength = SOC_NAME_LENGTH;
	return SOC_NAME;
}

inline const char *cpu_name(uint8_t &nLength) {
	static constexpr const char CPU_NAME[] = "Cortex-A7";
	static constexpr auto CPU_NAME_LENGTH = sizeof(CPU_NAME) - 1;
	nLength = CPU_NAME_LENGTH;
	return CPU_NAME;
}

inline const char *machine_name(uint8_t &nLength) {
	static constexpr const char MACHINE_NAME[] = "arm";
	static constexpr auto MACHINE_NAME_LENGTH = sizeof(MACHINE_NAME) - 1;
	nLength = MACHINE_NAME_LENGTH;
	return MACHINE_NAME;
}

inline const char *sys_name(uint8_t &nLength) {
	static constexpr const char SYS_NAME[] = "Baremetal";
	static constexpr auto SYS_NAME_LENGTH = sizeof(SYS_NAME) - 1;
	nLength = SYS_NAME_LENGTH;
	return SYS_NAME;
}

inline hal::BootDevice boot_device() {
	return static_cast<hal::BootDevice>(h3_get_boot_device());
}

inline uint32_t millis() {
	return H3_TIMER->AVS_CNT0;
}

inline uint32_t micros() {
	return H3_TIMER->AVS_CNT1;
}

inline uint32_t uptime() {
	return h3_uptime();
}

inline bool set_rtc([[maybe_unused]] const struct tm *pTime) {
#if !defined(DISABLE_RTC)
	assert(HwClock::Get() != nullptr);
	HwClock::Get()->Set(pTime);
	return true;
#else
	return false;
#endif
}

static constexpr float CORE_TEMPERATURE_MIN = -40.0;
static constexpr float CORE_TEMPERATURE_MAX = +90.0;

inline float core_temperature_current() {
	return static_cast<float>(h3_thermal_gettemp());
}

inline void watchdog_init() {
	g_bWatchdog = true;
	h3_watchdog_enable();
}

inline void watchdog_feed() {
	h3_watchdog_restart();
}

inline void watchdog_stop() {
	g_bWatchdog = false;
	h3_watchdog_disable();
}

inline bool watchdog() {
	return g_bWatchdog;
}

bool reboot();

inline bool power_off() {
	return false;
}

inline void run() {
	SoftwareTimerRun();
#if defined (DEBUG_STACK)
	stack_debug_run();
#endif
}
}  // namespace hal

#endif /* H3_HAL_H_ */
