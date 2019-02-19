/**
 * @file dmxmonitorparams.cpp
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

#include <stdint.h>
#include <string.h>
#ifndef NDEBUG
 #include <stdio.h>
#endif
#include <assert.h>

#ifndef ALIGNED
 #define ALIGNED __attribute__ ((aligned (4)))
#endif

#include "dmxmonitorparams.h"

#include "readconfigfile.h"
#include "sscan.h"

#define SET_DMX_START_ADDRESS		(1 << 0)
#define SET_DMX_MAX_CHANNELS		(1 << 1)
#define SET_FORMAT					(1 << 2)

static const char PARAMS_FILE_NAME[] ALIGNED = "mon.txt";
static const char PARAMS_DMX_START_ADDRESS[] ALIGNED = "dmx_start_address";
static const char PARAMS_DMX_MAX_CHANNELS[] ALIGNED = "dmx_max_channels";
static const char PARAMS_FORMAT[] ALIGNED = "format";

DMXMonitorParams::DMXMonitorParams(DMXMonitorParamsStore* pDMXMonitorParamsStore): m_pDMXMonitorParamsStore(pDMXMonitorParamsStore) {
	m_tDMXMonitorParams.nSetList = 0;
	m_tDMXMonitorParams.nDmxStartAddress = 1;
	m_tDMXMonitorParams.nDmxMaxChannels = 512;
	m_tDMXMonitorParams.tFormat = DMX_MONITOR_FORMAT_HEX;
}

DMXMonitorParams::~DMXMonitorParams(void) {
}

bool DMXMonitorParams::Load(void) {
	m_tDMXMonitorParams.nSetList = 0;

	ReadConfigFile configfile(DMXMonitorParams::staticCallbackFunction, this);

	if (configfile.Read(PARAMS_FILE_NAME)) {
		// There is a configuration file
		if (m_pDMXMonitorParamsStore != 0) {
			m_pDMXMonitorParamsStore->Update(&m_tDMXMonitorParams);
		}
	} else if (m_pDMXMonitorParamsStore != 0) {
		m_pDMXMonitorParamsStore->Copy(&m_tDMXMonitorParams);
	} else {
		return false;
	}

	return true;
}

void DMXMonitorParams::Set(DMXMonitor* pDMXMonitor) {
	assert(pDMXMonitor != 0);

	if (isMaskSet(SET_DMX_START_ADDRESS)) {
		pDMXMonitor->SetDmxStartAddress(m_tDMXMonitorParams.nDmxStartAddress);
	}

	if (isMaskSet(SET_DMX_MAX_CHANNELS)) {
#if defined (__linux__) || defined (__CYGWIN__) || defined(__APPLE__)
		pDMXMonitor->SetMaxDmxChannels(m_tDMXMonitorParams.nDmxMaxChannels);
#endif
	}

	if (isMaskSet(SET_FORMAT)) {
		pDMXMonitor->SetFormat(m_tDMXMonitorParams.tFormat);
	}
}

void DMXMonitorParams::Dump(void) {
#ifndef NDEBUG
	if (m_tDMXMonitorParams.nSetList == 0) {
		return;
	}

	printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, PARAMS_FILE_NAME);

	if (isMaskSet(SET_DMX_START_ADDRESS)) {
		printf(" %s=%d\n", PARAMS_DMX_START_ADDRESS, (int) m_tDMXMonitorParams.nDmxStartAddress);
	}

	if (isMaskSet(SET_DMX_MAX_CHANNELS)) {
		printf(" %s=%d\n", PARAMS_DMX_MAX_CHANNELS, (int) m_tDMXMonitorParams.nDmxMaxChannels);
	}

	if (isMaskSet(SET_FORMAT)) {
		printf(" %s=%d [%s]\n", PARAMS_FORMAT, (int) m_tDMXMonitorParams.tFormat, m_tDMXMonitorParams.tFormat == DMX_MONITOR_FORMAT_PCT ? "pct" : (m_tDMXMonitorParams.tFormat == DMX_MONITOR_FORMAT_DEC ? "dec" : "hex"));
	}
#endif
}

void DMXMonitorParams::staticCallbackFunction(void* p, const char* s) {
	assert(p != 0);
	assert(s != 0);

	((DMXMonitorParams *) p)->callbackFunction(s);
}

void DMXMonitorParams::callbackFunction(const char* pLine) {
	assert(pLine != 0);

	uint16_t value16;
	char value[8];
	uint8_t len;

	if (Sscan::Uint16(pLine, PARAMS_DMX_START_ADDRESS, &value16) == SSCAN_OK) {
		if (value16 != 0 && value16 <= 512) {
			m_tDMXMonitorParams.nDmxStartAddress = value16;
			m_tDMXMonitorParams.nSetList |= SET_DMX_START_ADDRESS;
		}
		return;
	}

	if (Sscan::Uint16(pLine, PARAMS_DMX_MAX_CHANNELS, &value16) == SSCAN_OK) {
		if (value16 != 0 && value16 <= 512) {
			m_tDMXMonitorParams.nDmxMaxChannels = value16;
			m_tDMXMonitorParams.nSetList |= SET_DMX_MAX_CHANNELS;
		}
		return;
	}

	len = 3;
	if (Sscan::Char(pLine, PARAMS_FORMAT, value, &len) == SSCAN_OK) {
		if (memcmp(value, "pct", 3) == 0) {
			m_tDMXMonitorParams.tFormat = DMX_MONITOR_FORMAT_PCT;
		} else if (memcmp(value, "dec", 3) == 0) {
			m_tDMXMonitorParams.tFormat = DMX_MONITOR_FORMAT_DEC;
		} else {
			m_tDMXMonitorParams.tFormat = DMX_MONITOR_FORMAT_HEX;
		}
		m_tDMXMonitorParams.nSetList |= SET_FORMAT;
		return;
	}
}

bool DMXMonitorParams::isMaskSet(uint32_t nMask) const {
	return (m_tDMXMonitorParams.nSetList & nMask) == nMask;
}
