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
 * copies of thnDmxDataDirecte Software, and to permit persons to whom the Software is
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
#include <cstddef>
#include <string.h>
#include <time.h>
#include <cassert>

#include "hardware.h"

#include "h3_watchdog.h"
#include "h3_gpio.h"
#include "h3_board.h"

#include "arm/synchronize.h"

#if defined (DEBUG_I2C)
# include "i2cdetect.h"
#endif

#if !defined(DISABLE_RTC)
# include "hwclock.h"
#endif

#include "logic_analyzer.h"

namespace soc {
#if defined (ORANGE_PI)
	static constexpr char NAME[] = "H2+";
	static constexpr auto NAME_LENGTH = sizeof(NAME) - 1;
#elif defined(ORANGE_PI_ONE)
	static constexpr char NAME[] = "H3";
	static constexpr auto NAME_LENGTH = sizeof(NAME) - 1;
#else
# error Platform not supported
#endif
}

namespace cpu {
	static constexpr char NAME[] = "Cortex-A7";
	static constexpr auto NAME_LENGTH = sizeof(NAME) - 1;
}

namespace machine {
	static constexpr char NAME[] = "arm";
	static constexpr auto NAME_LENGTH = sizeof(NAME) - 1;
}

namespace sysname {
	static constexpr char NAME[] = "Baremetal";
	static constexpr auto NAME_LENGTH = sizeof(NAME) - 1;
}

namespace hal {
void uuid_init(uuid_t);
}  // namespace hardware

Hardware *Hardware::s_pThis;

void hardware_init();

Hardware::Hardware() {
	assert(s_pThis == nullptr);
	s_pThis = this;

	hardware_init();
	hal::uuid_init(m_uuid);

#if defined (DEBUG_I2C)
	I2cDetect i2cdetect;
#endif

#if !defined(DISABLE_RTC)
	m_HwClock.RtcProbe();
	m_HwClock.Print();
	m_HwClock.HcToSys();
#endif

	hardware_led_set(1);

	logic_analyzer::init();
}

const char *Hardware::GetMachine(uint8_t &nLength) {
	nLength = machine::NAME_LENGTH;
	return machine::NAME;
}

const char *Hardware::GetSysName(uint8_t &nLength) {
	nLength = sysname::NAME_LENGTH;
	return sysname::NAME;
}

const char *Hardware::GetBoardName(uint8_t &nLength) {
	nLength = sizeof(H3_BOARD_NAME)  - 1;
	return H3_BOARD_NAME;
}

const char *Hardware::GetCpuName(uint8_t &nLength) {
	nLength = cpu::NAME_LENGTH;
	return cpu::NAME;
}

const char *Hardware::GetSocName(uint8_t &nLength) {
	nLength = soc::NAME_LENGTH;
	return soc::NAME;
}

#include <cstdio>

bool Hardware::Reboot() {
	puts("Rebooting ...");
	
#if !defined(DISABLE_RTC)
	m_HwClock.SysToHc();
#endif

	h3_watchdog_disable();

	RebootHandler();

	h3_watchdog_enable();

	clean_data_cache();
	invalidate_data_cache();

	h3_gpio_fsel(EXT_SPI_MOSI, GPIO_FSEL_INPUT);
	h3_gpio_set_pud(EXT_SPI_MOSI, GPIO_PULL_DOWN);
	h3_gpio_fsel(EXT_SPI_CLK, GPIO_FSEL_INPUT);
	h3_gpio_set_pud(EXT_SPI_CLK, GPIO_PULL_DOWN);
	h3_gpio_fsel(EXT_SPI_CS, GPIO_FSEL_INPUT);
	h3_gpio_set_pud(EXT_SPI_CS, GPIO_PULL_DOWN);

	SetMode(hardware::ledblink::Mode::REBOOT);

	for (;;) {
		Run();
	}

	__builtin_unreachable();
	return true;
}
