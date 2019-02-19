/**
 * @file dmxmonitor.h
 *
 */
/* Copyright (C) 2016-2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

enum TDMXMonitorFormat {
	DMX_MONITOR_FORMAT_HEX,
	DMX_MONITOR_FORMAT_PCT,
	DMX_MONITOR_FORMAT_DEC,
};

class DMXMonitor: public LightSet {
public:
	DMXMonitor(void);
	~DMXMonitor(void);

	void SetFormat(TDMXMonitorFormat tFormat = DMX_MONITOR_FORMAT_HEX);
	TDMXMonitorFormat GetFormat(void) {
		return m_tFormat;
	}

	bool SetDmxStartAddress(uint16_t nDmxStartAddress);
	uint16_t GetDmxStartAddress(void);

	uint16_t GetDmxFootprint(void);

	void Start(uint8_t nPortId);
	void Stop(uint8_t nPortId);

	void SetData(uint8_t nPortId, const uint8_t *pData, uint16_t nLength);

#if defined (__linux__) || defined (__CYGWIN__) || defined(__APPLE__)
#else
	void Cls(void);
#endif

#if defined (__linux__) || defined (__CYGWIN__) || defined(__APPLE__)
	void SetMaxDmxChannels(uint16_t nMaxChannels);

private:
	void DisplayDateTime(uint8_t nPortId, const char *pString);
#endif

private:
	void Update(void);

private:
	TDMXMonitorFormat m_tFormat;
	uint16_t m_nSlots;
#if defined (__linux__) || defined (__CYGWIN__) || defined(__APPLE__)
	bool m_bIsStarted[DMXMONITOR_MAX_PORTS];
	uint16_t m_nDmxStartAddress;
	uint16_t m_nMaxChannels;
#else
	bool m_bIsStarted;
	alignas(uint32_t) uint8_t m_Data[512];
#endif
};

#endif /* DMXMONITOR_H_ */
