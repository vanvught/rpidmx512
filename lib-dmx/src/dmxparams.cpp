/**
 * @file dmxparams.cpp
 *
 */
/* Copyright (C) 2017-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include "dmxparams.h"
#include "dmxparamsconst.h"
#include "dmxconst.h"

#include "readconfigfile.h"
#include "sscan.h"

#include "propertiesbuilder.h"

#include "debug.h"

DmxParams::DmxParams() {
	m_Params.nSetList = 0;
	m_Params.nBreakTime = dmx::transmit::BREAK_TIME_TYPICAL;
	m_Params.nMabTime = dmx::transmit::MAB_TIME_MIN;
	m_Params.nRefreshRate = dmx::transmit::REFRESH_RATE_DEFAULT;
	m_Params.nSlotsCount = dmxsendparams::rounddown_slots(dmx::max::CHANNELS);
}

void DmxParams::Load() {
	DEBUG_ENTRY

	m_Params.nSetList = 0;

#if !defined(DISABLE_FS)
	ReadConfigFile configfile(DmxParams::StaticCallbackFunction, this);

	if (configfile.Read(DmxParamsConst::FILE_NAME)) {
		dmxsendparams::store::update(&m_Params);
	} else
#endif
		dmxsendparams::store::copy(&m_Params);

#ifndef NDEBUG
	Dump();
#endif
	DEBUG_EXIT
}

void DmxParams::Load(const char* pBuffer, uint32_t nLength) {
	DEBUG_ENTRY

	assert(pBuffer != nullptr);
	assert(nLength != 0);

	m_Params.nSetList = 0;

	ReadConfigFile config(DmxParams::StaticCallbackFunction, this);

	config.Read(pBuffer, nLength);

	dmxsendparams::store::update(&m_Params);

#ifndef NDEBUG
	Dump();
#endif
	DEBUG_EXIT
}

void DmxParams::callbackFunction(const char *pLine) {
	assert(pLine != nullptr);

	uint16_t nValue16;

	if (Sscan::Uint16(pLine, DmxParamsConst::BREAK_TIME, nValue16) == Sscan::OK) {
		if ((nValue16 >= dmx::transmit::BREAK_TIME_MIN) && (nValue16 != dmx::transmit::BREAK_TIME_TYPICAL)) {
			m_Params.nBreakTime = nValue16;
			m_Params.nSetList |= dmxsendparams::Mask::BREAK_TIME;
		} else {
			m_Params.nBreakTime = dmx::transmit::BREAK_TIME_TYPICAL;
			m_Params.nSetList &= ~dmxsendparams::Mask::BREAK_TIME;
		}
		return;
	}

	if (Sscan::Uint16(pLine, DmxParamsConst::MAB_TIME, nValue16) == Sscan::OK) {
		if (nValue16 > dmx::transmit::MAB_TIME_MIN)  { // && (nValue32 <= dmx::transmit::MAB_TIME_MAX)) {
			m_Params.nMabTime = nValue16;
			m_Params.nSetList |= dmxsendparams::Mask::MAB_TIME;
		} else {
			m_Params.nMabTime = dmx::transmit::MAB_TIME_MIN;
			m_Params.nSetList &= ~dmxsendparams::Mask::MAB_TIME;
		}
		return;
	}

	uint8_t nValue8;

	if (Sscan::Uint8(pLine, DmxParamsConst::REFRESH_RATE, nValue8) == Sscan::OK) {
		if (nValue8 != dmx::transmit::REFRESH_RATE_DEFAULT) {
			m_Params.nRefreshRate = nValue8;
			m_Params.nSetList |= dmxsendparams::Mask::REFRESH_RATE;
		} else {
			m_Params.nRefreshRate = dmx::transmit::REFRESH_RATE_DEFAULT;
			m_Params.nSetList &= ~dmxsendparams::Mask::REFRESH_RATE;
		}
		return;
	}

	if (Sscan::Uint16(pLine, DmxParamsConst::SLOTS_COUNT, nValue16) == Sscan::OK) {
		if ((nValue16 >= 2) && (nValue16 < dmx::max::CHANNELS)) {
			m_Params.nSlotsCount = dmxsendparams::rounddown_slots(nValue16);
			m_Params.nSetList |= dmxsendparams::Mask::SLOTS_COUNT;
		} else {
			m_Params.nSlotsCount = dmxsendparams::rounddown_slots(dmx::max::CHANNELS);
			m_Params.nSetList &= ~dmxsendparams::Mask::SLOTS_COUNT;
		}
		return;
	}
}

void DmxParams::Builder(const struct dmxsendparams::Params *ptDMXParams, char *pBuffer, uint32_t nLength, uint32_t& nSize) {
	DEBUG_ENTRY

	assert(pBuffer != nullptr);

	if (ptDMXParams != nullptr) {
		memcpy(&m_Params, ptDMXParams, sizeof(struct dmxsendparams::Params));
	} else {
		dmxsendparams::store::copy(&m_Params);
	}

	PropertiesBuilder builder(DmxParamsConst::FILE_NAME, pBuffer, nLength);

	builder.Add(DmxParamsConst::BREAK_TIME, m_Params.nBreakTime, isMaskSet(dmxsendparams::Mask::BREAK_TIME));
	builder.Add(DmxParamsConst::MAB_TIME, m_Params.nMabTime, isMaskSet(dmxsendparams::Mask::MAB_TIME));
	builder.Add(DmxParamsConst::REFRESH_RATE, m_Params.nRefreshRate, isMaskSet(dmxsendparams::Mask::REFRESH_RATE));
	builder.Add(DmxParamsConst::SLOTS_COUNT, dmxsendparams::roundup_slots(m_Params.nSlotsCount), isMaskSet(dmxsendparams::Mask::SLOTS_COUNT));

	nSize = builder.GetSize();

	DEBUG_PRINTF("nSize=%d", nSize);
	DEBUG_EXIT
}

void DmxParams::Set(Dmx *p) {
	assert(p != nullptr);

	if (isMaskSet(dmxsendparams::Mask::BREAK_TIME)) {
		p->SetDmxBreakTime(m_Params.nBreakTime);
	}

	if (isMaskSet(dmxsendparams::Mask::MAB_TIME)) {
		p->SetDmxMabTime(m_Params.nMabTime);
	}

	if (isMaskSet(dmxsendparams::Mask::REFRESH_RATE)) {
		uint32_t period = 0;
		if (m_Params.nRefreshRate != 0) {
			period = 1000000U / m_Params.nRefreshRate;
		}
		p->SetDmxPeriodTime(period);
	}

	if (isMaskSet(dmxsendparams::Mask::SLOTS_COUNT)) {
		p->SetDmxSlots(dmxsendparams::roundup_slots(m_Params.nSlotsCount));
	}
}

void DmxParams::StaticCallbackFunction(void *p, const char *s) {
	assert(p != nullptr);
	assert(s != nullptr);

	(static_cast<DmxParams *>(p))->callbackFunction(s);
}

void DmxParams::Dump() {
	printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, DmxParamsConst::FILE_NAME);
	printf(" %s=%d\n", DmxParamsConst::BREAK_TIME, m_Params.nBreakTime);
	printf(" %s=%d\n", DmxParamsConst::MAB_TIME, m_Params.nMabTime);
	printf(" %s=%d\n", DmxParamsConst::REFRESH_RATE, m_Params.nRefreshRate);
	printf(" %s=%d [%d]\n", DmxParamsConst::SLOTS_COUNT, m_Params.nSlotsCount, dmxsendparams::roundup_slots(m_Params.nSlotsCount));
}
