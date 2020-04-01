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
#include <assert.h>

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

#define BOOL2STRING(b)			(b) ? "Yes" : "No"
#define MERGEMODE2STRING(m)		(m == ARTNET_MERGE_HTP) ? "HTP" : "LTP"
#define PROTOCOL2STRING(p)		(p == PORT_ARTNET_ARTNET) ? "Art-Net" : "sACN"

ArtNetParams::ArtNetParams(ArtNetParamsStore *pArtNetParamsStore): m_pArtNetParamsStore(pArtNetParamsStore) {
	DEBUG_ENTRY
	DEBUG_PRINTF("sizeof(struct TArtNetParams)=%d", (int) sizeof(struct TArtNetParams));

	uint8_t *p = (uint8_t *) &m_tArtNetParams;

	for (uint32_t i = 0; i < sizeof(struct TArtNetParams); i++) {
		*p++ = 0;
	}

	m_tArtNetParams.nUniverse = 1;

	for (uint32_t i = 0; i < ARTNET_MAX_PORTS; i++) {
		m_tArtNetParams.nUniversePort[i] = 1 + i;
	}

	m_tArtNetParams.aOemValue[0] = ArtNetConst::OEM_ID[1];
	m_tArtNetParams.aOemValue[1] = ArtNetConst::OEM_ID[0];
	m_tArtNetParams.nDirection = ARTNET_OUTPUT_PORT;

	DEBUG_EXIT
}

ArtNetParams::~ArtNetParams(void) {
	m_tArtNetParams.nSetList = 0;
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
		m_tArtNetParams.nSetList |= ARTNET_PARAMS_MASK_TIMECODE;
		return;
	}

	if (Sscan::Uint8(pLine, ArtNetParamsConst::TIMESYNC, &nValue8) == SSCAN_OK) {
		m_tArtNetParams.bUseTimeSync = (nValue8 != 0);
		m_tArtNetParams.nSetList |= ARTNET_PARAMS_MASK_TIMESYNC;
		return;
	}

	if (Sscan::Uint8(pLine, ArtNetParamsConst::RDM, &nValue8) == SSCAN_OK) {
		m_tArtNetParams.bEnableRdm = (nValue8 != 0);
		m_tArtNetParams.nSetList |= ARTNET_PARAMS_MASK_RDM;
		return;
	}

	if (Sscan::Uint8(pLine, ArtNetParamsConst::RDM_DISCOVERY, &nValue8) == SSCAN_OK) {
		m_tArtNetParams.bRdmDiscovery = (nValue8 != 0);
		//FIXME Missing m_tArtNetParams.nSetList |= ARTNET_PARAMS_MASK_RDM_DISCOVERY
		return;
	}

	nLength = ARTNET_SHORT_NAME_LENGTH - 1;
	if (Sscan::Char(pLine, ArtNetParamsConst::NODE_SHORT_NAME, (char *) m_tArtNetParams.aShortName, &nLength) == SSCAN_OK) {
		m_tArtNetParams.aShortName[nLength] = '\0';
		m_tArtNetParams.nSetList |= ARTNET_PARAMS_MASK_SHORT_NAME;
		return;
	}

	nLength = ARTNET_LONG_NAME_LENGTH - 1;
	if (Sscan::Char(pLine, ArtNetParamsConst::NODE_LONG_NAME, (char *)m_tArtNetParams.aLongName, &nLength) == SSCAN_OK) {
		m_tArtNetParams.aLongName[nLength] = '\0';
		m_tArtNetParams.nSetList |= ARTNET_PARAMS_MASK_LONG_NAME;
		return;
	}

	if (Sscan::HexUint16(pLine, ArtNetParamsConst::NODE_OEM_VALUE, &nValue16) == SSCAN_OK) {
		m_tArtNetParams.aOemValue[0] = (uint8_t) (nValue16 >> 8);
		m_tArtNetParams.aOemValue[1] = (uint8_t) (nValue16 & 0xFF);
		m_tArtNetParams.nSetList |= ARTNET_PARAMS_MASK_OEM_VALUE;
		return;
	}

	if (Sscan::Uint8(pLine, ArtNetParamsConst::NODE_NETWORK_DATA_LOSS_TIMEOUT, &nValue8) == SSCAN_OK) {
		m_tArtNetParams.nNetworkTimeout = (time_t) nValue8;
		m_tArtNetParams.nSetList |= ARTNET_PARAMS_MASK_NETWORK_TIMEOUT;
		return;
	}

	if (Sscan::Uint8(pLine, ArtNetParamsConst::NODE_DISABLE_MERGE_TIMEOUT, &nValue8) == SSCAN_OK) {
		m_tArtNetParams.bDisableMergeTimeout = (nValue8 != 0);
		m_tArtNetParams.nSetList |= ARTNET_PARAMS_MASK_MERGE_TIMEOUT;
		return;
	}

	if (Sscan::Uint8(pLine, ArtNetParamsConst::NET, &nValue8) == SSCAN_OK) {
		m_tArtNetParams.nNet = nValue8;
		m_tArtNetParams.nSetList |= ARTNET_PARAMS_MASK_NET;
		return;
	}

	if (Sscan::Uint8(pLine, ArtNetParamsConst::SUBNET, &nValue8) == SSCAN_OK) {
		m_tArtNetParams.nSubnet = nValue8;
		m_tArtNetParams.nSetList |= ARTNET_PARAMS_MASK_SUBNET;
		return;
	}

	if (Sscan::Uint8(pLine, LightSetConst::PARAMS_UNIVERSE, &nValue8) == SSCAN_OK) {
		if (nValue8 <= 0xF) {
			m_tArtNetParams.nUniverse = nValue8;
			m_tArtNetParams.nSetList |= ARTNET_PARAMS_MASK_UNIVERSE;
		}
		return;
	}

	nLength = 3;
	if (Sscan::Char(pLine, ArtNetParamsConst::MERGE_MODE, value, &nLength) == SSCAN_OK) {
		if (memcmp(value, "ltp", 3) == 0) {
			m_tArtNetParams.nMergeMode = ARTNET_MERGE_LTP;
			m_tArtNetParams.nSetList |= ARTNET_PARAMS_MASK_MERGE_MODE;
		} else if (memcmp(value, "htp", 3) == 0) {
			m_tArtNetParams.nMergeMode = ARTNET_MERGE_HTP;
			m_tArtNetParams.nSetList |= ARTNET_PARAMS_MASK_MERGE_MODE;
		}
		return;
	}

	nLength = 4;
	if (Sscan::Char(pLine, ArtNetParamsConst::PROTOCOL, value, &nLength) == SSCAN_OK) {
		if(memcmp(value, "sacn", 4) == 0) {
			m_tArtNetParams.nProtocol = PORT_ARTNET_SACN;
			m_tArtNetParams.nSetList |= ARTNET_PARAMS_MASK_PROTOCOL;
		} else {
			m_tArtNetParams.nProtocol = PORT_ARTNET_ARTNET;
		}
		return;
	}

	for (unsigned i = 0; i < ARTNET_MAX_PORTS; i++) {
		if (Sscan::Uint8(pLine, ArtNetParamsConst::UNIVERSE_PORT[i], &nValue8) == SSCAN_OK) {
			m_tArtNetParams.nUniversePort[i] = nValue8;
			m_tArtNetParams.nSetList |= (ARTNET_PARAMS_MASK_UNIVERSE_A << i);
			return;
		}

		nLength = 3;
		if (Sscan::Char(pLine, ArtNetParamsConst::MERGE_MODE_PORT[i], value, &nLength) == SSCAN_OK) {
			if (memcmp(value, "ltp", 3) == 0) {
				m_tArtNetParams.nMergeModePort[i] = ARTNET_MERGE_LTP;
				m_tArtNetParams.nSetList |= (ARTNET_PARAMS_MASK_MERGE_MODE_A << i);
			} else if (memcmp(value, "htp", 3) == 0) {
				m_tArtNetParams.nMergeModePort[i] = ARTNET_MERGE_HTP;
				m_tArtNetParams.nSetList |= (ARTNET_PARAMS_MASK_MERGE_MODE_A << i);
			}
			return;
		}

		nLength = 4;
		if (Sscan::Char(pLine, ArtNetParamsConst::PROTOCOL_PORT[i], value, &nLength) == SSCAN_OK) {
			if (memcmp(value, "sacn", 4) == 0) {
				m_tArtNetParams.nProtocolPort[i] = PORT_ARTNET_SACN;
				m_tArtNetParams.nSetList |= (ARTNET_PARAMS_MASK_PROTOCOL_A << i);
			} else {
				m_tArtNetParams.nProtocolPort[i] = PORT_ARTNET_ARTNET;
			}
			return;
		}

		if (Sscan::IpAddress(pLine, ArtNetParamsConst::DESTINATION_IP_PORT[i], &nValue32) == SSCAN_OK) {
			m_tArtNetParams.nDestinationIpPort[i] = nValue32;

			if (nValue32 != 0) {
				m_tArtNetParams.nMultiPortOptions |= (ARTNET_PARAMS_MASK_MULTI_PORT_DESTINATION_IP_A << i);
			} else {
				m_tArtNetParams.nMultiPortOptions &= ~(ARTNET_PARAMS_MASK_MULTI_PORT_DESTINATION_IP_A << i);
			}
		}
	}

	if (Sscan::Uint8(pLine, LightSetConst::PARAMS_ENABLE_NO_CHANGE_UPDATE, &nValue8) == SSCAN_OK) {
		m_tArtNetParams.bEnableNoChangeUpdate = (nValue8 != 0);
		m_tArtNetParams.nSetList |= ARTNET_PARAMS_MASK_ENABLE_NO_CHANGE_OUTPUT;
		return;
	}

	nLength = 5;
	if (Sscan::Char(pLine, ArtNetParamsConst::DIRECTION, value, &nLength) == SSCAN_OK) {
		if (memcmp(value, "input", 5) == 0) {
			m_tArtNetParams.nDirection = (uint8_t) ARTNET_INPUT_PORT;
			m_tArtNetParams.nSetList |= ARTNET_PARAMS_MASK_DIRECTION;
		}
		return;
	}

	nLength = 6;
	if (Sscan::Char(pLine, ArtNetParamsConst::DIRECTION, value, &nLength) == SSCAN_OK) {
		if (memcmp(value, "output", 6) == 0) {
			m_tArtNetParams.nDirection = (uint8_t) ARTNET_OUTPUT_PORT;
			m_tArtNetParams.nSetList |= ARTNET_PARAMS_MASK_DIRECTION;
		}
		return;
	}
}

void ArtNetParams::Dump(void) {
#ifndef NDEBUG
	printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, ArtNetParamsConst::FILE_NAME);

	if(isMaskSet(ARTNET_PARAMS_MASK_UNIVERSE)) {
		printf(" %s=%d\n", LightSetConst::PARAMS_UNIVERSE, m_tArtNetParams.nUniverse);
	}

	if(isMaskSet(ARTNET_PARAMS_MASK_SUBNET)) {
		printf(" %s=%d\n", ArtNetParamsConst::SUBNET, m_tArtNetParams.nSubnet);
	}

	if(isMaskSet(ARTNET_PARAMS_MASK_NET)) {
		printf(" %s=%d\n", ArtNetParamsConst::NET, m_tArtNetParams.nNet);
	}

	if(isMaskSet(ARTNET_PARAMS_MASK_SHORT_NAME)) {
		printf(" %s=%s\n", ArtNetParamsConst::NODE_SHORT_NAME, m_tArtNetParams.aShortName);
	}

	if(isMaskSet(ARTNET_PARAMS_MASK_LONG_NAME)) {
		printf(" %s=%s\n", ArtNetParamsConst::NODE_LONG_NAME, m_tArtNetParams.aLongName);
	}

	if(isMaskSet(ARTNET_PARAMS_MASK_PROTOCOL)) {
		printf(" %s=%d [%s]\n", ArtNetParamsConst::PROTOCOL, m_tArtNetParams.nProtocol, PROTOCOL2STRING(m_tArtNetParams.nProtocol));
	}

	if (isMaskSet(ARTNET_PARAMS_MASK_RDM)) {
		printf(" %s=%d [%s]\n", ArtNetParamsConst::RDM, (int) m_tArtNetParams.bEnableRdm, BOOL2STRING(m_tArtNetParams.bEnableRdm));
		if (m_tArtNetParams.bEnableRdm) {
			printf("  %s=%d [%s]\n", ArtNetParamsConst::RDM_DISCOVERY, (int) m_tArtNetParams.bRdmDiscovery, BOOL2STRING(m_tArtNetParams.bRdmDiscovery));
		}
	}

	if(isMaskSet(ARTNET_PARAMS_MASK_TIMECODE)) {
		printf(" %s=%d [%s]\n", ArtNetParamsConst::TIMECODE, (int) m_tArtNetParams.bUseTimeCode, BOOL2STRING(m_tArtNetParams.bUseTimeCode));
	}

	if(isMaskSet(ARTNET_PARAMS_MASK_TIMESYNC)) {
		printf(" %s=%d [%s]\n", ArtNetParamsConst::TIMESYNC, (int) m_tArtNetParams.bUseTimeCode, BOOL2STRING(m_tArtNetParams.bUseTimeSync));
	}

	if(isMaskSet(ARTNET_PARAMS_MASK_OEM_VALUE)) {
		printf(" %s=0x%.2X%.2X\n", ArtNetParamsConst::NODE_OEM_VALUE, m_tArtNetParams.aOemValue[0], m_tArtNetParams.aOemValue[1]);
	}

	if (isMaskSet(ARTNET_PARAMS_MASK_NETWORK_TIMEOUT)) {
		printf(" %s=%d [%s]\n", ArtNetParamsConst::NODE_NETWORK_DATA_LOSS_TIMEOUT, (int) m_tArtNetParams.nNetworkTimeout, (m_tArtNetParams.nNetworkTimeout == 0) ? "Disabled" : "");
	}

	if(isMaskSet(ARTNET_PARAMS_MASK_MERGE_TIMEOUT)) {
		printf(" %s=%d [%s]\n", ArtNetParamsConst::NODE_DISABLE_MERGE_TIMEOUT, (int) m_tArtNetParams.bDisableMergeTimeout, BOOL2STRING(m_tArtNetParams.bDisableMergeTimeout));
	}

	for (unsigned i = 0; i < ARTNET_MAX_PORTS; i++) {
		if (isMaskSet(ARTNET_PARAMS_MASK_UNIVERSE_A << i)) {
			printf(" %s=%d\n", ArtNetParamsConst::UNIVERSE_PORT[i], m_tArtNetParams.nUniversePort[i]);
		}
	}

	if (isMaskSet(ARTNET_PARAMS_MASK_MERGE_MODE)) {
		printf(" %s=%s\n", ArtNetParamsConst::MERGE_MODE, MERGEMODE2STRING(m_tArtNetParams.nMergeMode));
	}

	for (unsigned i = 0; i < ARTNET_MAX_PORTS; i++) {
		if (isMaskSet(ARTNET_PARAMS_MASK_MERGE_MODE_A << i)) {
			printf(" %s=%s\n", ArtNetParamsConst::MERGE_MODE_PORT[i], MERGEMODE2STRING(m_tArtNetParams.nMergeModePort[i]));
		}
	}

	for (unsigned i = 0; i < ARTNET_MAX_PORTS; i++) {
		if (isMaskSet(ARTNET_PARAMS_MASK_PROTOCOL_A << i)) {
			printf(" %s=%s\n", ArtNetParamsConst::PROTOCOL_PORT[i], PROTOCOL2STRING(m_tArtNetParams.nProtocolPort[i]));
		}
	}

	if(isMaskSet(ARTNET_PARAMS_MASK_ENABLE_NO_CHANGE_OUTPUT)) {
		printf(" %s=%d [%s]\n", LightSetConst::PARAMS_ENABLE_NO_CHANGE_UPDATE, (int) m_tArtNetParams.bEnableNoChangeUpdate, BOOL2STRING(m_tArtNetParams.bEnableNoChangeUpdate));
	}

	if(isMaskSet(ARTNET_PARAMS_MASK_DIRECTION)) {
		printf(" %s=%d [%s]\n", ArtNetParamsConst::DIRECTION, (int) m_tArtNetParams.nDirection, m_tArtNetParams.nDirection == ARTNET_INPUT_PORT ? "Input" : "Output");
	}

	for (unsigned i = 0; i < ARTNET_MAX_PORTS; i++) {
		if (isMaskMultiPortOptionsSet(ARTNET_PARAMS_MASK_MULTI_PORT_DESTINATION_IP_A << i)) {
			printf(" %s=" IPSTR "\n", ArtNetParamsConst::DESTINATION_IP_PORT[i], IP2STR(m_tArtNetParams.nDestinationIpPort[i]));
		}
	}
#endif
}

uint8_t ArtNetParams::GetUniverse(uint8_t nPort, bool& IsSet) {
	assert(nPort < ARTNET_MAX_PORTS);

	IsSet = isMaskSet(ARTNET_PARAMS_MASK_UNIVERSE_A << nPort);

	return m_tArtNetParams.nUniversePort[nPort];
}

void ArtNetParams::staticCallbackFunction(void *p, const char *s) {
	assert(p != 0);
	assert(s != 0);

	((ArtNetParams *) p)->callbackFunction(s);
}
