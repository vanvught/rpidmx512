/**
 * @file e131params.cpp
 *
 */
/* Copyright (C) 2016-2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "lightset.h"
#include "lightsetparamsconst.h"

#include "debug.h"

namespace e131bridge {
namespace configstore {
extern uint32_t DMXPORT_OFFSET;
}  // namespace configstore
}  // namespace e131bridge

static uint32_t s_nPortsMax;

namespace e131params {
static constexpr uint16_t portdir_shift_left(const lightset::PortDir portDir, const uint32_t i) {
	return static_cast<uint16_t>((static_cast<uint32_t>(portDir) & 0x3) << (i * 2));
}

static constexpr uint16_t portdir_clear(const uint32_t i) {
	return static_cast<uint16_t>(~(0x3 << (i * 2)));
}
}  // namespace e131params

using namespace e131params;

E131Params::E131Params() {
	DEBUG_ENTRY
	DEBUG_PRINTF("sizeof(struct Params)=%d", static_cast<int>(sizeof(struct Params)));

	memset(&m_Params, 0, sizeof(struct Params));
	
	for (uint32_t i = 0; i < e131params::MAX_PORTS; i++) {
		m_Params.nUniverse[i] = static_cast<uint16_t>(i + 1);
#if defined (E131_HAVE_DMXIN)
		m_Params.nPriority[i] = e131::priority::DEFAULT;
#endif
		constexpr auto n = static_cast<uint32_t>(lightset::PortDir::OUTPUT) & 0x3;
		m_Params.nDirection |= static_cast<uint16_t>(n << (i * 2));
	}

	m_Params.nFailSafe = static_cast<uint8_t>(lightset::FailSafe::HOLD);

	DEBUG_PRINTF("s_nPortsMax=%u", s_nPortsMax);
	DEBUG_EXIT
}

bool E131Params::Load() {
	DEBUG_ENTRY

	m_Params.nSetList = 0;

#if !defined(DISABLE_FS)
	ReadConfigFile configfile(E131Params::staticCallbackFunction, this);

	if (configfile.Read(E131ParamsConst::FILE_NAME)) {
		E131ParamsStore::Update(&m_Params);
	} else
#endif
		E131ParamsStore::Copy(&m_Params);

#ifndef NDEBUG
	Dump();
#endif
	DEBUG_EXIT
	return true;
}

void E131Params::Load(const char* pBuffer, uint32_t nLength) {
	DEBUG_ENTRY

	assert(pBuffer != nullptr);
	assert(nLength != 0);

	m_Params.nSetList = 0;

	ReadConfigFile config(E131Params::staticCallbackFunction, this);

	config.Read(pBuffer, nLength);

	E131ParamsStore::Update(&m_Params);

#ifndef NDEBUG
	Dump();
#endif
	DEBUG_EXIT
}

void E131Params::callbackFunction(const char *pLine) {
	assert(pLine != nullptr);

	uint8_t value8;
	uint16_t value16;
	char aValue[lightset::node::LABEL_NAME_LENGTH];

	uint32_t nLength = 8;

	if (Sscan::Char(pLine, LightSetParamsConst::FAILSAFE, aValue, nLength) == Sscan::OK) {
		const auto failsafe = lightset::get_failsafe(aValue);

		if (failsafe == lightset::FailSafe::HOLD) {
			m_Params.nSetList &= ~Mask::FAILSAFE;
		} else {
			m_Params.nSetList |= Mask::FAILSAFE;
		}

		m_Params.nFailSafe = static_cast<uint8_t>(failsafe);
		return;
	}

	for (uint32_t nPortIndex = 0; nPortIndex < e131params::MAX_PORTS; nPortIndex++) {
		if (Sscan::Uint16(pLine, LightSetParamsConst::UNIVERSE_PORT[nPortIndex], value16) == Sscan::OK) {
			if ((value16 == 0) || (value16 > e131::universe::MAX)) {
				m_Params.nUniverse[nPortIndex] = static_cast<uint16_t>(nPortIndex + 1);
				m_Params.nSetList &= ~(Mask::UNIVERSE_A << nPortIndex);
			} else {
				m_Params.nUniverse[nPortIndex] = value16;
				m_Params.nSetList |= (Mask::UNIVERSE_A << nPortIndex);
			}
			return;
		}

		nLength = 3;

		if (Sscan::Char(pLine, LightSetParamsConst::MERGE_MODE_PORT[nPortIndex], aValue, nLength) == Sscan::OK) {
			m_Params.nMergeMode &= e131params::mergemode_clear(nPortIndex);
			m_Params.nMergeMode |= mergemode_set(nPortIndex, lightset::get_merge_mode(aValue));
			return;
		}

		nLength = lightset::node::LABEL_NAME_LENGTH - 1;

		if (Sscan::Char(pLine, LightSetParamsConst::NODE_LABEL[nPortIndex], reinterpret_cast<char*>(m_Params.aLabel[nPortIndex]), nLength) == Sscan::OK) {
			m_Params.aLabel[nPortIndex][nLength] = '\0';
			static_assert(sizeof(aValue) >= lightset::node::LABEL_NAME_LENGTH, "");
			lightset::node::get_short_name_default(nPortIndex, aValue);

			if (strcmp(reinterpret_cast<char*>(m_Params.aLabel[nPortIndex]), aValue) == 0) {
				m_Params.nSetList &= ~(Mask::LABEL_A << nPortIndex);
			} else {
				m_Params.nSetList |= (Mask::LABEL_A << nPortIndex);
			}
			return;
		}

		nLength = 7;

		if (Sscan::Char(pLine, LightSetParamsConst::DIRECTION[nPortIndex], aValue, nLength) == Sscan::OK) {
			const auto portDir = lightset::get_direction(aValue);
			m_Params.nDirection &= e131params::portdir_clear(nPortIndex);

			DEBUG_PRINTF("%u portDir=%u, m_Params.nDirection=%x", nPortIndex, static_cast<uint32_t>(portDir), m_Params.nDirection);

#if defined (E131_HAVE_DMXIN)
			if (portDir == lightset::PortDir::INPUT) {
				m_Params.nDirection |= e131params::portdir_shift_left(lightset::PortDir::INPUT, nPortIndex);
			} else
#endif
			if (portDir == lightset::PortDir::DISABLE) {
				m_Params.nDirection |= e131params::portdir_shift_left(lightset::PortDir::DISABLE, nPortIndex);
			} else {
				m_Params.nDirection |= e131params::portdir_shift_left(lightset::PortDir::OUTPUT, nPortIndex);
			}

			DEBUG_PRINTF("m_Params.nDirection=%x", m_Params.nDirection);

			return;
		}

#if defined (E131_HAVE_DMXIN)
		if (Sscan::Uint8(pLine, E131ParamsConst::PRIORITY[nPortIndex], value8) == Sscan::OK) {
			if ((value8 >= e131::priority::LOWEST) && (value8 <= e131::priority::HIGHEST) && (value8 != e131::priority::DEFAULT)) {
				m_Params.nPriority[nPortIndex] = value8;
				m_Params.nSetList |= (Mask::PRIORITY_A << nPortIndex);
			} else {
				m_Params.nPriority[nPortIndex] = e131::priority::DEFAULT;
				m_Params.nSetList &= ~(Mask::PRIORITY_A << nPortIndex);
			}
			return;
		}
#endif

#if defined (OUTPUT_HAVE_STYLESWITCH)
		nLength = 6;

		if (Sscan::Char(pLine, LightSetParamsConst::OUTPUT_STYLE[nPortIndex], aValue, nLength) == Sscan::OK) {
			const auto nOutputStyle = static_cast<uint32_t>(lightset::get_output_style(aValue));

			if (nOutputStyle != 0) {
				m_Params.nOutputStyle |= static_cast<uint8_t>(1U << nPortIndex);
			} else {
				m_Params.nOutputStyle &= static_cast<uint8_t>(~(1U << nPortIndex));
			}

			return;
		}
#endif
	}

	if (Sscan::Uint8(pLine, LightSetParamsConst::DISABLE_MERGE_TIMEOUT, value8) == Sscan::OK) {
		if (value8 != 0) {
			m_Params.nSetList |= Mask::DISABLE_MERGE_TIMEOUT;
		} else {
			m_Params.nSetList &= ~Mask::DISABLE_MERGE_TIMEOUT;
		}
		return;
	}
}

void E131Params::Builder(const struct Params *pParams, char *pBuffer, uint32_t nLength, uint32_t& nSize) {
	DEBUG_ENTRY

	if (pParams != nullptr) {
		memcpy(&m_Params, pParams, sizeof(struct Params));
	} else {
		E131ParamsStore::Copy(&m_Params);
	}

	PropertiesBuilder builder(E131ParamsConst::FILE_NAME, pBuffer, nLength);

	for (uint32_t nPortIndex = 0; nPortIndex < s_nPortsMax; nPortIndex++) {
		builder.Add(LightSetParamsConst::UNIVERSE_PORT[nPortIndex], m_Params.nUniverse[nPortIndex], isMaskSet(Mask::UNIVERSE_A << nPortIndex));
		const auto portDir = static_cast<lightset::PortDir>(e131params::portdir_shif_right(m_Params.nDirection, nPortIndex));
		const auto isDefault = (portDir == lightset::PortDir::OUTPUT);
		builder.Add(LightSetParamsConst::DIRECTION[nPortIndex], lightset::get_direction(portDir), !isDefault);

		const auto isLabelSet = isMaskSet(Mask::LABEL_A << nPortIndex);
		if (!isLabelSet) {
			lightset::node::get_short_name_default(nPortIndex, reinterpret_cast<char *>(m_Params.aLabel[nPortIndex]));
		}
		builder.Add(LightSetParamsConst::NODE_LABEL[nPortIndex], reinterpret_cast<const char *>(m_Params.aLabel[nPortIndex]), isLabelSet);
	}

	builder.Add(LightSetParamsConst::FAILSAFE, lightset::get_failsafe(static_cast<lightset::FailSafe>(m_Params.nFailSafe)), isMaskSet(Mask::FAILSAFE));

	builder.AddComment("DMX Output");

	for (uint32_t nPortIndex = 0; nPortIndex < s_nPortsMax; nPortIndex++) {
		const auto mergeMode = mergemode_get(nPortIndex);
		const auto isDefault = (mergeMode == lightset::MergeMode::HTP);
		builder.Add(LightSetParamsConst::MERGE_MODE_PORT[nPortIndex], lightset::get_merge_mode(mergeMode), !isDefault);

#if defined (OUTPUT_HAVE_STYLESWITCH)
		const auto isSet = isOutputStyleSet(1U << nPortIndex);
		builder.Add(LightSetParamsConst::OUTPUT_STYLE[nPortIndex], lightset::get_output_style(static_cast<lightset::OutputStyle>(isSet)), isSet);
#endif
	}

#if defined (E131_HAVE_DMXIN)
	builder.AddComment("DMX Input");
	for (uint32_t nPortIndex = 0; nPortIndex < s_nPortsMax; nPortIndex++) {
		builder.Add(E131ParamsConst::PRIORITY[nPortIndex], m_Params.nPriority[nPortIndex], isMaskSet(Mask::PRIORITY_A << nPortIndex));
	}
#endif

	builder.AddComment("#");
	builder.Add(LightSetParamsConst::DISABLE_MERGE_TIMEOUT, isMaskSet(Mask::DISABLE_MERGE_TIMEOUT));

	nSize = builder.GetSize();

	DEBUG_PRINTF("nSize=%d", nSize);
	DEBUG_EXIT
}

void E131Params::Set() {
	DEBUG_ENTRY

	if (e131bridge::configstore::DMXPORT_OFFSET <= e131bridge::MAX_PORTS) {
		s_nPortsMax = std::min(e131params::MAX_PORTS, e131bridge::MAX_PORTS - e131bridge::configstore::DMXPORT_OFFSET);
	}

	DEBUG_PRINTF("e131bridge::MAX_PORTS=%u, e131bridge::configstore::DMXPORT_OFFSET=%u, s_nPortsMax=%u", e131bridge::MAX_PORTS, e131bridge::configstore::DMXPORT_OFFSET, s_nPortsMax);

	if (m_Params.nSetList == 0) {
		return;
	}

	auto *p = E131Bridge::Get();
	assert(p != nullptr);

	for (uint32_t nPortIndex = 0; nPortIndex < s_nPortsMax; nPortIndex++) {
		const auto nOffset = nPortIndex + e131bridge::configstore::DMXPORT_OFFSET;

		if (nOffset >= e131bridge::MAX_PORTS) {
			DEBUG_EXIT
			break;
		}

		p->SetMergeMode(nOffset, mergemode_get(nPortIndex));

#if defined (E131_HAVE_DMXIN)
		if (isMaskSet(Mask::PRIORITY_A << nPortIndex)) {
			p->SetPriority(nPortIndex, m_Params.nPriority[nPortIndex]);
		}
#endif
	}

	p->SetFailSafe(static_cast<lightset::FailSafe>(m_Params.nFailSafe));

	/**
	 * Extra's
	 */

	if (isMaskSet(Mask::DISABLE_MERGE_TIMEOUT)) {
		p->SetDisableMergeTimeout(true);
	}
}

void E131Params::staticCallbackFunction(void *p, const char *s) {
	assert(p != nullptr);
	assert(s != nullptr);

	(static_cast<E131Params*>(p))->callbackFunction(s);
}

void E131Params::Dump() {
	printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, E131ParamsConst::FILE_NAME);

	if (isMaskSet(e131params::Mask::FAILSAFE)) {
		printf(" %s=%d [%s]\n", LightSetParamsConst::FAILSAFE, m_Params.nFailSafe, lightset::get_failsafe(static_cast<lightset::FailSafe>(m_Params.nFailSafe)));
	}

	for (uint32_t i = 0; i < e131params::MAX_PORTS; i++) {
		if (isMaskSet(e131params::Mask::UNIVERSE_A << i)) {
			printf(" %s=%d\n", LightSetParamsConst::UNIVERSE_PORT[i], m_Params.nUniverse[i]);
		}
	}

	for (uint32_t i = 0; i < e131params::MAX_PORTS; i++) {
		printf(" %s=%s\n", LightSetParamsConst::MERGE_MODE_PORT[i], lightset::get_merge_mode(mergemode_get(i)));
	}

	for (uint32_t i = 0; i < e131params::MAX_PORTS; i++) {
		const auto portDir = static_cast<lightset::PortDir>(e131params::portdir_shif_right(m_Params.nDirection, i));
		printf(" %s=%d [%s]\n", LightSetParamsConst::DIRECTION[i], e131params::portdir_shif_right(m_Params.nDirection, i), lightset::get_direction(portDir));
	}

	for (uint32_t i = 0; i < e131params::MAX_PORTS; i++) {
		if (isMaskSet(e131params::Mask::PRIORITY_A << i)) {
			printf(" %s=%d\n", E131ParamsConst::PRIORITY[i], m_Params.nPriority[i]);
		}
	}

	for (uint32_t i = 0; i < e131params::MAX_PORTS; i++) {
		const auto nOutputStyle = static_cast<uint32_t>(isOutputStyleSet(1U << i));
		printf(" %s=%u [%s]\n", LightSetParamsConst::OUTPUT_STYLE[i], static_cast<unsigned int>(nOutputStyle), lightset::get_output_style(static_cast<lightset::OutputStyle>(nOutputStyle)));
	}

	if (isMaskSet(e131params::Mask::DISABLE_MERGE_TIMEOUT)) {
		printf(" %s=1 [Yes]\n", LightSetParamsConst::DISABLE_MERGE_TIMEOUT);
	}
}
