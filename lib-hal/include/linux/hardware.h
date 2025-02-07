/**
 * @file hardware.h
 *
 */
/* Copyright (C) 2020-2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef LINUX_HARDWARE_H_
#define LINUX_HARDWARE_H_

#if defined(__linux__) || defined (__APPLE__)
#else
# error
#endif

#include <cstdint>
#include <cstring>
#include <cstdio>
#include <time.h>
#include <uuid/uuid.h>
#include <sys/utsname.h>

#if !defined(DISABLE_RTC)
# include "hwclock.h"
#endif

#include "linux/hal_api.h"

#include "superloop/softwaretimers.h"

namespace hardware {
enum class LedStatus {
	OFF, ON, HEARTBEAT, FLASH
};
}  // namespace hardware

class Hardware {
public:
	Hardware();

	uint32_t GetReleaseId();

	void Print();

	const char *GetMachine(uint8_t &nLength);
	const char *GetSysName(uint8_t &nLength);
	const char *GetBoardName(uint8_t &nLength);
	const char *GetCpuName(uint8_t &nLength);
	const char *GetSocName(uint8_t &nLength);

	uint32_t GetBoardId() {
		return m_nBoardId;
	}

	float GetCoreTemperature();
	float GetCoreTemperatureMin() {
		return -40.0f;
	}
	float GetCoreTemperatureMax();

	void SetLed(hardware::LedStatus tLedStatus);

	bool Reboot();

	bool PowerOff();

	uint32_t GetUpTime();

	bool SetTime(const struct tm *pTime);

	bool SetAlarm(const struct tm *pTime);
	void GetAlarm(struct tm *pTime);

	uint32_t Micros();
	uint32_t Millis();

	bool IsWatchdog() { return false;}
	void WatchdogInit() { } // Not implemented
	void WatchdogFeed() { } // Not implemented
	void WatchdogStop() { } // Not implemented

	const char *GetWebsiteUrl() {
		return "www.gd32-dmx.org";
	}

	hardware::BootDevice GetBootDevice() {
#if defined (RASPPI)
		return hardware::BootDevice::MMC0;
#else
		return hardware::BootDevice::HDD;
#endif
	}

	void SetModeWithLock(hardware::ledblink::Mode mode, bool doLock);

	void SetMode(hardware::ledblink::Mode mode);
	hardware::ledblink::Mode GetMode() {
		return m_Mode;
	}

	void Run() {
		SoftwareTimerRun();
	}

	 static Hardware *Get() {
		return s_pThis;
	}

private:
	void SetFrequency(uint32_t nFreqHz) {
		if (nFreqHz == 0) {
			SetLed(hardware::LedStatus::OFF);
		} else if (nFreqHz > 20) {
			SetLed(hardware::LedStatus::ON);
		} else {
			if (nFreqHz > 1) {
				SetLed(hardware::LedStatus::HEARTBEAT);
			} else {
				SetLed(hardware::LedStatus::FLASH);
			}
		}
	}

private:
#if !defined(DISABLE_RTC)
	HwClock m_HwClock;
#endif
	uuid_t m_uuid;

	enum class Board {
		TYPE_LINUX,
		TYPE_RASPBIAN,
		TYPE_OSX,
		TYPE_UNKNOWN
	};

	Board m_boardType;

	struct utsname m_TOsInfo;

	char m_aCpuName[64];
	char m_aSocName[64];
	char m_aBoardName[64];

	uint32_t m_nBoardId;

	hardware::ledblink::Mode m_Mode { hardware::ledblink::Mode::UNKNOWN };
	bool m_doLock { false };

	static Hardware *s_pThis;
};

#endif /* LINUX_HARDWARE_H_ */
