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

#ifndef LINUX_HAL_H_
#define LINUX_HAL_H_

#include <cstdint>
#include <time.h>

#include "superloop/softwaretimers.h"

#include "debug.h"

enum class Board {
	TYPE_LINUX,
	TYPE_RASPBIAN,
	TYPE_OSX,
	TYPE_UNKNOWN
};

Board linux_board_type();
const char *linux_board_name(uint8_t& nLength);
const char *linux_soc_name(uint8_t& nLength);
const char *linux_cpu_name(uint8_t& nLength);
const char *linux_machine_name(uint8_t& nLength);
const char *linux_sys_name(uint8_t& nLength);
void linux_print();
float linux_core_temperature_current();
uint32_t linux_micros();
uint32_t linux_millis();
bool linux_reboot();

namespace hal {
static constexpr uint32_t BOARD_ID = 0;
static constexpr uint32_t RELEASE_ID = 0;
static constexpr const char WEBSITE[] = "www.gd32-dmx.org";

inline const char *board_name(uint8_t &nLength) {
	return linux_board_name(nLength);
}

inline const char *soc_name(uint8_t &nLength) {
	return linux_soc_name(nLength);
}

inline const char *cpu_name(uint8_t &nLength) {
	return linux_cpu_name(nLength);
}

inline const char *machine_name(uint8_t &nLength) {
	return linux_machine_name(nLength);
}

inline const char *sys_name(uint8_t &nLength) {
	return linux_sys_name(nLength);
}

uint32_t get_uptime();

inline hal::BootDevice boot_device() {
#if defined (RASPPI)
	return hal::BootDevice::MMC0;
#else
	return hal::BootDevice::HDD;
#endif
}

inline uint32_t millis() {
	return linux_millis();
}

inline uint32_t micros() {
	return linux_micros();
}

inline uint32_t uptime() {
	return hal::get_uptime();
}

inline bool set_rtc([[maybe_unused]] const struct tm *pTime) {
	DEBUG_PRINTF("%s", asctime(pTime));
	return true;
}

static constexpr float CORE_TEMPERATURE_MIN = -40.0;
static constexpr float CORE_TEMPERATURE_MAX = +90.0;

inline float core_temperature_current() {
	return linux_core_temperature_current();
}

inline void print() {
	linux_print();
}

inline bool watchdog() { return false; } // Not implemented
inline void watchdog_init() { } // Not implemented
inline void watchdog_feed() { } // Not implemented
inline void watchdog_stop() { } // Not implemented

inline bool reboot() {
	return linux_reboot();
}

inline void run() {
	SoftwareTimerRun();
}
}  // namespace hal

#endif /* LINUX_HAL_H_ */
