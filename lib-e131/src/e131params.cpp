/**
 * @file e131params.cpp
 *
 */
/* Copyright (C) 2016-2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <climits>
#include <algorithm>
#include <cassert>

#include "e131params.h"
#include "e131paramsconst.h"
#include "e131.h"

#include "readconfigfile.h"
#include "sscan.h"

#include "propertiesbuilder.h"

#include "lightsetparamsconst.h"

#include "debug.h"

using namespace e131;
using namespace e131params;

E131Params::E131Params(E131ParamsStore *pE131ParamsStore):m_pE131ParamsStore(pE131ParamsStore) {
	DEBUG_PRINTF("sizeof(struct Params)=%d", static_cast<int>(sizeof(struct Params)));

	memset(&m_Params, 0, sizeof(struct Params));
	
#if defined (OUTPUT_DMX_ARTNET)
	m_Params.nUniverse = universe::DEFAULT;
#endif

	for (uint32_t i = 0; i < e131params::MAX_PORTS; i++) {
		m_Params.nUniversePort[i] = static_cast<uint16_t>(i + 1);
		m_Params.nPriority[i] = priority::DEFAULT;
	}

	for (uint32_t i = 0; i < CHAR_BIT; i++) {
		const uint32_t n = static_cast<uint32_t>(lightset::PortDir::OUTPUT) & 0x1;
		m_Params.nDirection |= static_cast<uint8_t>(n << i);
	}
}

bool E131Params::Load() {
	DEBUG_ENTRY

	m_Params.nSetList = 0;

#if !defined(DISABLE_FS)
	ReadConfigFile configfile(E131Params::staticCallbackFunction, this);

	if (configfile.Read(E131ParamsConst::FILE_NAME)) {
		if (m_pE131ParamsStore != nullptr) {
			m_pE131ParamsStore->Update(&m_Params);
		}
	} else
#endif
	if (m_pE131ParamsStore != nullptr) {
		m_pE131ParamsStore->Copy(&m_Params);
	} else {
		DEBUG_EXIT
		return false;
	}

	DEBUG_EXIT
	return true;
}

void E131Params::Load(const char* pBuffer, uint32_t nLength) {
	DEBUG_ENTRY

	assert(pBuffer != nullptr);
	assert(nLength != 0);

	assert(m_pE131ParamsStore != nullptr);

	if (m_pE131ParamsStore == nullptr) {
		return;
	}

	m_Params.nSetList = 0;

	ReadConfigFile config(E131Params::staticCallbackFunction, this);

	config.Read(pBuffer, nLength);

	m_pE131ParamsStore->Update(&m_Params);

	DEBUG_EXIT
}

void E131Params::callbackFunction(const char *pLine) {
	assert(pLine != nullptr);

	uint8_t value8;
	uint16_t value16;
	uint32_t nLength;
	char value[16];

	const auto nPorts = static_cast<uint32_t>(std::min(e131params::MAX_PORTS, E131::PORTS));

	for (uint32_t i = 0; i < nPorts; i++) {
		if (Sscan::Uint16(pLine, LightSetParamsConst::UNIVERSE_PORT[i], value16) == Sscan::OK) {
			if ((value16 == 0) || (value16 > universe::MAX)) {
				m_Params.nUniversePort[i] = static_cast<uint16_t>(i + 1);
				m_Params.nSetList &= ~(Mask::UNIVERSE_A << i);
			} else {
				m_Params.nUniversePort[i] = value16;
				m_Params.nSetList |= (Mask::UNIVERSE_A << i);
			}
			return;
		}

		nLength = 3;

		if (Sscan::Char(pLine, LightSetParamsConst::MERGE_MODE_PORT[i], value, nLength) == Sscan::OK) {
			if (lightset::get_merge_mode(value) == lightset::MergeMode::LTP) {
				m_Params.nMergeModePort[i] = static_cast<uint8_t>(lightset::MergeMode::LTP);
				m_Params.nSetList |= (Mask::MERGE_MODE_A << i);
			} else {
				m_Params.nMergeModePort[i] = static_cast<uint8_t>(lightset::MergeMode::HTP);
				m_Params.nSetList &= ~(Mask::MERGE_MODE_A << i);
			}
			return;
		}

		if (i < CHAR_BIT) {
			nLength = 5;

			if (Sscan::Char(pLine, LightSetParamsConst::DIRECTION[i], value, nLength) == Sscan::OK) {
				m_Params.nDirection &= static_cast<uint8_t>(~(1U << i));

				if (lightset::get_direction(value) == lightset::PortDir::INPUT) {
					m_Params.nDirection |= static_cast<uint8_t>((static_cast<uint8_t>(lightset::PortDir::INPUT) & 0x1) << i);
				} else {
					m_Params.nDirection |= static_cast<uint8_t>((static_cast<uint8_t>(lightset::PortDir::OUTPUT) & 0x1) << i);
				}
				return;
			}
		}

		if (Sscan::Uint8(pLine, E131ParamsConst::PRIORITY[i], value8) == Sscan::OK) {
			if ((value8 >= priority::LOWEST) && (value8 <= priority::HIGHEST) && (value8 != priority::DEFAULT)) {
				m_Params.nPriority[i] = value8;
				m_Params.nSetList |= (Mask::PRIORITY_A << i);
			} else {
				m_Params.nPriority[i] = priority::DEFAULT;
				m_Params.nSetList &= ~(Mask::PRIORITY_A << i);
			}
			return;
		}
	}

	if (Sscan::Uint8(pLine, E131ParamsConst::DISABLE_NETWORK_DATA_LOSS_TIMEOUT, value8) == Sscan::OK) {
		if (value8 != 0) {
			m_Params.nSetList |= Mask::DISABLE_NETWORK_DATA_LOSS_TIMEOUT;
		} else {
			m_Params.nSetList &= ~Mask::DISABLE_NETWORK_DATA_LOSS_TIMEOUT;
		}
		return;
	}

	if (Sscan::Uint8(pLine, E131ParamsConst::DISABLE_MERGE_TIMEOUT, value8) == Sscan::OK) {
		if (value8 != 0) {
			m_Params.nSetList |= Mask::DISABLE_MERGE_TIMEOUT;
		} else {
			m_Params.nSetList &= ~Mask::DISABLE_MERGE_TIMEOUT;
		}
		return;
	}

#if defined (OUTPUT_DMX_ARTNET)
	if (Sscan::Uint16(pLine, LightSetParamsConst::UNIVERSE, value16) == Sscan::OK) {
		if ((value16 == 0) || (value16 > universe::MAX)) {
			m_Params.nUniverse = universe::DEFAULT;
			m_Params.nSetList &= ~Mask::UNIVERSE;
		} else {
			m_Params.nUniverse = value16;
			m_Params.nSetList |= Mask::UNIVERSE;
		}
		return;
	}

	if (Sscan::Char(pLine, LightSetParamsConst::MERGE_MODE, value, nLength) == Sscan::OK) {
		if (lightset::get_merge_mode(value) == lightset::MergeMode::LTP) {
			m_Params.nMergeMode = static_cast<uint8_t>(lightset::MergeMode::LTP);
			m_Params.nSetList |= Mask::MERGE_MODE;
		} else {
			m_Params.nMergeMode = static_cast<uint8_t>(lightset::MergeMode::HTP);
			m_Params.nSetList &= ~Mask::MERGE_MODE;
		}
		return;
	}
#endif
}

void E131Params::staticCallbackFunction(void *p, const char *s) {
	assert(p != nullptr);
	assert(s != nullptr);

	(static_cast<E131Params*>(p))->callbackFunction(s);
}

void E131Params::Builder(const struct Params *ptE131Params, char *pBuffer, uint32_t nLength, uint32_t& nSize) {
	DEBUG_ENTRY

	if (ptE131Params != nullptr) {
		memcpy(&m_Params, ptE131Params, sizeof(struct Params));
	} else {
		m_pE131ParamsStore->Copy(&m_Params);
	}

	PropertiesBuilder builder(E131ParamsConst::FILE_NAME, pBuffer, nLength);

	const auto nPorts = static_cast<uint32_t>(std::min(e131params::MAX_PORTS, E131::PORTS));

	for (uint32_t i = 0; i < nPorts; i++) {
		const auto isDefault = (((m_Params.nDirection >> i) & 0x1)  == static_cast<uint8_t>(lightset::PortDir::OUTPUT));
		builder.Add(LightSetParamsConst::DIRECTION[i], lightset::get_direction(i, m_Params.nDirection), !isDefault);
	}

#if defined (OUTPUT_DMX_ARTNET)
	builder.Add(LightSetParamsConst::UNIVERSE, m_Params.nUniverse, isMaskSet(Mask::UNIVERSE));
#endif

	for (uint32_t i = 0; i < nPorts; i++) {
		builder.Add(LightSetParamsConst::UNIVERSE_PORT[i], m_Params.nUniversePort[i], isMaskSet(Mask::UNIVERSE_A << i));
	}

	builder.AddComment("DMX Output");

#if defined (OUTPUT_DMX_ARTNET)
	builder.Add(LightSetParamsConst::MERGE_MODE, lightset::get_merge_mode(m_Params.nMergeMode), isMaskSet(Mask::MERGE_MODE));
#endif

	for (uint32_t i = 0; i < nPorts; i++) {
		builder.Add(LightSetParamsConst::MERGE_MODE_PORT[i], lightset::get_merge_mode(m_Params.nMergeModePort[i]), isMaskSet(Mask::MERGE_MODE_A << i));
	}

	builder.AddComment("DMX Input");
	for (uint32_t i = 0; i < nPorts; i++) {
		builder.Add(E131ParamsConst::PRIORITY[i], m_Params.nPriority[i], isMaskSet(Mask::PRIORITY_A << i));
	}

	builder.AddComment("Other");
	builder.Add(E131ParamsConst::DISABLE_NETWORK_DATA_LOSS_TIMEOUT, isMaskSet(Mask::DISABLE_NETWORK_DATA_LOSS_TIMEOUT));
	builder.Add(E131ParamsConst::DISABLE_MERGE_TIMEOUT, isMaskSet(Mask::DISABLE_MERGE_TIMEOUT));

	nSize = builder.GetSize();

	DEBUG_PRINTF("nSize=%d", nSize);
	DEBUG_EXIT
}

void E131Params::Save(char *pBuffer, uint32_t nLength, uint32_t& nSize) {
	DEBUG_ENTRY

	if (m_pE131ParamsStore == nullptr) {
		nSize = 0;
		DEBUG_EXIT
		return;
	}

	Builder(nullptr, pBuffer, nLength, nSize);
}

void E131Params::Set(E131Bridge *pE131Bridge) {
	assert(pE131Bridge != nullptr);

	if (m_Params.nSetList == 0) {
		return;
	}

	const auto nPorts = static_cast<uint32_t>(std::min(e131params::MAX_PORTS, E131::PORTS));

	for (uint32_t i = 0; i < nPorts; i++) {
		if (isMaskSet(Mask::MERGE_MODE_A << i)) {
			pE131Bridge->SetMergeMode(i, static_cast<lightset::MergeMode>(m_Params.nMergeModePort[i]));
		}
#if defined (OUTPUT_DMX_ARTNET)
		else {
			pE131Bridge->SetMergeMode(i, static_cast<lightset::MergeMode>(m_Params.nMergeMode));
		}
#endif
		if (isMaskSet(Mask::PRIORITY_A << i)) {
			pE131Bridge->SetPriority(m_Params.nPriority[i]);
		}
	}

	if (isMaskSet(Mask::DISABLE_NETWORK_DATA_LOSS_TIMEOUT)) {
		pE131Bridge->SetDisableNetworkDataLossTimeout(true);
	}

	if (isMaskSet(Mask::DISABLE_MERGE_TIMEOUT)) {
		pE131Bridge->SetDisableMergeTimeout(true);
	}
}
