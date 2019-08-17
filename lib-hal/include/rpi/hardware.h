/**
 * @file hardware.h
 *
 */
/* Copyright (C) 2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include <stdint.h>
#include <time.h>

#include "hardware.h"

#include "c/hardware.h"
#include "c/sys_time.h"

#include "bcm2835.h"
#include "bcm2835_vc.h"
#include "bcm2835_wdog.h"

enum TSocType {
	SOC_TYPE_BCM2835, SOC_TYPE_BCM2836, SOC_TYPE_BCM2837, SOC_TYPE_UNKNOWN
};

class Hardware {
public:
	Hardware(void);
	~Hardware(void);

	const char* GetMachine(uint8_t &nLength);
	const char* GetSysName(uint8_t &nLength);

	uint32_t GetReleaseId(void) {
		return (uint32_t) bcm2835_vc_get_get_firmware_revision();
	}

	const char* GetBoardName(uint8_t &nLength);

	uint32_t GetBoardId(void) {
		return m_nBoardId;
	}

	const char* GetCpuName(uint8_t &nLength);
	const char* GetSocName(uint8_t &nLength);

	float GetCoreTemperature(void) {
		return (float) bcm2835_vc_get_temperature() / 1000;
	}

	float GetCoreTemperatureMax(void) {
		return (float) 85;
	}

	void SetLed(THardwareLedStatus tLedStatus) {
		if (tLedStatus == HARDWARE_LED_OFF) {
			hardware_led_set(0);
		} else {
			hardware_led_set(1);
		}
	}

	bool Reboot(void);
	bool PowerOff(void) {
		return false;
	}

	bool SetTime(const struct THardwareTime &pTime);
	void GetTime(struct THardwareTime *pTime);

	time_t GetTime(void) {
		return time(0);
	}

	uint64_t GetUpTime(void) {
		return hardware_uptime_seconds();
	}

	uint32_t Micros(void) {
		return BCM2835_ST->CLO;
	}

	uint32_t Millis(void) {
		return millis();
	}

	void WatchdogInit(void) {
		bcm2835_watchdog_init();
	}

	void WatchdogFeed(void) {
		bcm2835_watchdog_feed();
	}

	void WatchdogStop(void) {
		bcm2835_watchdog_stop();
	}

	const char* GetWebsiteUrl(void) {
		return "www.orangepi-dmx.org";
	}

	TBootDevice GetBootDevice(void) {
		return BOOT_DEVICE_MMC0;
	}

public:
	 static Hardware* Get(void) {
		return s_pThis;
	}

private:
	uint32_t m_nBoardId;
	uint32_t m_nBoardRevision;
	TSocType m_tSocType;

	static Hardware *s_pThis;
};

#endif /* RPI_HARDWARE_H_ */
