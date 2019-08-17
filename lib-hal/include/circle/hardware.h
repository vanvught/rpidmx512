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

#ifndef CIRCLE_HARDWARE_H_
#define CIRCLE_HARDWARE_H_

#include <stdint.h>
#include <time.h>

#include "circle/actled.h"
#include "circle/bcmpropertytags.h"
#include "circle/machineinfo.h"
#include "circle/timer.h"
#include "circle/util.h"
#include "circle/version.h"

#include "hardware.h"

class Hardware {
public:
	Hardware(void);
	~Hardware(void);

	const char* GetMachine(uint8_t &nLength);
	const char* GetSysName(uint8_t &nLength);

	uint32_t GetReleaseId(void) {
		return CIRCLE_MAJOR_VERSION;
	}

	const char* GetBoardName(uint8_t &nLength);
	uint32_t GetBoardId(void);


	const char* GetCpuName(uint8_t &nLength);
	const char* GetSocName(uint8_t &nLength);

	float GetCoreTemperature(void);

	float GetCoreTemperatureMax(void) {
		return (float) 85;
	}

	void SetLed(THardwareLedStatus tLedStatus) {
		if (tLedStatus == HARDWARE_LED_OFF) {
			CActLED::Get()->Off();
		} else {
			CActLED::Get()->On();
		}
	}

	bool Reboot(void) {
		return false;
	}

	bool PowerOff(void) {
		return false;	// Not implemented
	}

	uint64_t GetUpTime(void) {
		return CTimer::Get()->GetUptime();
	}

	time_t GetTime(void) {
		return CTimer::Get()->GetTime();
	}

	bool SetTime(const struct THardwareTime &pTime) {
		return false; // Not needed
	}

	void GetTime(struct THardwareTime *pTime)  { } // Not implemented

	uint32_t Micros(void) {
		return 0;	// Not needed
	}

	uint32_t Millis(void) {
		return 0;	// Not needed
	}

	void WatchdogInit(void) { } // Not implemented
	void WatchdogFeed(void) { } // Not implemented
	void WatchdogStop(void) { } // Not implemented

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
	static Hardware *s_pThis;
};

#endif /* CIRCLE_HARDWARE_H_ */
