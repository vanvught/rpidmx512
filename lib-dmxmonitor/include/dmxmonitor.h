/**
 * @file dmxmonitor.h
 *
 */
/* Copyright (C) 2016-2017 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#ifndef DMXMONITOR_H_
#define DMXMONITOR_H_

#include <stdint.h>
#include <stdbool.h>

#include "lightset.h"

#if defined (__linux__) || defined (__CYGWIN__) || defined(__APPLE__)
 #define DMXMONITOR_MAX_PORTS	4
#endif

class DMXMonitor: public LightSet {
public:
	DMXMonitor(void);
	~DMXMonitor(void);

	bool SetDmxStartAddress(uint16_t nDmxStartAddress);

	uint16_t GetDmxStartAddress(void);
	uint16_t GetDmxFootprint(void);

	void Start(uint8_t nPort = 0);
	void Stop(uint8_t nPort = 0);

	void SetData(uint8_t nPort, const uint8_t *, uint16_t);

#if defined (__linux__) || defined (__CYGWIN__) || defined(__APPLE__)
#else
	void Cls(void);
#endif

#if defined (__linux__) || defined (__CYGWIN__)  || defined(__APPLE__)
	void SetMaxDmxChannels(uint16_t);

private:
	void DisplayDateTime(uint8_t nPort, const char *);
#endif

private:
	void Update(void);

private:
	uint16_t m_nSlots;
#if defined (__linux__) || defined (__CYGWIN__)  || defined(__APPLE__)
	bool m_bIsStarted[DMXMONITOR_MAX_PORTS];
	uint16_t m_nDmxStartAddress;
	uint16_t m_nMaxChannels;
#else
	bool m_bIsStarted;
	uint8_t m_Data[512];
#endif
};

#endif /* DMXMONITOR_H_ */
