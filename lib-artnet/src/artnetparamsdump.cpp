/**
 * @file artnetparamsdump.cpp
 *
 */
/**
 * Art-Net Designed by and Copyright Artistic Licence Holdings Ltd.
 */
/* Copyright (C) 2020-2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <cstdio>

#include "artnetparams.h"
#include "artnetparamsconst.h"

#include "lightsetparamsconst.h"

#include "network.h"

using namespace artnet;
using namespace artnetparams;

void ArtNetParams::Dump() {
#ifndef NDEBUG
	printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, ArtNetParamsConst::FILE_NAME);

//	if(isMaskSet(Mask::UNIVERSE)) {
//		printf(" %s=%d\n", LightSetParamsConst::UNIVERSE, m_tArtNetParams.nUniverse);
//	}

	if(isMaskSet(Mask::SUBNET)) {
		printf(" %s=%d\n", ArtNetParamsConst::SUBNET, m_tArtNetParams.nSubnet);
	}

	if(isMaskSet(Mask::NET)) {
		printf(" %s=%d\n", ArtNetParamsConst::NET, m_tArtNetParams.nNet);
	}

	if(isMaskSet(Mask::SHORT_NAME)) {
		printf(" %s=%s\n", ArtNetParamsConst::NODE_SHORT_NAME, m_tArtNetParams.aShortName);
	}

	if(isMaskSet(Mask::LONG_NAME)) {
		printf(" %s=%s\n", ArtNetParamsConst::NODE_LONG_NAME, m_tArtNetParams.aLongName);
	}

	if(isMaskSet(Mask::PROTOCOL)) {
		printf(" %s=%d [%s]\n", ArtNetParamsConst::PROTOCOL, m_tArtNetParams.nProtocol, ArtNet::GetProtocolMode(m_tArtNetParams.nProtocol, true));
	}

	if (isMaskSet(Mask::RDM)) {
		printf(" %s=1 [Yes]\n", ArtNetParamsConst::ENABLE_RDM);
	}

	if(isMaskSet(Mask::OEM_VALUE)) {
		printf(" %s=0x%.2X%.2X\n", ArtNetParamsConst::NODE_OEM_VALUE, m_tArtNetParams.aOemValue[0], m_tArtNetParams.aOemValue[1]);
	}

	if (isMaskSet(Mask::NETWORK_TIMEOUT)) {
		printf(" %s=%d [%s]\n", ArtNetParamsConst::NODE_NETWORK_DATA_LOSS_TIMEOUT, static_cast<int>(m_tArtNetParams.nNetworkTimeout), (m_tArtNetParams.nNetworkTimeout == 0) ? "Disabled" : "");
	}

	if(isMaskSet(Mask::DISABLE_MERGE_TIMEOUT)) {
		printf(" %s=1 [Yes]\n", ArtNetParamsConst::DISABLE_MERGE_TIMEOUT);
	}

	for (uint32_t i = 0; i < ArtNet::PORTS; i++) {
		if (isMaskSet(Mask::UNIVERSE_A << i)) {
			printf(" %s=%d\n", LightSetParamsConst::UNIVERSE_PORT[i], m_tArtNetParams.nUniversePort[i]);
		}
	}

	if (isMaskSet(Mask::MERGE_MODE)) {
		printf(" %s=%s\n", LightSetParamsConst::MERGE_MODE, lightset::get_merge_mode(m_tArtNetParams.nMergeMode));
	}

	for (uint32_t i = 0; i < ArtNet::PORTS; i++) {
		if (isMaskSet(Mask::MERGE_MODE_A << i)) {
			printf(" %s=%s\n", LightSetParamsConst::MERGE_MODE_PORT[i], lightset::get_merge_mode(m_tArtNetParams.nMergeModePort[i]));
		}
	}

	for (uint32_t i = 0; i < ArtNet::PORTS; i++) {
		if (isMaskSet(Mask::PROTOCOL_A << i)) {
			printf(" %s=%s\n", ArtNetParamsConst::PROTOCOL_PORT[i], ArtNet::GetProtocolMode(m_tArtNetParams.nProtocolPort[i], true));
		}
	}

	for (uint32_t i = 0; i < ArtNet::PORTS; i++) {
		printf(" %s=%d [%s]\n", LightSetParamsConst::DIRECTION[i], (m_tArtNetParams.nDirection >> i) & 0x1, lightset::get_direction(i, m_tArtNetParams.nDirection));
	}

	for (uint32_t i = 0; i < ArtNet::PORTS; i++) {
		if (isMaskMultiPortOptionsSet(MaskMultiPortOptions::DESTINATION_IP_A << i)) {
			printf(" %s=" IPSTR "\n", ArtNetParamsConst::DESTINATION_IP_PORT[i], IP2STR(m_tArtNetParams.nDestinationIpPort[i]));
		}
	}
#endif
}

