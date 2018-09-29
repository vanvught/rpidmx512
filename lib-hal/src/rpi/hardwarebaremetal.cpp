/**
 * @file hardwarebaremetal.h
 *
 */
/* Copyright (C) 2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include "hardwarebaremetal.h"

#include "c/hardware.h"
#include "c/sys_time.h"

#include "bcm2835_vc.h"
#include "bcm2835_wdog.h"

#include "arm/synchronize.h"

const char s_Release[] __attribute__((aligned(4))) = "1.3";
#define RELEASE_LENGTH (sizeof(s_Release)/sizeof(s_Release[0]) - 1)

static const char s_SocName[4][8] __attribute__((aligned(4))) = { "BCM2835", "BCM2836", "BCM2837", "Unknown" };
static const char s_CpuName[4][24] __attribute__((aligned(4))) = { "ARM1176JZF-S", "Cortex-A7", "Cortex-A53 (ARMv8)", "Unknown" };
static const uint8_t s_nCpuNameLength[4] __attribute__((aligned(4))) = {(uint8_t) 12, (uint8_t) 9, (uint8_t) 18, (uint8_t) 8};

const char s_Machine[] __attribute__((aligned(4))) = "arm";
#define MACHINE_LENGTH (sizeof(s_Machine)/sizeof(s_Machine[0]) - 1)

const char s_SysName[] __attribute__((aligned(4))) = "Baremetal";
#define SYSNAME_LENGTH (sizeof(s_SysName)/sizeof(s_SysName[0]) - 1)

const char s_Version[] __attribute__((aligned(4))) = __DATE__ "" "" __TIME__;
#define VERSION_LENGTH (sizeof(s_Version)/sizeof(s_Version[0]) - 1)

HardwareBaremetal::HardwareBaremetal(void): m_nBoardRevision(-1), m_tSocType(SOC_TYPE_UNKNOWN) {
	m_nBoardRevision = bcm2835_vc_get_get_board_revision();

	if ((m_nBoardRevision & ((int32_t) 1 << 23)) == ((int32_t) 1 << 23)) {
		TSocType type = (TSocType) (((uint32_t) m_nBoardRevision >> 12) & 0xF);
		if (type > SOC_TYPE_UNKNOWN) {
		} else {
			m_tSocType = type;
		}
	} else {
		m_tSocType = SOC_TYPE_BCM2835;
	}

	m_nBoardId = bcm2835_vc_get_get_board_revision();
}

HardwareBaremetal::~HardwareBaremetal(void) {
}

const char* HardwareBaremetal::GetMachine(uint8_t& nLength) {
	nLength = MACHINE_LENGTH;
	return s_Machine;
}

const char* HardwareBaremetal::GetRelease(uint8_t& nLength) {
	nLength = RELEASE_LENGTH;
	return s_Release;
}

const char* HardwareBaremetal::GetSysName(uint8_t& nLength) {
	nLength = SYSNAME_LENGTH;
	return s_SysName;
}

const char* HardwareBaremetal::GetVersion(uint8_t& nLength) {
	nLength = VERSION_LENGTH;
	return s_Version;
}

const char* HardwareBaremetal::GetBoardName(uint8_t& nLength) {
	nLength = hardware_board_get_model_length();
	return hardware_board_get_model();
}

uint32_t HardwareBaremetal::GetBoardId(void) {
	return m_nBoardId;
}

const char* HardwareBaremetal::GetCpuName(uint8_t& nLength) {
	nLength = s_nCpuNameLength[m_tSocType];
	return s_CpuName[m_tSocType];
}

const char* HardwareBaremetal::GetSocName(uint8_t& nLength) {
	nLength = sizeof s_SocName[0]; // Same length for all
	return s_SocName[m_tSocType];
}

float HardwareBaremetal::GetCoreTemperature(void) {
	return (float) bcm2835_vc_get_temperature() / 1000;
}

float HardwareBaremetal::GetCoreTemperatureMax(void) {
	return 85; //TODO GetCoreTemperatureMax
}

uint32_t HardwareBaremetal::GetReleaseId(void) {
	return (uint32_t) bcm2835_vc_get_get_firmware_revision();
}

uint64_t HardwareBaremetal::GetUpTime(void) {
	return hardware_uptime_seconds();
}

bool HardwareBaremetal::SetTime(const struct THardwareTime& pTime) {
	struct hardware_time tm_hw;

	tm_hw.year = pTime.tm_year;
	tm_hw.month = pTime.tm_mon;
	tm_hw.day = pTime.tm_mday;
	tm_hw.hour = pTime.tm_hour;
	tm_hw.minute = pTime.tm_min;
	tm_hw.second = pTime.tm_sec;

	hardware_rtc_set(&tm_hw);

	return true;
}

void HardwareBaremetal::GetTime(struct THardwareTime* pTime) {
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

void HardwareBaremetal::SetLed(THardwareLedStatus tLedStatus) {
	if (tLedStatus == HARDWARE_LED_OFF) {
		hardware_led_set(0);
	} else {
		hardware_led_set(1);
	}
}

bool HardwareBaremetal::Reboot(void) {
	hardware_led_set(1);

	bcm2835_watchdog_init();

	invalidate_instruction_cache();
	clean_data_cache();
	invalidate_data_cache();

	for (;;)
		;

	return true;
}

bool HardwareBaremetal::PowerOff(void) {
	// int32_t bcm2835_vc_set_power_state(uint32_t, uint32_t);
	return false;
}

void HardwareBaremetal::WatchdogInit(void) {
	bcm2835_watchdog_init();
}

void HardwareBaremetal::WatchdogFeed(void) {
	bcm2835_watchdog_feed();
}

void HardwareBaremetal::WatchdogStop(void) {
	bcm2835_watchdog_stop();
}

uint32_t HardwareBaremetal::Micros(void) {
	return BCM2835_ST->CLO;
}

uint32_t HardwareBaremetal::Millis(void) {
	return millis();
}

bool HardwareBaremetal::IsButtonPressed(void) {
	return false;
}
