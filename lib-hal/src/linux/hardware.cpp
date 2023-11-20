/**
 * @file hardware.cpp
 *
 */
/* Copyright (C) 2020-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <cstdint>
#include <cstdio>
#include <cctype>
#include <cstring>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/reboot.h>
#if defined (__APPLE__)
# include <sys/sysctl.h>
#else
# include <sys/sysinfo.h>
#endif
#include <sys/utsname.h>
#include <cassert>

#include "hal_i2c.h"
#include "hal_spi.h"

#include "hardware.h"

#if defined (DEBUG_I2C)
# include "i2cdetect.h"
#endif

#include "debug.h"

#if !defined (__APPLE__)
extern "C" {
int __attribute__((weak)) bcm2835_init(void) {
	return 0;
}
}
#endif

static char* str_find_replace(char *str, const char *find, const char *replace) {
	assert(strlen(replace) <= strlen(find));

	unsigned i, j, k, n, m;

	i = j = m = n = 0;

	while (str[i] != '\0') {
		if (str[m] == find[n]) {
			m++;
			n++;
			if (find[n] == '\0') {
				for (k = 0; replace[k] != '\0'; k++, j++) {
					str[j] = replace[k];
				}
				n = 0;
				i = m;
			}
		} else {
			str[j] = str[i];
			j++;
			i++;
			m = i;
			n = 0;
		}
	}

	for (; j < i; j++) {
		str[j] = '\0';
	}

	return str;
}

#if defined (__linux__)
static constexpr char RASPBIAN_LED_INIT[] = "echo gpio | sudo tee /sys/class/leds/led0/trigger";
static constexpr char RASPBIAN_LED_OFF[] = "echo 0 | sudo tee /sys/class/leds/led0/brightness";
static constexpr char RASPBIAN_LED_ON[] = "echo 1 | sudo tee /sys/class/leds/led0/brightness";
static constexpr char RASPBIAN_LED_HB[] = "echo heartbeat | sudo tee /sys/class/leds/led0/trigger";
static constexpr char RASPBIAN_LED_FLASH[] = "echo timer | sudo tee /sys/class/leds/led0/trigger";
#endif

static constexpr char UNKNOWN[] = "Unknown";

Hardware *Hardware::s_pThis = 0;

Hardware::Hardware():
#if defined (__linux__)
	m_boardType(Board::TYPE_LINUX)
#elif defined (__APPLE__)
	m_boardType(Board::TYPE_OSX)
#else
	m_boardType(Board::TYPE_UNKNOWN)
#endif
{
	s_pThis = this;

	memset(&m_TOsInfo, 0, sizeof(struct utsname));

	strcpy(m_aCpuName, UNKNOWN);
	strcpy(m_aBoardName, UNKNOWN);

	m_aSocName[0] = '\0';

#if defined (__linux__)
	constexpr char cmd[] = "which vcgencmd";
	char buf[16];

	FILE *fp = popen(cmd, "r");

	if (fgets(buf, sizeof(buf)-1, fp) != 0) {
		m_boardType = Board::TYPE_RASPBIAN;
		if (system(RASPBIAN_LED_INIT) == 0) {
			// Just make the compile happy
		}
	}

	if (fp != nullptr) {
		pclose(fp);
	}
#endif

	if (m_boardType != Board::TYPE_UNKNOWN) {
		uname(&m_TOsInfo);
	}

#ifndef NDEBUG
	printf("m_boardType=%d\n", static_cast<int>(m_boardType));
#endif

#if !defined (__APPLE__)
	if (m_boardType == Board::TYPE_RASPBIAN) {
		if (getuid() != 0) {
			fprintf(stderr, "Program is not started as \'root\' (sudo)\n");
			exit(-1);
		}

		if (bcm2835_init() == 0) {
			fprintf(stderr, "Function bcm2835_init() failed\n");
			exit(-1);
		}
	}
#endif

	{ // Board Name
#if defined (__APPLE__)
		constexpr char cat[] = "sysctl -n hw.model";
		ExecCmd(cat, m_aBoardName, sizeof(m_aBoardName));
#elif defined (__linux__)
		constexpr char cat[] = "cat /sys/firmware/devicetree/base/model";
		if (!ExecCmd(cat, m_aBoardName, sizeof(m_aBoardName))) {
			constexpr char cat[] = "cat /sys/class/dmi/id/board_name";
			ExecCmd(cat, m_aBoardName, sizeof(m_aBoardName));
		}
#endif
		str_find_replace(m_aBoardName, "Rev ", "V");
	}

	{ // CPU Name
#if defined (__APPLE__)
		constexpr char cmd[] = "sysctl -n machdep.cpu.brand_string";
#else
		constexpr char cmd[] = "cat /proc/cpuinfo | grep 'model name' | head -n 1 | sed 's/^[^:]*://g' |  sed 's/^[^ ]* //g'";
#endif
		ExecCmd(cmd, m_aCpuName, sizeof(m_aCpuName));
	}

	{ // SoC Name
		constexpr char cmd[] = "cat /proc/cpuinfo | grep 'Hardware' | awk '{print $3}'";
		ExecCmd(cmd, m_aSocName, sizeof(m_aSocName));
	}

	if (m_boardType == Board::TYPE_RASPBIAN) {
		char aResult[16];
		constexpr char cmd[] = "cat /proc/cpuinfo | grep 'Revision' | awk '{print $3}'";
		ExecCmd(cmd, aResult, sizeof(aResult));
		m_nBoardId = static_cast<uint32_t>(strtol(aResult, NULL, 16));
	}

	FUNC_PREFIX(i2c_begin());
	FUNC_PREFIX(spi_begin());

#if defined (DEBUG_I2C)
	I2cDetect i2cdetect;
#endif
}

const char* Hardware::GetMachine(uint8_t &nLength) {
	nLength = strlen(m_TOsInfo.machine);
	return m_TOsInfo.machine;
}

const char* Hardware::GetSysName(uint8_t& nLength) {
	nLength = strlen(m_TOsInfo.sysname);
	return m_TOsInfo.sysname;
}

const char* Hardware::GetCpuName(uint8_t& nLength) {
	nLength = strlen(m_aCpuName);
	return m_aCpuName;
}

const char* Hardware::GetSocName(uint8_t& nLength) {
	nLength = strlen(m_aSocName);
	return m_aSocName;
}

uint32_t Hardware::GetReleaseId() {
	const auto nLength = strlen(m_TOsInfo.release);
	uint32_t rev = 0;

	for (size_t i = 0; i < nLength ; i++) {
		if (isdigit(m_TOsInfo.release[i])) {
			rev *= 10;
			rev += static_cast<uint32_t>(m_TOsInfo.release[i] - '0');
		}
	}

	return rev;
}

const char* Hardware::GetBoardName(uint8_t& nLength) {
	nLength = strlen(m_aBoardName);
	return m_aBoardName;
}

uint32_t Hardware::GetUpTime() {
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

	return static_cast<uint32_t>(s_info.uptime);
#endif
}

bool Hardware::SetTime(__attribute__((unused)) const struct tm *pTime) {
	DEBUG_PRINTF("%s", asctime(pTime));
	return true;
}

void Hardware::GetTime(struct tm *pTime) {
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

bool Hardware::Reboot() {
#if defined (__APPLE__)
	return false;
#else
	if(geteuid() == 0) {
		sync();

		if (reboot(RB_AUTOBOOT) == 0) {
			return true;
		}

		perror("Call to reboot(RB_AUTOBOOT) failed.\n");
	}
	printf("Only the superuser may call reboot(RB_AUTOBOOT).\n");
#endif
	return false;
}

//void Hardware::SoftReset() {
//	if (m_argv != 0) {
//		sync();
//
//		if (m_pSoftResetHandler != 0) {
//			m_pSoftResetHandler->Run();
//		}
//
//		if (execve(m_argv[0], m_argv, NULL) == -1) {
//			perror("call to execve failed.\n");
//		}
//	}
//}

bool Hardware::PowerOff() {
#if defined (__APPLE__)
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

float Hardware::GetCoreTemperature() {
#if defined (__linux__)
	if (m_boardType == Board::TYPE_RASPBIAN) {
		const char cmd[] = "vcgencmd measure_temp| egrep \"[0-9.]{4,}\" -o";
		char aResult[8];

		ExecCmd(cmd, aResult, sizeof(aResult));

		return atof(aResult);
	} else {
		const char cmd[] = "sensors | grep 'Core 0' | awk '{print $3}' | cut -c2-3";
		char aResult[6];

		ExecCmd(cmd, aResult, sizeof(aResult));

		return atof(aResult);
	}
#endif
	return -1.0f;
}

float Hardware::GetCoreTemperatureMax() {
#if defined (__linux__)
	return 85.0f; //TODO GetCoreTemperatureMax
#else
	return -1.0f;
#endif
}

#if defined (__linux__)
 static hardware::LedStatus s_ledStatus;
#endif

void Hardware::SetLed(__attribute__((unused)) hardware::LedStatus ledStatus) {
#if defined (__linux__)
	if (m_boardType == Board::TYPE_RASPBIAN) {
		if (s_ledStatus == ledStatus) {
			return;
		}
		s_ledStatus = ledStatus;
		char *p = 0;

		switch (ledStatus) {
		case hardware::LedStatus::OFF:
			p = const_cast<char*>(RASPBIAN_LED_OFF);
			break;
		case hardware::LedStatus::ON:
			p = const_cast<char*>(RASPBIAN_LED_ON);
			break;
		case hardware::LedStatus::HEARTBEAT:
			p = const_cast<char*>(RASPBIAN_LED_HB);
			break;
		case hardware::LedStatus::FLASH:
			p = const_cast<char*>(RASPBIAN_LED_FLASH);
			break;
		default:
			break;
		}

		if (p != 0) {
			if (system(p) == 0) {
				// Just make the compile happy
			}
		}
	}
#endif
}

bool Hardware::ExecCmd(const char *pCmd, char *Result, int nResultSize) {
	FILE *fp = popen(pCmd, "r");

	if (fgets(Result, nResultSize - 1, fp) == 0) {
		pclose(fp);
		return false;
	}

	auto nLength = strlen(Result);

	if (Result[nLength - 1] < ' ') {
		Result[nLength - 1] = '\0';
	}

	pclose(fp);
	return true;
}

uint32_t Hardware::Micros() {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return static_cast<uint32_t>((tv.tv_sec * 1000000) + tv.tv_usec);
}

uint32_t Hardware::Millis() {
	struct timeval tv;
	gettimeofday(&tv, NULL);

#if defined (__APPLE__)
	return (tv.tv_sec * static_cast<__darwin_time_t>(1000) + (tv.tv_usec / static_cast<__darwin_time_t>(1000)));
#else
	return static_cast<uint32_t>(tv.tv_sec * static_cast<__time_t >(1000) + (tv.tv_usec / static_cast<__suseconds_t >(1000)));
#endif
}

static constexpr auto  UUID_STRING_LENGTH =	36;

void Hardware::GetUuid(uuid_t out) {
	char uuid_str[UUID_STRING_LENGTH + 2];

#if defined (__APPLE__)
	constexpr char cmd[] = "sysctl -n kern.uuid";
#else
	constexpr char cmd[] = "cat /etc/machine-id";
#endif
	ExecCmd(cmd, uuid_str, sizeof(uuid_str));

#if defined (__APPLE__)
#else
	for (uint32_t i = 13; i > 0; i--) {
		uuid_str[36 - 13 + i] = uuid_str[36 - 13 + i - 4];
	}

	for (uint32_t i = 5; i > 0; i--) {
		uuid_str[23 - 5 + i] = uuid_str[23 - 5 + i - 3];
	}

	for (uint32_t i = 5; i > 0; i--) {
		uuid_str[18 - 5 + i] = uuid_str[18 - 5 + i - 2];
	}

	for (uint32_t i = 5; i > 0; i--) {
		uuid_str[13 - 5 + i] = uuid_str[13 - 5 + i - 1];
	}

	uuid_str[23] = '-';
	uuid_str[18] = '-';
	uuid_str[13] = '-';
	uuid_str[8] = '-';
#endif

	uuid_parse(uuid_str, out);
}

void Hardware::Print() {
	char uuid_str[UUID_STRING_LENGTH + 1];
	uuid_str[UUID_STRING_LENGTH] = '\0';

	uuid_t out;
	GetUuid(out);

	uuid_unparse(out, uuid_str);

	printf("CPU  : %s\n", m_aCpuName);
	printf("SoC  : %s\n", m_aSocName);
	printf("Board: %s\n", m_aBoardName);
	printf("UUID : %s\n", uuid_str);
}
