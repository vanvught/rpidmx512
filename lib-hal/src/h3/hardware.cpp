/**
 * @file hardware.cpp
 *
 */
/* Copyright (C) 2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <stdint.h>
#include <stddef.h>
#include <time.h>
#include <cassert>

#include "hardware.h"

#include "c/hardware.h"
#include "c/sys_time.h"
#include "c/led.h"

#include "h3.h"
#include "h3_timer.h"
#include "h3_watchdog.h"
#include "h3_board.h"

#include "arm/arm.h"
#include "arm/synchronize.h"

extern "C" {
void _start(void);
}

#if defined(ORANGE_PI)
#elif defined(ORANGE_PI_ONE)
#else
 #error Platform not supported
#endif

namespace SOC {
	static constexpr char NAME[2][4] = { "H2+", "H3\0" };
	static constexpr uint8_t NAME_LENGTH[2] = { 3, 2 };
}

namespace CPU {
	static constexpr char NAME[] = "Cortex-A7";
	static constexpr auto NAME_LENGTH = sizeof(NAME) - 1;
}

namespace MACHINE {
	static constexpr char NAME[] = "arm";
	static constexpr auto NAME_LENGTH = sizeof(NAME) - 1;
}

namespace SYSNAME {
	static constexpr char NAME[] = "Baremetal";
	static constexpr auto NAME_LENGTH = sizeof(NAME) - 1;
}

Hardware *Hardware::s_pThis = 0;

Hardware::Hardware(void) {
	assert(s_pThis == 0);
	s_pThis = this;
}

Hardware::~Hardware(void) {
}

const char *Hardware::GetMachine(uint8_t &nLength) {
	nLength = MACHINE::NAME_LENGTH;
	return MACHINE::NAME;
}

const char *Hardware::GetSysName(uint8_t &nLength) {
	nLength = SYSNAME::NAME_LENGTH;
	return SYSNAME::NAME;
}

const char *Hardware::GetBoardName(uint8_t &nLength) {
	nLength = sizeof(H3_BOARD_NAME)  - 1;
	return H3_BOARD_NAME;
}

const char *Hardware::GetCpuName(uint8_t &nLength) {
	nLength = CPU::NAME_LENGTH;
	return CPU::NAME;
}

const char *Hardware::GetSocName(uint8_t &nLength) {
#if defined(ORANGE_PI)
	nLength = SOC::NAME_LENGTH[0];
	return SOC::NAME[0];
#else
	nLength = SOC::NAME_LENGTH[1];
	return SOC::NAME[1];
#endif
}

bool Hardware::SetTime(const struct tm *pTime) {
	hardware_rtc_set(pTime);
	return true;
}

void Hardware::GetTime(struct tm *pTime) {
	time_t ltime;
	struct tm *local_time;

	ltime = time(0);
    local_time = localtime(&ltime);

    pTime->tm_year = local_time->tm_year;
    pTime->tm_mon = local_time->tm_mon ;
    pTime->tm_mday = local_time->tm_mday;
    //
    pTime->tm_hour = local_time->tm_hour;
    pTime->tm_min = local_time->tm_min;
    pTime->tm_sec = local_time->tm_sec;
}

bool Hardware::Reboot(void) {
	hardware_led_set(1);

	if (m_pRebootHandler != 0) {
		h3_watchdog_disable();
		m_pRebootHandler->Run();
	}

	h3_watchdog_enable();

	invalidate_instruction_cache();
	flush_branch_target_cache();
	flush_prefetch_buffer();
	clean_data_cache();
	invalidate_data_cache();

	led_set_ticks_per_second(1000000 / 8);

	for (;;) {
		led_blink();
	}

	__builtin_unreachable ();
	return true;
}
