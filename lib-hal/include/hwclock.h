
/**
 * @file hwclock.h
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

#ifndef HWCLOCK_H_
#define HWCLOCK_H_

#include <stdint.h>
#include <sys/time.h>

namespace rtc {
enum {
	MCP7941X,
	DS3231
};
}  // namespace rtc

struct rtc_time {
    int tm_sec;
    int tm_min;
    int tm_hour;
    int tm_mday;
    int tm_mon;
    int tm_year;
    int tm_wday;     /* unused */
    int tm_yday;     /* unused */
    int tm_isdst;    /* unused */
};

class HwClock {
public:
	HwClock();

	void HcToSys(); // Set the System Clock from the Hardware Clock
	void SysToHc(); // Set the Hardware Clock from the System Clock

	bool Set(const struct rtc_time *pRtcTime);
	bool Get(struct rtc_time *pRtcTime) {
		return RtcGet(pRtcTime);
	}

	bool IsConnected() const {
		return m_bIsConnected;
	}

	void Run(bool bDoRun);

	void Print();

	static HwClock *Get() {
		return s_pThis;
	}

private:
	void RtcProbe();
	bool RtcSet(const struct rtc_time *pRtcTime);
	bool RtcGet(struct rtc_time *pRtcTime);

private:
	bool m_bIsConnected{false};
	uint32_t m_nType;
	uint8_t m_nAddress;
	uint32_t m_nSetDelayMicros;
	uint32_t m_nLastHcToSysMillis;

	static HwClock *s_pThis;
};

#endif /* HWCLOCK_H_ */
