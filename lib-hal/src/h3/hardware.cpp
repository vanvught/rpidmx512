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
#include <assert.h>

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

constexpr char aSocName[2][4] = { "H2+", "H3\0" };
constexpr uint8_t s_SocNameLenghth[2] = { 3, 2 };

constexpr char aCpuName[] = "Cortex-A7";
#define CPU_NAME_LENGHTH (sizeof(aCpuName) - 1)

const char aMachine[] = "arm";
#define MACHINE_LENGTH (sizeof(aMachine) - 1)

const char aSysName[] = "Baremetal";
#define SYSNAME_LENGTH (sizeof(aSysName) - 1)

Hardware *Hardware::s_pThis = 0;

Hardware::Hardware(void) : m_pRebootHandler(0), m_pSoftResetHandler(0), m_bIsWatchdog(false) {
	assert(s_pThis == 0);
	s_pThis = this;
}

Hardware::~Hardware(void) {
}

const char *Hardware::GetMachine(uint8_t &nLength) {
	nLength = MACHINE_LENGTH;
	return aMachine;
}

const char *Hardware::GetSysName(uint8_t &nLength) {
	nLength = SYSNAME_LENGTH;
	return aSysName;
}

const char *Hardware::GetBoardName(uint8_t &nLength) {
	nLength = sizeof(H3_BOARD_NAME)  - 1;
	return H3_BOARD_NAME;
}

const char *Hardware::GetCpuName(uint8_t &nLength) {
	nLength = CPU_NAME_LENGHTH;
	return aCpuName;
}

const char *Hardware::GetSocName(uint8_t &nLength) {
#if defined(ORANGE_PI)
	nLength = s_SocNameLenghth[0];
	return aSocName[0];
#else
	nLength = s_SocNameLenghth[1];
	return aSocName[1];
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

void Hardware::SoftReset(void) {
	__disable_irq();
	__disable_fiq();
	dmb();

	if (m_pSoftResetHandler != 0) {
		h3_watchdog_disable();
		m_pSoftResetHandler->Run();
	}

	invalidate_instruction_cache();
	flush_branch_target_cache();
	flush_prefetch_buffer();
	clean_data_cache();
	invalidate_data_cache();

	_start();

	__builtin_unreachable();
}

