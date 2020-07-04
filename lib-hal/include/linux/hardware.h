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

#ifndef LINUX_HARDWARE_H_
#define LINUX_HARDWARE_H_

#include <time.h>
#include <stdint.h>
#include <sys/utsname.h>

#include "hardware.h"

#include "reboothandler.h"

class Hardware {
public:
	Hardware();

	const char *GetMachine(uint8_t &nLength);
	const char *GetSysName(uint8_t &nLength);
	const char *GetBoardName(uint8_t &nLength);
	const char *GetCpuName(uint8_t &nLength);
	const char *GetSocName(uint8_t &nLength);

	uint32_t GetReleaseId();

	uint32_t GetBoardId() {
		return m_nBoardId;
	}

	float GetCoreTemperature();
	float GetCoreTemperatureMax();

	void SetLed(THardwareLedStatus tLedStatus);

	bool Reboot();

	void SetRebootHandler(RebootHandler *pRebootHandler) {
		m_pRebootHandler = pRebootHandler;
	}

	bool PowerOff();

	uint32_t GetUpTime();

	time_t GetTime() {
		return time(nullptr);
	}

	void SetSysTime(time_t nTime);

	bool SetTime(const struct tm *pTime);
	void GetTime(struct tm *pTime);

	uint32_t Micros();
	uint32_t Millis();

	bool IsWatchdog() { return false;}
	void WatchdogInit() { } // Not implemented
	void WatchdogFeed() { } // Not implemented
	void WatchdogStop() { } // Not implemented

	const char *GetWebsiteUrl() {
		return "www.orangepi-dmx.org";
	}

	TBootDevice GetBootDevice() {
#if defined (RASPPI)
		return BOOT_DEVICE_MMC0;
#else
		return BOOT_DEVICE_HDD;
#endif
	}

	 static Hardware *Get() {
		return s_pThis;
	}

private:
	bool ExecCmd(const char* pCmd, char *Result, int nResultSize);

private:
	RebootHandler *m_pRebootHandler = nullptr;
	enum TBoardType {
		BOARD_TYPE_LINUX,
		BOARD_TYPE_CYGWIN,
		BOARD_TYPE_RASPBIAN,
		BOARD_TYPE_OSX,
		BOARD_TYPE_UNKNOWN
	};

	TBoardType m_tBoardType;

	struct utsname m_TOsInfo;

	char m_aCpuName[64];
	char m_aSocName[64];
	char m_aBoardName[64];

	uint32_t m_nBoardId;

	static Hardware *s_pThis;
};

#endif /* LINUX_HARDWARE_H_ */
