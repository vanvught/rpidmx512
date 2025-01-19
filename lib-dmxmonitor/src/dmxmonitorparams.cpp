/**
 * @file dmxmonitorparams.cpp
 *
 */
/* Copyright (C) 2019-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include <cstdint>
#include <cstring>
#ifndef NDEBUG
# include <cstdio>
#endif
#include <cassert>

#include "dmxmonitor.h"
#include "dmxmonitorparams.h"
#include "dmxmonitorparamsconst.h"

#include "lightset.h"
#include "lightsetparamsconst.h"

#include "readconfigfile.h"
#include "sscan.h"

#include "propertiesbuilder.h"

#include "debug.h"

using namespace dmxmonitor;
using namespace lightset;

DMXMonitorParams::DMXMonitorParams() {
	m_Params.nSetList = 0;
	m_Params.nDmxStartAddress = dmx::START_ADDRESS_DEFAULT;
	m_Params.nDmxMaxChannels = dmx::UNIVERSE_SIZE;
	m_Params.tFormat = static_cast<uint8_t>(Format::HEX);
}

void DMXMonitorParams::Load() {
	DEBUG_ENTRY

	m_Params.nSetList = 0;

#if !defined(DISABLE_FS)
	ReadConfigFile configfile(DMXMonitorParams::StaticCallbackFunction, this);

	if (configfile.Read(DMXMonitorParamsConst::FILE_NAME)) {
		DEBUG_PUTS("");
		DmxMonitorParamsStore::Update(&m_Params);
		DEBUG_PUTS("");
	} else
#endif
	{
		DEBUG_PUTS("");
		DmxMonitorParamsStore::Copy(&m_Params);
		DEBUG_PUTS("");
	}
#ifndef NDEBUG
	Dump();
#endif
	DEBUG_EXIT
}

void DMXMonitorParams::Load(const char *pBuffer, uint32_t nLength) {
	DEBUG_ENTRY

	assert(pBuffer != nullptr);
	assert(nLength != 0);

	m_Params.nSetList = 0;

	ReadConfigFile config(DMXMonitorParams::StaticCallbackFunction, this);

	config.Read(pBuffer, nLength);

	DmxMonitorParamsStore::Update(&m_Params);

#ifndef NDEBUG
	Dump();
#endif
	DEBUG_EXIT
}

void DMXMonitorParams::Builder(const struct TDMXMonitorParams *ptDMXMonitorParams, char *pBuffer, uint32_t nLength, uint32_t& nSize) {
	DEBUG_ENTRY
	assert(pBuffer != nullptr);

	if (ptDMXMonitorParams != nullptr) {
		memcpy(&m_Params, ptDMXMonitorParams, sizeof(struct TDMXMonitorParams));
	} else {
		DmxMonitorParamsStore::Copy(&m_Params);
	}

	PropertiesBuilder builder(DMXMonitorParamsConst::FILE_NAME, pBuffer, nLength);

	builder.Add(DMXMonitorParamsConst::FORMAT, m_Params.tFormat == static_cast<uint8_t>(Format::PCT) ? "pct" : (m_Params.tFormat == static_cast<uint8_t>(Format::DEC) ? "dec" : "hex"), isMaskSet(DMXMonitorParamsMask::FORMAT));

	builder.AddComment("DMX");
	builder.Add(LightSetParamsConst::DMX_START_ADDRESS, m_Params.nDmxStartAddress, isMaskSet(DMXMonitorParamsMask::START_ADDRESS));
	builder.Add(DMXMonitorParamsConst::DMX_MAX_CHANNELS, m_Params.nDmxMaxChannels, isMaskSet(DMXMonitorParamsMask::MAX_CHANNELS));

	nSize = builder.GetSize();
	DEBUG_EXIT
}

void DMXMonitorParams::Set(DMXMonitor* pDMXMonitor) {
	assert(pDMXMonitor != nullptr);

	if (isMaskSet(DMXMonitorParamsMask::START_ADDRESS)) {
		pDMXMonitor->SetDmxStartAddress(m_Params.nDmxStartAddress);
	}

	if (isMaskSet(DMXMonitorParamsMask::MAX_CHANNELS)) {
#if defined (__linux__) || defined (__CYGWIN__) || defined(__APPLE__)
		pDMXMonitor->SetMaxDmxChannels(m_Params.nDmxMaxChannels);
#endif
	}

	if (isMaskSet(DMXMonitorParamsMask::FORMAT)) {
		pDMXMonitor->SetFormat(static_cast<Format>(m_Params.tFormat));
	}
}

void DMXMonitorParams::callbackFunction(const char* pLine) {
	assert(pLine != nullptr);

	uint16_t value16;
	char value[8];
	uint32_t nLength;

	if (Sscan::Uint16(pLine, LightSetParamsConst::DMX_START_ADDRESS, value16) == Sscan::OK) {
		if (value16 != 0 && value16 <= 512) {
			m_Params.nDmxStartAddress = value16;
			m_Params.nSetList |= DMXMonitorParamsMask::START_ADDRESS;
		}
		return;
	}

	if (Sscan::Uint16(pLine, DMXMonitorParamsConst::DMX_MAX_CHANNELS, value16) == Sscan::OK) {
		if (value16 != 0 && value16 <= 512) {
			m_Params.nDmxMaxChannels = value16;
			m_Params.nSetList |= DMXMonitorParamsMask::MAX_CHANNELS;
		}
		return;
	}

	nLength = 3;
	if (Sscan::Char(pLine, DMXMonitorParamsConst::FORMAT, value, nLength) == Sscan::OK) {
		if (memcmp(value, "pct", 3) == 0) {
			m_Params.tFormat = static_cast<uint8_t>(Format::PCT);
			m_Params.nSetList |= DMXMonitorParamsMask::FORMAT;
		} else if (memcmp(value, "dec", 3) == 0) {
			m_Params.tFormat = static_cast<uint8_t>(Format::DEC);
			m_Params.nSetList |= DMXMonitorParamsMask::FORMAT;
		} else {
			m_Params.tFormat = static_cast<uint8_t>(Format::HEX);
			m_Params.nSetList &= ~DMXMonitorParamsMask::FORMAT;
		}
		return;
	}
}

void DMXMonitorParams::StaticCallbackFunction(void *p, const char *s) {
	assert(p != nullptr);
	assert(s != nullptr);

	(static_cast<DMXMonitorParams*>(p))->callbackFunction(s);
}

void DMXMonitorParams::Dump() {
	printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, DMXMonitorParamsConst::FILE_NAME);
	printf(" %s=%d\n", LightSetParamsConst::DMX_START_ADDRESS, m_Params.nDmxStartAddress);
	printf(" %s=%d\n", DMXMonitorParamsConst::DMX_MAX_CHANNELS, m_Params.nDmxMaxChannels);
	printf(" %s=%d [%s]\n", DMXMonitorParamsConst::FORMAT, static_cast<int>(m_Params.tFormat), m_Params.tFormat == static_cast<uint8_t>(Format::PCT) ? "pct" : (m_Params.tFormat == static_cast<uint8_t>(Format::DEC) ? "dec" : "hex"));
}
