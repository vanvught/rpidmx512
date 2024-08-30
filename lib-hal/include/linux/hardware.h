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

namespace hardware {
enum class LedStatus {
	OFF, ON, HEARTBEAT, FLASH
};
}  // namespace hardware

class Hardware {
public:
	Hardware();

	uint32_t GetReleaseId();

	void GetUuid(uuid_t out) {
		memcpy(out, m_uuid, sizeof(uuid_t));
	}

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

	struct Timer {
	    uint32_t nExpireTime;
	    uint32_t nIntervalMillis;
	    int32_t nId;
	    hal::TimerCallback callback;
	};

	int32_t SoftwareTimerAdd(const uint32_t nIntervalMillis, const hal::TimerCallback callback) {
	    if (m_nTimersCount >= hal::SOFTWARE_TIMERS_MAX) {
#ifdef NDEBUG
            fprintf(stderr, "SoftwareTimerAdd\n");
#endif
	        return -1;
	    }

	    const auto nCurrentTime = Hardware::Millis();

	    Timer newTimer = {
	        .nExpireTime = nCurrentTime + nIntervalMillis,
	        .nIntervalMillis = nIntervalMillis,
			.nId = m_nNextId++,
	        .callback = callback,
	    };

	    m_Timers[m_nTimersCount++] = newTimer;

	    return newTimer.nId;
	}

    bool SoftwareTimerDelete(int32_t& nId) {
        for (uint32_t i = 0; i < m_nTimersCount; ++i) {
            if (m_Timers[i].nId == nId) {
                for (uint32_t j = i; j < m_nTimersCount - 1; ++j) {
                    m_Timers[j] = m_Timers[j + 1];
                }
                --m_nTimersCount;
                nId = -1;
                return true;
            }
        }

        return false;
    }

    bool SoftwareTimerChange(const int32_t nId, const uint32_t nIntervalMillis) {
        for (uint32_t i = 0; i < m_nTimersCount; ++i) {
            if (m_Timers[i].nId == nId) {
            	m_Timers[i].nExpireTime = Hardware::Millis() + nIntervalMillis;
            	m_Timers[i].nIntervalMillis = nIntervalMillis;
            	return true;
            }
        }

        return false;
    }

	void Run() {
	    const auto nCurrentTime = Hardware::Get()->Millis();

	    for (uint32_t i = 0; i < m_nTimersCount; i++) {
	        if (m_Timers[i].nExpireTime <= nCurrentTime) {
	        	m_Timers[i].callback();
	            m_Timers[i].nExpireTime = nCurrentTime + m_Timers[i].nIntervalMillis;
	        }
	    }
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

	Timer m_Timers[hal::SOFTWARE_TIMERS_MAX];
	uint32_t m_nTimersCount { 0 };
	int32_t m_nNextId { 0 };

	static Hardware *s_pThis;
};

#endif /* LINUX_HARDWARE_H_ */
