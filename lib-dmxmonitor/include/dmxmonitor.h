/**
 * @file dmxmonitor.h
 *
 */
/* Copyright (C) 2016-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "lightset.h"

enum class DMXMonitorFormat {
	DMX_MONITOR_FORMAT_HEX,
	DMX_MONITOR_FORMAT_PCT,
	DMX_MONITOR_FORMAT_DEC,
};

class DMXMonitor: public LightSet {
public:
	DMXMonitor();
	~DMXMonitor() override;

	void SetFormat(DMXMonitorFormat tFormat = DMXMonitorFormat::DMX_MONITOR_FORMAT_HEX) {
		m_tFormat = tFormat;
	}
	DMXMonitorFormat GetFormat() const {
		return m_tFormat;
	}

	bool SetDmxStartAddress(uint16_t nDmxStartAddress) override;
	uint16_t GetDmxStartAddress() override;

	uint16_t GetDmxFootprint() override;

	void Start(uint8_t nPortId) override;
	void Stop(uint8_t nPortId) override;

	void SetData(uint8_t nPortId, const uint8_t *pData, uint16_t nLength) override;

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
	void Update();

private:
	DMXMonitorFormat m_tFormat = DMXMonitorFormat::DMX_MONITOR_FORMAT_HEX;
	uint16_t m_nSlots = 0;
#if defined (__linux__) || defined (__CYGWIN__) || defined(__APPLE__)
	enum {
		DMX_DEFAULT_MAX_CHANNELS = 32,
		DMX_DEFAULT_START_ADDRESS = 1
	};
	#define DMXMONITOR_MAX_PORTS	4
	bool m_bIsStarted[DMXMONITOR_MAX_PORTS];
	uint16_t m_nDmxStartAddress = DMX_DEFAULT_START_ADDRESS;
	uint16_t m_nMaxChannels = DMX_DEFAULT_MAX_CHANNELS;
#else
	bool m_bIsStarted = false;
	alignas(uint32_t) uint8_t m_Data[512];
#endif
};

#endif /* DMXMONITOR_H_ */
