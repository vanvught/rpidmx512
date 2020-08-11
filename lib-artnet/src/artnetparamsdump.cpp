/**
 * @file artnetparamsdump.cpp
 *
 */
/**
 * Art-Net Designed by and Copyright Artistic Licence Holdings Ltd.
 */
/* Copyright (C) 2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <stdio.h>

#include "artnetparams.h"
#include "artnetparamsconst.h"

#include "lightsetconst.h"

#include "network.h"

void ArtNetParams::Dump() {
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

	for (unsigned i = 0; i < ArtNet::MAX_PORTS; i++) {
		if (isMaskSet(ArtnetParamsMask::UNIVERSE_A << i)) {
			printf(" %s=%d\n", LightSetConst::PARAMS_UNIVERSE_PORT[i], m_tArtNetParams.nUniversePort[i]);
		}
	}

	if (isMaskSet(ArtnetParamsMask::MERGE_MODE)) {
		printf(" %s=%s\n", LightSetConst::PARAMS_MERGE_MODE, ArtNet::GetMergeMode(m_tArtNetParams.nMergeMode));
	}

	for (unsigned i = 0; i < ArtNet::MAX_PORTS; i++) {
		if (isMaskSet(ArtnetParamsMask::MERGE_MODE_A << i)) {
			printf(" %s=%s\n", LightSetConst::PARAMS_MERGE_MODE_PORT[i], ArtNet::GetMergeMode(m_tArtNetParams.nMergeModePort[i]));
		}
	}

	for (unsigned i = 0; i < ArtNet::MAX_PORTS; i++) {
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

	for (unsigned i = 0; i < ArtNet::MAX_PORTS; i++) {
		if (isMaskMultiPortOptionsSet(ArtnetParamsMaskMultiPortOptions::DESTINATION_IP_A << i)) {
			printf(" %s=" IPSTR "\n", ArtNetParamsConst::DESTINATION_IP_PORT[i], IP2STR(m_tArtNetParams.nDestinationIpPort[i]));
		}
	}
#endif
}

