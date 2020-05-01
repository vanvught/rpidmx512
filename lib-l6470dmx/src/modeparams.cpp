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
#include <assert.h>

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
	assert(m_pDmxSlotInfo != 0);

	assert(sizeof(m_aFileName) > strlen(L6470DmxConst::FILE_NAME_MOTOR));
	strncpy(m_aFileName, L6470DmxConst::FILE_NAME_MOTOR, sizeof(m_aFileName));

	DEBUG_EXIT
}

ModeParams::~ModeParams(void) {
	delete m_pDmxSlotInfo;
	m_pDmxSlotInfo = 0;

	m_tModeParams.nSetList = 0;
}

bool ModeParams::Load(uint8_t nMotorIndex) {
	DEBUG_ENTRY

	m_aFileName[5] = nMotorIndex + '0';

	m_tModeParams.nSetList = 0;

	ReadConfigFile configfile(ModeParams::staticCallbackFunction, this);

	if (configfile.Read(m_aFileName)) {
		// There is a configuration file
		if (m_pModeParamsStore != 0) {
			m_pModeParamsStore->Update(nMotorIndex, &m_tModeParams);
		}
	} else if (m_pModeParamsStore != 0) {
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

	assert(pBuffer != 0);
	assert(nLength != 0);
	assert(m_pModeParamsStore != 0);

	if (m_pModeParamsStore == 0) {
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
	assert(pLine != 0);

	float f;
	char value[128];
	uint8_t len;
	uint8_t value8;
	uint16_t value16;
	uint32_t value32;

	if (Sscan::Uint8(pLine, ModeParamsConst::DMX_MODE, &value8) == SSCAN_OK) {
		if (value8 < L6470DMXMODE_UNDEFINED) {
			m_tModeParams.nDmxMode = value8;
			m_tModeParams.nSetList |= MODE_PARAMS_MASK_DMX_MODE;
		}
		return;
	}

	if (Sscan::Uint16(pLine, LightSetConst::PARAMS_DMX_START_ADDRESS, &value16) == SSCAN_OK) {
		if ((value16 != 0) && (value16 <= DMX_UNIVERSE_SIZE)) {
			m_tModeParams.nDmxStartAddress = value16;
			m_tModeParams.nSetList |= MODE_PARAMS_MASK_DMX_START_ADDRESS;
		}
		return;
	}

	len = sizeof(value) - 1;
	if (Sscan::Char(pLine, LightSetConst::PARAMS_DMX_SLOT_INFO, value, &len) == SSCAN_OK) {
		value[len] = '\0';
		uint32_t nMask = 0;
		m_pDmxSlotInfo->FromString(value, nMask);
		m_tModeParams.nSetList |= (nMask << MODE_PARAMS_MASK_SLOT_INFO_SHIFT);
	}

	if (Sscan::Uint32(pLine, ModeParamsConst::MAX_STEPS, &value32) == SSCAN_OK) {
		m_tModeParams.nMaxSteps = value32;
		m_tModeParams.nSetList |= MODE_PARAMS_MASK_MAX_STEPS;
		return;
	}

	len = 5; //  copy, reset
	if (Sscan::Char(pLine, ModeParamsConst::SWITCH_ACT, value, &len) == SSCAN_OK) {
		if (len == 4) {
			if (memcmp(value, "copy", 4) == 0) {
				m_tModeParams.tSwitchAction = L6470_ABSPOS_COPY;
				m_tModeParams.nSetList |= MODE_PARAMS_MASK_SWITCH_ACT;
				return;
			}
		}
		if (len == 5) {
			if (memcmp(value, "reset", 5) == 0) {
				m_tModeParams.tSwitchAction = L6470_ABSPOS_RESET;
				m_tModeParams.nSetList |= MODE_PARAMS_MASK_SWITCH_ACT;
				return;
			}
		}
	}

	len = 7; //  reverse, forward
	if (Sscan::Char(pLine, ModeParamsConst::SWITCH_DIR, value, &len) == SSCAN_OK) {
		if (len != 7) {
			return;
		}
		if (memcmp(value, "forward", 7) == 0) {
			m_tModeParams.tSwitchDir = L6470_DIR_FWD;
			m_tModeParams.nSetList |= MODE_PARAMS_MASK_SWITCH_DIR;
			return;
		}
		if (memcmp(value, "reverse", 7) == 0) {
			m_tModeParams.tSwitchDir = L6470_DIR_REV;
			m_tModeParams.nSetList |= MODE_PARAMS_MASK_SWITCH_DIR;
			return;
		}
	}

	if (Sscan::Float(pLine, ModeParamsConst::SWITCH_SPS, &f) == SSCAN_OK) {
		m_tModeParams.fSwitchStepsPerSec = f;
		m_tModeParams.nSetList |= MODE_PARAMS_MASK_SWITCH_SPS;
		return;
	}

	if (Sscan::Uint8(pLine, ModeParamsConst::SWITCH, &value8) == SSCAN_OK) {
		if (value8 == 0) {
			m_tModeParams.bSwitch = false;
			m_tModeParams.nSetList |= MODE_PARAMS_MASK_SWITCH;
		}
	}
}

void ModeParams::Builder(uint8_t nMotorIndex, const struct TModeParams *ptModeParams, char *pBuffer, uint32_t nLength, uint32_t &nSize) {
	DEBUG1_ENTRY

	assert(pBuffer != 0);

	m_aFileName[5] = nMotorIndex + '0';

	if (ptModeParams != 0) {
		memcpy(&m_tModeParams, ptModeParams, sizeof(struct TModeParams));
	} else {
		debug_dump( &m_tModeParams, sizeof(struct TModeParams));
		m_pModeParamsStore->Copy(nMotorIndex, &m_tModeParams);
		debug_dump( &m_tModeParams, sizeof(struct TModeParams));
	}

	PropertiesBuilder builder(m_aFileName, pBuffer, nLength);

	builder.Add(ModeParamsConst::DMX_MODE, m_tModeParams.nDmxMode, isMaskSet(MODE_PARAMS_MASK_DMX_MODE));
	builder.Add(LightSetConst::PARAMS_DMX_START_ADDRESS, m_tModeParams.nDmxStartAddress, isMaskSet(MODE_PARAMS_MASK_DMX_START_ADDRESS));

	const uint32_t nMask = (m_tModeParams.nSetList >> MODE_PARAMS_MASK_SLOT_INFO_SHIFT);
	builder.Add(LightSetConst::PARAMS_DMX_SLOT_INFO, m_pDmxSlotInfo->ToString(nMask), nMask != 0);

	builder.Add(ModeParamsConst::MAX_STEPS, m_tModeParams.nMaxSteps, isMaskSet(MODE_PARAMS_MASK_MAX_STEPS));

	builder.Add(ModeParamsConst::SWITCH_ACT, m_tModeParams.tSwitchAction == L6470_ABSPOS_COPY ? "copy" : "reset", isMaskSet(MODE_PARAMS_MASK_SWITCH_ACT));
	builder.Add(ModeParamsConst::SWITCH_DIR, m_tModeParams.tSwitchDir == L6470_DIR_FWD ? "forward" : "reverse", isMaskSet(MODE_PARAMS_MASK_SWITCH_DIR));
	builder.Add(ModeParamsConst::SWITCH_SPS, m_tModeParams.fSwitchStepsPerSec, isMaskSet(MODE_PARAMS_MASK_SWITCH_SPS));
	builder.Add(ModeParamsConst::SWITCH, m_tModeParams.bSwitch, isMaskSet(MODE_PARAMS_MASK_SWITCH));

	nSize = builder.GetSize();

	DEBUG1_EXIT
}

void ModeParams::Save(uint8_t nMotorIndex, char *pBuffer, uint32_t nLength, uint32_t &nSize) {
	DEBUG_ENTRY

	if (m_pModeParamsStore == 0) {
		nSize = 0;
		DEBUG_EXIT
		return;
	}

	Builder(nMotorIndex, 0, pBuffer, nLength, nSize);
	DEBUG_EXIT
}

void ModeParams::Dump(void) {
#ifndef NDEBUG
	if (m_tModeParams.nSetList == 0) {
		return;
	}

	if (isMaskSet(MODE_PARAMS_MASK_DMX_MODE)) {
		printf(" %s=%d\n", ModeParamsConst::DMX_MODE, m_tModeParams.nDmxMode);
	}

	if (isMaskSet(MODE_PARAMS_MASK_DMX_START_ADDRESS)) {
		printf(" %s=%d\n", LightSetConst::PARAMS_DMX_START_ADDRESS, m_tModeParams.nDmxStartAddress);
	}

	for (uint32_t i = 0; i < MODE_PARAMS_MAX_DMX_FOOTPRINT; i++) {
		printf(" SlotInfo\n");
		printf("  Slot:%d %2x:%4x\n", i, m_tModeParams.tLightSetSlotInfo[i].nType, m_tModeParams.tLightSetSlotInfo[i].nCategory);
	}

	if (isMaskSet(MODE_PARAMS_MASK_MAX_STEPS)) {
		printf(" %s=%d steps\n", ModeParamsConst::MAX_STEPS, m_tModeParams.nMaxSteps);
	}

	if(isMaskSet(MODE_PARAMS_MASK_SWITCH_ACT)) {
		printf(" %s=%s\n", ModeParamsConst::SWITCH_ACT, m_tModeParams.tSwitchAction == L6470_ABSPOS_RESET ? "reset" : "copy");

	}

	if(isMaskSet(MODE_PARAMS_MASK_SWITCH_DIR)) {
		printf(" %s=%s\n", ModeParamsConst::SWITCH_DIR, m_tModeParams.tSwitchDir == L6470_DIR_REV ? "reverse" : "forward");
	}

	if(isMaskSet(MODE_PARAMS_MASK_SWITCH_SPS)) {
		printf(" %s=%f step/s\n", ModeParamsConst::SWITCH_SPS, m_tModeParams.fSwitchStepsPerSec);
	}

	if(isMaskSet(MODE_PARAMS_MASK_SWITCH)) {
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
	assert(p != 0);
	assert(s != 0);

	(static_cast<ModeParams*>(p))->callbackFunction(s);
}
