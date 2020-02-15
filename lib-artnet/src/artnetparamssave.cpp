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
#include <assert.h>

#include "artnetparams.h"
#include "artnetparamsconst.h"
#include "artnetnode.h"

#include "propertiesbuilder.h"

#include "lightsetconst.h"

#include "debug.h"

#define BOOL2STRING(b)			(b) ? "Yes" : "No"
#define MERGEMODE2STRING(m)		(m == ARTNET_MERGE_HTP) ? "htp" : "ltp"
#define PROTOCOL2STRING(p)		(p == PORT_ARTNET_ARTNET) ? "artnet" : "sacn"

void ArtNetParams::Builder(const struct TArtNetParams *pArtNetParams, uint8_t *pBuffer, uint32_t nLength, uint32_t &nSize) {
	DEBUG_ENTRY

	assert(pBuffer != 0);

	if (pArtNetParams != 0) {
		memcpy(&m_tArtNetParams, pArtNetParams, sizeof(struct TArtNetParams));
	} else {
		m_pArtNetParamsStore->Copy(&m_tArtNetParams);
	}

	PropertiesBuilder builder(ArtNetParamsConst::FILE_NAME, pBuffer, nLength);

	builder.Add(ArtNetParamsConst::NET, m_tArtNetParams.nNet, isMaskSet(ARTNET_PARAMS_MASK_NET));
	builder.Add(ArtNetParamsConst::SUBNET, m_tArtNetParams.nSubnet, isMaskSet(ARTNET_PARAMS_MASK_SUBNET));
	builder.Add(LightSetConst::PARAMS_UNIVERSE, m_tArtNetParams.nUniverse, isMaskSet(ARTNET_PARAMS_MASK_UNIVERSE));

	builder.Add(ArtNetParamsConst::NODE_LONG_NAME, (const char *) m_tArtNetParams.aLongName, isMaskSet(ARTNET_PARAMS_MASK_LONG_NAME));
	builder.Add(ArtNetParamsConst::NODE_SHORT_NAME, (const char *) m_tArtNetParams.aShortName, isMaskSet(ARTNET_PARAMS_MASK_SHORT_NAME));

	builder.AddHex16(ArtNetParamsConst::NODE_OEM_VALUE, m_tArtNetParams.aOemValue, isMaskSet(ARTNET_PARAMS_MASK_OEM_VALUE));

	builder.Add(ArtNetParamsConst::RDM, m_tArtNetParams.bEnableRdm, isMaskSet(ARTNET_PARAMS_MASK_RDM));
	builder.Add(ArtNetParamsConst::TIMECODE, m_tArtNetParams.bUseTimeCode, isMaskSet(ARTNET_PARAMS_MASK_TIMECODE));
	builder.Add(ArtNetParamsConst::TIMESYNC, m_tArtNetParams.bUseTimeSync, isMaskSet(ARTNET_PARAMS_MASK_TIMESYNC));

	builder.Add(ArtNetParamsConst::MERGE_MODE, (const char *) MERGEMODE2STRING(m_tArtNetParams.nMergeMode), isMaskSet(ARTNET_PARAMS_MASK_MERGE_MODE));
	builder.Add(ArtNetParamsConst::PROTOCOL, (const char *) PROTOCOL2STRING(m_tArtNetParams.nProtocol), isMaskSet(ARTNET_PARAMS_MASK_PROTOCOL));

	for (unsigned i = 0; i < ARTNET_MAX_PORTS; i++) {
		builder.Add(ArtNetParamsConst::UNIVERSE_PORT[i], m_tArtNetParams.nUniversePort[i], isMaskSet(ARTNET_PARAMS_MASK_UNIVERSE_A << i));
		builder.Add(ArtNetParamsConst::MERGE_MODE_PORT[i], (const char *) MERGEMODE2STRING(m_tArtNetParams.nMergeModePort[i]), isMaskSet(ARTNET_PARAMS_MASK_MERGE_MODE_A << i));
		builder.Add(ArtNetParamsConst::PROTOCOL_PORT[i], (const char *) PROTOCOL2STRING(m_tArtNetParams.nProtocolPort[i]), isMaskSet(ARTNET_PARAMS_MASK_PROTOCOL_A << i));
	}

	builder.Add(ArtNetParamsConst::NODE_NETWORK_DATA_LOSS_TIMEOUT, m_tArtNetParams.nNetworkTimeout, isMaskSet(ARTNET_PARAMS_MASK_NETWORK_TIMEOUT));
	builder.Add(ArtNetParamsConst::NODE_DISABLE_MERGE_TIMEOUT, m_tArtNetParams.bDisableMergeTimeout, isMaskSet(ARTNET_PARAMS_MASK_MERGE_TIMEOUT));

	builder.Add(LightSetConst::PARAMS_ENABLE_NO_CHANGE_UPDATE, m_tArtNetParams.bEnableNoChangeUpdate, isMaskSet(ARTNET_PARAMS_MASK_ENABLE_NO_CHANGE_OUTPUT));

	// DMX Input
	builder.Add(ArtNetParamsConst::DIRECTION, m_tArtNetParams.nDirection == (uint8_t) ARTNET_INPUT_PORT ? "input" : "output" , isMaskSet(ARTNET_PARAMS_MASK_DIRECTION));

	if (!isMaskSet(ARTNET_PARAMS_MASK_DESTINATION_IP)) {
		m_tArtNetParams.nDestinationIp = ArtNetNode::Get()->GetDestinationIp();
	}
	builder.AddIpAddress(ArtNetParamsConst::DESTINATION_IP, m_tArtNetParams.nDestinationIp, isMaskSet(ARTNET_PARAMS_MASK_DESTINATION_IP));

	nSize = builder.GetSize();

	DEBUG_PRINTF("nSize=%d", nSize);

	DEBUG_EXIT
	return;
}

void ArtNetParams::Save(uint8_t* pBuffer, uint32_t nLength, uint32_t &nSize) {
	DEBUG_ENTRY

	if (m_pArtNetParamsStore == 0) {
		nSize = 0;
		DEBUG_EXIT
		return;
	}

	Builder(0, pBuffer, nLength, nSize);

	return;
}
