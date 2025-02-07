/**
 * @file hardware.h
 *
 */
/* Copyright (C) 2020-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include <cstdint>
#include <cstring>
#include <time.h>
#include <uuid/uuid.h>

#if !defined(DISABLE_RTC)
# include "hwclock.h"
#endif

#include "h3.h"
#include "h3_watchdog.h"
#include "h3_thermal.h"

#include "superloop/softwaretimers.h"

#include "debug.h"

#if defined (DEBUG_STACK)
 void stack_debug_run();
#endif

uint32_t hardware_uptime_seconds();
void hardware_led_set(int);
void console_error(const char *);

class Hardware {
public:
	Hardware();

	uint32_t GetReleaseId() const {
		return 0;
	}

	const char *GetMachine(uint8_t &nLength);
	const char *GetSysName(uint8_t &nLength);
	const char *GetBoardName(uint8_t &nLength);
	const char *GetCpuName(uint8_t &nLength);
	const char *GetSocName(uint8_t &nLength);

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

	float GetCoreTemperatureMin() {
		return -40.0f;
	}

	float GetCoreTemperatureMax() {
		return static_cast<float>(h3_thermal_getalarm());
	}

	bool Reboot();

	bool PowerOff() const {
		return false;
	}

	bool SetTime([[maybe_unused]] const struct tm *pTime) {
#if !defined(DISABLE_RTC)
		m_HwClock.Set(pTime);
		return true;
#else
		return false;
#endif
	}
	
#if !defined(DISABLE_RTC)
	bool SetAlarm(const struct tm *pTime) {
		const auto b = m_HwClock.AlarmSet(pTime);
		return b;
	}

	void GetAlarm(struct tm *pTime) {
		m_HwClock.AlarmGet(pTime);
	}
#endif

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

	hardware::BootDevice GetBootDevice() const {
		return static_cast<hardware::BootDevice>(h3_get_boot_device());
	}

	const char *GetWebsiteUrl() const {
		return "www.orangepi-dmx.org";
	}

	void SetModeWithLock(hardware::ledblink::Mode mode, bool doLock);
	void SetMode(hardware::ledblink::Mode mode);
	hardware::ledblink::Mode GetMode() const {
		return m_Mode;
	}

	void Run() {
		SoftwareTimerRun();
#if defined (DEBUG_STACK)
		stack_debug_run();
#endif
	}

	static Hardware *Get() {
		return s_pThis;
	}

private:
	static void ledblink([[maybe_unused]] TimerHandle_t nHandle) {
		m_nToggleLed ^= 0x1;
		hardware_led_set(m_nToggleLed);
	}

	void SetFrequency(const uint32_t nFreqHz) {
		if (nFreqHz == 0) {
			SoftwareTimerDelete(m_nTimerId);
			hardware_led_set(0);
			return;
		}

		if (nFreqHz == 255) {
			SoftwareTimerDelete(m_nTimerId);
			hardware_led_set(1);
			return;
		}

		if (m_nTimerId < 0) {
			m_nTimerId = SoftwareTimerAdd((1000U / nFreqHz),ledblink);
			DEBUG_PRINTF("m_nTimerId=%d", m_nTimerId);
			return;
		}

		DEBUG_PRINTF("m_nTimerId=%d", m_nTimerId);
		SoftwareTimerChange(m_nTimerId, (1000U / nFreqHz));
	}

private:
#if !defined(DISABLE_RTC)
	HwClock m_HwClock;
#endif
	bool m_bIsWatchdog { false };

	hardware::ledblink::Mode m_Mode { hardware::ledblink::Mode::UNKNOWN };
	bool m_doLock { false };
	int32_t m_nTimerId { -1 };

	static inline int32_t m_nToggleLed { 0 };
	static inline Hardware *s_pThis;
};

#endif /* H3_HARDWARE_H_ */
