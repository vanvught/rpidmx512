/**
 * @file modeparams.cpp
 *
 */
/* Copyright (C) 2017-2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <cstdio>
#include <cassert>

#include "modeparams.h"
#include "modeparamsconst.h"

#include "l6470dmxmode.h"
#include "l6470dmxconst.h"

#include "lightsetparamsconst.h"

#include "dmxslotinfo.h"

#include "readconfigfile.h"
#include "sscan.h"
#include "parse.h"
#include "propertiesbuilder.h"

#include "debug.h"

using namespace lightset;

ModeParams::ModeParams() {
	DEBUG_ENTRY

	memset(&m_Params, 0, sizeof(struct modeparams::Params));
	m_Params.nDmxMode = L6470DMXMODE_UNDEFINED;
	m_Params.nDmxStartAddress = dmx::ADDRESS_INVALID;
	m_Params.tSwitchAction = L6470_ABSPOS_RESET;
	m_Params.tSwitchDir = L6470_DIR_REV;
	m_Params.bSwitch = true;

	auto *pSlotInfo = new SlotInfo[modeparams::MODE_PARAMS_MAX_DMX_FOOTPRINT];
	memcpy(pSlotInfo, m_Params.tLightSetSlotInfo, sizeof(m_Params.tLightSetSlotInfo));

	m_pDmxSlotInfo = new DmxSlotInfo(pSlotInfo, modeparams::MODE_PARAMS_MAX_DMX_FOOTPRINT);
	assert(m_pDmxSlotInfo != nullptr);

	assert(sizeof(m_aFileName) > strlen(L6470DmxConst::FILE_NAME_MOTOR));
	strncpy(m_aFileName, L6470DmxConst::FILE_NAME_MOTOR, sizeof(m_aFileName));

	DEBUG_EXIT
}

ModeParams::~ModeParams() {
	delete m_pDmxSlotInfo;
	m_pDmxSlotInfo = nullptr;
}

bool ModeParams::Load(uint32_t nMotorIndex) {
	DEBUG_ENTRY

	m_aFileName[5] = static_cast<char>(nMotorIndex + '0');

	m_Params.nSetList = 0;

	ReadConfigFile configfile(ModeParams::StaticCallbackFunction, this);

	if (configfile.Read(m_aFileName)) {
		ModeParamsStore::Update(nMotorIndex, &m_Params);
	} else  {
		ModeParamsStore::Copy(nMotorIndex, &m_Params);
	}

#ifndef NDEBUG
	Dump();
#endif
	DEBUG_EXIT
	return true;
}

void ModeParams::Load(uint32_t nMotorIndex, const char *pBuffer, uint32_t nLength) {
	DEBUG_ENTRY

	assert(pBuffer != nullptr);
	assert(nLength != 0);

	m_Params.nSetList = 0;

	ReadConfigFile config(ModeParams::StaticCallbackFunction, this);

	config.Read(pBuffer, nLength);

	ModeParamsStore::Update(nMotorIndex, &m_Params);

#ifndef NDEBUG
	Dump();
#endif
	DEBUG_EXIT
}

void ModeParams::callbackFunction(const char *pLine) {
	assert(pLine != nullptr);

	float fValue;
	char value[128];
	uint32_t nLength;
	uint8_t nValue8;
	uint16_t nValue16;
	uint32_t nValue32;

	if (Sscan::Uint8(pLine, ModeParamsConst::DMX_MODE, nValue8) == Sscan::OK) {
		if (nValue8 < L6470DMXMODE_UNDEFINED) {
			m_Params.nDmxMode = nValue8;
			m_Params.nSetList |= modeparams::Mask::DMX_MODE;
		}
		return;
	}

	if (Sscan::Uint16(pLine, LightSetParamsConst::DMX_START_ADDRESS, nValue16) == Sscan::OK) {
		if ((nValue16 != 0) && (nValue16 <= dmx::UNIVERSE_SIZE)) {
			m_Params.nDmxStartAddress = nValue16;
			m_Params.nSetList |= modeparams::Mask::DMX_START_ADDRESS;
		}
		return;
	}

	nLength = sizeof(value) - 1;
	if (Sscan::Char(pLine, LightSetParamsConst::DMX_SLOT_INFO, value, nLength) == Sscan::OK) {
		value[nLength] = '\0';
		uint32_t nMask = 0;
		m_pDmxSlotInfo->FromString(value, nMask);
		m_Params.nSetList |= (nMask << modeparams::Mask::SLOT_INFO_SHIFT);
	}

	if (Sscan::Uint32(pLine, ModeParamsConst::MAX_STEPS, nValue32) == Sscan::OK) {
		m_Params.nMaxSteps = nValue32;
		m_Params.nSetList |= modeparams::Mask::MAX_STEPS;
		return;
	}

	nLength = 5; //  copy, reset
	if (Sscan::Char(pLine, ModeParamsConst::SWITCH_ACT, value, nLength) == Sscan::OK) {
		if (nLength == 4) {
			if (memcmp(value, "copy", 4) == 0) {
				m_Params.tSwitchAction = L6470_ABSPOS_COPY;
				m_Params.nSetList |= modeparams::Mask::SWITCH_ACT;
				return;
			}
		}
		if (nLength == 5) {
			if (memcmp(value, "reset", 5) == 0) {
				m_Params.tSwitchAction = L6470_ABSPOS_RESET;
				m_Params.nSetList |= modeparams::Mask::SWITCH_ACT;
				return;
			}
		}
	}

	nLength = 7; //  reverse, forward
	if (Sscan::Char(pLine, ModeParamsConst::SWITCH_DIR, value, nLength) == Sscan::OK) {
		if (nLength != 7) {
			return;
		}
		if (memcmp(value, "forward", 7) == 0) {
			m_Params.tSwitchDir = L6470_DIR_FWD;
			m_Params.nSetList |= modeparams::Mask::SWITCH_DIR;
			return;
		}
		if (memcmp(value, "reverse", 7) == 0) {
			m_Params.tSwitchDir = L6470_DIR_REV;
			m_Params.nSetList |= modeparams::Mask::SWITCH_DIR;
			return;
		}
	}

	if (Sscan::Float(pLine, ModeParamsConst::SWITCH_SPS, fValue) == Sscan::OK) {
		m_Params.fSwitchStepsPerSec = fValue;
		m_Params.nSetList |= modeparams::Mask::SWITCH_SPS;
		return;
	}

	if (Sscan::Uint8(pLine, ModeParamsConst::SWITCH, nValue8) == Sscan::OK) {
		if (nValue8 == 0) {
			m_Params.bSwitch = false;
			m_Params.nSetList |= modeparams::Mask::SWITCH;
		}
	}
}

void ModeParams::Builder(uint32_t nMotorIndex, const struct modeparams::Params *ptModeParams, char *pBuffer, uint32_t nLength, uint32_t& nSize) {
	DEBUG_ENTRY

	assert(pBuffer != nullptr);

	m_aFileName[5] = static_cast<char>(nMotorIndex + '0');

	if (ptModeParams != nullptr) {
		memcpy(&m_Params, ptModeParams, sizeof(struct modeparams::Params));
	} else {
		debug_dump( &m_Params, sizeof(struct modeparams::Params));
		ModeParamsStore::Copy(nMotorIndex, &m_Params);
		debug_dump( &m_Params, sizeof(struct modeparams::Params));
	}

	PropertiesBuilder builder(m_aFileName, pBuffer, nLength);

	builder.Add(ModeParamsConst::DMX_MODE, m_Params.nDmxMode, isMaskSet(modeparams::Mask::DMX_MODE));
	builder.Add(LightSetParamsConst::DMX_START_ADDRESS, m_Params.nDmxStartAddress, isMaskSet(modeparams::Mask::DMX_START_ADDRESS));

	const uint32_t nMask = (m_Params.nSetList >> modeparams::Mask::SLOT_INFO_SHIFT);
	builder.Add(LightSetParamsConst::DMX_SLOT_INFO, m_pDmxSlotInfo->ToString(nMask), nMask != 0);

	builder.Add(ModeParamsConst::MAX_STEPS, m_Params.nMaxSteps, isMaskSet(modeparams::Mask::MAX_STEPS));

	builder.Add(ModeParamsConst::SWITCH_ACT, m_Params.tSwitchAction == L6470_ABSPOS_COPY ? "copy" : "reset", isMaskSet(modeparams::Mask::SWITCH_ACT));
	builder.Add(ModeParamsConst::SWITCH_DIR, m_Params.tSwitchDir == L6470_DIR_FWD ? "forward" : "reverse", isMaskSet(modeparams::Mask::SWITCH_DIR));
	builder.Add(ModeParamsConst::SWITCH_SPS, m_Params.fSwitchStepsPerSec, isMaskSet(modeparams::Mask::SWITCH_SPS));
	builder.Add(ModeParamsConst::SWITCH, m_Params.bSwitch, isMaskSet(modeparams::Mask::SWITCH));

	nSize = builder.GetSize();

	DEBUG_EXIT
}

void ModeParams::Save(uint32_t nMotorIndex, char *pBuffer, uint32_t nLength, uint32_t& nSize) {
	DEBUG_ENTRY
	Builder(nMotorIndex, nullptr, pBuffer, nLength, nSize);
	DEBUG_EXIT
}

void ModeParams::Dump() {
	if (isMaskSet(modeparams::Mask::DMX_MODE)) {
		printf(" %s=%d\n", ModeParamsConst::DMX_MODE, m_Params.nDmxMode);
	}

	if (isMaskSet(modeparams::Mask::DMX_START_ADDRESS)) {
		printf(" %s=%d\n", LightSetParamsConst::DMX_START_ADDRESS, m_Params.nDmxStartAddress);
	}

	for (uint32_t i = 0; i < modeparams::MODE_PARAMS_MAX_DMX_FOOTPRINT; i++) {
		printf(" SlotInfo\n");
		printf("  Slot:%d %2x:%4x\n", i, m_Params.tLightSetSlotInfo[i].nType, m_Params.tLightSetSlotInfo[i].nCategory);
	}

	if (isMaskSet(modeparams::Mask::MAX_STEPS)) {
		printf(" %s=%d steps\n", ModeParamsConst::MAX_STEPS, m_Params.nMaxSteps);
	}

	if(isMaskSet(modeparams::Mask::SWITCH_ACT)) {
		printf(" %s=%s\n", ModeParamsConst::SWITCH_ACT, m_Params.tSwitchAction == L6470_ABSPOS_RESET ? "reset" : "copy");

	}

	if(isMaskSet(modeparams::Mask::SWITCH_DIR)) {
		printf(" %s=%s\n", ModeParamsConst::SWITCH_DIR, m_Params.tSwitchDir == L6470_DIR_REV ? "reverse" : "forward");
	}

	if(isMaskSet(modeparams::Mask::SWITCH_SPS)) {
		printf(" %s=%f step/s\n", ModeParamsConst::SWITCH_SPS, m_Params.fSwitchStepsPerSec);
	}

	if(isMaskSet(modeparams::Mask::SWITCH)) {
		printf(" %s={%s}\n", ModeParamsConst::SWITCH, m_Params.bSwitch ? "Yes": "No");
	}
}

void ModeParams::GetSlotInfo(uint32_t nOffset, SlotInfo &tLightSetSlotInfo) {
	if (nOffset < modeparams::MODE_PARAMS_MAX_DMX_FOOTPRINT) {
		tLightSetSlotInfo.nType = m_Params.tLightSetSlotInfo[nOffset].nType;
		tLightSetSlotInfo.nCategory = m_Params.tLightSetSlotInfo[nOffset].nCategory;
		return;
	}

	tLightSetSlotInfo.nType = 0x00;			// ST_PRIMARY
	tLightSetSlotInfo.nCategory = 0xFFFF;	// SD_UNDEFINED
}

void ModeParams::StaticCallbackFunction(void *p, const char *s) {
	assert(p != nullptr);
	assert(s != nullptr);

	(static_cast<ModeParams*>(p))->callbackFunction(s);
}
