/**
 * @file artnetparamssave.cpp
 *
 */
/* Copyright (C) 2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include <stdint.h>
#include <string.h>
#include <assert.h>

#include "artnetparams.h"
#include "artnetparamsconst.h"

#include "propertiesbuilder.h"

#include "lightset.h"
#include "lightsetconst.h"

#include "debug.h"

#define BOOL2STRING(b)			(b) ? "Yes" : "No"
#define MERGEMODE2STRING(m)		(m == ARTNET_MERGE_HTP) ? "htp" : "ltp"
#define PROTOCOL2STRING(p)		(p == PORT_ARTNET_ARTNET) ? "artnet" : "sacn"

bool ArtNetParams::Builder(const struct TArtNetParams *pArtNetParams, uint8_t *pBuffer, uint32_t nLength, uint32_t& nSize) {
	DEBUG_ENTRY

	if (pArtNetParams != 0) {
		memcpy(&m_tArtNetParams, pArtNetParams, sizeof(struct TArtNetParams));
	} else {
		m_pArtNetParamsStore->Copy(&m_tArtNetParams);
	}

	PropertiesBuilder builder(ArtNetParamsConst::FILE_NAME, pBuffer, nLength);

	bool isAdded = builder.Add(ArtNetParamsConst::NET, (uint32_t) m_tArtNetParams.nNet, isMaskSet(ARTNET_PARAMS_MASK_NET));
	isAdded &= builder.Add(ArtNetParamsConst::SUBNET, (uint32_t) m_tArtNetParams.nSubnet, isMaskSet(ARTNET_PARAMS_MASK_SUBNET));
	isAdded &= builder.Add(LightSetConst::PARAMS_UNIVERSE, (uint32_t) m_tArtNetParams.nUniverse, isMaskSet(ARTNET_PARAMS_MASK_UNIVERSE));

	isAdded &= builder.Add(LightSetConst::PARAMS_OUTPUT, (const char *) LightSet::GetOutputType(m_tArtNetParams.tOutputType), isMaskSet(ARTNET_PARAMS_MASK_OUTPUT));

	isAdded &= builder.Add(ArtNetParamsConst::NODE_LONG_NAME, (const char *) m_tArtNetParams.aLongName, isMaskSet(ARTNET_PARAMS_MASK_LONG_NAME));
	isAdded &= builder.Add(ArtNetParamsConst::NODE_SHORT_NAME, (const char *) m_tArtNetParams.aShortName, isMaskSet(ARTNET_PARAMS_MASK_SHORT_NAME));

	isAdded &= builder.AddHex16(ArtNetParamsConst::NODE_MANUFACTURER_ID, m_tArtNetParams.aManufacturerId, isMaskSet(ARTNET_PARAMS_MASK_ID));
	isAdded &= builder.AddHex16(ArtNetParamsConst::NODE_OEM_VALUE, m_tArtNetParams.aOemValue, isMaskSet(ARTNET_PARAMS_MASK_OEM_VALUE));

	isAdded &= builder.Add(ArtNetParamsConst::RDM, (uint32_t) m_tArtNetParams.bEnableRdm, isMaskSet(ARTNET_PARAMS_MASK_RDM));
	isAdded &= builder.Add(ArtNetParamsConst::TIMECODE, (uint32_t) m_tArtNetParams.bUseTimeCode, isMaskSet(ARTNET_PARAMS_MASK_TIMECODE));
	isAdded &= builder.Add(ArtNetParamsConst::TIMESYNC, (uint32_t) m_tArtNetParams.bUseTimeSync, isMaskSet(ARTNET_PARAMS_MASK_TIMESYNC));

	isAdded &= builder.Add(ArtNetParamsConst::MERGE_MODE, (const char *) MERGEMODE2STRING(m_tArtNetParams.nMergeMode), isMaskSet(ARTNET_PARAMS_MASK_MERGE_MODE));
	isAdded &= builder.Add(ArtNetParamsConst::PROTOCOL, (const char *) PROTOCOL2STRING(m_tArtNetParams.nProtocol), isMaskSet(ARTNET_PARAMS_MASK_PROTOCOL));

	for (unsigned i = 0; i < ARTNET_MAX_PORTS; i++) {
		isAdded &= builder.Add(ArtNetParamsConst::UNIVERSE_PORT[i], (uint32_t) m_tArtNetParams.nUniversePort[i], isMaskSet(ARTNET_PARAMS_MASK_UNIVERSE_A << i));
		isAdded &= builder.Add(ArtNetParamsConst::MERGE_MODE_PORT[i], (const char *) MERGEMODE2STRING(m_tArtNetParams.nMergeModePort[i]), isMaskSet(ARTNET_PARAMS_MASK_MERGE_MODE_A << i));
		isAdded &= builder.Add(ArtNetParamsConst::PROTOCOL_PORT[i], (const char *) PROTOCOL2STRING(m_tArtNetParams.nProtocolPort[i]), isMaskSet(ARTNET_PARAMS_MASK_PROTOCOL_A << i));
	}

	isAdded &= builder.Add(ArtNetParamsConst::NODE_NETWORK_DATA_LOSS_TIMEOUT, (uint32_t) m_tArtNetParams.nNetworkTimeout, isMaskSet(ARTNET_PARAMS_MASK_NETWORK_TIMEOUT));
	isAdded &= builder.Add(ArtNetParamsConst::NODE_DISABLE_MERGE_TIMEOUT, (uint32_t) m_tArtNetParams.bDisableMergeTimeout, isMaskSet(ARTNET_PARAMS_MASK_MERGE_TIMEOUT));

	nSize = builder.GetSize();

	DEBUG_PRINTF("isAdded=%d, nSize=%d", isAdded, nSize);

	DEBUG_EXIT
	return isAdded;
}

bool ArtNetParams::Save(uint8_t* pBuffer, uint32_t nLength, uint32_t& nSize) {
	DEBUG_ENTRY

	if (m_pArtNetParamsStore == 0) {
		nSize = 0;
		DEBUG_EXIT
		return false;
	}

	return Builder(0, pBuffer, nLength, nSize);
}
