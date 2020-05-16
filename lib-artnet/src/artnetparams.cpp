/**
 * @file artnetparams.cpp
 *
 */
/**
 * Art-Net Designed by and Copyright Artistic Licence Holdings Ltd.
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

#if !defined(__clang__)	// Needed for compiling on MacOS
 #pragma GCC push_options
 #pragma GCC optimize ("Os")
#endif

#include <stdint.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#ifndef NDEBUG
 #include <stdio.h>
#endif
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

ArtNetParams::ArtNetParams(ArtNetParamsStore *pArtNetParamsStore): m_pArtNetParamsStore(pArtNetParamsStore) {
	DEBUG_ENTRY
	DEBUG_PRINTF("sizeof(struct TArtNetParams)=%d", static_cast<int>(sizeof(struct TArtNetParams)));

	memset(&m_tArtNetParams, 0, sizeof(struct TArtNetParams));

	m_tArtNetParams.nUniverse = 1;

	for (uint32_t i = 0; i < TArtNetConst::MAX_PORTS; i++) {
		m_tArtNetParams.nUniversePort[i] = 1 + i;
	}

	m_tArtNetParams.aOemValue[0] = ArtNetConst::OEM_ID[1];
	m_tArtNetParams.aOemValue[1] = ArtNetConst::OEM_ID[0];
	m_tArtNetParams.nDirection = ARTNET_OUTPUT_PORT;

	DEBUG_EXIT
}

ArtNetParams::~ArtNetParams(void) {
}

bool ArtNetParams::Load(void) {
	m_tArtNetParams.nSetList = 0;
	m_tArtNetParams.nMultiPortOptions = 0;

	ReadConfigFile configfile(ArtNetParams::staticCallbackFunction, this);

	if (configfile.Read(ArtNetParamsConst::FILE_NAME)) {
		// There is a configuration file
		if (m_pArtNetParamsStore != 0) {
			m_pArtNetParamsStore->Update(&m_tArtNetParams);
		}
	} else if (m_pArtNetParamsStore != 0) {
		m_pArtNetParamsStore->Copy(&m_tArtNetParams);
	} else {
		return false;
	}

	return true;
}

void ArtNetParams::Load(const char *pBuffer, uint32_t nLength) {
	assert(pBuffer != 0);
	assert(nLength != 0);
	assert(m_pArtNetParamsStore != 0);

	if (m_pArtNetParamsStore == 0) {
		return;
	}

	m_tArtNetParams.nSetList = 0;
	m_tArtNetParams.nMultiPortOptions = 0;

	ReadConfigFile config(ArtNetParams::staticCallbackFunction, this);

	config.Read(pBuffer, nLength);

	m_pArtNetParamsStore->Update(&m_tArtNetParams);
}

void ArtNetParams::callbackFunction(const char *pLine) {
	assert(pLine != 0);

	char value[128];
	uint8_t nLength;
	uint8_t nValue8;
	uint16_t nValue16;
	uint32_t nValue32;

	if (Sscan::Uint8(pLine, ArtNetParamsConst::TIMECODE, &nValue8) == SSCAN_OK) {
		m_tArtNetParams.bUseTimeCode = (nValue8 != 0);
		m_tArtNetParams.nSetList |= ArtnetParamsMask::TIMECODE;
		return;
	}

	if (Sscan::Uint8(pLine, ArtNetParamsConst::TIMESYNC, &nValue8) == SSCAN_OK) {
		m_tArtNetParams.bUseTimeSync = (nValue8 != 0);
		m_tArtNetParams.nSetList |= ArtnetParamsMask::TIMESYNC;
		return;
	}

	if (Sscan::Uint8(pLine, ArtNetParamsConst::RDM, &nValue8) == SSCAN_OK) {
		m_tArtNetParams.bEnableRdm = (nValue8 != 0);
		m_tArtNetParams.nSetList |= ArtnetParamsMask::RDM;
		return;
	}

	if (Sscan::Uint8(pLine, ArtNetParamsConst::RDM_DISCOVERY, &nValue8) == SSCAN_OK) {
		m_tArtNetParams.bRdmDiscovery = (nValue8 != 0);
		//FIXME Missing m_tArtNetParams.nSetList |= ArtnetParamsMask::RDM_DISCOVERY
		return;
	}

	nLength = ARTNET_SHORT_NAME_LENGTH - 1;
	if (Sscan::Char(pLine, ArtNetParamsConst::NODE_SHORT_NAME, reinterpret_cast<char*>(m_tArtNetParams.aShortName), &nLength) == SSCAN_OK) {
		m_tArtNetParams.aShortName[nLength] = '\0';
		m_tArtNetParams.nSetList |= ArtnetParamsMask::SHORT_NAME;
		return;
	}

	nLength = ARTNET_LONG_NAME_LENGTH - 1;
	if (Sscan::Char(pLine, ArtNetParamsConst::NODE_LONG_NAME, reinterpret_cast<char*>(m_tArtNetParams.aLongName), &nLength) == SSCAN_OK) {
		m_tArtNetParams.aLongName[nLength] = '\0';
		m_tArtNetParams.nSetList |= ArtnetParamsMask::LONG_NAME;
		return;
	}

	if (Sscan::HexUint16(pLine, ArtNetParamsConst::NODE_OEM_VALUE, &nValue16) == SSCAN_OK) {
		m_tArtNetParams.aOemValue[0] = (nValue16 >> 8);
		m_tArtNetParams.aOemValue[1] = (nValue16 & 0xFF);
		m_tArtNetParams.nSetList |= ArtnetParamsMask::OEM_VALUE;
		return;
	}

	if (Sscan::Uint8(pLine, ArtNetParamsConst::NODE_NETWORK_DATA_LOSS_TIMEOUT, &nValue8) == SSCAN_OK) {
		m_tArtNetParams.nNetworkTimeout = nValue8;
		m_tArtNetParams.nSetList |= ArtnetParamsMask::NETWORK_TIMEOUT;
		return;
	}

	if (Sscan::Uint8(pLine, ArtNetParamsConst::NODE_DISABLE_MERGE_TIMEOUT, &nValue8) == SSCAN_OK) {
		m_tArtNetParams.bDisableMergeTimeout = (nValue8 != 0);
		m_tArtNetParams.nSetList |= ArtnetParamsMask::MERGE_TIMEOUT;
		return;
	}

	if (Sscan::Uint8(pLine, ArtNetParamsConst::NET, &nValue8) == SSCAN_OK) {
		m_tArtNetParams.nNet = nValue8;
		m_tArtNetParams.nSetList |= ArtnetParamsMask::NET;
		return;
	}

	if (Sscan::Uint8(pLine, ArtNetParamsConst::SUBNET, &nValue8) == SSCAN_OK) {
		m_tArtNetParams.nSubnet = nValue8;
		m_tArtNetParams.nSetList |= ArtnetParamsMask::SUBNET;
		return;
	}

	if (Sscan::Uint8(pLine, LightSetConst::PARAMS_UNIVERSE, &nValue8) == SSCAN_OK) {
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
	if (Sscan::Char(pLine, ArtNetParamsConst::MERGE_MODE, value, &nLength) == SSCAN_OK) {
		if(ArtNet::GetMergeMode(value) == ArtNetMerge::LTP) {
			m_tArtNetParams.nMergeMode = static_cast<uint8_t>(ArtNetMerge::LTP);
			m_tArtNetParams.nSetList |= ArtnetParamsMask::MERGE_MODE;
			return;
		}

		m_tArtNetParams.nMergeMode = static_cast<uint8_t>(ArtNetMerge::HTP);
		m_tArtNetParams.nSetList &= ~ArtnetParamsMask::MERGE_MODE;
		return;
	}

	nLength = 4;
	if (Sscan::Char(pLine, ArtNetParamsConst::PROTOCOL, value, &nLength) == SSCAN_OK) {
		if(memcmp(value, "sacn", 4) == 0) {
			m_tArtNetParams.nProtocol = PORT_ARTNET_SACN;
			m_tArtNetParams.nSetList |= ArtnetParamsMask::PROTOCOL;
		} else {
			m_tArtNetParams.nProtocol = PORT_ARTNET_ARTNET;
		}
		return;
	}

	for (unsigned i = 0; i < TArtNetConst::MAX_PORTS; i++) {
		if (Sscan::Uint8(pLine, ArtNetParamsConst::UNIVERSE_PORT[i], &nValue8) == SSCAN_OK) {
			if ((nValue8 != (i + 1)) && (nValue8 <= 0xF)) {
				m_tArtNetParams.nUniversePort[i] = nValue8;
				m_tArtNetParams.nSetList |= (ArtnetParamsMask::UNIVERSE_A << i);
			} else {
				m_tArtNetParams.nUniversePort[i] = i + 1;
				m_tArtNetParams.nSetList &= ~(ArtnetParamsMask::UNIVERSE_A << i);
			}
			return;
		}

		nLength = 3;
		if (Sscan::Char(pLine, ArtNetParamsConst::MERGE_MODE_PORT[i], value, &nLength) == SSCAN_OK) {
			if(ArtNet::GetMergeMode(value) == ArtNetMerge::LTP) {
				m_tArtNetParams.nMergeModePort[i] = static_cast<uint8_t>(ArtNetMerge::LTP);
				m_tArtNetParams.nSetList |= (ArtnetParamsMask::MERGE_MODE_A << i);
			} else {
				m_tArtNetParams.nMergeModePort[i] = static_cast<uint8_t>(ArtNetMerge::HTP);
				m_tArtNetParams.nSetList &= ~(ArtnetParamsMask::MERGE_MODE_A << i);
			}
			return;
		}

		nLength = 4;
		if (Sscan::Char(pLine, ArtNetParamsConst::PROTOCOL_PORT[i], value, &nLength) == SSCAN_OK) {
			if (memcmp(value, "sacn", 4) == 0) {
				m_tArtNetParams.nProtocolPort[i] = PORT_ARTNET_SACN;
				m_tArtNetParams.nSetList |= (ArtnetParamsMask::PROTOCOL_A << i);
			} else {
				m_tArtNetParams.nProtocolPort[i] = PORT_ARTNET_ARTNET;
			}
			return;
		}

		if (Sscan::IpAddress(pLine, ArtNetParamsConst::DESTINATION_IP_PORT[i], &nValue32) == SSCAN_OK) {
			m_tArtNetParams.nDestinationIpPort[i] = nValue32;

			if (nValue32 != 0) {
				m_tArtNetParams.nMultiPortOptions |= (ArtnetParamsMaskMultiPortOptions::DESTINATION_IP_A << i);
			} else {
				m_tArtNetParams.nMultiPortOptions &= ~(ArtnetParamsMaskMultiPortOptions::DESTINATION_IP_A << i);
			}
		}
	}

	if (Sscan::Uint8(pLine, LightSetConst::PARAMS_ENABLE_NO_CHANGE_UPDATE, &nValue8) == SSCAN_OK) {
		m_tArtNetParams.bEnableNoChangeUpdate = (nValue8 != 0);
		m_tArtNetParams.nSetList |= ArtnetParamsMask::ENABLE_NO_CHANGE_OUTPUT;
		return;
	}

	nLength = 5;
	if (Sscan::Char(pLine, ArtNetParamsConst::DIRECTION, value, &nLength) == SSCAN_OK) {
		if (memcmp(value, "input", 5) == 0) {
			m_tArtNetParams.nDirection = ARTNET_INPUT_PORT;
			m_tArtNetParams.nSetList |= ArtnetParamsMask::DIRECTION;
		}
		return;
	}

	nLength = 6;
	if (Sscan::Char(pLine, ArtNetParamsConst::DIRECTION, value, &nLength) == SSCAN_OK) {
		if (memcmp(value, "output", 6) == 0) {
			m_tArtNetParams.nDirection = ARTNET_OUTPUT_PORT;
			m_tArtNetParams.nSetList |= ArtnetParamsMask::DIRECTION;
		}
		return;
	}
}

void ArtNetParams::Dump(void) {
#ifndef NDEBUG
	printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, ArtNetParamsConst::FILE_NAME);

	if(isMaskSet(ArtnetParamsMask::UNIVERSE)) {
		printf(" %s=%d\n", LightSetConst::PARAMS_UNIVERSE, m_tArtNetParams.nUniverse);
	}

	if(isMaskSet(ArtnetParamsMask::SUBNET)) {
		printf(" %s=%d\n", ArtNetParamsConst::SUBNET, m_tArtNetParams.nSubnet);
	}

	if(isMaskSet(ArtnetParamsMask::NET)) {
		printf(" %s=%d\n", ArtNetParamsConst::NET, m_tArtNetParams.nNet);
	}

	if(isMaskSet(ArtnetParamsMask::SHORT_NAME)) {
		printf(" %s=%s\n", ArtNetParamsConst::NODE_SHORT_NAME, m_tArtNetParams.aShortName);
	}

	if(isMaskSet(ArtnetParamsMask::LONG_NAME)) {
		printf(" %s=%s\n", ArtNetParamsConst::NODE_LONG_NAME, m_tArtNetParams.aLongName);
	}

	if(isMaskSet(ArtnetParamsMask::PROTOCOL)) {
		printf(" %s=%d [%s]\n", ArtNetParamsConst::PROTOCOL, m_tArtNetParams.nProtocol, ArtNet::GetProtocolMode(m_tArtNetParams.nProtocol, true));
	}

	if (isMaskSet(ArtnetParamsMask::RDM)) {
		printf(" %s=%d [%s]\n", ArtNetParamsConst::RDM, static_cast<int>(m_tArtNetParams.bEnableRdm), BOOL2STRING::Get(m_tArtNetParams.bEnableRdm));
		if (m_tArtNetParams.bEnableRdm) {
			printf("  %s=%d [%s]\n", ArtNetParamsConst::RDM_DISCOVERY, static_cast<int>(m_tArtNetParams.bRdmDiscovery), BOOL2STRING::Get(m_tArtNetParams.bRdmDiscovery));
		}
	}

	if(isMaskSet(ArtnetParamsMask::TIMECODE)) {
		printf(" %s=%d [%s]\n", ArtNetParamsConst::TIMECODE, static_cast<int>(m_tArtNetParams.bUseTimeCode), BOOL2STRING::Get(m_tArtNetParams.bUseTimeCode));
	}

	if(isMaskSet(ArtnetParamsMask::TIMESYNC)) {
		printf(" %s=%d [%s]\n", ArtNetParamsConst::TIMESYNC, static_cast<int>(m_tArtNetParams.bUseTimeCode), BOOL2STRING::Get(m_tArtNetParams.bUseTimeSync));
	}

	if(isMaskSet(ArtnetParamsMask::OEM_VALUE)) {
		printf(" %s=0x%.2X%.2X\n", ArtNetParamsConst::NODE_OEM_VALUE, m_tArtNetParams.aOemValue[0], m_tArtNetParams.aOemValue[1]);
	}

	if (isMaskSet(ArtnetParamsMask::NETWORK_TIMEOUT)) {
		printf(" %s=%d [%s]\n", ArtNetParamsConst::NODE_NETWORK_DATA_LOSS_TIMEOUT, static_cast<int>(m_tArtNetParams.nNetworkTimeout), (m_tArtNetParams.nNetworkTimeout == 0) ? "Disabled" : "");
	}

	if(isMaskSet(ArtnetParamsMask::MERGE_TIMEOUT)) {
		printf(" %s=%d [%s]\n", ArtNetParamsConst::NODE_DISABLE_MERGE_TIMEOUT, static_cast<int>(m_tArtNetParams.bDisableMergeTimeout), BOOL2STRING::Get(m_tArtNetParams.bDisableMergeTimeout));
	}

	for (unsigned i = 0; i < TArtNetConst::MAX_PORTS; i++) {
		if (isMaskSet(ArtnetParamsMask::UNIVERSE_A << i)) {
			printf(" %s=%d\n", ArtNetParamsConst::UNIVERSE_PORT[i], m_tArtNetParams.nUniversePort[i]);
		}
	}

	if (isMaskSet(ArtnetParamsMask::MERGE_MODE)) {
		printf(" %s=%s\n", ArtNetParamsConst::MERGE_MODE, ArtNet::GetMergeMode(m_tArtNetParams.nMergeMode));
	}

	for (unsigned i = 0; i < TArtNetConst::MAX_PORTS; i++) {
		if (isMaskSet(ArtnetParamsMask::MERGE_MODE_A << i)) {
			printf(" %s=%s\n", ArtNetParamsConst::MERGE_MODE_PORT[i], ArtNet::GetMergeMode(m_tArtNetParams.nMergeModePort[i]));
		}
	}

	for (unsigned i = 0; i < TArtNetConst::MAX_PORTS; i++) {
		if (isMaskSet(ArtnetParamsMask::PROTOCOL_A << i)) {
			printf(" %s=%s\n", ArtNetParamsConst::PROTOCOL_PORT[i], ArtNet::GetProtocolMode(m_tArtNetParams.nProtocolPort[i], true));
		}
	}

	if(isMaskSet(ArtnetParamsMask::ENABLE_NO_CHANGE_OUTPUT)) {
		printf(" %s=%d [%s]\n", LightSetConst::PARAMS_ENABLE_NO_CHANGE_UPDATE, static_cast<int>(m_tArtNetParams.bEnableNoChangeUpdate), BOOL2STRING::Get(m_tArtNetParams.bEnableNoChangeUpdate));
	}

	if(isMaskSet(ArtnetParamsMask::DIRECTION)) {
		printf(" %s=%d [%s]\n", ArtNetParamsConst::DIRECTION, static_cast<int>(m_tArtNetParams.nDirection), m_tArtNetParams.nDirection == ARTNET_INPUT_PORT ? "Input" : "Output");
	}

	for (unsigned i = 0; i < TArtNetConst::MAX_PORTS; i++) {
		if (isMaskMultiPortOptionsSet(ArtnetParamsMaskMultiPortOptions::DESTINATION_IP_A << i)) {
			printf(" %s=" IPSTR "\n", ArtNetParamsConst::DESTINATION_IP_PORT[i], IP2STR(m_tArtNetParams.nDestinationIpPort[i]));
		}
	}
#endif
}

uint8_t ArtNetParams::GetUniverse(uint8_t nPort, bool& IsSet) {
	assert(nPort < TArtNetConst::MAX_PORTS);

	IsSet = isMaskSet(ArtnetParamsMask::UNIVERSE_A << nPort);

	return m_tArtNetParams.nUniversePort[nPort];
}

void ArtNetParams::staticCallbackFunction(void *p, const char *s) {
	assert(p != 0);
	assert(s != 0);

	(static_cast<ArtNetParams*>(p))->callbackFunction(s);
}
