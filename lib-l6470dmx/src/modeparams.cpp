/**
 * @file modeparams.cpp
 *
 */
/* Copyright (C) 2017-2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include "modeparams.h"
#include "modeparamsconst.h"

#include "l6470dmxmode.h"
#include "l6470dmxconst.h"

#include "readconfigfile.h"
#include "sscan.h"
#include "propertiesbuilder.h"

#include "debug.h"

ModeParams::ModeParams(ModeParamsStore *pModeParamsStore): m_pModeParamsStore(pModeParamsStore) {
	uint8_t *p = (uint8_t*) &m_tModeParams;

	for (uint32_t i = 0; i < sizeof(struct TModeParams); i++) {
		*p++ = 0;
	}

	m_tModeParams.tSwitchAction = L6470_ABSPOS_RESET;
	m_tModeParams.tSwitchDir = L6470_DIR_REV;
	m_tModeParams.bSwitch = true;

	assert(sizeof(m_aFileName) > strlen(L6470DmxConst::FILE_NAME_MOTOR));
	const char *src = (char *)L6470DmxConst::FILE_NAME_MOTOR;
	strncpy(m_aFileName, src, sizeof(m_aFileName));
}

ModeParams::~ModeParams(void) {
	m_tModeParams.nSetList = 0;
}

bool ModeParams::Load(uint8_t nMotorIndex) {
	m_aFileName[5] = (char) nMotorIndex + '0';

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
		return false;
	}

	return true;
}

void ModeParams::Load(uint8_t nMotorIndex, const char *pBuffer, uint32_t nLength) {
	assert(pBuffer != 0);
	assert(nLength != 0);
	assert(m_pModeParamsStore != 0);

	if (m_pModeParamsStore == 0) {
		return;
	}

	m_tModeParams.nSetList = 0;

	ReadConfigFile config(ModeParams::staticCallbackFunction, this);

	config.Read(pBuffer, nLength);

	m_pModeParamsStore->Update(nMotorIndex, &m_tModeParams);
}

void ModeParams::callbackFunction(const char *pLine) {
	assert(pLine != 0);

	float f;
	char value[128];
	uint8_t len;
	uint8_t value8;

	if (Sscan::Uint8(pLine, ModeParamsConst::DMX_MODE, &m_tModeParams.nDmxMode) == SSCAN_OK) {
		m_tModeParams.nSetList |= MODE_PARAMS_MASK_DMX_MODE;
		return;
	}

	if (Sscan::Uint16(pLine, ModeParamsConst::DMX_START_ADDRESS, &m_tModeParams.nDmxStartAddress) == SSCAN_OK) {
		m_tModeParams.nSetList |= MODE_PARAMS_MASK_DMX_START_ADDRESS;
		return;
	}

	if (Sscan::Uint32(pLine, ModeParamsConst::MAX_STEPS, &m_tModeParams.nMaxSteps) == SSCAN_OK) {
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

void ModeParams::Builder(uint8_t nMotorIndex, const struct TModeParams *ptModeParams, uint8_t *pBuffer, uint32_t nLength, uint32_t &nSize) {
	assert(pBuffer != 0);

	m_aFileName[5] = (char) nMotorIndex + '0';

	if (ptModeParams != 0) {
		memcpy(&m_tModeParams, ptModeParams, sizeof(struct TModeParams));
	} else {
		m_pModeParamsStore->Copy(nMotorIndex, &m_tModeParams);
	}

	PropertiesBuilder builder(m_aFileName, pBuffer, nLength);

	builder.Add(ModeParamsConst::DMX_MODE, m_tModeParams.nDmxMode, isMaskSet(MODE_PARAMS_MASK_DMX_MODE));
	builder.Add(ModeParamsConst::DMX_START_ADDRESS, m_tModeParams.nDmxStartAddress, isMaskSet(MODE_PARAMS_MASK_DMX_START_ADDRESS));

	nSize = builder.GetSize();

	return;
}

void ModeParams::Save(uint8_t nMotorIndex, uint8_t *pBuffer, uint32_t nLength, uint32_t &nSize) {

	if (m_pModeParamsStore == 0) {
		nSize = 0;
		return;
	}

	Builder(nMotorIndex, 0, pBuffer, nLength, nSize);

	return;
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
		printf(" %s=%d\n", ModeParamsConst::DMX_START_ADDRESS, m_tModeParams.nDmxStartAddress);
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

void ModeParams::staticCallbackFunction(void *p, const char *s) {
	assert(p != 0);
	assert(s != 0);

	((ModeParams *) p)->callbackFunction(s);
}

bool ModeParams::isMaskSet(uint32_t nMask) const {
	return (m_tModeParams.nSetList & nMask) == nMask;
}

