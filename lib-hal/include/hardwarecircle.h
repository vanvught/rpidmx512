/**
 * @file hardwarecircle.h
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

#ifndef HARDWARECIRCLE_H_
#define HARDWARECIRCLE_H_

#include <stdint.h>

#include "circle/timer.h"

#include "hardware.h"

class HardwareCircle: public Hardware {
public:
	HardwareCircle(void);
	~HardwareCircle(void);

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

	uint64_t GetUpTime(void);

	inline time_t GetTime(void) {
		return CTimer::Get()->GetTime();
	}

	bool SetTime(const struct THardwareTime &pTime);
	void GetTime(struct THardwareTime *pTime);

	uint32_t Millis(void);

	bool IsButtonPressed(void);

private:
};

#endif /* HARDWARECIRCLE_H_ */
