/**
 * @file artnetparams.cpp
 *
 */
/**
 * Art-Net Designed by and Copyright Artistic Licence Holdings Ltd.
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

#include <cstring>
#include <cstdint>
#include <cstdio>
#include <climits>
#include <cassert>
#include <algorithm>

#include "artnetparams.h"
#include "artnetparamsconst.h"
#include "artnetnode.h"
#include "artnet.h"
#include "artnetconst.h"
#if (ARTNET_VERSION == 4)
# include "e131.h"
#endif

#include "lightsetparamsconst.h"
#include "lightset.h"

#include "network.h"

#include "readconfigfile.h"
#include "sscan.h"

#include "propertiesbuilder.h"

#include "debug.h"

namespace artnetnode {
namespace configstore {
extern uint32_t DMXPORT_OFFSET;
}  // namespace configstore
}  // namespace artnetnode

static uint32_t s_nPortsMax;

namespace artnetparams {
#if defined (RDM_CONTROLLER)
static constexpr bool is_set(const uint16_t nValue, const uint32_t i) {
	return (nValue & static_cast<uint16_t>(1U << (i + 8))) == static_cast<uint16_t>(1U << (i + 8));
}
#endif
}  // namespace artnetparams

using namespace artnetparams;

ArtNetParams::ArtNetParams() {
	DEBUG_ENTRY

	auto *const pArtnetNode = ArtNetNode::Get();
	assert(pArtnetNode != nullptr);

	memset(&m_Params, 0, sizeof(struct Params));

	for (uint32_t nPortIndex = 0; nPortIndex < artnet::PORTS; nPortIndex++) {
		m_Params.nUniverse[nPortIndex] = static_cast<uint16_t>(1 + nPortIndex);
		constexpr auto n = static_cast<uint32_t>(lightset::PortDir::OUTPUT) & 0x3;
		m_Params.nDirection |= static_cast<uint16_t>(n << (nPortIndex * 2));
#if defined (E131_HAVE_DMXIN)
		m_Params.nPriority[nPortIndex] = e131::priority::DEFAULT;
#endif
	}

	pArtnetNode->GetLongNameDefault(reinterpret_cast<char *>(m_Params.aLongName));
	m_Params.nFailSafe = static_cast<uint8_t>(lightset::FailSafe::HOLD);

	DEBUG_PRINTF("s_nPortsMax=%u", s_nPortsMax);
	DEBUG_EXIT
}

bool ArtNetParams::Load() {
	m_Params.nSetList = 0;

#if !defined(DISABLE_FS)
	ReadConfigFile configfile(ArtNetParams::staticCallbackFunction, this);

	if (configfile.Read(ArtNetParamsConst::FILE_NAME)) {
		ArtNetParamsStore::Update(&m_Params);
	} else
#endif
		ArtNetParamsStore::Copy(&m_Params);

#ifndef NDEBUG
	Dump();
#endif
	return true;
}

void ArtNetParams::Load(const char *pBuffer, uint32_t nLength) {
	DEBUG_ENTRY

	assert(pBuffer != nullptr);
	assert(nLength != 0);

	m_Params.nSetList = 0;

	ReadConfigFile config(ArtNetParams::staticCallbackFunction, this);

	config.Read(pBuffer, nLength);

	ArtNetParamsStore::Update(&m_Params);

#ifndef NDEBUG
	Dump();
#endif

	DEBUG_EXIT
}

void ArtNetParams::SetBool(const uint8_t nValue, const uint32_t nMask) {
	if (nValue != 0) {
		m_Params.nSetList |= nMask;
	} else {
		m_Params.nSetList &= ~nMask;
	}
}

void ArtNetParams::callbackFunction(const char *pLine) {
	assert(pLine != nullptr);

	char aValue[artnet::LONG_NAME_LENGTH];
	uint8_t nValue8;

#if defined (RDM_CONTROLLER)
	if (Sscan::Uint8(pLine, ArtNetParamsConst::ENABLE_RDM, nValue8) == Sscan::OK) {
		SetBool(nValue8, Mask::ENABLE_RDM);
		return;
	}
#endif

	/*
	 * Node
	 */

	for (uint32_t nPortIndex = 0; nPortIndex < artnet::PORTS; nPortIndex++) {
		uint16_t nValue16;

		if (Sscan::Uint16(pLine, LightSetParamsConst::UNIVERSE_PORT[nPortIndex], nValue16) == Sscan::OK) {
			if (nValue16 != 0) {
				m_Params.nUniverse[nPortIndex] = nValue16;
				if (nValue16 != static_cast<uint16_t>(nPortIndex + 1)) {
					m_Params.nSetList |= (Mask::UNIVERSE_A << nPortIndex);
				} else {
					m_Params.nSetList &= ~(Mask::UNIVERSE_A << nPortIndex);
				}
			}
			return;
		}

		uint32_t nLength = 7;

		if (Sscan::Char(pLine, LightSetParamsConst::DIRECTION[nPortIndex], aValue, nLength) == Sscan::OK) {
			const auto portDir = lightset::get_direction(aValue);

			m_Params.nDirection &= artnetparams::portdir_clear(nPortIndex);

#if defined (ARTNET_HAVE_DMXIN)
			if (portDir == lightset::PortDir::INPUT) {
				m_Params.nDirection |= portdir_set(nPortIndex, lightset::PortDir::INPUT);
			} else
#endif
			if (portDir == lightset::PortDir::DISABLE) {
				m_Params.nDirection |= portdir_set(nPortIndex, lightset::PortDir::DISABLE);
			} else {
				m_Params.nDirection |= portdir_set(nPortIndex, lightset::PortDir::OUTPUT);
			}

			return;
		}

		nLength = 3;

		if (Sscan::Char(pLine, LightSetParamsConst::MERGE_MODE_PORT[nPortIndex], aValue, nLength) == Sscan::OK) {
			m_Params.nMergeMode &= artnetparams::mergemode_clear(nPortIndex);
			m_Params.nMergeMode |= mergemode_set(nPortIndex, lightset::get_merge_mode(aValue));
			return;
		}

		nLength = artnet::SHORT_NAME_LENGTH - 1;

		if (Sscan::Char(pLine, LightSetParamsConst::NODE_LABEL[nPortIndex], reinterpret_cast<char*>(m_Params.aLabel[nPortIndex]), nLength) == Sscan::OK) {
			m_Params.aLabel[nPortIndex][nLength] = '\0';
			static_assert(sizeof(aValue) >= artnet::SHORT_NAME_LENGTH, "");
			lightset::node::get_short_name_default(nPortIndex, aValue);

			if (strcmp(reinterpret_cast<char*>(m_Params.aLabel[nPortIndex]), aValue) == 0) {
				m_Params.nSetList &= ~(Mask::LABEL_A << nPortIndex);
			} else {
				m_Params.nSetList |= (Mask::LABEL_A << nPortIndex);
			}
			return;
		}

#if defined (OUTPUT_HAVE_STYLESWITCH)
		nLength = 6;

		if (Sscan::Char(pLine, LightSetParamsConst::OUTPUT_STYLE[nPortIndex], aValue, nLength) == Sscan::OK) {
			const auto nOutputStyle = lightset::get_output_style(aValue);

			if (nOutputStyle != lightset::OutputStyle::DELTA) {
				m_Params.nOutputStyle |= static_cast<uint8_t>(1U << nPortIndex);
			} else {
				m_Params.nOutputStyle &= static_cast<uint8_t>(~(1U << nPortIndex));
			}

			return;
		}
#endif
	}

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

	nLength = artnet::LONG_NAME_LENGTH - 1;

	if (Sscan::Char(pLine, LightSetParamsConst::NODE_LONG_NAME, reinterpret_cast<char*>(m_Params.aLongName), nLength) == Sscan::OK) {
		m_Params.aLongName[nLength] = '\0';
		static_assert(sizeof(aValue) >= artnet::LONG_NAME_LENGTH, "");
		ArtNetNode::Get()->GetLongNameDefault(aValue);
		if (strcmp(reinterpret_cast<char*>(m_Params.aLongName), aValue) == 0) {
			m_Params.nSetList &= ~Mask::LONG_NAME;
		} else {
			m_Params.nSetList |= Mask::LONG_NAME;
		}
		return;
	}

	/*
	 * Art-Net
	 */

	for (uint32_t nPortIndex = 0; nPortIndex < artnet::PORTS; nPortIndex++) {
		uint32_t nLength = 4;

		if (Sscan::Char(pLine, ArtNetParamsConst::PROTOCOL_PORT[nPortIndex], aValue, nLength) == Sscan::OK) {
			m_Params.nProtocol &= artnetparams::protocol_clear(nPortIndex);
			m_Params.nProtocol |= protocol_set(nPortIndex, artnet::get_protocol_mode(aValue));
			return;
		}

#if defined (ARTNET_HAVE_DMXIN)
		uint32_t nValue32;

		if (Sscan::IpAddress(pLine, ArtNetParamsConst::DESTINATION_IP_PORT[nPortIndex], nValue32) == Sscan::OK) {
			m_Params.nDestinationIp[nPortIndex] = nValue32;

			if (nValue32 != 0) {
				m_Params.nSetList |= (Mask::DESTINATION_IP_A << nPortIndex);
			} else {
				m_Params.nSetList &= ~(Mask::DESTINATION_IP_A << nPortIndex);
			}
			return;
		}
#endif

#if defined (E131_HAVE_DMXIN)
		uint8_t value8;

		if (Sscan::Uint8(pLine, LightSetParamsConst::PRIORITY[nPortIndex], value8) == Sscan::OK) {
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

#if defined (RDM_CONTROLLER)
		if (Sscan::Uint8(pLine, ArtNetParamsConst::RDM_ENABLE_PORT[nPortIndex], nValue8) == Sscan::OK) {
			m_Params.nRdm &= artnetparams::clear_mask(nPortIndex);

			if (nValue8 != 0) {
				m_Params.nRdm |= artnetparams::shift_left(1, nPortIndex);
				m_Params.nRdm |= static_cast<uint16_t>(1U << (nPortIndex + 8));
			}
			return;
		}
#endif
	}

	if (Sscan::Uint8(pLine, ArtNetParamsConst::MAP_UNIVERSE0, nValue8) == Sscan::OK) {
		SetBool(nValue8, Mask::MAP_UNIVERSE0);
		return;
	}

	/**
	 * Extra's
	 */

	if (Sscan::Uint8(pLine, LightSetParamsConst::DISABLE_MERGE_TIMEOUT, nValue8) == Sscan::OK) {
		SetBool(nValue8, Mask::DISABLE_MERGE_TIMEOUT);
		return;
	}
}

void ArtNetParams::Builder(const struct Params *pParams, char *pBuffer, uint32_t nLength, uint32_t& nSize) {
	DEBUG_ENTRY
	DEBUG_PRINTF("s_nPortsMax=%u", s_nPortsMax);

	assert(pBuffer != nullptr);

	if (pParams != nullptr) {
		memcpy(&m_Params, pParams, sizeof(struct Params));
	} else {
		ArtNetParamsStore::Copy(&m_Params);
	}

	PropertiesBuilder builder(ArtNetParamsConst::FILE_NAME, pBuffer, nLength);

	if (!isMaskSet(Mask::LONG_NAME)) {
		ArtNetNode::Get()->GetLongNameDefault(reinterpret_cast<char *>(m_Params.aLongName));
	}
	builder.Add(LightSetParamsConst::NODE_LONG_NAME, reinterpret_cast<const char*>(m_Params.aLongName), isMaskSet(Mask::LONG_NAME));

#if defined (RDM_CONTROLLER)
	builder.Add(ArtNetParamsConst::ENABLE_RDM, isMaskSet(Mask::ENABLE_RDM));
#endif
	builder.Add(LightSetParamsConst::FAILSAFE, lightset::get_failsafe(static_cast<lightset::FailSafe>(m_Params.nFailSafe)), isMaskSet(Mask::FAILSAFE));

	for (uint32_t nPortIndex = 0; nPortIndex < s_nPortsMax; nPortIndex++) {
		const auto nOffset = nPortIndex + artnetnode::configstore::DMXPORT_OFFSET;

		if (nOffset >= artnetnode::MAX_PORTS) {
			DEBUG_PUTS("break");
			break;
		}

		builder.Add(LightSetParamsConst::UNIVERSE_PORT[nPortIndex], m_Params.nUniverse[nPortIndex], isMaskSet(Mask::UNIVERSE_A << nPortIndex));
		const auto portDir = portdir_get(nPortIndex);
		const auto isDefault = (portDir == lightset::PortDir::OUTPUT);
		builder.Add(LightSetParamsConst::DIRECTION[nPortIndex], lightset::get_direction(portDir), !isDefault);

		const auto isLabelSet = isMaskSet(Mask::LABEL_A << nPortIndex);

		if (!isLabelSet) {
			memcpy(m_Params.aLabel[nPortIndex], ArtNetNode::Get()->GetShortName(nOffset), artnet::SHORT_NAME_LENGTH);
		}

		builder.Add(LightSetParamsConst::NODE_LABEL[nPortIndex], reinterpret_cast<const char *>(m_Params.aLabel[nPortIndex]), isLabelSet);
	}

	builder.AddComment("DMX Output");
	for (uint32_t nPortIndex = 0; nPortIndex < s_nPortsMax; nPortIndex++) {
		const auto mergeMode = mergemode_get(nPortIndex);
		const auto isDefault = (mergeMode == lightset::MergeMode::HTP);
		builder.Add(LightSetParamsConst::MERGE_MODE_PORT[nPortIndex], lightset::get_merge_mode(mergeMode), !isDefault);

#if defined (OUTPUT_HAVE_STYLESWITCH)
		const auto isSet = isOutputStyleSet(1U << nPortIndex);
		builder.Add(LightSetParamsConst::OUTPUT_STYLE[nPortIndex], lightset::get_output_style(static_cast<lightset::OutputStyle>(isSet)), isSet);
#endif

#if defined (RDM_CONTROLLER)
		builder.Add(ArtNetParamsConst::RDM_ENABLE_PORT[nPortIndex], artnetparams::is_set(m_Params.nRdm, nPortIndex));
#endif
	}

#if defined (ARTNET_HAVE_DMXIN)
	builder.AddComment("DMX Input");
	for (uint32_t nPortIndex = 0; nPortIndex < s_nPortsMax; nPortIndex++) {
		if (!isMaskSet(Mask::DESTINATION_IP_A << nPortIndex)) {
			m_Params.nDestinationIp[nPortIndex] = ArtNetNode::Get()->GetDestinationIp(nPortIndex);
		}
		builder.AddIpAddress(ArtNetParamsConst::DESTINATION_IP_PORT[nPortIndex], m_Params.nDestinationIp[nPortIndex], isMaskSet(Mask::DESTINATION_IP_A << nPortIndex));
	}
#endif

	builder.AddComment("Art-Net 4");
	for (uint32_t nPortIndex = 0; nPortIndex < s_nPortsMax; nPortIndex++) {
		const auto portProtocol = protocol_get(nPortIndex);
		const auto isDefault = (portProtocol == artnet::PortProtocol::ARTNET);
		builder.Add(ArtNetParamsConst::PROTOCOL_PORT[nPortIndex], artnet::get_protocol_mode(portProtocol), !isDefault);
#if defined (E131_HAVE_DMXIN)
		builder.Add(LightSetParamsConst::PRIORITY[nPortIndex], m_Params.nPriority[nPortIndex], isMaskSet(Mask::PRIORITY_A << nPortIndex));
#endif
	}
	builder.Add(ArtNetParamsConst::MAP_UNIVERSE0, isMaskSet(Mask::MAP_UNIVERSE0));

	builder.AddComment("#");

	builder.Add(LightSetParamsConst::DISABLE_MERGE_TIMEOUT, isMaskSet(Mask::DISABLE_MERGE_TIMEOUT));

	nSize = builder.GetSize();

	DEBUG_PRINTF("nSize=%d", nSize);
	DEBUG_EXIT
}

void ArtNetParams::Set() {
	DEBUG_ENTRY

	if (artnetnode::configstore::DMXPORT_OFFSET <= artnetnode::MAX_PORTS) {
		s_nPortsMax = std::min(artnet::PORTS, artnetnode::MAX_PORTS - artnetnode::configstore::DMXPORT_OFFSET);
	}

	DEBUG_PRINTF("artnetnode::MAX_PORTS=%u, artnetnode::configstore::DMXPORT_OFFSET=%u, s_nPortsMax=%u", artnetnode::MAX_PORTS, artnetnode::configstore::DMXPORT_OFFSET, s_nPortsMax);

	auto *const p = ArtNetNode::Get();
	assert(p != nullptr);

	if (isMaskSet(Mask::LONG_NAME)) {
		p->SetLongName(reinterpret_cast<const char*>(m_Params.aLongName));
	}

	for (uint32_t nPortIndex = 0; nPortIndex < s_nPortsMax; nPortIndex++) {
		const auto nOffset = nPortIndex + artnetnode::configstore::DMXPORT_OFFSET;

		if (nOffset >= artnetnode::MAX_PORTS) {
			DEBUG_PUTS("break");
			break;
		}

		if (isMaskSet(Mask::LABEL_A << nPortIndex)) {
			p->SetShortName(nOffset, reinterpret_cast<const char *>(m_Params.aLabel[nPortIndex]));
		} else {
			p->SetShortName(nOffset, nullptr);
		}

		p->SetMergeMode(nOffset, mergemode_get(nPortIndex));

#if (ARTNET_VERSION >= 4)
		p->SetPortProtocol4(nOffset, protocol_get(nPortIndex));
#endif

#if defined (ARTNET_HAVE_DMXIN)
		if (isMaskSet(Mask::DESTINATION_IP_A << nPortIndex)) {
			p->SetDestinationIp(nOffset, m_Params.nDestinationIp[nPortIndex]);
		}
#endif

#if defined (OUTPUT_HAVE_STYLESWITCH)
		p->SetOutputStyle(nPortIndex, (isOutputStyleSet(1U << nPortIndex)) ? lightset::OutputStyle::CONSTANT : lightset::OutputStyle::DELTA);
#endif

#if defined (RDM_CONTROLLER)
		if (artnetparams::is_set(m_Params.nRdm, nPortIndex)) {
			p->SetRdm(nOffset, true);
		}
#endif

#if (ARTNET_VERSION >= 4)
		if (isMaskSet(Mask::PRIORITY_A << nPortIndex)) {
			p->SetPriority4(m_Params.nPriority[nOffset]);
		}
#endif
	}

	p->SetFailSafe(artnetnode::convert_failsafe(static_cast<lightset::FailSafe>(m_Params.nFailSafe)));

#if (ARTNET_VERSION >= 4)
	if (isMaskSet(Mask::MAP_UNIVERSE0)) {
		p->SetMapUniverse0(true);
	}
#endif

	/**
	 * Extra's
	 */

	if (isMaskSet(Mask::DISABLE_MERGE_TIMEOUT)) {
		p->SetDisableMergeTimeout(true);
	}

	DEBUG_EXIT
}

void ArtNetParams::staticCallbackFunction(void *p, const char *s) {
	assert(p != nullptr);
	assert(s != nullptr);

	(static_cast<ArtNetParams*>(p))->callbackFunction(s);
}

void ArtNetParams::Dump() {
	printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, ArtNetParamsConst::FILE_NAME);

	printf(" %s=%d [%s]\n", LightSetParamsConst::FAILSAFE, m_Params.nFailSafe, lightset::get_failsafe(static_cast<lightset::FailSafe>(m_Params.nFailSafe)));

	for (uint32_t i = 0; i < artnet::PORTS; i++) {
		printf(" %s=%s\n", LightSetParamsConst::NODE_LABEL[i], m_Params.aLabel[i]);
	}

	printf(" %s=%s\n", LightSetParamsConst::NODE_LONG_NAME, m_Params.aLongName);
	printf(" %s=1 [Yes]\n", ArtNetParamsConst::ENABLE_RDM);


	for (uint32_t i = 0; i < artnet::PORTS; i++) {
		printf(" %s=%d\n", LightSetParamsConst::UNIVERSE_PORT[i], m_Params.nUniverse[i]);
	}

	for (uint32_t i = 0; i < artnet::PORTS; i++) {
		printf(" %s=%s\n", LightSetParamsConst::MERGE_MODE_PORT[i], lightset::get_merge_mode(mergemode_get(i)));
	}

	for (uint32_t i = 0; i < artnet::PORTS; i++) {
		printf(" %s=%s\n", ArtNetParamsConst::PROTOCOL_PORT[i], artnet::get_protocol_mode(i));
	}

	for (uint32_t i = 0; i < artnet::PORTS; i++) {
		const auto portDir = portdir_get(i);
		printf(" %s=%u [%s]\n", LightSetParamsConst::DIRECTION[i], static_cast<unsigned int>(portDir), lightset::get_direction(portDir));
	}

	for (uint32_t i = 0; i < artnet::PORTS; i++) {
		printf(" %s=" IPSTR "\n", ArtNetParamsConst::DESTINATION_IP_PORT[i], IP2STR(m_Params.nDestinationIp[i]));

	}

	for (uint32_t i = 0; i < artnet::PORTS; i++) {
		const auto nOutputStyle = static_cast<uint32_t>(isOutputStyleSet(1U << i));
		printf(" %s=%u [%s]\n", LightSetParamsConst::OUTPUT_STYLE[i], static_cast<unsigned int>(nOutputStyle), lightset::get_output_style(static_cast<lightset::OutputStyle>(nOutputStyle)));
	}

	/**
	 * Art-Net 4
	 */

	printf(" %s=1 [Yes]\n", ArtNetParamsConst::MAP_UNIVERSE0);

	for (uint32_t i = 0; i < artnet::PORTS; i++) {
		printf(" %s=%u\n", LightSetParamsConst::PRIORITY[i], m_Params.nPriority[i]);
	}

	/**
	 * Extra's
	 */

	printf(" %s=1 [Yes]\n", LightSetParamsConst::DISABLE_MERGE_TIMEOUT);
}
