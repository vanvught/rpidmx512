/**
 * @file hardware.h
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

#ifndef H3_HARDWARE_H_
#define H3_HARDWARE_H_

#include <stdint.h>
#include <time.h>

#include "hardware.h"

#include "hwclock.h"

#include "c/hardware.h"

#include "h3.h"
#include "h3_watchdog.h"
#include "h3_thermal.h"

#include "reboothandler.h"

class Hardware {
public:
	Hardware();

	const char *GetMachine(uint8_t &nLength);
	const char *GetSysName(uint8_t &nLength);
	const char *GetBoardName(uint8_t &nLength);
	const char *GetCpuName(uint8_t &nLength);
	const char *GetSocName(uint8_t &nLength);

	uint32_t GetReleaseId() const {
		return 0;	// TODO U-Boot version
	}

	uint32_t GetBoardId() {
	#if defined(ORANGE_PI)
		return 0;
	#elif defined(ORANGE_PI_ONE)
		return 1;
	#else
	 #error Platform not supported
	#endif
	}

	float GetCoreTemperature() {
		return static_cast<float>(h3_thermal_gettemp());
	}

	float GetCoreTemperatureMax() {
		return static_cast<float>(h3_thermal_getalarm());
	}

	void SetLed(THardwareLedStatus tLedStatus) {
		if (tLedStatus == HARDWARE_LED_OFF) {
			hardware_led_set(0);
		} else {
			hardware_led_set(1);
		}
	}

	bool Reboot();

	void SetRebootHandler(RebootHandler *pRebootHandler) {
		m_pRebootHandler = pRebootHandler;
	}

	bool PowerOff() const {
		return false;
	}

	bool SetTime(const struct tm *pTime);
	void GetTime(struct tm *pTime);

	time_t GetTime() {
		return time(0);
	}

	uint32_t GetUpTime() {
		return hardware_uptime_seconds();
	}

	uint32_t Micros() {
		return H3_TIMER->AVS_CNT1;
	}

	uint32_t Millis() {
		return H3_TIMER->AVS_CNT0;
	}

	void WatchdogInit() {
		m_bIsWatchdog = true;
		h3_watchdog_enable();
	}

	void WatchdogFeed() {
		h3_watchdog_restart();
	}

	void WatchdogStop() {
		m_bIsWatchdog = false;
		h3_watchdog_disable();
	}

	bool IsWatchdog() const {
		return m_bIsWatchdog;
	}

	TBootDevice GetBootDevice() const {
		return static_cast<TBootDevice>(h3_get_boot_device());
	}

	const char *GetWebsiteUrl() const {
		return "www.orangepi-dmx.org";
	}

	static Hardware *Get() {
		return s_pThis;
	}

private:
#if !defined(DISABLE_RTC)
	HwClock m_HwClock;
#endif
	RebootHandler *m_pRebootHandler{nullptr};
	bool m_bIsWatchdog{false};

	static Hardware *s_pThis;
};

#endif /* H3_HARDWARE_H_ */
