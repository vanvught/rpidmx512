/**
 * @file dmxparams.cpp
 *
 */
/* Copyright (C) 2017-2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
# pragma GCC push_options
# pragma GCC optimize ("Os")
#endif

#include <cstdint>
#include <cstring>
#ifndef NDEBUG
# include <cstdio>
#endif
#include <cassert>

#include "dmxparams.h"
#include "dmxparamsconst.h"
#include "dmxconst.h"

#include "readconfigfile.h"
#include "sscan.h"

#include "propertiesbuilder.h"

#include "debug.h"

DmxParams::DmxParams(DmxParamsStore *pDMXParamsStore) : m_pDmxParamsStore(pDMXParamsStore) {
	m_tDmxParams.nSetList = 0;
	m_tDmxParams.nBreakTime = dmx::transmit::BREAK_TIME_TYPICAL;
	m_tDmxParams.nMabTime = dmx::transmit::MAB_TIME_MIN;
	m_tDmxParams.nRefreshRate = dmx::transmit::REFRESH_RATE_DEFAULT;
	m_tDmxParams.nSlotsCount = dmxparams::rounddown_slots(dmx::max::CHANNELS);

	DEBUG_PRINTF("m_tDmxParams.nSlotsCount=%d", m_tDmxParams.nSlotsCount);
}

bool DmxParams::Load() {
	m_tDmxParams.nSetList = 0;

#if !defined(DISABLE_FS)
	ReadConfigFile configfile(DmxParams::staticCallbackFunction, this);

	if (configfile.Read(DmxParamsConst::FILE_NAME)) {
		// There is a configuration file
		if (m_pDmxParamsStore != nullptr) {
			m_pDmxParamsStore->Update(&m_tDmxParams);
		}
	} else
#endif
	if (m_pDmxParamsStore != nullptr) {
		m_pDmxParamsStore->Copy(&m_tDmxParams);
	} else {
		return false;
	}

	return true;
}

void DmxParams::Load(const char* pBuffer, uint32_t nLength) {
	DEBUG_ENTRY

	assert(pBuffer != nullptr);
	assert(nLength != 0);

	m_tDmxParams.nSetList = 0;

	ReadConfigFile config(DmxParams::staticCallbackFunction, this);

	config.Read(pBuffer, nLength);

	assert(m_pDmxParamsStore != nullptr);
	m_pDmxParamsStore->Update(&m_tDmxParams);

	DEBUG_EXIT
}

void DmxParams::callbackFunction(const char *pLine) {
	assert(pLine != nullptr);

	uint16_t nValue16;

	if (Sscan::Uint16(pLine, DmxParamsConst::BREAK_TIME, nValue16) == Sscan::OK) {
		if ((nValue16 >= dmx::transmit::BREAK_TIME_MIN) && (nValue16 != dmx::transmit::BREAK_TIME_TYPICAL)) {
			m_tDmxParams.nBreakTime = nValue16;
			m_tDmxParams.nSetList |= DmxParamsMask::BREAK_TIME;
		} else {
			m_tDmxParams.nBreakTime = dmx::transmit::BREAK_TIME_TYPICAL;
			m_tDmxParams.nSetList &= ~DmxParamsMask::BREAK_TIME;
		}
		return;
	}

	if (Sscan::Uint16(pLine, DmxParamsConst::MAB_TIME, nValue16) == Sscan::OK) {
		if (nValue16 > dmx::transmit::MAB_TIME_MIN)  { // && (nValue32 <= dmx::transmit::MAB_TIME_MAX)) {
			m_tDmxParams.nMabTime = nValue16;
			m_tDmxParams.nSetList |= DmxParamsMask::MAB_TIME;
		} else {
			m_tDmxParams.nMabTime = dmx::transmit::MAB_TIME_MIN;
			m_tDmxParams.nSetList &= ~DmxParamsMask::MAB_TIME;
		}
		return;
	}

	uint8_t nValue8;

	if (Sscan::Uint8(pLine, DmxParamsConst::REFRESH_RATE, nValue8) == Sscan::OK) {
		if (nValue8 != dmx::transmit::REFRESH_RATE_DEFAULT) {
			m_tDmxParams.nRefreshRate = nValue8;
			m_tDmxParams.nSetList |= DmxParamsMask::REFRESH_RATE;
		} else {
			m_tDmxParams.nRefreshRate = dmx::transmit::REFRESH_RATE_DEFAULT;
			m_tDmxParams.nSetList &= ~DmxParamsMask::REFRESH_RATE;
		}
		return;
	}

	if (Sscan::Uint16(pLine, DmxParamsConst::SLOTS_COUNT, nValue16) == Sscan::OK) {
		if ((nValue16 >= 2) && (nValue16 < dmx::max::CHANNELS)) {
			m_tDmxParams.nSlotsCount = dmxparams::rounddown_slots(nValue16);
			m_tDmxParams.nSetList |= DmxParamsMask::SLOTS_COUNT;
		} else {
			m_tDmxParams.nSlotsCount = dmxparams::rounddown_slots(dmx::max::CHANNELS);
			m_tDmxParams.nSetList &= ~DmxParamsMask::SLOTS_COUNT;
		}
		return;
	}
}

void DmxParams::Builder(const struct TDmxParams *ptDMXParams, char *pBuffer, uint32_t nLength, uint32_t& nSize) {
	DEBUG_ENTRY

	assert(pBuffer != nullptr);

	if (ptDMXParams != nullptr) {
		memcpy(&m_tDmxParams, ptDMXParams, sizeof(struct TDmxParams));
	} else {
		assert(m_pDmxParamsStore != nullptr);
		m_pDmxParamsStore->Copy(&m_tDmxParams);
	}

	PropertiesBuilder builder(DmxParamsConst::FILE_NAME, pBuffer, nLength);

	builder.Add(DmxParamsConst::BREAK_TIME, m_tDmxParams.nBreakTime, isMaskSet(DmxParamsMask::BREAK_TIME));
	builder.Add(DmxParamsConst::MAB_TIME, m_tDmxParams.nMabTime, isMaskSet(DmxParamsMask::MAB_TIME));
	builder.Add(DmxParamsConst::REFRESH_RATE, m_tDmxParams.nRefreshRate, isMaskSet(DmxParamsMask::REFRESH_RATE));
	builder.Add(DmxParamsConst::SLOTS_COUNT, dmxparams::roundup_slots(m_tDmxParams.nSlotsCount), isMaskSet(DmxParamsMask::SLOTS_COUNT));

	nSize = builder.GetSize();

	DEBUG_PRINTF("nSize=%d", nSize);
	DEBUG_EXIT
}

void DmxParams::Set(Dmx *p) {
	assert(p != nullptr);

	if (isMaskSet(DmxParamsMask::BREAK_TIME)) {
		p->SetDmxBreakTime(m_tDmxParams.nBreakTime);
	}

	if (isMaskSet(DmxParamsMask::MAB_TIME)) {
		p->SetDmxMabTime(m_tDmxParams.nMabTime);
	}

	if (isMaskSet(DmxParamsMask::REFRESH_RATE)) {
		uint32_t period = 0;
		if (m_tDmxParams.nRefreshRate != 0) {
			period = 1000000U / m_tDmxParams.nRefreshRate;
		}
		p->SetDmxPeriodTime(period);
	}

	if (isMaskSet(DmxParamsMask::SLOTS_COUNT)) {
		p->SetDmxSlots(dmxparams::roundup_slots(m_tDmxParams.nSlotsCount));
	}
}

void DmxParams::Dump() {
#ifndef NDEBUG
	printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, DmxParamsConst::FILE_NAME);

	if (isMaskSet(DmxParamsMask::BREAK_TIME)) {
		printf(" %s=%d\n", DmxParamsConst::BREAK_TIME, m_tDmxParams.nBreakTime);
	}

	if (isMaskSet(DmxParamsMask::MAB_TIME)) {
		printf(" %s=%d\n", DmxParamsConst::MAB_TIME, m_tDmxParams.nMabTime);
	}

	if (isMaskSet(DmxParamsMask::REFRESH_RATE)) {
		printf(" %s=%d\n", DmxParamsConst::REFRESH_RATE, m_tDmxParams.nRefreshRate);
	}

	if (isMaskSet(DmxParamsMask::SLOTS_COUNT)) {
		printf(" %s=%d [%d]\n", DmxParamsConst::SLOTS_COUNT, m_tDmxParams.nSlotsCount, dmxparams::roundup_slots(m_tDmxParams.nSlotsCount));
	}
#endif
}

void DmxParams::staticCallbackFunction(void *p, const char *s) {
	assert(p != nullptr);
	assert(s != nullptr);

	(static_cast<DmxParams*>(p))->callbackFunction(s);
}
