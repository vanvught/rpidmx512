/**
 * @file artnetparamsset.cpp
 *
 */
/**
 * Art-Net Designed by and Copyright Artistic Licence Holdings Ltd.
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

#include <cassert>

#include "artnetparams.h"

#include "artnetnode.h"
#include "artnet.h"

void ArtNetParams::Set(ArtNetNode *pArtNetNode) {
	assert(pArtNetNode != nullptr);

	if (m_tArtNetParams.nSetList == 0) {
		return;
	}

	if (isMaskSet(ArtnetParamsMask::SHORT_NAME)) {
		pArtNetNode->SetShortName(reinterpret_cast<const char*>(m_tArtNetParams.aShortName));
	}

	if (isMaskSet(ArtnetParamsMask::LONG_NAME)) {
		pArtNetNode->SetLongName(reinterpret_cast<const char*>(m_tArtNetParams.aLongName));
	}

	if (isMaskSet(ArtnetParamsMask::NET)) {
		pArtNetNode->SetNetSwitch(m_tArtNetParams.nNet);
	}

	if (isMaskSet(ArtnetParamsMask::SUBNET)) {
		pArtNetNode->SetSubnetSwitch(m_tArtNetParams.nSubnet);
	}

	if (isMaskSet(ArtnetParamsMask::OEM_VALUE)) {
		pArtNetNode->SetOemValue(m_tArtNetParams.aOemValue);
	}

	if (isMaskSet(ArtnetParamsMask::NETWORK_TIMEOUT)) {
		pArtNetNode->SetNetworkTimeout(static_cast<uint32_t>(m_tArtNetParams.nNetworkTimeout));
	}

	if (isMaskSet(ArtnetParamsMask::MERGE_TIMEOUT)) {
		pArtNetNode->SetDisableMergeTimeout(m_tArtNetParams.bDisableMergeTimeout);
	}

	unsigned i;

	for (i = 0; i < ArtNet::MAX_PORTS; i++) {
		if (isMaskSet(ArtnetParamsMask::MERGE_MODE_A << i)) {
			pArtNetNode->SetMergeMode(i, static_cast<ArtNetMerge>(m_tArtNetParams.nMergeModePort[i]));
		} else {
			pArtNetNode->SetMergeMode(i, static_cast<ArtNetMerge>(m_tArtNetParams.nMergeMode));
		}

		if (isMaskSet(ArtnetParamsMask::PROTOCOL_A << i)) {
			pArtNetNode->SetPortProtocol(i, static_cast<TPortProtocol>(m_tArtNetParams.nProtocolPort[i]));
		} else {
			pArtNetNode->SetPortProtocol(i, static_cast<TPortProtocol>(m_tArtNetParams.nProtocol));
		}

		if (isMaskMultiPortOptionsSet(ArtnetParamsMaskMultiPortOptions::DESTINATION_IP_A << i)) {
			pArtNetNode->SetDestinationIp(i, m_tArtNetParams.nDestinationIpPort[i]);
		}
	}

	for (;i < (ArtNet::MAX_PORTS * ArtNet::MAX_PAGES); i++) {
		pArtNetNode->SetMergeMode(i, static_cast<ArtNetMerge>(m_tArtNetParams.nMergeMode));
		pArtNetNode->SetPortProtocol(i, static_cast<TPortProtocol>(m_tArtNetParams.nProtocol));
	}

	if (isMaskSet(ArtnetParamsMask::ENABLE_NO_CHANGE_OUTPUT)) {
		pArtNetNode->SetDirectUpdate(m_tArtNetParams.bEnableNoChangeUpdate);
	}
}
