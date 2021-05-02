/**
 * @file artnetparams.cpp
 *
 */
/**
 * Art-Net Designed by and Copyright Artistic Licence Holdings Ltd.
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

#include <cstring>
#include <cstdint>
#include <cassert>

#include "artnetparams.h"
#include "artnetparamsconst.h"
#include "artnetnode.h"
#include "artnet.h"
#include "artnetconst.h"

#include "lightsetconst.h"

#include "network.h"

#include "readconfigfile.h"
#include "sscan.h"

#include "propertiesbuilder.h"

#include "debug.h"

using namespace artnet;

ArtNetParams::ArtNetParams(ArtNetParamsStore *pArtNetParamsStore): m_pArtNetParamsStore(pArtNetParamsStore) {
	DEBUG_ENTRY
	DEBUG_PRINTF("sizeof(struct TArtNetParams)=%d", static_cast<int>(sizeof(struct TArtNetParams)));

	memset(&m_tArtNetParams, 0, sizeof(struct TArtNetParams));

	m_tArtNetParams.nUniverse = 1;

	for (uint32_t i = 0; i < ArtNet::MAX_PORTS; i++) {
		m_tArtNetParams.nUniversePort[i] = static_cast<uint8_t>(1 + i);
	}

	m_tArtNetParams.aOemValue[0] = ArtNetConst::OEM_ID[1];
	m_tArtNetParams.aOemValue[1] = ArtNetConst::OEM_ID[0];
	m_tArtNetParams.nDirection = static_cast<uint8_t>(PortDir::OUTPUT);

	DEBUG_EXIT
}

bool ArtNetParams::Load() {
	m_tArtNetParams.nSetList = 0;
	m_tArtNetParams.nMultiPortOptions = 0;

	ReadConfigFile configfile(ArtNetParams::staticCallbackFunction, this);

	if (configfile.Read(ArtNetParamsConst::FILE_NAME)) {
		// There is a configuration file
		if (m_pArtNetParamsStore != nullptr) {
			m_pArtNetParamsStore->Update(&m_tArtNetParams);
		}
	} else if (m_pArtNetParamsStore != nullptr) {
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
	uint32_t nLength;
	uint8_t nValue8;
	uint16_t nValue16;
	uint32_t nValue32;

	if (Sscan::Uint8(pLine, ArtNetParamsConst::USE_TIMECODE, nValue8) == Sscan::OK) {
		SetBool(nValue8, ArtnetParamsMask::TIMECODE);
		return;
	}

	if (Sscan::Uint8(pLine, ArtNetParamsConst::USE_TIMESYNC, nValue8) == Sscan::OK) {
		SetBool(nValue8, ArtnetParamsMask::TIMESYNC);
		return;
	}

	if (Sscan::Uint8(pLine, ArtNetParamsConst::ENABLE_RDM, nValue8) == Sscan::OK) {
		SetBool(nValue8, ArtnetParamsMask::RDM);
		return;
	}

	nLength = ArtNet::SHORT_NAME_LENGTH - 1;
	if (Sscan::Char(pLine, ArtNetParamsConst::NODE_SHORT_NAME, reinterpret_cast<char*>(m_tArtNetParams.aShortName), nLength) == Sscan::OK) {
		m_tArtNetParams.aShortName[nLength] = '\0';
		m_tArtNetParams.nSetList |= ArtnetParamsMask::SHORT_NAME;
		return;
	}

	nLength = ArtNet::LONG_NAME_LENGTH - 1;
	if (Sscan::Char(pLine, ArtNetParamsConst::NODE_LONG_NAME, reinterpret_cast<char*>(m_tArtNetParams.aLongName), nLength) == Sscan::OK) {
		m_tArtNetParams.aLongName[nLength] = '\0';
		m_tArtNetParams.nSetList |= ArtnetParamsMask::LONG_NAME;
		return;
	}

	if (Sscan::HexUint16(pLine, ArtNetParamsConst::NODE_OEM_VALUE, nValue16) == Sscan::OK) {
		m_tArtNetParams.aOemValue[0] = static_cast<uint8_t>(nValue16 >> 8);
		m_tArtNetParams.aOemValue[1] = static_cast<uint8_t>(nValue16 & 0xFF);
		m_tArtNetParams.nSetList |= ArtnetParamsMask::OEM_VALUE;
		return;
	}

	if (Sscan::Uint8(pLine, ArtNetParamsConst::NODE_NETWORK_DATA_LOSS_TIMEOUT, nValue8) == Sscan::OK) {
		m_tArtNetParams.nNetworkTimeout = nValue8;
		m_tArtNetParams.nSetList |= ArtnetParamsMask::NETWORK_TIMEOUT;
		return;
	}

	if (Sscan::Uint8(pLine, ArtNetParamsConst::NODE_DISABLE_MERGE_TIMEOUT, nValue8) == Sscan::OK) {
		SetBool(nValue8, ArtnetParamsMask::DISABLE_MERGE_TIMEOUT);
		return;
	}

	if (Sscan::Uint8(pLine, ArtNetParamsConst::NET, nValue8) == Sscan::OK) {
		if (nValue8 != 0) {
			m_tArtNetParams.nSetList |= ArtnetParamsMask::NET;
		} else {
			m_tArtNetParams.nSetList &= ~ArtnetParamsMask::NET;
		}
		m_tArtNetParams.nNet = nValue8;
		return;
	}

	if (Sscan::Uint8(pLine, ArtNetParamsConst::SUBNET, nValue8) == Sscan::OK) {
		if (nValue8 != 0) {
			m_tArtNetParams.nSetList |= ArtnetParamsMask::SUBNET;
		} else {
			m_tArtNetParams.nSetList &= ~ArtnetParamsMask::SUBNET;
		}
		m_tArtNetParams.nSubnet = nValue8;
		return;
	}

	if (Sscan::Uint8(pLine, LightSetConst::PARAMS_UNIVERSE, nValue8) == Sscan::OK) {
		if ((nValue8 != 1) && (nValue8 <= 0xF)) {
			m_tArtNetParams.nUniverse = nValue8;
			m_tArtNetParams.nSetList |= ArtnetParamsMask::UNIVERSE;
		} else {
			m_tArtNetParams.nUniverse = 1;
			m_tArtNetParams.nSetList &= ~ArtnetParamsMask::UNIVERSE;
		}
		return;
	}

	nLength = 3;
	if (Sscan::Char(pLine, LightSetConst::PARAMS_MERGE_MODE, value, nLength) == Sscan::OK) {
		if(ArtNet::GetMergeMode(value) == Merge::LTP) {
			m_tArtNetParams.nMergeMode = static_cast<uint8_t>(Merge::LTP);
			m_tArtNetParams.nSetList |= ArtnetParamsMask::MERGE_MODE;
			return;
		}

		m_tArtNetParams.nMergeMode = static_cast<uint8_t>(Merge::HTP);
		m_tArtNetParams.nSetList &= ~ArtnetParamsMask::MERGE_MODE;
		return;
	}

	nLength = 4;
	if (Sscan::Char(pLine, ArtNetParamsConst::PROTOCOL, value, nLength) == Sscan::OK) {
		if(memcmp(value, "sacn", 4) == 0) {
			m_tArtNetParams.nProtocol = static_cast<uint8_t>(PortProtocol::SACN);
			m_tArtNetParams.nSetList |= ArtnetParamsMask::PROTOCOL;
		} else {
			m_tArtNetParams.nProtocol = static_cast<uint8_t>(PortProtocol::ARTNET);
			m_tArtNetParams.nSetList &= ~ArtnetParamsMask::PROTOCOL;
		}
		return;
	}

	for (unsigned i = 0; i < ArtNet::MAX_PORTS; i++) {
		if (Sscan::Uint8(pLine, LightSetConst::PARAMS_UNIVERSE_PORT[i], nValue8) == Sscan::OK) {
			if ((nValue8 != (i + 1)) && (nValue8 <= 0xF)) {
				m_tArtNetParams.nUniversePort[i] = nValue8;
				m_tArtNetParams.nSetList |= (ArtnetParamsMask::UNIVERSE_A << i);
			} else {
				m_tArtNetParams.nUniversePort[i] = static_cast<uint8_t>(i + 1);
				m_tArtNetParams.nSetList &= ~(ArtnetParamsMask::UNIVERSE_A << i);
			}
			return;
		}

		nLength = 3;
		if (Sscan::Char(pLine, LightSetConst::PARAMS_MERGE_MODE_PORT[i], value, nLength) == Sscan::OK) {
			if(ArtNet::GetMergeMode(value) == Merge::LTP) {
				m_tArtNetParams.nMergeModePort[i] = static_cast<uint8_t>(Merge::LTP);
				m_tArtNetParams.nSetList |= (ArtnetParamsMask::MERGE_MODE_A << i);
			} else {
				m_tArtNetParams.nMergeModePort[i] = static_cast<uint8_t>(Merge::HTP);
				m_tArtNetParams.nSetList &= ~(ArtnetParamsMask::MERGE_MODE_A << i);
			}
			return;
		}

		nLength = 4;
		if (Sscan::Char(pLine, ArtNetParamsConst::PROTOCOL_PORT[i], value, nLength) == Sscan::OK) {
			if (memcmp(value, "sacn", 4) == 0) {
				m_tArtNetParams.nProtocolPort[i] = static_cast<uint8_t>(PortProtocol::SACN);
				m_tArtNetParams.nSetList |= (ArtnetParamsMask::PROTOCOL_A << i);
			} else {
				m_tArtNetParams.nProtocolPort[i] = static_cast<uint8_t>(PortProtocol::ARTNET);
				m_tArtNetParams.nSetList &= ~(ArtnetParamsMask::PROTOCOL_A << i);
			}
			return;
		}

		if (Sscan::IpAddress(pLine, ArtNetParamsConst::DESTINATION_IP_PORT[i], nValue32) == Sscan::OK) {
			m_tArtNetParams.nDestinationIpPort[i] = nValue32;

			if (nValue32 != 0) {
				m_tArtNetParams.nMultiPortOptions |= static_cast<uint16_t>(ArtnetParamsMaskMultiPortOptions::DESTINATION_IP_A << i);
			} else {
				m_tArtNetParams.nMultiPortOptions &= static_cast<uint16_t>(~(ArtnetParamsMaskMultiPortOptions::DESTINATION_IP_A << i));
			}
		}
	}

	if (Sscan::Uint8(pLine, LightSetConst::PARAMS_ENABLE_NO_CHANGE_UPDATE, nValue8) == Sscan::OK) {
		SetBool(nValue8, ArtnetParamsMask::ENABLE_NO_CHANGE_OUTPUT);
		return;
	}

	nLength = 5;
	if (Sscan::Char(pLine, ArtNetParamsConst::DIRECTION, value, nLength) == Sscan::OK) {
		if (memcmp(value, "input", 5) == 0) {
			m_tArtNetParams.nDirection = static_cast<uint8_t>(PortDir::INPUT);
			m_tArtNetParams.nSetList |= ArtnetParamsMask::DIRECTION;
		} else {
			m_tArtNetParams.nDirection = static_cast<uint8_t>(PortDir::OUTPUT);
			m_tArtNetParams.nSetList &= ~ArtnetParamsMask::DIRECTION;
		}
		return;
	}
}

uint8_t ArtNetParams::GetUniverse(uint8_t nPort, bool& IsSet) {
	assert(nPort < ArtNet::MAX_PORTS);

	IsSet = isMaskSet(ArtnetParamsMask::UNIVERSE_A << nPort);

	return m_tArtNetParams.nUniversePort[nPort];
}

void ArtNetParams::staticCallbackFunction(void *p, const char *s) {
	assert(p != nullptr);
	assert(s != nullptr);

	(static_cast<ArtNetParams*>(p))->callbackFunction(s);
}
