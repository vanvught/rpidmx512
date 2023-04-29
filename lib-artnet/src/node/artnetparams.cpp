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
#include <climits>
#include <cassert>
#include <algorithm>

#include "artnetparams.h"
#include "artnetparamsconst.h"
#include "artnetnode.h"
#include "artnet.h"
#include "artnetconst.h"

#include "lightsetparamsconst.h"
#include "lightset.h"

#include "network.h"

#include "readconfigfile.h"
#include "sscan.h"

#include "propertiesbuilder.h"

#include "debug.h"

static uint32_t s_nPortsMax;

namespace artnetparams {
#if defined (RDM_CONTROLLER)
static constexpr bool is_set(const uint16_t nValue, const uint32_t i) {
	return (nValue & static_cast<uint16_t>(1U << (i + 8))) == static_cast<uint16_t>(1U << (i + 8));
}
#endif
static constexpr uint16_t portdir_shift_left(const lightset::PortDir portDir, const uint32_t i) {
	return static_cast<uint16_t>((static_cast<uint32_t>(portDir) & 0x3) << (i * 2));
}

static constexpr uint16_t portdir_clear(const uint32_t i) {
	return static_cast<uint16_t>(~(0x3 << (i * 2)));
}
}  // namespace artnetparams

using namespace artnetparams;

ArtNetParams::ArtNetParams(ArtNetParamsStore *pArtNetParamsStore): m_pArtNetParamsStore(pArtNetParamsStore) {
	DEBUG_ENTRY

	memset(&m_Params, 0, sizeof(struct Params));

	for (uint32_t nPortIndex = 0; nPortIndex < artnet::PORTS; nPortIndex++) {
		m_Params.nUniversePort[nPortIndex] = static_cast<uint8_t>(1 + nPortIndex);
		constexpr auto n = static_cast<uint32_t>(lightset::PortDir::OUTPUT) & 0x3;
		m_Params.nDirection |= static_cast<uint16_t>(n << (nPortIndex * 2));
	}

	auto *const p = ArtNetNode::Get();
	assert(p != nullptr);

	p->GetShortNameDefault(reinterpret_cast<char*>(m_Params.aShortName));
	p->GetLongNameDefault(reinterpret_cast<char*>(m_Params.aLongName));

	m_Params.nFailSafe = static_cast<uint8_t>(lightset::FailSafe::HOLD);

	DEBUG_PRINTF("s_nPortsMax=%u", s_nPortsMax);
	DEBUG_EXIT
}

bool ArtNetParams::Load() {
	m_Params.nSetList = 0;
	m_Params.nMultiPortOptions = 0;

#if !defined(DISABLE_FS)
	ReadConfigFile configfile(ArtNetParams::staticCallbackFunction, this);

	if (configfile.Read(ArtNetParamsConst::FILE_NAME)) {
		// There is a configuration file
		if (m_pArtNetParamsStore != nullptr) {
			m_pArtNetParamsStore->Update(&m_Params);
		}
	} else
#endif
	if (m_pArtNetParamsStore != nullptr) {
		m_pArtNetParamsStore->Copy(&m_Params);
	} else {
		return false;
	}

	return true;
}

void ArtNetParams::Load(const char *pBuffer, uint32_t nLength) {
	assert(pBuffer != nullptr);
	assert(nLength != 0);
	assert(m_pArtNetParamsStore != nullptr);

	if (m_pArtNetParamsStore == nullptr) {
		return;
	}

	m_Params.nSetList = 0;
	m_Params.nMultiPortOptions = 0;

	ReadConfigFile config(ArtNetParams::staticCallbackFunction, this);

	config.Read(pBuffer, nLength);

	m_pArtNetParamsStore->Update(&m_Params);
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

	char value[artnet::LONG_NAME_LENGTH];
	uint8_t nValue8;

#if defined (RDM_CONTROLLER)
	if (Sscan::Uint8(pLine, ArtNetParamsConst::ENABLE_RDM, nValue8) == Sscan::OK) {
		SetBool(nValue8, Mask::RDM);
		return;
	}
#endif

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

	nLength = artnet::SHORT_NAME_LENGTH - 1;

	if (Sscan::Char(pLine, ArtNetParamsConst::NODE_SHORT_NAME, reinterpret_cast<char*>(m_Params.aShortName), nLength) == Sscan::OK) {
		m_Params.aShortName[nLength] = '\0';
		static_assert(sizeof(value) >= artnet::SHORT_NAME_LENGTH, "");
		ArtNetNode::Get()->GetShortNameDefault(value);
		if (strcmp(reinterpret_cast<char*>(m_Params.aShortName), value) == 0) {
			m_Params.nSetList &= ~Mask::SHORT_NAME;
		} else {
			m_Params.nSetList |= Mask::SHORT_NAME;
		}
		return;
	}

	nLength = artnet::LONG_NAME_LENGTH - 1;

	if (Sscan::Char(pLine, ArtNetParamsConst::NODE_LONG_NAME, reinterpret_cast<char*>(m_Params.aLongName), nLength) == Sscan::OK) {
		m_Params.aLongName[nLength] = '\0';
		static_assert(sizeof(value) >= artnet::LONG_NAME_LENGTH, "");
		ArtNetNode::Get()->GetLongNameDefault(value);
		if (strcmp(reinterpret_cast<char*>(m_Params.aLongName), value) == 0) {
			m_Params.nSetList &= ~Mask::LONG_NAME;
		} else {
			m_Params.nSetList |= Mask::LONG_NAME;
		}
		return;
	}

	if (Sscan::Uint8(pLine, ArtNetParamsConst::NET, nValue8) == Sscan::OK) {
		m_Params.nNet = nValue8;
		if (nValue8 != 0) {
			m_Params.nSetList |= Mask::NET;
		} else {
			m_Params.nSetList &= ~Mask::NET;
		}
		return;
	}

	if (Sscan::Uint8(pLine, ArtNetParamsConst::SUBNET, nValue8) == Sscan::OK) {
		m_Params.nSubnet = nValue8;
		if (nValue8 != 0) {
			m_Params.nSetList |= Mask::SUBNET;
		} else {
			m_Params.nSetList &= ~Mask::SUBNET;
		}
		return;
	}

	for (uint32_t i = 0; i < artnet::PORTS; i++) {
		if (Sscan::Uint8(pLine, LightSetParamsConst::UNIVERSE_PORT[i], nValue8) == Sscan::OK) {
			if (nValue8 <= 0xF) {
				m_Params.nUniversePort[i] = nValue8;
				m_Params.nSetList |= (Mask::UNIVERSE_A << i);
			} else {
				m_Params.nUniversePort[i] = static_cast<uint8_t>(i + 1);
				m_Params.nSetList &= ~(Mask::UNIVERSE_A << i);
			}
			return;
		}

		nLength = 3;

		if (Sscan::Char(pLine, LightSetParamsConst::MERGE_MODE_PORT[i], value, nLength) == Sscan::OK) {
			if(lightset::get_merge_mode(value) == lightset::MergeMode::LTP) {
				m_Params.nMergeModePort[i] = static_cast<uint8_t>(lightset::MergeMode::LTP);
				m_Params.nSetList |= (Mask::MERGE_MODE_A << i);
			} else {
				m_Params.nMergeModePort[i] = static_cast<uint8_t>(lightset::MergeMode::HTP);
				m_Params.nSetList &= ~(Mask::MERGE_MODE_A << i);
			}
			return;
		}

		nLength = 4;

		if (Sscan::Char(pLine, ArtNetParamsConst::PROTOCOL_PORT[i], value, nLength) == Sscan::OK) {
			if (memcmp(value, "sacn", 4) == 0) {
				m_Params.nProtocolPort[i] = static_cast<uint8_t>(artnet::PortProtocol::SACN);
				m_Params.nSetList |= (Mask::PROTOCOL_A << i);
			} else {
				m_Params.nProtocolPort[i] = static_cast<uint8_t>(artnet::PortProtocol::ARTNET);
				m_Params.nSetList &= ~(Mask::PROTOCOL_A << i);
			}
			return;
		}

#if __GNUC__ < 10
/*
error: conversion from 'int' to 'uint16_t' {aka 'short unsigned int'} may change value [-Werror=conversion]
    m_Params.nDirection &= artnetparams::portdir_clear(i);
                                                        ^
error: conversion from 'int' to 'uint16_t' {aka 'short unsigned int'} may change value [-Werror=conversion]
     m_Params.nDirection |= artnetparams::portdir_shift_left(lightset::PortDir::INPUT, i);
                                                                                        ^
error: conversion from 'int' to 'uint16_t' {aka 'short unsigned int'} may change value [-Werror=conversion]
     m_Params.nDirection |= artnetparams::portdir_shift_left(lightset::PortDir::DISABLE, i);
                                                                                          ^
error: conversion from 'int' to 'uint16_t' {aka 'short unsigned int'} may change value [-Werror=conversion]
     m_Params.nDirection |= artnetparams::portdir_shift_left(lightset::PortDir::OUTPUT, i);
 */
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wconversion"	// FIXME ignored "-Wconversion"
#endif

		nLength = 7;

		if (Sscan::Char(pLine, LightSetParamsConst::DIRECTION[i], value, nLength) == Sscan::OK) {
			const auto portDir = lightset::get_direction(value);
			m_Params.nDirection &= artnetparams::portdir_clear(i);

			DEBUG_PRINTF("%u portDir=%u, m_Params.nDirection=%x", i, static_cast<uint32_t>(portDir), m_Params.nDirection);

#if defined (ARTNET_HAVE_DMXIN)
			if (portDir == lightset::PortDir::INPUT) {
				m_Params.nDirection |= artnetparams::portdir_shift_left(lightset::PortDir::INPUT, i);
			} else 
#endif			
			if (portDir == lightset::PortDir::DISABLE) {
				m_Params.nDirection |= artnetparams::portdir_shift_left(lightset::PortDir::DISABLE, i);
			} else {
				m_Params.nDirection |= artnetparams::portdir_shift_left(lightset::PortDir::OUTPUT, i);
			}

			DEBUG_PRINTF("m_Params.nDirection=%x", m_Params.nDirection);

			return;
		}

#if __GNUC__ < 10
# pragma GCC diagnostic pop
#endif
#if defined (ARTNET_HAVE_DMXIN)
		uint32_t nValue32;

		if (Sscan::IpAddress(pLine, ArtNetParamsConst::DESTINATION_IP_PORT[i], nValue32) == Sscan::OK) {
			m_Params.nDestinationIpPort[i] = nValue32;

			if (nValue32 != 0) {
				m_Params.nMultiPortOptions |= static_cast<uint16_t>(MaskMultiPortOptions::DESTINATION_IP_A << i);
			} else {
				m_Params.nMultiPortOptions &= static_cast<uint16_t>(~(MaskMultiPortOptions::DESTINATION_IP_A << i));
			}
			return;
		}
#endif

#if __GNUC__ < 10
/*
error: conversion from 'int' to 'uint16_t' {aka 'short unsigned int'} may change value [-Werror=conversion]
    m_Params.nRdm &= artnetparams::clear_mask(i);
                                               ^
error: conversion from 'int' to 'uint16_t' {aka 'short unsigned int'} may change value [-Werror=conversion]
     m_Params.nRdm |= artnetparams::shift_left(1, i);
 */
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wconversion"	// FIXME ignored "-Wconversion"
#endif
#if defined (RDM_CONTROLLER)
		if (Sscan::Uint8(pLine, ArtNetParamsConst::RDM_ENABLE_PORT[i], nValue8) == Sscan::OK) {
			m_Params.nRdm &= artnetparams::clear_mask(i);

			if (nValue8 != 0) {
				m_Params.nRdm |= artnetparams::shift_left(1, i);
				m_Params.nRdm |= static_cast<uint16_t>(1U << (i + 8));
			}
			return;
		}
#endif
#if __GNUC__ < 10
# pragma GCC diagnostic pop
#endif
	}

	/**
	 * Art-Net 4
	 */

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

void ArtNetParams::staticCallbackFunction(void *p, const char *s) {
	assert(p != nullptr);
	assert(s != nullptr);

	(static_cast<ArtNetParams*>(p))->callbackFunction(s);
}

void ArtNetParams::Builder(const struct Params *pParams, char *pBuffer, uint32_t nLength, uint32_t& nSize) {
	DEBUG_ENTRY

	assert(pBuffer != nullptr);

	if (pParams != nullptr) {
		memcpy(&m_Params, pParams, sizeof(struct Params));
	} else {
		m_pArtNetParamsStore->Copy(&m_Params);
	}

	PropertiesBuilder builder(ArtNetParamsConst::FILE_NAME, pBuffer, nLength);

	builder.Add(ArtNetParamsConst::NODE_LONG_NAME, reinterpret_cast<const char*>(m_Params.aLongName), isMaskSet(Mask::LONG_NAME));
	builder.Add(ArtNetParamsConst::NODE_SHORT_NAME, reinterpret_cast<const char*>(m_Params.aShortName), isMaskSet(Mask::SHORT_NAME));

	builder.Add(ArtNetParamsConst::NET, m_Params.nNet, isMaskSet(Mask::NET));
	builder.Add(ArtNetParamsConst::SUBNET, m_Params.nSubnet, isMaskSet(Mask::SUBNET));
#if defined (RDM_CONTROLLER)
	builder.Add(ArtNetParamsConst::ENABLE_RDM, isMaskSet(Mask::RDM));
#endif
	builder.Add(LightSetParamsConst::FAILSAFE, lightset::get_failsafe(static_cast<lightset::FailSafe>(m_Params.nFailSafe)), isMaskSet(Mask::FAILSAFE));

	for (uint32_t nPortIndex = 0; nPortIndex < s_nPortsMax; nPortIndex++) {
		builder.Add(LightSetParamsConst::UNIVERSE_PORT[nPortIndex], m_Params.nUniversePort[nPortIndex], isMaskSet(Mask::UNIVERSE_A << nPortIndex));
		const auto portDir = static_cast<lightset::PortDir>(artnetparams::portdir_shif_right(m_Params.nDirection, nPortIndex));
		const auto isDefault = (portDir == lightset::PortDir::OUTPUT);
		builder.Add(LightSetParamsConst::DIRECTION[nPortIndex], lightset::get_direction(portDir), !isDefault);
	}

	builder.AddComment("DMX Output");
	for (uint32_t nPortIndex = 0; nPortIndex < s_nPortsMax; nPortIndex++) {
		builder.Add(LightSetParamsConst::MERGE_MODE_PORT[nPortIndex], lightset::get_merge_mode(m_Params.nMergeModePort[nPortIndex]), isMaskSet(Mask::MERGE_MODE_A << nPortIndex));
#if defined (RDM_CONTROLLER)
		builder.Add(ArtNetParamsConst::RDM_ENABLE_PORT[nPortIndex], artnetparams::is_set(m_Params.nRdm, nPortIndex));
#endif
	}

#if defined (ARTNET_HAVE_DMXIN)
	builder.AddComment("DMX Input");
	for (uint32_t nPortIndex = 0; nPortIndex < s_nPortsMax; nPortIndex++) {
		if (!isMaskMultiPortOptionsSet(static_cast<uint16_t>(MaskMultiPortOptions::DESTINATION_IP_A << nPortIndex))) {
			m_Params.nDestinationIpPort[nPortIndex] = ArtNetNode::Get()->GetDestinationIp(nPortIndex);
		}
		builder.AddIpAddress(ArtNetParamsConst::DESTINATION_IP_PORT[nPortIndex], m_Params.nDestinationIpPort[nPortIndex], isMaskMultiPortOptionsSet(static_cast<uint16_t>(MaskMultiPortOptions::DESTINATION_IP_A << nPortIndex)));
	}
#endif

	builder.AddComment("Art-Net 4");
	for (uint32_t nPortIndex = 0; nPortIndex < s_nPortsMax; nPortIndex++) {
		builder.Add(ArtNetParamsConst::PROTOCOL_PORT[nPortIndex], artnet::get_protocol_mode(m_Params.nProtocolPort[nPortIndex]), isMaskSet(Mask::PROTOCOL_A << nPortIndex));
	}
	builder.Add(ArtNetParamsConst::MAP_UNIVERSE0, isMaskSet(Mask::MAP_UNIVERSE0));

	builder.AddComment("#");

	builder.Add(LightSetParamsConst::DISABLE_MERGE_TIMEOUT, isMaskSet(Mask::DISABLE_MERGE_TIMEOUT));

	nSize = builder.GetSize();

	DEBUG_PRINTF("nSize=%d", nSize);
	DEBUG_EXIT
}

void ArtNetParams::Save(char *pBuffer, uint32_t nLength, uint32_t& nSize) {
	if (m_pArtNetParamsStore == nullptr) {
		nSize = 0;
		return;
	}

	Builder(nullptr, pBuffer, nLength, nSize);
}

void ArtNetParams::Set(uint32_t nPortIndexOffset) {
	DEBUG_ENTRY

	if (nPortIndexOffset <= artnetnode::MAX_PORTS) {
		s_nPortsMax = std::min(artnet::PORTS, artnetnode::MAX_PORTS - nPortIndexOffset);
	}

	DEBUG_PRINTF("artnetnode::MAX_PORTS=%u, nPortIndexOffset=%u, s_nPortsMax=%u", artnetnode::MAX_PORTS, nPortIndexOffset, s_nPortsMax);

	if (m_Params.nSetList == 0) {
		DEBUG_EXIT
		return;
	}

	auto *const p = ArtNetNode::Get();
	assert(p != nullptr);

	if (isMaskSet(Mask::SHORT_NAME)) {
		p->SetShortName(reinterpret_cast<const char*>(m_Params.aShortName));
	}

	if (isMaskSet(Mask::LONG_NAME)) {
		p->SetLongName(reinterpret_cast<const char*>(m_Params.aLongName));
	}

	if (isMaskSet(Mask::NET)) {
		p->SetNetSwitch(m_Params.nNet, 0);
	}

	if (isMaskSet(Mask::SUBNET)) {
		p->SetSubnetSwitch(m_Params.nSubnet, 0);
	}

	for (uint32_t nPortIndex = 0; nPortIndex < s_nPortsMax; nPortIndex++) {
		const auto nOffset = nPortIndex + nPortIndexOffset;

		if (nOffset >= artnetnode::MAX_PORTS) {
			DEBUG_PUTS("break");
			break;
		}

		if (isMaskSet(Mask::PROTOCOL_A << nPortIndex)) {
			p->SetPortProtocol(nOffset, static_cast<artnet::PortProtocol>(m_Params.nProtocolPort[nPortIndex]));
		}

		if (isMaskSet(Mask::MERGE_MODE_A << nPortIndex)) {
			p->SetMergeMode(nOffset, static_cast<lightset::MergeMode>(m_Params.nMergeModePort[nPortIndex]));
		}
#if defined (ARTNET_HAVE_DMXIN)
		if (isMaskMultiPortOptionsSet(static_cast<uint16_t>(MaskMultiPortOptions::DESTINATION_IP_A << nPortIndex))) {
			p->SetDestinationIp(nOffset, m_Params.nDestinationIpPort[nPortIndex]);
		}
#endif
#if defined (RDM_CONTROLLER)
		if (artnetparams::is_set(m_Params.nRdm, nPortIndex)) {
			p->SetRmd(nOffset, true);
		}
#endif
	}

	p->SetFailSafe(artnetnode::convert_failsafe(static_cast<lightset::FailSafe>(m_Params.nFailSafe)));

	/**
	 * Art-Net 4
	 */

	if (isMaskSet(Mask::MAP_UNIVERSE0)) {
		p->SetMapUniverse0(true);
	}

	/**
	 * Extra's
	 */

	if (isMaskSet(Mask::DISABLE_MERGE_TIMEOUT)) {
		p->SetDisableMergeTimeout(true);
	}

	DEBUG_EXIT
}
