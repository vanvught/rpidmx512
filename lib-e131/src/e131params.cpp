/**
 * @file e131params.cpp
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

#include <stdint.h>
#include <string.h>
#include <cassert>

#include "e131params.h"
#include "e131paramsconst.h"
#include "e131.h"

#include "readconfigfile.h"
#include "sscan.h"

#include "lightsetconst.h"

#include "debug.h"

E131Params::E131Params(E131ParamsStore *pE131ParamsStore):m_pE131ParamsStore(pE131ParamsStore) {
	memset(&m_tE131Params, 0, sizeof(struct TE131Params));

	m_tE131Params.nUniverse = E131_UNIVERSE_DEFAULT;

	for (uint32_t i = 0; i < E131_PARAMS::MAX_PORTS; i++) {
		m_tE131Params.nUniversePort[i] = i + 1;
	}

	m_tE131Params.nNetworkTimeout = E131_NETWORK_DATA_LOSS_TIMEOUT_SECONDS;
	m_tE131Params.nDirection = E131_OUTPUT_PORT;
	m_tE131Params.nPriority = E131_PRIORITY_DEFAULT;
}

bool E131Params::Load() {
	m_tE131Params.nSetList = 0;

	ReadConfigFile configfile(E131Params::staticCallbackFunction, this);

	if (configfile.Read(E131ParamsConst::FILE_NAME)) {
		// There is a configuration file
		if (m_pE131ParamsStore != nullptr) {
			m_pE131ParamsStore->Update(&m_tE131Params);
		}
	} else if (m_pE131ParamsStore != nullptr) {
		m_pE131ParamsStore->Copy(&m_tE131Params);
	} else {
		return false;
	}

	return true;
}

void E131Params::Load(const char* pBuffer, uint32_t nLength) {
	assert(pBuffer != nullptr);
	assert(nLength != 0);

	assert(m_pE131ParamsStore != nullptr);

	if (m_pE131ParamsStore == nullptr) {
		return;
	}

	m_tE131Params.nSetList = 0;

	ReadConfigFile config(E131Params::staticCallbackFunction, this);

	config.Read(pBuffer, nLength);

	m_pE131ParamsStore->Update(&m_tE131Params);
}

void E131Params::callbackFunction(const char *pLine) {
	assert(pLine != nullptr);

	char value[16];
	uint32_t nLength;
	uint8_t value8;
	uint16_t value16;
	float fValue;

	if (Sscan::Uint16(pLine, LightSetConst::PARAMS_UNIVERSE, value16) == Sscan::OK) {
		if ((value16 == 0) || (value16 > E131_UNIVERSE_MAX)) {
			m_tE131Params.nUniverse = E131_UNIVERSE_DEFAULT;
			m_tE131Params.nSetList &= ~E131ParamsMask::UNIVERSE;
		} else {
			m_tE131Params.nUniverse = value16;
			m_tE131Params.nSetList |= E131ParamsMask::UNIVERSE;
		}
		return;
	}

	nLength = 3;
	if (Sscan::Char(pLine, LightSetConst::PARAMS_MERGE_MODE, value, nLength) == Sscan::OK) {
		if (E131::GetMergeMode(value) == E131Merge::LTP) {
			m_tE131Params.nMergeMode = static_cast<uint8_t>(E131Merge::LTP);
			m_tE131Params.nSetList |= E131ParamsMask::MERGE_MODE;
			return;
		}

		m_tE131Params.nMergeMode = static_cast<uint8_t>(E131Merge::HTP);
		m_tE131Params.nSetList &= ~E131ParamsMask::MERGE_MODE;
		return;
	}

	for (uint32_t i = 0; i < E131_PARAMS::MAX_PORTS; i++) {
		if (Sscan::Uint16(pLine, LightSetConst::PARAMS_UNIVERSE_PORT[i], value16) == Sscan::OK) {
			if ((value16 == 0) || (value16 == (i + 1)) || (value16 > E131_UNIVERSE_MAX)) {
				m_tE131Params.nUniversePort[i] = i + 1;
				m_tE131Params.nSetList &= ~(E131ParamsMask::UNIVERSE_A << i);
			} else {
				m_tE131Params.nUniversePort[i] = value16;
				m_tE131Params.nSetList |= (E131ParamsMask::UNIVERSE_A << i);
			}
			return;
		}

		nLength = 3;
		if (Sscan::Char(pLine, LightSetConst::PARAMS_MERGE_MODE_PORT[i], value, nLength) == Sscan::OK) {
			if (E131::GetMergeMode(value) == E131Merge::LTP) {
				m_tE131Params.nMergeModePort[i] = static_cast<uint8_t>(E131Merge::LTP);
				m_tE131Params.nSetList |= (E131ParamsMask::MERGE_MODE_A << i);
			} else {
				m_tE131Params.nMergeModePort[i] = static_cast<uint8_t>(E131Merge::HTP);
				m_tE131Params.nSetList &= ~(E131ParamsMask::MERGE_MODE_A << i);
			}
			return;
		}
	}

	if (Sscan::Float(pLine, E131ParamsConst::NETWORK_DATA_LOSS_TIMEOUT, fValue) == Sscan::OK) {
		m_tE131Params.nNetworkTimeout = fValue;
		m_tE131Params.nSetList |= E131ParamsMask::NETWORK_TIMEOUT;
		return;
	}

	if (Sscan::Uint8(pLine, E131ParamsConst::DISABLE_MERGE_TIMEOUT, value8) == Sscan::OK) {
		m_tE131Params.bDisableMergeTimeout = (value8 != 0);
		m_tE131Params.nSetList |= E131ParamsMask::MERGE_TIMEOUT;
		return;
	}

	if (Sscan::Uint8(pLine, LightSetConst::PARAMS_ENABLE_NO_CHANGE_UPDATE, value8) == Sscan::OK) {
		m_tE131Params.bEnableNoChangeUpdate = (value8 != 0);
		m_tE131Params.nSetList |= E131ParamsMask::ENABLE_NO_CHANGE_OUTPUT;
		return;
	}

	nLength = 5;
	if (Sscan::Char(pLine, E131ParamsConst::DIRECTION, value, nLength) == Sscan::OK) {
		if (memcmp(value, "input", 5) == 0) {
			m_tE131Params.nDirection = E131_INPUT_PORT;
			m_tE131Params.nSetList |= E131ParamsMask::DIRECTION;
		}
		return;
	}

	nLength = 6;
	if (Sscan::Char(pLine, E131ParamsConst::DIRECTION, value, nLength) == Sscan::OK) {
		if (memcmp(value, "output", 6) == 0) {
			m_tE131Params.nDirection = E131_OUTPUT_PORT;
			m_tE131Params.nSetList |= E131ParamsMask::DIRECTION;
		}
		return;
	}

	if (Sscan::Uint8(pLine, E131ParamsConst::PRIORITY, value8) == Sscan::OK) {
		if ((value8 >= E131_PRIORITY_LOWEST) && (value8 <= E131_PRIORITY_HIGHEST)) {
			m_tE131Params.nPriority = value8;
			m_tE131Params.nSetList |= E131ParamsMask::PRIORITY;
		}
		return;
	}

}

uint16_t E131Params::GetUniverse(uint8_t nPort, bool &IsSet) {
	assert(nPort < E131_PARAMS::MAX_PORTS);

	IsSet = isMaskSet(E131ParamsMask::UNIVERSE_A << nPort);

	return m_tE131Params.nUniversePort[nPort];
}

void E131Params::staticCallbackFunction(void *p, const char *s) {
	assert(p != nullptr);
	assert(s != nullptr);

	(static_cast<E131Params*>(p))->callbackFunction(s);
}
