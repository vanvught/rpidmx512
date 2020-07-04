/**
 * @file hardware.c
 *
 */
/* Copyright (C) 2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <cassert>

#include "hardware.h"

#include "reboothandler.h"

#include "debug.h"

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
# if defined (RASPPI)
 extern "C" {
  int bcm2835_init();
  int bcm2835_i2c_begin();
  int bcm2835_spi_begin();
 }
# endif
static constexpr char RASPBIAN_LED_INIT[] = "echo gpio | sudo tee /sys/class/leds/led0/trigger";
static constexpr char RASPBIAN_LED_OFF[] = "echo 0 | sudo tee /sys/class/leds/led0/brightness";
static constexpr char RASPBIAN_LED_ON[] = "echo 1 | sudo tee /sys/class/leds/led0/brightness";
static constexpr char RASPBIAN_LED_HB[] = "echo heartbeat | sudo tee /sys/class/leds/led0/trigger";
static constexpr char RASPBIAN_LED_FLASH[] = "echo timer | sudo tee /sys/class/leds/led0/trigger";
#endif

static constexpr char UNKNOWN[] = "Unknown";

Hardware *Hardware::s_pThis = 0;

Hardware::Hardware():
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
	s_pThis = this;

	memset(&m_TOsInfo, 0, sizeof(struct utsname));

	strcpy(m_aCpuName, UNKNOWN);
	strcpy(m_aBoardName, UNKNOWN);

	m_aSocName[0] = '\0';

#if defined (__linux__)
	constexpr char cmd[] = "which /opt/vc/bin/vcgencmd";
	char buf[16];

	FILE *fp = popen(cmd, "r");

	if (fgets(buf, sizeof(buf)-1, fp) != 0) {
		m_tBoardType = BOARD_TYPE_RASPBIAN;
		if (system(RASPBIAN_LED_INIT) == 0) {
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
	printf("m_tBoardType=%d\n", static_cast<int>(m_tBoardType));
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

	if (m_tBoardType == BOARD_TYPE_RASPBIAN) {
		char aResult[16];
		constexpr char cmd[] = "cat /proc/cpuinfo | grep 'Revision' | awk '{print $3}'";
		ExecCmd(cmd, aResult, sizeof(aResult));
		m_nBoardId = static_cast<uint32_t>(strtol(aResult, NULL, 16));
#if defined (RASPPI)
		if (getuid() == 0) {
			if (bcm2835_init() == 0) {
				fprintf(stderr, "bcm2835_init() failed\n");
			}
			if (bcm2835_i2c_begin() == 0) {
				fprintf(stderr, "bcm2835_i2c_begin() failed\n");
			}
			if (bcm2835_spi_begin() == 0) {
				fprintf(stderr, "bcm2835_spi_begin() failed\n");
			}
		}
#endif
	}
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
	const size_t len = strlen(m_TOsInfo.release);
	uint32_t rev = 0;

	for (size_t i = 0; i < len ; i++) {
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

void Hardware::SetSysTime(__attribute__((unused)) time_t nTime) {
	DEBUG_PRINTF("%s", asctime(localtime(&nTime)));
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
#if defined (__CYGWIN__) || defined (__APPLE__)
	return false;
#else
	if(geteuid() == 0) {
		if (m_pRebootHandler != 0) {
			m_pRebootHandler->Run();
		}

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

float Hardware::GetCoreTemperature() {
#if defined (__linux__)
	if (m_tBoardType == BOARD_TYPE_RASPBIAN) {
		const char cmd[] = "/opt/vc/bin/vcgencmd measure_temp| egrep \"[0-9.]{4,}\" -o";
		char buf[8];

		FILE *fp = popen(cmd, "r");

		if (fgets(buf, sizeof(buf) - 1, fp) == NULL) {
			pclose(fp);
			return -1;
		}

		return atof(buf);

	} else {
		const char cmd[] = "sensors | grep 'Core 0' | awk '{print $3}' | cut -c2-3";
		char buf[8];

		FILE *fp = popen(cmd, "r");

		if (fgets(buf, sizeof(buf) - 1, fp) == NULL) {
			pclose(fp);
			return -1;
		}

		return atof(buf);
	}
#endif
	return -1;
}

float Hardware::GetCoreTemperatureMax() {
#if defined (__linux__)
	return 85; //TODO GetCoreTemperatureMax
#else
	return -1;
#endif
}

void Hardware::SetLed(__attribute__((unused)) THardwareLedStatus tLedStatus) {
#if defined (__linux__)
	if (m_tBoardType == BOARD_TYPE_RASPBIAN) {
		char *p = 0;

		switch (tLedStatus) {
		case HARDWARE_LED_OFF:
			p = const_cast<char*>(RASPBIAN_LED_OFF);
			break;
		case HARDWARE_LED_ON:
			p = const_cast<char*>(RASPBIAN_LED_ON);
			break;
		case HARDWARE_LED_HEARTBEAT:
			p = const_cast<char*>(RASPBIAN_LED_HB);
			break;
		case HARDWARE_LED_FLASH:
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

	size_t nLength = strlen(Result);

	if (Result[nLength - 1] < ' ') {
		Result[nLength - 1] = '\0';
	}

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
