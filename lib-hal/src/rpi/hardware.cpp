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

#include "bcm2835_vc.h"
#include "bcm2835_wdog.h"

#include "arm/synchronize.h"

static constexpr char aSocName[4][8] = { "BCM2835", "BCM2836", "BCM2837", "Unknown" };

namespace cpu {
	static constexpr char NAME[4][24] = { "ARM1176JZF-S", "Cortex-A7", "Cortex-A53 (ARMv8)", "Unknown" };
	static constexpr uint8_t NAME_LENGTH[4] = {12, 9, 18, 8};
}

namespace machine {
	static constexpr char NAME[] = "arm";
	static constexpr auto NAME_LENGTH = sizeof(NAME) - 1;
}

namespace sysname {
	static constexpr char NAME[] = "Baremetal";
	static constexpr auto NAME_LENGTH = sizeof(NAME) - 1;
}

Hardware *Hardware::s_pThis = 0;

Hardware::Hardware(): m_nBoardId(-1), m_nBoardRevision(-1), m_tSocType(SOC_TYPE_UNKNOWN) {
	assert(s_pThis == 0);
	s_pThis = this;

	m_nBoardRevision = bcm2835_vc_get_get_board_revision();

	if ((m_nBoardRevision & (static_cast<int32_t>(1) << 23)) == (static_cast<int32_t>(1) << 23)) {
		TSocType type = static_cast<TSocType>(((static_cast<uint32_t>((m_nBoardRevision)) >> 12) & 0xF));
		if (type > SOC_TYPE_UNKNOWN) {
		} else {
			m_tSocType = type;
		}
	} else {
		m_tSocType = SOC_TYPE_BCM2835;
	}

	m_nBoardId = bcm2835_vc_get_get_board_revision();
}

const char *Hardware::GetMachine(uint8_t& nLength) {
	nLength = machine::NAME_LENGTH;
	return machine::NAME;
}

const char *Hardware::GetSysName(uint8_t& nLength) {
	nLength = sysname::NAME_LENGTH;
	return sysname::NAME;
}

const char *Hardware::GetBoardName(uint8_t& nLength) {
	nLength = hardware_board_get_model_length();
	return hardware_board_get_model();
}

const char *Hardware::GetCpuName(uint8_t& nLength) {
	nLength = cpu::NAME_LENGTH[m_tSocType];
	return cpu::NAME[m_tSocType];
}

const char *Hardware::GetSocName(uint8_t& nLength) {
	nLength = sizeof aSocName[0]; // Same length for all
	return aSocName[m_tSocType];
}

bool Hardware::SetTime(const struct tm *pTime) {
	hardware_rtc_set(pTime);
	return true;
}

void Hardware::GetTime(struct tm *pTime) {
	time_t ltime;
	struct tm *local_time;

	ltime = time(NULL);
    local_time = localtime(&ltime);

    pTime->tm_year = local_time->tm_year;
    pTime->tm_mon = local_time->tm_mon ;
    pTime->tm_mday = local_time->tm_mday;
    //
    pTime->tm_hour = local_time->tm_hour;
    pTime->tm_min = local_time->tm_min;
    pTime->tm_sec = local_time->tm_sec;
}

bool Hardware::Reboot() {
	hardware_led_set(1);

	bcm2835_watchdog_init();

	invalidate_instruction_cache();
	clean_data_cache();
	invalidate_data_cache();

	for (;;)
		;

	__builtin_unreachable ();

	return true;
}

