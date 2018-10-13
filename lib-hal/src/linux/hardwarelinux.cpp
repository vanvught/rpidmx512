/**
 * @file hardwarelinux.c
 *
 */
/* Copyright (C) 2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include <stdint.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#if !defined (__CYGWIN__)
 #include <sys/reboot.h>
#endif
#if defined (__APPLE__)
 #include <sys/sysctl.h>
#else
 #include <sys/sysinfo.h>
#endif
#include <sys/utsname.h>
#include <assert.h>

#include "hardwarelinux.h"

extern "C" {
 char *str_find_replace(char *str, const char *find, const char *replace);
}

#if defined (__linux__)
 static const char RASPBIAN_LED_INIT[] = "echo gpio | sudo tee /sys/class/leds/led0/trigger";
 static const char RASPBIAN_LED_OFF[] = "echo 0 | sudo tee /sys/class/leds/led0/brightness";
 static const char RASPBIAN_LED_ON[] = "echo 1 | sudo tee /sys/class/leds/led0/brightness";
 static const char RASPBIAN_LED_HB[] = "echo heartbeat | sudo tee /sys/class/leds/led0/trigger";
 static const char RASPBIAN_LED_FLASH[] = "echo timer | sudo tee /sys/class/leds/led0/trigger";
#endif

 static const char UNKNOWN[] = "Unknown";

HardwareLinux::HardwareLinux(void):
#if defined (__CYGWIN__)
	m_tBoardType(BOARD_TYPE_CYGWIN)
#elif defined (__linux__)
	m_tBoardType(BOARD_TYPE_LINUX)
#elif defined (__APPLE__)
	m_tBoardType(BOARD_TYPE_OSX)
#else
	m_tBoardType(BOARD_TYPE_UNKNOWN)
#endif
{
	memset(&m_TOsInfo, 0, sizeof(struct utsname));

	strcpy(m_aCpuName, UNKNOWN);
	strcpy(m_aBoardName, UNKNOWN);

	m_aSocName[0] = '\0';

#if defined (__linux__)
	const char cmd[] = "which /opt/vc/bin/vcgencmd";
	char buf[16];

	FILE *fp = popen(cmd, "r");

	if (fgets(buf, sizeof(buf)-1, fp) != 0) {
		m_tBoardType = BOARD_TYPE_RASPBIAN;
		if (system((const char*) RASPBIAN_LED_INIT) == 0) {
			// Just make the compile happy
		}
	}

	if (fp != 0) {
		pclose(fp);
	}
#endif

	if (m_tBoardType != BOARD_TYPE_UNKNOWN) {
		uname(&m_TOsInfo);
	}

#ifndef NDEBUG
	printf("m_tBoardType=%d\n", (int) m_tBoardType);
#endif


	{ // Board Name
#if defined (__APPLE__)
		const char cat[] = "sysctl -n hw.model";
		ExecCmd(cat, m_aBoardName, sizeof(m_aBoardName));
#elif defined (__linux__)
		const char cat[] = "cat /sys/firmware/devicetree/base/model";
		if (!ExecCmd(cat, m_aBoardName, sizeof(m_aBoardName))) {
			const char cat[] = "cat /sys/class/dmi/id/board_name";
			ExecCmd(cat, m_aBoardName, sizeof(m_aBoardName));
		}
#endif
		str_find_replace(m_aBoardName, "Rev ", "V");
	}

	{ // CPU Name
#if defined (__APPLE__)
		const char cmd[] = "sysctl -n machdep.cpu.brand_string";
#else
		const char cmd[] = "cat /proc/cpuinfo | grep 'model name' | head -n 1 | sed 's/^[^:]*://g' |  sed 's/^[^ ]* //g'";
#endif
		ExecCmd(cmd, m_aCpuName, sizeof(m_aCpuName));
	}

	{ // SoC Name
		const char cmd[] = "cat /proc/cpuinfo | grep 'Hardware' | awk '{print $3}'";
		ExecCmd(cmd, m_aSocName, sizeof(m_aSocName));
	}

	if (m_tBoardType == BOARD_TYPE_RASPBIAN) {
		char aResult[16];
		const char cmd[] = "cat /proc/cpuinfo | grep 'Revision' | awk '{print $3}'";
		ExecCmd(cmd, aResult, sizeof(aResult));
		m_nBoardId = strtol(aResult, NULL, 16);
	}
}

HardwareLinux::~HardwareLinux(void) {
	m_tBoardType = BOARD_TYPE_UNKNOWN;
}

const char* HardwareLinux::GetMachine(uint8_t &nLength) {
	nLength = (uint8_t) strlen(m_TOsInfo.machine);
	return m_TOsInfo.machine;
}

const char* HardwareLinux::GetRelease(uint8_t& nLength) {
	nLength = (uint8_t) strlen(m_TOsInfo.release);
	return m_TOsInfo.release;
}

const char* HardwareLinux::GetSysName(uint8_t& nLength) {
	nLength = (uint8_t) strlen(m_TOsInfo.sysname);
	return m_TOsInfo.sysname;
}

const char* HardwareLinux::GetVersion(uint8_t& nLength) {
	nLength = (uint8_t) strlen(m_TOsInfo.version);
	return m_TOsInfo.version;
}

const char* HardwareLinux::GetCpuName(uint8_t& nLength) {
	nLength = (uint8_t) strlen(m_aCpuName);
	return m_aCpuName;
}

const char* HardwareLinux::GetSocName(uint8_t& nLength) {
	nLength = (uint8_t) strlen(m_aSocName);
	return m_aSocName;
}

uint32_t HardwareLinux::GetReleaseId(void) {
	int len = strlen(m_TOsInfo.release);
	uint32_t rev = 0;

	for (int i = 0; i < len ; i++) {
		if (isdigit(m_TOsInfo.release[i])) {
			rev *= 10;
			rev += m_TOsInfo.release[i] - '0';
		}
	}

	return rev;
}

const char* HardwareLinux::GetBoardName(uint8_t& nLength) {
	nLength = strlen(m_aBoardName);
	return m_aBoardName;
}

uint32_t HardwareLinux::GetBoardId(void) {
	return  m_nBoardId;
}

uint64_t HardwareLinux::GetUpTime(void) {
#if defined (__APPLE__)
	struct timeval boottime;
	size_t len = sizeof(boottime);
	int mib[2] = {CTL_KERN, KERN_BOOTTIME};

	if (sysctl(mib, 2, &boottime, &len, NULL, 0) < 0 ) {
		return 0;
	}

	time_t bsec = boottime.tv_sec;
	time_t csec = time(NULL);

	return difftime(csec, bsec);
#else
	struct sysinfo s_info;
	int error = sysinfo(&s_info);

	if (error != 0) {
		printf("code error = %d\n", error);
	}

	return (uint64_t) s_info.uptime;
#endif
}

bool HardwareLinux::SetTime(const struct THardwareTime& pTime) {
	return false; //TODO SetTime(const struct THardwareTime& pTime)
}

void HardwareLinux::GetTime(struct THardwareTime* pTime) {
	time_t ltime;
	struct tm *local_time;

	ltime = time(NULL);
    local_time = localtime(&ltime);

    pTime->tm_year = local_time->tm_year;
    pTime->tm_mon = local_time->tm_mon;
    pTime->tm_mday = local_time->tm_mday;
    //
    pTime->tm_hour = local_time->tm_hour;
    pTime->tm_min = local_time->tm_min;
    pTime->tm_sec = local_time->tm_sec;
}

bool HardwareLinux::Reboot(void) {
#if defined (__CYGWIN__) || defined (__APPLE__)
	return false;
#else
	if(geteuid() == 0) {
		sync();

		if (reboot(RB_AUTOBOOT) == 0) {
			return true;
		}

		perror("Call to reboot(RB_AUTOBOOT) failed.\n");
		return false;
	}

	printf("Only the superuser may call reboot(RB_AUTOBOOT).\n");
	return false;
#endif
}

bool HardwareLinux::PowerOff(void) {
#if defined (__CYGWIN__) || defined (__APPLE__)
	return false;
#else
	if(geteuid() == 0) {
		sync();

		if (reboot(RB_POWER_OFF) == 0) {
			return true;
		}

		perror("Call to reboot(RB_POWER_OFF) failed.\n");
		return false;
	}

	printf("Only the superuser may call reboot(RB_POWER_OFF).\n");
	return false;
#endif
}

float HardwareLinux::GetCoreTemperature(void) {
#if defined (__linux__)
	if (m_tBoardType == BOARD_TYPE_RASPBIAN) {
		const char cmd[] = "/opt/vc/bin/vcgencmd measure_temp| egrep \"[0-9.]{4,}\" -o";
		char buf[8];

		FILE *fp = popen(cmd, "r");

		if (fgets(buf, sizeof(buf) - 1, fp) == NULL) {
			pclose(fp);
			return (float) -1;
		}

		return (float) atof(buf);

	} else {
		const char cmd[] = "sensors | grep 'Core 0' | awk '{print $3}' | cut -c2-3";
		char buf[8];

		FILE *fp = popen(cmd, "r");

		if (fgets(buf, sizeof(buf) - 1, fp) == NULL) {
			pclose(fp);
			return (float) -1;
		}

		return (float) atof(buf);
	}
#endif

	return (float) -1;
}

float HardwareLinux::GetCoreTemperatureMax(void) {
#if defined (__linux__)
	return (float) 85; //TODO GetCoreTemperatureMax
#else
	return (float) -1;
#endif
}

void HardwareLinux::SetLed(THardwareLedStatus tLedStatus) {
#if defined (__linux__)
	if (m_tBoardType == BOARD_TYPE_RASPBIAN) {
		char *p = 0;

		switch (tLedStatus) {
			case HARDWARE_LED_OFF:
				p = (char *)RASPBIAN_LED_OFF;
				break;
			case HARDWARE_LED_ON:
				p = (char *)RASPBIAN_LED_ON;
				break;
			case HARDWARE_LED_HEARTBEAT:
				p = (char *)RASPBIAN_LED_HB;
				break;
			case HARDWARE_LED_FLASH:
				p = (char *)RASPBIAN_LED_FLASH;
				break;
			default:
				break;
		}

		if (p != 0) {
			if (system((const char*) p) == 0) {
				// Just make the compile happy
			}
		}
	}
#endif
}

bool HardwareLinux::ExecCmd(const char* pCmd, char* Result, int nResultSize) {
	FILE *fp = popen(pCmd, "r");

	if (fgets(Result, nResultSize-1, fp) == 0) {
		pclose(fp);
		return false;
	}

	size_t nLength = strlen(Result);

	if (Result[nLength-1] < ' ') {
		Result[nLength-1] = '\0';
	}

	return true;
}

uint32_t HardwareLinux::Millis(void) {
	struct timeval tv;
	gettimeofday(&tv, NULL);

#if defined (__APPLE__)
	return (tv.tv_sec * (__darwin_time_t) 1000) + (tv.tv_usec / (__darwin_suseconds_t) 1000);
#else
	return (tv.tv_sec * (__time_t) 1000) + (tv.tv_usec / (__suseconds_t) 1000);
#endif
}

bool HardwareLinux::IsButtonPressed(void) {
	return false;
}
