/**
 * @file hardware.h
 *
 */
/* Copyright (C) 2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#ifndef HARDWARE_H_
#define HARDWARE_H_

#include <stdint.h>
#include <time.h>

#ifdef __cplusplus

enum THardwareLedStatus {
	HARDWARE_LED_OFF = 0,
	HARDWARE_LED_ON,
	HARDWARE_LED_HEARTBEAT,
	HARDWARE_LED_FLASH
};

struct THardwareTime {
	int tm_sec;		///< Seconds.		[0-60]	(1 leap second)
	int tm_min;		///< Minutes.		[0-59]
	int tm_hour;	///< Hours.			[0-23]
	int tm_mday;	///< Day.		 	[1-31]
	int tm_mon;		///< Month.			[0-11]
	int tm_year;	///< Year - 1900
	int tm_wday;	///< Day of week.	[0-6]
	int tm_yday;	///< Days in year.	[0-365]
	int tm_isdst;	///< DST.			[-1/0/1]
};

class Hardware {
public:
	Hardware(void);
	virtual ~Hardware(void);

	virtual const char* GetMachine(uint8_t &nLength)=0;
	virtual const char* GetSysName(uint8_t &nLength)=0;
	virtual const char* GetVersion(uint8_t &nLength)=0;

	virtual const char* GetRelease(uint8_t &nLength)=0;
	virtual uint32_t GetReleaseId(void)=0;

	virtual const char* GetBoardName(uint8_t &nLength)=0;
	virtual uint32_t GetBoardId(void)=0;

	virtual const char* GetCpuName(uint8_t &nLength)=0;
	virtual const char* GetSocName(uint8_t &nLength)=0;

	virtual float GetCoreTemperature(void)=0;
	virtual float GetCoreTemperatureMax(void)=0;

	virtual uint64_t GetUpTime(void)=0;

	virtual void SetLed(THardwareLedStatus tLedStatus)=0;

	virtual bool Reboot(void)=0;
	virtual bool PowerOff(void)=0;

	virtual time_t GetTime(void)=0;

	virtual bool SetTime(const struct THardwareTime &pTime)=0;
	virtual void GetTime(struct THardwareTime *pTime)=0;

	virtual uint32_t Millis(void)=0;

	virtual bool IsButtonPressed(void);

// Only implemented in BAREMETAL
	virtual void WatchdogInit(void);
	virtual void WatchdogFeed(void);
	virtual void WatchdogStop(void);

public:
	inline static Hardware* Get(void) {
		return s_pThis;
	}

private:
	static Hardware *s_pThis;
};

#elif defined(BARE_METAL)
 #include "c/hardware.h"
#endif

#endif /* HARDWARE_H_ */
