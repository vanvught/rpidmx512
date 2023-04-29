/**
 * @file e131params.cpp
 *
 */
/* Copyright (C) 2016-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

E131Params::E131Params(E131ParamsStore *pE131ParamsStore):m_pE131ParamsStore(pE131ParamsStore) {
	DEBUG_ENTRY
	DEBUG_PRINTF("sizeof(struct Params)=%d", static_cast<int>(sizeof(struct Params)));

	memset(&m_Params, 0, sizeof(struct Params));
	
	for (uint32_t i = 0; i < e131params::MAX_PORTS; i++) {
		m_Params.nUniversePort[i] = static_cast<uint16_t>(i + 1);
		m_Params.nPriority[i] = e131::priority::DEFAULT;
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
	char value[16];

	uint32_t nLength = 8;

	if (Sscan::Char(pLine, LightSetParamsConst::FAILSAFE, value, nLength) == Sscan::OK) {
		const auto failsafe = lightset::get_failsafe(value);

		if (failsafe == lightset::FailSafe::HOLD) {
			m_Params.nSetList &= ~Mask::FAILSAFE;
		} else {
			m_Params.nSetList |= Mask::FAILSAFE;
		}

		m_Params.nFailSafe = static_cast<uint8_t>(failsafe);
		return;
	}

	for (uint32_t i = 0; i < e131params::MAX_PORTS; i++) {
		if (Sscan::Uint16(pLine, LightSetParamsConst::UNIVERSE_PORT[i], value16) == Sscan::OK) {
			if ((value16 == 0) || (value16 > e131::universe::MAX)) {
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

#if __GNUC__ < 10
/*
error: conversion from 'int' to 'uint16_t' {aka 'short unsigned int'} may change value [-Werror=conversion]
    m_Params.nDirection &= e131params::portdir_clear(i);
                                                      ^
error: conversion from 'int' to 'uint16_t' {aka 'short unsigned int'} may change value [-Werror=conversion]
     m_Params.nDirection |= e131params::portdir_shift_left(lightset::PortDir::INPUT, i);
                                                                                      ^
error: conversion from 'int' to 'uint16_t' {aka 'short unsigned int'} may change value [-Werror=conversion]
     m_Params.nDirection |= e131params::portdir_shift_left(lightset::PortDir::DISABLE, i);
                                                                                        ^
error: conversion from 'int' to 'uint16_t' {aka 'short unsigned int'} may change value [-Werror=conversion]
     m_Params.nDirection |= e131params::portdir_shift_left(lightset::PortDir::OUTPUT, i);
 */

# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wconversion"	// FIXME ignored "-Wconversion"
#endif

		nLength = 7;

		if (Sscan::Char(pLine, LightSetParamsConst::DIRECTION[i], value, nLength) == Sscan::OK) {
			const auto portDir = lightset::get_direction(value);
			m_Params.nDirection &= e131params::portdir_clear(i);

			DEBUG_PRINTF("%u portDir=%u, m_Params.nDirection=%x", i, static_cast<uint32_t>(portDir), m_Params.nDirection);

#if defined (E131_HAVE_DMXIN)
			if (portDir == lightset::PortDir::INPUT) {
				m_Params.nDirection |= e131params::portdir_shift_left(lightset::PortDir::INPUT, i);
			} else
#endif
			if (portDir == lightset::PortDir::DISABLE) {
				m_Params.nDirection |= e131params::portdir_shift_left(lightset::PortDir::DISABLE, i);
			} else {
				m_Params.nDirection |= e131params::portdir_shift_left(lightset::PortDir::OUTPUT, i);
			}

			DEBUG_PRINTF("m_Params.nDirection=%x", m_Params.nDirection);

			return;
		}

#if __GNUC__ < 10
# pragma GCC diagnostic pop
#endif

#if defined (E131_HAVE_DMXIN)
		if (Sscan::Uint8(pLine, E131ParamsConst::PRIORITY[i], value8) == Sscan::OK) {
			if ((value8 >= e131::priority::LOWEST) && (value8 <= e131::priority::HIGHEST) && (value8 != e131::priority::DEFAULT)) {
				m_Params.nPriority[i] = value8;
				m_Params.nSetList |= (Mask::PRIORITY_A << i);
			} else {
				m_Params.nPriority[i] = e131::priority::DEFAULT;
				m_Params.nSetList &= ~(Mask::PRIORITY_A << i);
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

void E131Params::staticCallbackFunction(void *p, const char *s) {
	assert(p != nullptr);
	assert(s != nullptr);

	(static_cast<E131Params*>(p))->callbackFunction(s);
}

void E131Params::Builder(const struct Params *pParams, char *pBuffer, uint32_t nLength, uint32_t& nSize) {
	DEBUG_ENTRY

	if (pParams != nullptr) {
		memcpy(&m_Params, pParams, sizeof(struct Params));
	} else {
		m_pE131ParamsStore->Copy(&m_Params);
	}

	PropertiesBuilder builder(E131ParamsConst::FILE_NAME, pBuffer, nLength);

	for (uint32_t nPortIndex = 0; nPortIndex < s_nPortsMax; nPortIndex++) {
		builder.Add(LightSetParamsConst::UNIVERSE_PORT[nPortIndex], m_Params.nUniversePort[nPortIndex], isMaskSet(Mask::UNIVERSE_A << nPortIndex));
#if defined (E131_HAVE_DMXIN)
		const auto portDir = static_cast<lightset::PortDir>(e131params::portdir_shif_right(m_Params.nDirection, nPortIndex));
		const auto isDefault = (portDir == lightset::PortDir::OUTPUT);
		builder.Add(LightSetParamsConst::DIRECTION[nPortIndex], lightset::get_direction(portDir), !isDefault);
#endif
	}

	builder.Add(LightSetParamsConst::FAILSAFE, lightset::get_failsafe(static_cast<lightset::FailSafe>(m_Params.nFailSafe)), isMaskSet(Mask::FAILSAFE));

	builder.AddComment("DMX Output");

	for (uint32_t nPortIndex = 0; nPortIndex < s_nPortsMax; nPortIndex++) {
		builder.Add(LightSetParamsConst::MERGE_MODE_PORT[nPortIndex], lightset::get_merge_mode(m_Params.nMergeModePort[nPortIndex]), isMaskSet(Mask::MERGE_MODE_A << nPortIndex));
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

void E131Params::Save(char *pBuffer, uint32_t nLength, uint32_t& nSize) {
	DEBUG_ENTRY

	if (m_pE131ParamsStore == nullptr) {
		nSize = 0;
		DEBUG_EXIT
		return;
	}

	Builder(nullptr, pBuffer, nLength, nSize);
}

void E131Params::Set(uint32_t nPortIndexOffset) {
	DEBUG_ENTRY

	if (nPortIndexOffset <= e131bridge::MAX_PORTS) {
		s_nPortsMax = std::min(e131params::MAX_PORTS, e131bridge::MAX_PORTS - nPortIndexOffset);
	}

	DEBUG_PRINTF("e131bridge::MAX_PORTS=%u, nPortIndexOffset=%u, s_nPortsMax=%u", e131bridge::MAX_PORTS, nPortIndexOffset, s_nPortsMax);

	if (m_Params.nSetList == 0) {
		return;
	}

	auto *p = E131Bridge::Get();
	assert(p != nullptr);

	for (uint32_t nPortIndex = 0; nPortIndex < s_nPortsMax; nPortIndex++) {
		const auto nOffset = nPortIndex + nPortIndexOffset;

		if (nOffset >= e131bridge::MAX_PORTS) {
			DEBUG_EXIT
			break;
		}

		if (isMaskSet(Mask::MERGE_MODE_A << nPortIndex)) {
			p->SetMergeMode(nOffset, static_cast<lightset::MergeMode>(m_Params.nMergeModePort[nPortIndex]));
		}

		if (isMaskSet(Mask::PRIORITY_A << nPortIndex)) {
			p->SetPriority(m_Params.nPriority[nPortIndex], nPortIndex);
		}

	}

	p->SetFailSafe(static_cast<lightset::FailSafe>(m_Params.nFailSafe));

	/**
	 * Extra's
	 */

	if (isMaskSet(Mask::DISABLE_MERGE_TIMEOUT)) {
		p->SetDisableMergeTimeout(true);
	}
}
