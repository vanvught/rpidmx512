/**
 * @file hardware.cpp
 *
 */
/* Copyright (C) 2020-2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <cstring>
#include <time.h>
#include <cassert>

#include "hardware.h"

#include "bcm2835_vc.h"
#include "bcm2835_wdog.h"

#include "arm/synchronize.h"

extern "C" {
void hardware_rtc_set(const struct tm *);
bool hardware_rtc_get(struct tm *);
}

///< Reference http://www.raspberrypi-spy.co.uk/2012/09/checking-your-raspberry-pi-board-version/
///< Reference http://elinux.org/RPi_HardwareHistory

#define MAX_NAME_LENGTH 24		///< Length for model name

struct _hardware_revision_code {
	const uint32_t value;
	const char name[MAX_NAME_LENGTH + 1];	///< Including '\0' byte
} static constexpr board_version[] __attribute__((aligned(4))) = {
		{ 0x000000, "Model Unknown" },
		{ 0x000002, "Pi 1 Model B R1 256MB" },
		{ 0x000003, "Pi 1 Model B R1 256MB" },
		{ 0x000004, "Pi 1 Model B R2 256MB" },
		{ 0x000005, "Pi 1 Model B R2 256MB" },
		{ 0x000006, "Pi 1 Model B R2 256MB" },
		{ 0x000007, "Pi 1 Model A R2 256MB" },
		{ 0x000008, "Pi 1 Model A R2 256MB" },
		{ 0x000009, "Pi 1 Model A R2 256MB" },
		{ 0x00000d, "Pi 1 Model B R2 512MB" },
		{ 0x00000e, "Pi 1 Model B R2 512MB" },
		{ 0x00000f, "Pi 1 Model B R2 512MB" },
		{ 0x000010, "Pi 1 Model B+ 512MB V1.0" },
		{ 0x000011, "Compute Module 512MB" },
		{ 0x000012, "Pi 1 Model A+ 256MB V1.1" },
		{ 0x000013, "Pi 1 Model B+ 512MB V1.2" },
		{ 0x000014, "Compute Module 512MB" },
		{ 0x000015, "Pi 1 Model A+ 256MB V1.1" },
		{ 0x900021, "Pi 1 Model A+ 512MB" },
		{ 0xa01040, "Pi 2 Model B 1GB V1.0" },
		{ 0xa01041, "Pi 2 Model B 1GB V1.1" },
		{ 0xa21041, "Pi 2 Model B 1GB V1.1" },
		{ 0xa22042, "Pi 2 Model B 1GB V1.2" },		///< 2 Model B (with BCM2837)
		{ 0x900092, "Pi Zero 512MB V1.2" },
		{ 0x900093, "Pi Zero 512MB V1.3" },
		{ 0x9000c1, "Pi Zero W 512MB" },
		{ 0xa02082, "Pi 3 Model B 1GB V1.2" },
		{ 0xa22082, "Pi 3 Model B 1GB V1.2" },
		{ 0xa020d3, "Pi 3 Model B+ 1GB" }
};

static constexpr char aSocName[4][8] = { "BCM2835", "BCM2836", "BCM2837", "Unknown" };

namespace cpu {
static constexpr char NAME[4][24] = { "ARM1176JZF-S", "Cortex-A7", "Cortex-A53 (ARMv8)", "Unknown" };
static constexpr uint8_t NAME_LENGTH[4] = { 12, 9, 18, 8 };
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

Hardware::Hardware(): m_nBoardRevision(-1), m_tSocType(SOC_TYPE_UNKNOWN) {
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

	if (m_nBoardRevision <= 0) {
		m_pBoardName = const_cast<char*>(board_version[0].name);
	} else {
		for (uint32_t i = 1; i < sizeof(board_version) / sizeof(board_version[0]); i++) {
			if (m_nBoardRevision == board_version[i].value) {
				m_pBoardName = const_cast<char*>(board_version[i].name);
			}
		}
	}
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
	nLength = MAX_NAME_LENGTH;
	return m_pBoardName;
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

extern "C" {
uint32_t bcm2835_rng_get_number(void);
}

typedef union pcast32 {
	uuid_t uuid;
	uint32_t u32[4];
} _pcast32;

void Hardware::GetUuid(uuid_t out) {
	_pcast32 cast;

	cast.u32[0] = bcm2835_rng_get_number();
	cast.u32[1] = bcm2835_rng_get_number();
	cast.u32[2] = bcm2835_rng_get_number();
	cast.u32[3] = bcm2835_rng_get_number();

	cast.uuid[6] = 0x40 | (cast.uuid[6] & 0xf);
	cast.uuid[8] = 0x80 | (cast.uuid[8] & 0x3f);

	memcpy(out, cast.uuid, sizeof(uuid_t));
}
