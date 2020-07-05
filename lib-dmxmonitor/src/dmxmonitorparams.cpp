/**
 * @file dmxmonitorparams.cpp
 *
 */
/* Copyright (C) 2019-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#if !defined(__clang__)	// Needed for compiling on MacOS
 #pragma GCC push_options
 #pragma GCC optimize ("Os")
#endif

#include <stdint.h>
#include <string.h>
#ifndef NDEBUG
 #include <stdio.h>
#endif
#include <cassert>

#include "dmxmonitorparams.h"
#include "dmxmonitorparamsconst.h"

#include "lightset.h"
#include "lightsetconst.h"

#include "readconfigfile.h"
#include "sscan.h"

#include "propertiesbuilder.h"

#include "debug.h"

DMXMonitorParams::DMXMonitorParams(DMXMonitorParamsStore *pDMXMonitorParamsStore): m_pDMXMonitorParamsStore(pDMXMonitorParamsStore) {
	m_tDMXMonitorParams.nSetList = 0;
	m_tDMXMonitorParams.nDmxStartAddress = DMX_START_ADDRESS_DEFAULT;
	m_tDMXMonitorParams.nDmxMaxChannels = DMX_UNIVERSE_SIZE;
	m_tDMXMonitorParams.tFormat = DMXMonitorFormat::DMX_MONITOR_FORMAT_HEX;
}

bool DMXMonitorParams::Load() {
	m_tDMXMonitorParams.nSetList = 0;

	ReadConfigFile configfile(DMXMonitorParams::staticCallbackFunction, this);

	if (configfile.Read(DMXMonitorParamsConst::FILE_NAME)) {
		// There is a configuration file
		if (m_pDMXMonitorParamsStore != nullptr) {
			m_pDMXMonitorParamsStore->Update(&m_tDMXMonitorParams);
		}
	} else if (m_pDMXMonitorParamsStore != nullptr) {
		m_pDMXMonitorParamsStore->Copy(&m_tDMXMonitorParams);
	} else {
		return false;
	}

	return true;
}

void DMXMonitorParams::Load(const char *pBuffer, uint32_t nLength) {
	assert(pBuffer != nullptr);
	assert(nLength != 0);
	assert(m_pDMXMonitorParamsStore != nullptr);

	if (m_pDMXMonitorParamsStore == nullptr) {
		return;
	}

	m_tDMXMonitorParams.nSetList = 0;

	ReadConfigFile config(DMXMonitorParams::staticCallbackFunction, this);

	config.Read(pBuffer, nLength);

	m_pDMXMonitorParamsStore->Update(&m_tDMXMonitorParams);
}

void DMXMonitorParams::Builder(const struct TDMXMonitorParams *ptDMXMonitorParams, char *pBuffer, uint32_t nLength, uint32_t &nSize) {
	DEBUG_ENTRY
	assert(pBuffer != nullptr);

	if (ptDMXMonitorParams != nullptr) {
		memcpy(&m_tDMXMonitorParams, ptDMXMonitorParams, sizeof(struct TDMXMonitorParams));
	} else {
		m_pDMXMonitorParamsStore->Copy(&m_tDMXMonitorParams);
	}

	PropertiesBuilder builder(DMXMonitorParamsConst::FILE_NAME, pBuffer, nLength);

	builder.Add(DMXMonitorParamsConst::FORMAT, m_tDMXMonitorParams.tFormat == DMXMonitorFormat::DMX_MONITOR_FORMAT_PCT ? "pct" : (m_tDMXMonitorParams.tFormat == DMXMonitorFormat::DMX_MONITOR_FORMAT_DEC ? "dec" : "hex"), isMaskSet(DMXMonitorParamsMask::FORMAT));

	builder.AddComment("DMX");
	builder.Add(LightSetConst::PARAMS_DMX_START_ADDRESS, m_tDMXMonitorParams.nDmxStartAddress, isMaskSet(DMXMonitorParamsMask::START_ADDRESS));
	builder.Add(DMXMonitorParamsConst::DMX_MAX_CHANNELS, m_tDMXMonitorParams.nDmxMaxChannels, isMaskSet(DMXMonitorParamsMask::MAX_CHANNELS));

	nSize = builder.GetSize();
	DEBUG_EXIT
}

void DMXMonitorParams::Save(char *pBuffer, uint32_t nLength, uint32_t &nSize) {
	DEBUG_ENTRY

	if (m_pDMXMonitorParamsStore == nullptr) {
		nSize = 0;
		DEBUG_EXIT
		return;
	}

	Builder(nullptr, pBuffer, nLength, nSize);

	DEBUG_EXIT
}

void DMXMonitorParams::Set(DMXMonitor* pDMXMonitor) {
	assert(pDMXMonitor != nullptr);

	if (isMaskSet(DMXMonitorParamsMask::START_ADDRESS)) {
		pDMXMonitor->SetDmxStartAddress(m_tDMXMonitorParams.nDmxStartAddress);
	}

	if (isMaskSet(DMXMonitorParamsMask::MAX_CHANNELS)) {
#if defined (__linux__) || defined (__CYGWIN__) || defined(__APPLE__)
		pDMXMonitor->SetMaxDmxChannels(m_tDMXMonitorParams.nDmxMaxChannels);
#endif
	}

	if (isMaskSet(DMXMonitorParamsMask::FORMAT)) {
		pDMXMonitor->SetFormat(m_tDMXMonitorParams.tFormat);
	}
}

void DMXMonitorParams::callbackFunction(const char* pLine) {
	assert(pLine != nullptr);

	uint16_t value16;
	char value[8];
	uint32_t nLength;

	if (Sscan::Uint16(pLine, LightSetConst::PARAMS_DMX_START_ADDRESS, value16) == Sscan::OK) {
		if (value16 != 0 && value16 <= 512) {
			m_tDMXMonitorParams.nDmxStartAddress = value16;
			m_tDMXMonitorParams.nSetList |= DMXMonitorParamsMask::START_ADDRESS;
		}
		return;
	}

	if (Sscan::Uint16(pLine, DMXMonitorParamsConst::DMX_MAX_CHANNELS, value16) == Sscan::OK) {
		if (value16 != 0 && value16 <= 512) {
			m_tDMXMonitorParams.nDmxMaxChannels = value16;
			m_tDMXMonitorParams.nSetList |= DMXMonitorParamsMask::MAX_CHANNELS;
		}
		return;
	}

	nLength = 3;
	if (Sscan::Char(pLine, DMXMonitorParamsConst::FORMAT, value, nLength) == Sscan::OK) {
		if (memcmp(value, "pct", 3) == 0) {
			m_tDMXMonitorParams.tFormat = DMXMonitorFormat::DMX_MONITOR_FORMAT_PCT;
			m_tDMXMonitorParams.nSetList |= DMXMonitorParamsMask::FORMAT;
		} else if (memcmp(value, "dec", 3) == 0) {
			m_tDMXMonitorParams.tFormat = DMXMonitorFormat::DMX_MONITOR_FORMAT_DEC;
			m_tDMXMonitorParams.nSetList |= DMXMonitorParamsMask::FORMAT;
		} else {
			m_tDMXMonitorParams.tFormat = DMXMonitorFormat::DMX_MONITOR_FORMAT_HEX;
			m_tDMXMonitorParams.nSetList &= ~DMXMonitorParamsMask::FORMAT;
		}
		return;
	}
}

void DMXMonitorParams::Dump() {
#ifndef NDEBUG
	if (m_tDMXMonitorParams.nSetList == 0) {
		return;
	}

	printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, DMXMonitorParamsConst::FILE_NAME);

	if (isMaskSet(DMXMonitorParamsMask::START_ADDRESS)) {
		printf(" %s=%d\n", LightSetConst::PARAMS_DMX_START_ADDRESS, m_tDMXMonitorParams.nDmxStartAddress);
	}

	if (isMaskSet(DMXMonitorParamsMask::MAX_CHANNELS)) {
		printf(" %s=%d\n", DMXMonitorParamsConst::DMX_MAX_CHANNELS, m_tDMXMonitorParams.nDmxMaxChannels);
	}

	if (isMaskSet(DMXMonitorParamsMask::FORMAT)) {
		printf(" %s=%d [%s]\n", DMXMonitorParamsConst::FORMAT, static_cast<int>(m_tDMXMonitorParams.tFormat), m_tDMXMonitorParams.tFormat == DMXMonitorFormat::DMX_MONITOR_FORMAT_PCT ? "pct" : (m_tDMXMonitorParams.tFormat == DMXMonitorFormat::DMX_MONITOR_FORMAT_DEC ? "dec" : "hex"));
	}
#endif
}

void DMXMonitorParams::staticCallbackFunction(void *p, const char *s) {
	assert(p != nullptr);
	assert(s != nullptr);

	(static_cast<DMXMonitorParams*>(p))->callbackFunction(s);
}

