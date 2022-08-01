/**
 * @file hardware.h
 *
 */
/* Copyright (C) 2019-2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef RPI_HARDWARE_H_
#define RPI_HARDWARE_H_

#include <cstdint>
#include <time.h>
#include <uuid/uuid.h>

//#include "c/hardware.h"

#include "bcm2835.h"
#include "bcm2835_vc.h"
#include "bcm2835_wdog.h"

extern "C" {
uint32_t millis(void);
}

enum TSocType {
	SOC_TYPE_BCM2835, SOC_TYPE_BCM2836, SOC_TYPE_BCM2837, SOC_TYPE_UNKNOWN
};

extern "C" {
uint32_t hardware_uptime_seconds(void);
void hardware_led_set(int);
}

class Hardware {
public:
	Hardware();

	void GetUuid(uuid_t out);
	const char* GetMachine(uint8_t &nLength);
	const char* GetSysName(uint8_t &nLength);

	uint32_t GetReleaseId() {
		return static_cast<uint32_t>(bcm2835_vc_get_get_firmware_revision());
	}

	const char* GetBoardName(uint8_t &nLength);

	uint32_t GetBoardId() {
		return m_nBoardRevision;
	}

	const char* GetCpuName(uint8_t &nLength);
	const char* GetSocName(uint8_t &nLength);

	float GetCoreTemperature() {
		return static_cast<float>(bcm2835_vc_get_temperature()) / 1000;
	}

	float GetCoreTemperatureMin() {
		return -40.0f;
	}

	float GetCoreTemperatureMax() {
		return static_cast<float>(85);
	}

	void SetLed(hardware::LedStatus tLedStatus) {
		if (tLedStatus == hardware::LedStatus::OFF) {
			hardware_led_set(0);
		} else {
			hardware_led_set(1);
		}
	}

	bool Reboot();
	bool PowerOff() {
		return false;
	}

	bool SetTime(const struct tm *pTime);
	void GetTime(struct tm *pTime);

	time_t GetTime() {
		return time(nullptr);
	}

	uint32_t GetUpTime() {
		return hardware_uptime_seconds();
	}

	uint32_t Micros() {
		return BCM2835_ST->CLO;
	}

	uint32_t Millis() {
		return millis();
	}

	void WatchdogInit() {
		m_bIsWatchdog = true;
		bcm2835_watchdog_init();
	}

	void WatchdogFeed() {
		bcm2835_watchdog_feed();
	}

	void WatchdogStop() {
		m_bIsWatchdog = false;
		bcm2835_watchdog_stop();
	}

	bool IsWatchdog() {
		return m_bIsWatchdog;
	}

	const char* GetWebsiteUrl() {
		return "www.orangepi-dmx.org";
	}

	hardware::BootDevice GetBootDevice() {
		return hardware::BootDevice::MMC0;
	}

	void SoftReset() {}

public:
	 static Hardware* Get() {
		return s_pThis;
	}

private:
	uint32_t m_nBoardRevision;
	char *m_pBoardName;
	TSocType m_tSocType;
	bool m_bIsWatchdog { false };

	static Hardware *s_pThis;
};

#endif /* RPI_HARDWARE_H_ */
