/**
 * @file artnetparams.cpp
 *
 */
/**
 * Art-Net Designed by and Copyright Artistic Licence Holdings Ltd.
 */
/* Copyright (C) 2016-2022 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

using namespace artnet;
using namespace artnetparams;

ArtNetParams::ArtNetParams(ArtNetParamsStore *pArtNetParamsStore): m_pArtNetParamsStore(pArtNetParamsStore) {
	memset(&m_tArtNetParams, 0, sizeof(struct Params));

	for (uint32_t i = 0; i < ArtNet::PORTS; i++) {
		m_tArtNetParams.nUniversePort[i] = static_cast<uint8_t>(1 + i);
	}

	m_tArtNetParams.aOemValue[0] = ArtNetConst::OEM_ID[1];
	m_tArtNetParams.aOemValue[1] = ArtNetConst::OEM_ID[0];
	m_tArtNetParams.nNetworkTimeout = artnet::NETWORK_DATA_LOSS_TIMEOUT;

	for (uint32_t i = 0; i < CHAR_BIT; i++) {
		const uint32_t n = static_cast<uint32_t>(lightset::PortDir::OUTPUT) & 0x1;
		m_tArtNetParams.nDirection |= static_cast<uint8_t>(n << i);
	}
}

bool ArtNetParams::Load() {
	m_tArtNetParams.nSetList = 0;
	m_tArtNetParams.nMultiPortOptions = 0;

#if !defined(DISABLE_FS)
	ReadConfigFile configfile(ArtNetParams::staticCallbackFunction, this);

	if (configfile.Read(ArtNetParamsConst::FILE_NAME)) {
		// There is a configuration file
		if (m_pArtNetParamsStore != nullptr) {
			m_pArtNetParamsStore->Update(&m_tArtNetParams);
		}
	} else
#endif
	if (m_pArtNetParamsStore != nullptr) {
		m_pArtNetParamsStore->Copy(&m_tArtNetParams);
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

	m_tArtNetParams.nSetList = 0;
	m_tArtNetParams.nMultiPortOptions = 0;

	ReadConfigFile config(ArtNetParams::staticCallbackFunction, this);

	config.Read(pBuffer, nLength);

	m_pArtNetParamsStore->Update(&m_tArtNetParams);
}

void ArtNetParams::SetBool(const uint8_t nValue, const uint32_t nMask) {
	if (nValue != 0) {
		m_tArtNetParams.nSetList |= nMask;
	} else {
		m_tArtNetParams.nSetList &= ~nMask;
	}
}

void ArtNetParams::callbackFunction(const char *pLine) {
	assert(pLine != nullptr);

	char value[128];
	uint8_t nValue8;
	uint16_t nValue16;
	uint32_t nValue32;

	if (Sscan::Uint8(pLine, ArtNetParamsConst::ENABLE_RDM, nValue8) == Sscan::OK) {
		SetBool(nValue8, Mask::RDM);
		return;
	}

	uint32_t nLength = ArtNet::SHORT_NAME_LENGTH - 1;
	if (Sscan::Char(pLine, ArtNetParamsConst::NODE_SHORT_NAME, reinterpret_cast<char*>(m_tArtNetParams.aShortName), nLength) == Sscan::OK) {
		m_tArtNetParams.aShortName[nLength] = '\0';
		m_tArtNetParams.nSetList |= Mask::SHORT_NAME;
		return;
	}

	nLength = ArtNet::LONG_NAME_LENGTH - 1;
	if (Sscan::Char(pLine, ArtNetParamsConst::NODE_LONG_NAME, reinterpret_cast<char*>(m_tArtNetParams.aLongName), nLength) == Sscan::OK) {
		m_tArtNetParams.aLongName[nLength] = '\0';
		m_tArtNetParams.nSetList |= Mask::LONG_NAME;
		return;
	}

	if (Sscan::HexUint16(pLine, ArtNetParamsConst::NODE_OEM_VALUE, nValue16) == Sscan::OK) {
		m_tArtNetParams.aOemValue[0] = static_cast<uint8_t>(nValue16 >> 8);
		m_tArtNetParams.aOemValue[1] = static_cast<uint8_t>(nValue16 & 0xFF);
		m_tArtNetParams.nSetList |= Mask::OEM_VALUE;
		return;
	}

	if (Sscan::Uint8(pLine, ArtNetParamsConst::NODE_NETWORK_DATA_LOSS_TIMEOUT, nValue8) == Sscan::OK) {
		m_tArtNetParams.nNetworkTimeout = nValue8;
		if (m_tArtNetParams.nNetworkTimeout != artnet::NETWORK_DATA_LOSS_TIMEOUT) {
			m_tArtNetParams.nSetList |= Mask::NETWORK_TIMEOUT;
		} else {
			m_tArtNetParams.nSetList &= ~Mask::NETWORK_TIMEOUT;
		}
		return;
	}

	if (Sscan::Uint8(pLine, ArtNetParamsConst::DISABLE_MERGE_TIMEOUT, nValue8) == Sscan::OK) {
		SetBool(nValue8, Mask::DISABLE_MERGE_TIMEOUT);
		return;
	}

	if (Sscan::Uint8(pLine, ArtNetParamsConst::NET, nValue8) == Sscan::OK) {
		m_tArtNetParams.nNet = nValue8;
		if (nValue8 != 0) {
			m_tArtNetParams.nSetList |= Mask::NET;
		} else {
			m_tArtNetParams.nSetList &= ~Mask::NET;
		}
		return;
	}

	if (Sscan::Uint8(pLine, ArtNetParamsConst::SUBNET, nValue8) == Sscan::OK) {
		m_tArtNetParams.nSubnet = nValue8;
		if (nValue8 != 0) {
			m_tArtNetParams.nSetList |= Mask::SUBNET;
		} else {
			m_tArtNetParams.nSetList &= ~Mask::SUBNET;
		}
		return;
	}

	nLength = 3;
	if (Sscan::Char(pLine, LightSetParamsConst::MERGE_MODE, value, nLength) == Sscan::OK) {
		if(lightset::get_merge_mode(value) == lightset::MergeMode::LTP) {
			m_tArtNetParams.nMergeMode = static_cast<uint8_t>(lightset::MergeMode::LTP);
			m_tArtNetParams.nSetList |= Mask::MERGE_MODE;
			return;
		}

		m_tArtNetParams.nMergeMode = static_cast<uint8_t>(lightset::MergeMode::HTP);
		m_tArtNetParams.nSetList &= ~Mask::MERGE_MODE;
		return;
	}

	nLength = 4;
	if (Sscan::Char(pLine, ArtNetParamsConst::PROTOCOL, value, nLength) == Sscan::OK) {
		if(memcmp(value, "sacn", 4) == 0) {
			m_tArtNetParams.nProtocol = static_cast<uint8_t>(PortProtocol::SACN);
			m_tArtNetParams.nSetList |= Mask::PROTOCOL;
		} else {
			m_tArtNetParams.nProtocol = static_cast<uint8_t>(PortProtocol::ARTNET);
			m_tArtNetParams.nSetList &= ~Mask::PROTOCOL;
		}
		return;
	}

	for (unsigned i = 0; i < ArtNet::PORTS; i++) {
		if (Sscan::Uint8(pLine, LightSetParamsConst::UNIVERSE_PORT[i], nValue8) == Sscan::OK) {
			if (nValue8 <= 0xF) {
				m_tArtNetParams.nUniversePort[i] = nValue8;
				m_tArtNetParams.nSetList |= (Mask::UNIVERSE_A << i);
			} else {
				m_tArtNetParams.nUniversePort[i] = static_cast<uint8_t>(i + 1);
				m_tArtNetParams.nSetList &= ~(Mask::UNIVERSE_A << i);
			}
			return;
		}

		nLength = 3;
		if (Sscan::Char(pLine, LightSetParamsConst::MERGE_MODE_PORT[i], value, nLength) == Sscan::OK) {
			if(lightset::get_merge_mode(value) == lightset::MergeMode::LTP) {
				m_tArtNetParams.nMergeModePort[i] = static_cast<uint8_t>(lightset::MergeMode::LTP);
				m_tArtNetParams.nSetList |= (Mask::MERGE_MODE_A << i);
			} else {
				m_tArtNetParams.nMergeModePort[i] = static_cast<uint8_t>(lightset::MergeMode::HTP);
				m_tArtNetParams.nSetList &= ~(Mask::MERGE_MODE_A << i);
			}
			return;
		}

		nLength = 4;
		if (Sscan::Char(pLine, ArtNetParamsConst::PROTOCOL_PORT[i], value, nLength) == Sscan::OK) {
			if (memcmp(value, "sacn", 4) == 0) {
				m_tArtNetParams.nProtocolPort[i] = static_cast<uint8_t>(PortProtocol::SACN);
				m_tArtNetParams.nSetList |= (Mask::PROTOCOL_A << i);
			} else {
				m_tArtNetParams.nProtocolPort[i] = static_cast<uint8_t>(PortProtocol::ARTNET);
				m_tArtNetParams.nSetList &= ~(Mask::PROTOCOL_A << i);
			}
			return;
		}

		if (i < CHAR_BIT) {
			nLength = 5;

			if (Sscan::Char(pLine, LightSetParamsConst::DIRECTION[i], value, nLength) == Sscan::OK) {
				m_tArtNetParams.nDirection &= static_cast<uint8_t>(~(1U << i));

				if (lightset::get_direction(value) == lightset::PortDir::INPUT) {
					m_tArtNetParams.nDirection |= static_cast<uint8_t>((static_cast<uint8_t>(lightset::PortDir::INPUT) & 0x1) << i);
				} else {
					m_tArtNetParams.nDirection |= static_cast<uint8_t>((static_cast<uint8_t>(lightset::PortDir::OUTPUT) & 0x1) << i);
				}
				return;
			}
		}

		if (Sscan::IpAddress(pLine, ArtNetParamsConst::DESTINATION_IP_PORT[i], nValue32) == Sscan::OK) {
			m_tArtNetParams.nDestinationIpPort[i] = nValue32;

			if (nValue32 != 0) {
				m_tArtNetParams.nMultiPortOptions |= static_cast<uint16_t>(MaskMultiPortOptions::DESTINATION_IP_A << i);
			} else {
				m_tArtNetParams.nMultiPortOptions &= static_cast<uint16_t>(~(MaskMultiPortOptions::DESTINATION_IP_A << i));
			}
		}
	}

	/**
	 * Art-Net 4
	 */

	if (Sscan::Uint8(pLine, ArtNetParamsConst::MAP_UNIVERSE0, nValue8) == Sscan::OK) {
		SetBool(nValue8, Mask::MAP_UNIVERSE0);
		return;
	}
}

void ArtNetParams::staticCallbackFunction(void *p, const char *s) {
	assert(p != nullptr);
	assert(s != nullptr);

	(static_cast<ArtNetParams*>(p))->callbackFunction(s);
}

void ArtNetParams::Builder(const struct Params *pArtNetParams, char *pBuffer, uint32_t nLength, uint32_t& nSize) {
	DEBUG_ENTRY

	assert(pBuffer != nullptr);

	if (pArtNetParams != nullptr) {
		memcpy(&m_tArtNetParams, pArtNetParams, sizeof(struct Params));
	} else {
		m_pArtNetParamsStore->Copy(&m_tArtNetParams);
	}

	PropertiesBuilder builder(ArtNetParamsConst::FILE_NAME, pBuffer, nLength);

	for (uint32_t i = 0; i < ArtNet::PORTS; i++) {
		const auto isDefault = (((m_tArtNetParams.nDirection >> i) & 0x1)  == static_cast<uint8_t>(lightset::PortDir::OUTPUT));
		builder.Add(LightSetParamsConst::DIRECTION[i], lightset::get_direction(i, m_tArtNetParams.nDirection), !isDefault);
	}

	builder.Add(ArtNetParamsConst::NET, m_tArtNetParams.nNet, isMaskSet(Mask::NET));
	builder.Add(ArtNetParamsConst::SUBNET, m_tArtNetParams.nSubnet, isMaskSet(Mask::SUBNET));

	builder.AddComment("Multi port configuration");

	for (uint32_t i = 0; i < ArtNet::PORTS; i++) {
		builder.Add(LightSetParamsConst::UNIVERSE_PORT[i], m_tArtNetParams.nUniversePort[i], isMaskSet(Mask::UNIVERSE_A << i));
	}

	builder.AddComment("Node");

	if (!isMaskSet(Mask::LONG_NAME)) {
		strncpy(reinterpret_cast<char *>(m_tArtNetParams.aLongName), ArtNetNode::Get()->GetLongName(), sizeof(m_tArtNetParams.aLongName) - 1);
		m_tArtNetParams.aLongName[sizeof(m_tArtNetParams.aLongName) - 1] = '\0';
	}

	builder.Add(ArtNetParamsConst::NODE_LONG_NAME, reinterpret_cast<const char*>(m_tArtNetParams.aLongName), isMaskSet(Mask::LONG_NAME));

	if (!isMaskSet(Mask::SHORT_NAME)) {
		strncpy(reinterpret_cast<char *>(m_tArtNetParams.aShortName), ArtNetNode::Get()->GetShortName(), sizeof(m_tArtNetParams.aShortName) - 1);
		m_tArtNetParams.aShortName[sizeof(m_tArtNetParams.aShortName) - 1] = '\0';
	}

	builder.Add(ArtNetParamsConst::NODE_SHORT_NAME, reinterpret_cast<const char*>(m_tArtNetParams.aShortName), isMaskSet(Mask::SHORT_NAME));

	builder.AddHex16(ArtNetParamsConst::NODE_OEM_VALUE, m_tArtNetParams.aOemValue, isMaskSet(Mask::OEM_VALUE));

	builder.AddComment("DMX/RDM Output");
	builder.Add(ArtNetParamsConst::ENABLE_RDM, isMaskSet(Mask::RDM));

	builder.Add(LightSetParamsConst::MERGE_MODE, lightset::get_merge_mode(m_tArtNetParams.nMergeMode), isMaskSet(Mask::MERGE_MODE));
	builder.Add(ArtNetParamsConst::PROTOCOL, ArtNet::GetProtocolMode(m_tArtNetParams.nProtocol), isMaskSet(Mask::PROTOCOL));

	for (uint32_t i = 0; i < ArtNet::PORTS; i++) {
		builder.Add(LightSetParamsConst::MERGE_MODE_PORT[i], lightset::get_merge_mode(m_tArtNetParams.nMergeModePort[i]), isMaskSet(Mask::MERGE_MODE_A << i));
		builder.Add(ArtNetParamsConst::PROTOCOL_PORT[i], ArtNet::GetProtocolMode(m_tArtNetParams.nProtocolPort[i]), isMaskSet(Mask::PROTOCOL_A << i));
	}

	builder.Add(ArtNetParamsConst::NODE_NETWORK_DATA_LOSS_TIMEOUT, m_tArtNetParams.nNetworkTimeout, isMaskSet(Mask::NETWORK_TIMEOUT));
	builder.Add(ArtNetParamsConst::DISABLE_MERGE_TIMEOUT, isMaskSet(Mask::DISABLE_MERGE_TIMEOUT));

	builder.AddComment("DMX Input");

	for (uint8_t i = 0; i < ArtNet::PORTS; i++) {
		if (!isMaskMultiPortOptionsSet(static_cast<uint16_t>(MaskMultiPortOptions::DESTINATION_IP_A << i))) {
			m_tArtNetParams.nDestinationIpPort[i] = ArtNetNode::Get()->GetDestinationIp(i);
		}
		builder.AddIpAddress(ArtNetParamsConst::DESTINATION_IP_PORT[i], m_tArtNetParams.nDestinationIpPort[i], isMaskMultiPortOptionsSet(static_cast<uint16_t>(MaskMultiPortOptions::DESTINATION_IP_A << i)));
	}

	builder.AddComment("Art-Net 4");
	builder.Add(ArtNetParamsConst::MAP_UNIVERSE0, isMaskSet(Mask::MAP_UNIVERSE0));

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

void ArtNetParams::Set(ArtNetNode *pArtNetNode) {
	assert(pArtNetNode != nullptr);

	if (m_tArtNetParams.nSetList == 0) {
		return;
	}

	if (isMaskSet(Mask::SHORT_NAME)) {
		pArtNetNode->SetShortName(reinterpret_cast<const char*>(m_tArtNetParams.aShortName));
	}

	if (isMaskSet(Mask::LONG_NAME)) {
		pArtNetNode->SetLongName(reinterpret_cast<const char*>(m_tArtNetParams.aLongName));
	}

	if (isMaskSet(Mask::NET)) {
		pArtNetNode->SetNetSwitch(m_tArtNetParams.nNet, 0);
	}

	if (isMaskSet(Mask::SUBNET)) {
		pArtNetNode->SetSubnetSwitch(m_tArtNetParams.nSubnet, 0);
	}

	if (isMaskSet(Mask::OEM_VALUE)) {
		pArtNetNode->SetOemValue(m_tArtNetParams.aOemValue);
	}

	if (isMaskSet(Mask::NETWORK_TIMEOUT)) {
		pArtNetNode->SetNetworkTimeout(static_cast<uint32_t>(m_tArtNetParams.nNetworkTimeout));
	}

	if (isMaskSet(Mask::DISABLE_MERGE_TIMEOUT)) {
		pArtNetNode->SetDisableMergeTimeout(true);
	}

	uint32_t i;

	for (i = 0; i < ArtNet::PORTS; i++) {
		if (isMaskSet(Mask::MERGE_MODE_A << i)) {
			pArtNetNode->SetMergeMode(i, static_cast<lightset::MergeMode>(m_tArtNetParams.nMergeModePort[i]));
		} else {
			pArtNetNode->SetMergeMode(i, static_cast<lightset::MergeMode>(m_tArtNetParams.nMergeMode));
		}

		if (isMaskSet(Mask::PROTOCOL_A << i)) {
			pArtNetNode->SetPortProtocol(i, static_cast<PortProtocol>(m_tArtNetParams.nProtocolPort[i]));
		} else {
			pArtNetNode->SetPortProtocol(i, static_cast<PortProtocol>(m_tArtNetParams.nProtocol));
		}

		if (isMaskMultiPortOptionsSet(static_cast<uint16_t>(MaskMultiPortOptions::DESTINATION_IP_A << i))) {
			pArtNetNode->SetDestinationIp(i, m_tArtNetParams.nDestinationIpPort[i]);
		}
	}

	for (;i < (ArtNet::PORTS * ArtNet::PAGES); i++) {
		pArtNetNode->SetMergeMode(i, static_cast<lightset::MergeMode>(m_tArtNetParams.nMergeMode));
		pArtNetNode->SetPortProtocol(i, static_cast<PortProtocol>(m_tArtNetParams.nProtocol));
	}

	/**
	 * Art-Net 4
	 */

	if(isMaskSet(Mask::MAP_UNIVERSE0)) {
		pArtNetNode->SetMapUniverse0(true);
	}
}
