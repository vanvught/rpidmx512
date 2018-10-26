/**
 * @file hardwarebaremetal.h
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

#ifndef HARDWAREBAREMETAL_H_
#define HARDWAREBAREMETAL_H_

#include <stdint.h>

#include "hardware.h"

#ifdef H3
enum TSocType {
	SOC_TYPE_H2_PLUS, SOC_TYPE_H3, SOC_TYPE_UNKNOWN
};
#else
enum TSocType {
	SOC_TYPE_BCM2835, SOC_TYPE_BCM2836, SOC_TYPE_BCM2837, SOC_TYPE_UNKNOWN
};
#endif

enum TBootDevice {
	BOOT_DEVICE_UNK,
	BOOT_DEVICE_FEL,	// H3 Only
	BOOT_DEVICE_MMC0,
	BOOT_DEVICE_SPI		// H3 Only
};

class HardwareBaremetal: public Hardware {
public:
	HardwareBaremetal(void);
	~HardwareBaremetal(void);

	const char* GetMachine(uint8_t &nLength);
	const char* GetSysName(uint8_t &nLength);
	const char* GetVersion(uint8_t &nLength);

	const char* GetRelease(uint8_t &nLength);
	uint32_t GetReleaseId(void);

	const char* GetBoardName(uint8_t &nLength);
	uint32_t GetBoardId(void);

	const char* GetCpuName(uint8_t &nLength);
	const char* GetSocName(uint8_t &nLength);

	float GetCoreTemperature(void);
	float GetCoreTemperatureMax(void);

	void SetLed(THardwareLedStatus tLedStatus);

	bool Reboot(void);
	bool PowerOff(void);

	inline time_t GetTime(void) {
		return time(0);
	}

	uint64_t GetUpTime(void);

	bool SetTime(const struct THardwareTime &pTime);
	void GetTime(struct THardwareTime *pTime);

	uint32_t Millis(void);

public:	// Only available in BAREMETAL
	void WatchdogInit(void);
	void WatchdogFeed(void);
	void WatchdogStop(void);

	uint32_t Micros(void);

	bool IsButtonPressed(void);

#ifdef H3
	TBootDevice GetBootDevice(void);
#else
	TBootDevice GetBootDevice(void) { return BOOT_DEVICE_MMC0; }
#endif

private:
	int32_t m_nBoardRevision;
	TSocType m_tSocType;
	uint32_t m_nBoardId;
};

#endif /* HARDWAREBAREMETAL_H_ */
