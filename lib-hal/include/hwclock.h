
/**
 * @file hwclock.h
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

#ifndef HWCLOCK_H_
#define HWCLOCK_H_

#include <cstdint>
#include <time.h>
#include <sys/time.h>

namespace rtc {
enum class Type : uint8_t {
	MCP7941X, DS3231, PCF8563, SOC_INTERNAL, UNKNOWN
};
}  // namespace rtc

class HwClock {
public:
	HwClock();
	void RtcProbe();

	void HcToSys(); // Set the System Clock from the Hardware Clock
	void SysToHc(); // Set the Hardware Clock from the System Clock

	bool Set(const struct tm *pTime);
	bool Get(struct tm *pTime) {
		return RtcGet(pTime);
	}

	bool AlarmSet(const struct tm *pTime) {
		return RtcSetAlarm(pTime);
	}
	bool AlarmGet(struct tm *pTime) {
		return RtcGetAlarm(pTime);
	}
	void AlarmEnable(const bool bEnable) {
		m_bRtcAlarmEnabled = bEnable;
	}
	bool AlarmIsEnabled() const {
		return m_bRtcAlarmEnabled;
	}

	bool IsConnected() const {
		return m_bIsConnected;
	}

	void Run(const bool bDoRun) {
		if (!bDoRun || !m_bIsConnected) {
			return;
		}
		Process();
	}

	void Print();

	static HwClock *Get() {
		return s_pThis;
	}

private:
	void Process();
	bool RtcSet(const struct tm *pime);
	bool RtcGet(struct tm *pTime);
	bool RtcSetAlarm(const struct tm *pTime);
	bool RtcGetAlarm(struct tm *pTime);
	int MCP794xxAlarmWeekday(struct tm *pTime);
	void PCF8563GetAlarmMode();
	void PCF8563SetAlarmMode();

private:
	uint32_t m_nSetDelayMicros { 0 };
	uint32_t m_nLastHcToSysMillis { 0 };
	uint8_t m_nAddress { 0 };
	rtc::Type m_Type { rtc::Type::UNKNOWN };
	bool m_bIsConnected { false };
	bool m_bRtcAlarmEnabled { false };
	bool m_bRtcAlarmPending { false };

	static HwClock *s_pThis;
};

#endif /* HWCLOCK_H_ */
