/**
 * @file artnetparamssave.cpp
 *
 */
/* Copyright (C) 2019-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <string.h>
#include <cassert>

#include "artnetparams.h"
#include "artnetparamsconst.h"
#include "artnetnode.h"

#include "propertiesbuilder.h"

#include "lightsetconst.h"

#include "debug.h"

void ArtNetParams::Builder(const struct TArtNetParams *pArtNetParams, char *pBuffer, uint32_t nLength, uint32_t &nSize) {
	DEBUG_ENTRY

	assert(pBuffer != nullptr);

	if (pArtNetParams != nullptr) {
		memcpy(&m_tArtNetParams, pArtNetParams, sizeof(struct TArtNetParams));
	} else {
		m_pArtNetParamsStore->Copy(&m_tArtNetParams);
	}

	PropertiesBuilder builder(ArtNetParamsConst::FILE_NAME, pBuffer, nLength);

	builder.Add(ArtNetParamsConst::DIRECTION, m_tArtNetParams.nDirection == ARTNET_INPUT_PORT ? "input" : "output" , isMaskSet(ArtnetParamsMask::DIRECTION));

	builder.Add(ArtNetParamsConst::NET, m_tArtNetParams.nNet, isMaskSet(ArtnetParamsMask::NET));
	builder.Add(ArtNetParamsConst::SUBNET, m_tArtNetParams.nSubnet, isMaskSet(ArtnetParamsMask::SUBNET));
	builder.Add(LightSetConst::PARAMS_UNIVERSE, m_tArtNetParams.nUniverse, isMaskSet(ArtnetParamsMask::UNIVERSE));

	builder.AddComment("Multi port configuration");
	for (uint32_t i = 0; i < ArtNet::MAX_PORTS; i++) {
		builder.Add(LightSetConst::PARAMS_UNIVERSE_PORT[i], m_tArtNetParams.nUniversePort[i], isMaskSet(ArtnetParamsMask::UNIVERSE_A << i));
	}

	builder.AddComment("Node");
	builder.Add(ArtNetParamsConst::NODE_LONG_NAME, reinterpret_cast<const char*>(m_tArtNetParams.aLongName), isMaskSet(ArtnetParamsMask::LONG_NAME));
	builder.Add(ArtNetParamsConst::NODE_SHORT_NAME, reinterpret_cast<const char*>(m_tArtNetParams.aShortName), isMaskSet(ArtnetParamsMask::SHORT_NAME));

	builder.AddHex16(ArtNetParamsConst::NODE_OEM_VALUE, m_tArtNetParams.aOemValue, isMaskSet(ArtnetParamsMask::OEM_VALUE));

	builder.AddComment("Time");
	builder.Add(ArtNetParamsConst::TIMECODE, m_tArtNetParams.bUseTimeCode, isMaskSet(ArtnetParamsMask::TIMECODE));
	builder.Add(ArtNetParamsConst::TIMESYNC, m_tArtNetParams.bUseTimeSync, isMaskSet(ArtnetParamsMask::TIMESYNC));

	builder.AddComment("DMX/RDM Output");
	builder.Add(ArtNetParamsConst::RDM, m_tArtNetParams.bEnableRdm, isMaskSet(ArtnetParamsMask::RDM));

	builder.Add(LightSetConst::PARAMS_MERGE_MODE, ArtNet::GetMergeMode(m_tArtNetParams.nMergeMode), isMaskSet(ArtnetParamsMask::MERGE_MODE));
	builder.Add(ArtNetParamsConst::PROTOCOL, ArtNet::GetProtocolMode(m_tArtNetParams.nProtocol), isMaskSet(ArtnetParamsMask::PROTOCOL));

	for (uint32_t i = 0; i < ArtNet::MAX_PORTS; i++) {
		builder.Add(LightSetConst::PARAMS_MERGE_MODE_PORT[i], ArtNet::GetMergeMode(m_tArtNetParams.nMergeModePort[i]), isMaskSet(ArtnetParamsMask::MERGE_MODE_A << i));
		builder.Add(ArtNetParamsConst::PROTOCOL_PORT[i], ArtNet::GetProtocolMode(m_tArtNetParams.nProtocolPort[i]), isMaskSet(ArtnetParamsMask::PROTOCOL_A << i));
	}

	builder.Add(ArtNetParamsConst::NODE_NETWORK_DATA_LOSS_TIMEOUT, m_tArtNetParams.nNetworkTimeout, isMaskSet(ArtnetParamsMask::NETWORK_TIMEOUT));
	builder.Add(ArtNetParamsConst::NODE_DISABLE_MERGE_TIMEOUT, m_tArtNetParams.bDisableMergeTimeout, isMaskSet(ArtnetParamsMask::MERGE_TIMEOUT));

	builder.Add(LightSetConst::PARAMS_ENABLE_NO_CHANGE_UPDATE, m_tArtNetParams.bEnableNoChangeUpdate, isMaskSet(ArtnetParamsMask::ENABLE_NO_CHANGE_OUTPUT));

	builder.AddComment("DMX Input");
	for (uint32_t i = 0; i < ARTNET_NODE_MAX_PORTS_INPUT; i++) {
		if (!isMaskMultiPortOptionsSet(ArtnetParamsMaskMultiPortOptions::DESTINATION_IP_A << i)) {
			m_tArtNetParams.nDestinationIpPort[i] = ArtNetNode::Get()->GetDestinationIp(i);
		}
		builder.AddIpAddress(ArtNetParamsConst::DESTINATION_IP_PORT[i], m_tArtNetParams.nDestinationIpPort[i], isMaskMultiPortOptionsSet(ArtnetParamsMaskMultiPortOptions::DESTINATION_IP_A << i));
	}

	nSize = builder.GetSize();

	DEBUG_PRINTF("nSize=%d", nSize);
	DEBUG_EXIT
}

void ArtNetParams::Save(char *pBuffer, uint32_t nLength, uint32_t &nSize) {
	if (m_pArtNetParamsStore == nullptr) {
		nSize = 0;
		return;
	}

	Builder(nullptr, pBuffer, nLength, nSize);
}
