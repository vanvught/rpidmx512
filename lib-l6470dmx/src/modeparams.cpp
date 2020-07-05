/**
 * @file modeparams.cpp
 *
 */
/* Copyright (C) 2017-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "modeparams.h"
#include "modeparamsconst.h"

#include "l6470dmxmode.h"
#include "l6470dmxconst.h"

#include "lightsetconst.h"

#include "dmxslotinfo.h"

#include "readconfigfile.h"
#include "sscan.h"
#include "parse.h"
#include "propertiesbuilder.h"

#include "debug.h"

ModeParams::ModeParams(ModeParamsStore *pModeParamsStore): m_pModeParamsStore(pModeParamsStore) {
	DEBUG_ENTRY

	memset(&m_tModeParams, 0, sizeof(struct TModeParams));
	m_tModeParams.nDmxMode = L6470DMXMODE_UNDEFINED;
	m_tModeParams.nDmxStartAddress = DMX_ADDRESS_INVALID;
	m_tModeParams.tSwitchAction = L6470_ABSPOS_RESET;
	m_tModeParams.tSwitchDir = L6470_DIR_REV;
	m_tModeParams.bSwitch = true;

	m_pDmxSlotInfo = new DmxSlotInfo(m_tModeParams.tLightSetSlotInfo, MODE_PARAMS_MAX_DMX_FOOTPRINT);
	assert(m_pDmxSlotInfo != nullptr);

	assert(sizeof(m_aFileName) > strlen(L6470DmxConst::FILE_NAME_MOTOR));
	strncpy(m_aFileName, L6470DmxConst::FILE_NAME_MOTOR, sizeof(m_aFileName));

	DEBUG_EXIT
}

ModeParams::~ModeParams() {
	delete m_pDmxSlotInfo;
	m_pDmxSlotInfo = nullptr;
}

bool ModeParams::Load(uint8_t nMotorIndex) {
	DEBUG_ENTRY

	m_aFileName[5] = nMotorIndex + '0';

	m_tModeParams.nSetList = 0;

	ReadConfigFile configfile(ModeParams::staticCallbackFunction, this);

	if (configfile.Read(m_aFileName)) {
		// There is a configuration file
		if (m_pModeParamsStore != nullptr) {
			m_pModeParamsStore->Update(nMotorIndex, &m_tModeParams);
		}
	} else if (m_pModeParamsStore != nullptr) {
		m_pModeParamsStore->Copy(nMotorIndex, &m_tModeParams);
	} else {
		DEBUG_EXIT
		return false;
	}

	DEBUG_EXIT
	return true;
}

void ModeParams::Load(uint8_t nMotorIndex, const char *pBuffer, uint32_t nLength) {
	DEBUG_ENTRY

	assert(pBuffer != nullptr);
	assert(nLength != 0);
	assert(m_pModeParamsStore != nullptr);

	if (m_pModeParamsStore == nullptr) {
		DEBUG_EXIT
		return;
	}

	m_tModeParams.nSetList = 0;

	ReadConfigFile config(ModeParams::staticCallbackFunction, this);

	config.Read(pBuffer, nLength);

	m_pModeParamsStore->Update(nMotorIndex, &m_tModeParams);

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
			m_tModeParams.nDmxMode = nValue8;
			m_tModeParams.nSetList |= ModeParamsMask::DMX_MODE;
		}
		return;
	}

	if (Sscan::Uint16(pLine, LightSetConst::PARAMS_DMX_START_ADDRESS, nValue16) == Sscan::OK) {
		if ((nValue16 != 0) && (nValue16 <= DMX_UNIVERSE_SIZE)) {
			m_tModeParams.nDmxStartAddress = nValue16;
			m_tModeParams.nSetList |= ModeParamsMask::DMX_START_ADDRESS;
		}
		return;
	}

	nLength = sizeof(value) - 1;
	if (Sscan::Char(pLine, LightSetConst::PARAMS_DMX_SLOT_INFO, value, nLength) == Sscan::OK) {
		value[nLength] = '\0';
		uint32_t nMask = 0;
		m_pDmxSlotInfo->FromString(value, nMask);
		m_tModeParams.nSetList |= (nMask << ModeParamsMask::SLOT_INFO_SHIFT);
	}

	if (Sscan::Uint32(pLine, ModeParamsConst::MAX_STEPS, nValue32) == Sscan::OK) {
		m_tModeParams.nMaxSteps = nValue32;
		m_tModeParams.nSetList |= ModeParamsMask::MAX_STEPS;
		return;
	}

	nLength = 5; //  copy, reset
	if (Sscan::Char(pLine, ModeParamsConst::SWITCH_ACT, value, nLength) == Sscan::OK) {
		if (nLength == 4) {
			if (memcmp(value, "copy", 4) == 0) {
				m_tModeParams.tSwitchAction = L6470_ABSPOS_COPY;
				m_tModeParams.nSetList |= ModeParamsMask::SWITCH_ACT;
				return;
			}
		}
		if (nLength == 5) {
			if (memcmp(value, "reset", 5) == 0) {
				m_tModeParams.tSwitchAction = L6470_ABSPOS_RESET;
				m_tModeParams.nSetList |= ModeParamsMask::SWITCH_ACT;
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
			m_tModeParams.tSwitchDir = L6470_DIR_FWD;
			m_tModeParams.nSetList |= ModeParamsMask::SWITCH_DIR;
			return;
		}
		if (memcmp(value, "reverse", 7) == 0) {
			m_tModeParams.tSwitchDir = L6470_DIR_REV;
			m_tModeParams.nSetList |= ModeParamsMask::SWITCH_DIR;
			return;
		}
	}

	if (Sscan::Float(pLine, ModeParamsConst::SWITCH_SPS, fValue) == Sscan::OK) {
		m_tModeParams.fSwitchStepsPerSec = fValue;
		m_tModeParams.nSetList |= ModeParamsMask::SWITCH_SPS;
		return;
	}

	if (Sscan::Uint8(pLine, ModeParamsConst::SWITCH, nValue8) == Sscan::OK) {
		if (nValue8 == 0) {
			m_tModeParams.bSwitch = false;
			m_tModeParams.nSetList |= ModeParamsMask::SWITCH;
		}
	}
}

void ModeParams::Builder(uint8_t nMotorIndex, const struct TModeParams *ptModeParams, char *pBuffer, uint32_t nLength, uint32_t &nSize) {
	DEBUG1_ENTRY

	assert(pBuffer != nullptr);

	m_aFileName[5] = nMotorIndex + '0';

	if (ptModeParams != nullptr) {
		memcpy(&m_tModeParams, ptModeParams, sizeof(struct TModeParams));
	} else {
		debug_dump( &m_tModeParams, sizeof(struct TModeParams));
		m_pModeParamsStore->Copy(nMotorIndex, &m_tModeParams);
		debug_dump( &m_tModeParams, sizeof(struct TModeParams));
	}

	PropertiesBuilder builder(m_aFileName, pBuffer, nLength);

	builder.Add(ModeParamsConst::DMX_MODE, m_tModeParams.nDmxMode, isMaskSet(ModeParamsMask::DMX_MODE));
	builder.Add(LightSetConst::PARAMS_DMX_START_ADDRESS, m_tModeParams.nDmxStartAddress, isMaskSet(ModeParamsMask::DMX_START_ADDRESS));

	const uint32_t nMask = (m_tModeParams.nSetList >> ModeParamsMask::SLOT_INFO_SHIFT);
	builder.Add(LightSetConst::PARAMS_DMX_SLOT_INFO, m_pDmxSlotInfo->ToString(nMask), nMask != 0);

	builder.Add(ModeParamsConst::MAX_STEPS, m_tModeParams.nMaxSteps, isMaskSet(ModeParamsMask::MAX_STEPS));

	builder.Add(ModeParamsConst::SWITCH_ACT, m_tModeParams.tSwitchAction == L6470_ABSPOS_COPY ? "copy" : "reset", isMaskSet(ModeParamsMask::SWITCH_ACT));
	builder.Add(ModeParamsConst::SWITCH_DIR, m_tModeParams.tSwitchDir == L6470_DIR_FWD ? "forward" : "reverse", isMaskSet(ModeParamsMask::SWITCH_DIR));
	builder.Add(ModeParamsConst::SWITCH_SPS, m_tModeParams.fSwitchStepsPerSec, isMaskSet(ModeParamsMask::SWITCH_SPS));
	builder.Add(ModeParamsConst::SWITCH, m_tModeParams.bSwitch, isMaskSet(ModeParamsMask::SWITCH));

	nSize = builder.GetSize();

	DEBUG1_EXIT
}

void ModeParams::Save(uint8_t nMotorIndex, char *pBuffer, uint32_t nLength, uint32_t &nSize) {
	DEBUG_ENTRY

	if (m_pModeParamsStore == nullptr) {
		nSize = 0;
		DEBUG_EXIT
		return;
	}

	Builder(nMotorIndex, nullptr, pBuffer, nLength, nSize);
	DEBUG_EXIT
}

void ModeParams::Dump() {
#ifndef NDEBUG
	if (m_tModeParams.nSetList == 0) {
		return;
	}

	if (isMaskSet(ModeParamsMask::DMX_MODE)) {
		printf(" %s=%d\n", ModeParamsConst::DMX_MODE, m_tModeParams.nDmxMode);
	}

	if (isMaskSet(ModeParamsMask::DMX_START_ADDRESS)) {
		printf(" %s=%d\n", LightSetConst::PARAMS_DMX_START_ADDRESS, m_tModeParams.nDmxStartAddress);
	}

	for (uint32_t i = 0; i < MODE_PARAMS_MAX_DMX_FOOTPRINT; i++) {
		printf(" SlotInfo\n");
		printf("  Slot:%d %2x:%4x\n", i, m_tModeParams.tLightSetSlotInfo[i].nType, m_tModeParams.tLightSetSlotInfo[i].nCategory);
	}

	if (isMaskSet(ModeParamsMask::MAX_STEPS)) {
		printf(" %s=%d steps\n", ModeParamsConst::MAX_STEPS, m_tModeParams.nMaxSteps);
	}

	if(isMaskSet(ModeParamsMask::SWITCH_ACT)) {
		printf(" %s=%s\n", ModeParamsConst::SWITCH_ACT, m_tModeParams.tSwitchAction == L6470_ABSPOS_RESET ? "reset" : "copy");

	}

	if(isMaskSet(ModeParamsMask::SWITCH_DIR)) {
		printf(" %s=%s\n", ModeParamsConst::SWITCH_DIR, m_tModeParams.tSwitchDir == L6470_DIR_REV ? "reverse" : "forward");
	}

	if(isMaskSet(ModeParamsMask::SWITCH_SPS)) {
		printf(" %s=%f step/s\n", ModeParamsConst::SWITCH_SPS, m_tModeParams.fSwitchStepsPerSec);
	}

	if(isMaskSet(ModeParamsMask::SWITCH)) {
		printf(" %s={%s}\n", ModeParamsConst::SWITCH, m_tModeParams.bSwitch ? "Yes": "No");
	}
#endif
}

void ModeParams::GetSlotInfo(uint32_t nOffset, struct TLightSetSlotInfo &tLightSetSlotInfo) {
	if (nOffset < MODE_PARAMS_MAX_DMX_FOOTPRINT) {
		tLightSetSlotInfo.nType = m_tModeParams.tLightSetSlotInfo[nOffset].nType;
		tLightSetSlotInfo.nCategory = m_tModeParams.tLightSetSlotInfo[nOffset].nCategory;
		return;
	}

	tLightSetSlotInfo.nType = 0x00;			// ST_PRIMARY
	tLightSetSlotInfo.nCategory = 0xFFFF;	// SD_UNDEFINED
}

void ModeParams::staticCallbackFunction(void *p, const char *s) {
	assert(p != nullptr);
	assert(s != nullptr);

	(static_cast<ModeParams*>(p))->callbackFunction(s);
}
