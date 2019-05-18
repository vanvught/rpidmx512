/**
 * @file artnetparamsset.cpp
 *
 */
/**
 * Art-Net Designed by and Copyright Artistic Licence Holdings Ltd.
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

#include <assert.h>

#include "artnetparams.h"
#include "artnetnode.h"

void ArtNetParams::Set(ArtNetNode *pArtNetNode) {
	assert(pArtNetNode != 0);

	if (m_tArtNetParams.nSetList == 0) {
		return;
	}

	if(isMaskSet(ARTNET_PARAMS_MASK_SHORT_NAME)) {
		pArtNetNode->SetShortName((const char *)m_tArtNetParams.aShortName);
	}

	if(isMaskSet(ARTNET_PARAMS_MASK_LONG_NAME)) {
		pArtNetNode->SetLongName((const char *)m_tArtNetParams.aLongName);
	}

	if(isMaskSet(ARTNET_PARAMS_MASK_NET)) {
		pArtNetNode->SetNetSwitch(m_tArtNetParams.nNet);
	}

	if(isMaskSet(ARTNET_PARAMS_MASK_SUBNET)) {
		pArtNetNode->SetSubnetSwitch(m_tArtNetParams.nSubnet);
	}

	if(isMaskSet(ARTNET_PARAMS_MASK_ID)) {
		pArtNetNode->SetManufacturerId(m_tArtNetParams.aManufacturerId);
	}

	if(isMaskSet(ARTNET_PARAMS_MASK_OEM_VALUE)) {
		pArtNetNode->SetOemValue(m_tArtNetParams.aOemValue);
	}

	if(isMaskSet(ARTNET_PARAMS_MASK_NETWORK_TIMEOUT)) {
		pArtNetNode->SetNetworkTimeout(m_tArtNetParams.nNetworkTimeout);
	}

	if(isMaskSet(ARTNET_PARAMS_MASK_MERGE_TIMEOUT)) {
		pArtNetNode->SetDisableMergeTimeout(m_tArtNetParams.bDisableMergeTimeout);
	}

	unsigned i;

	for (i = 0; i < ARTNET_MAX_PORTS; i++) {
		if (isMaskSet(ARTNET_PARAMS_MASK_MERGE_MODE_A << i)) {
			pArtNetNode->SetMergeMode(i, (TMerge) m_tArtNetParams.nMergeModePort[i]);
		} else {
			pArtNetNode->SetMergeMode(i, (TMerge) m_tArtNetParams.nMergeMode);
		}

		if (isMaskSet(ARTNET_PARAMS_MASK_PROTOCOL_A << i)) {
			pArtNetNode->SetPortProtocol(i, (TPortProtocol) m_tArtNetParams.nProtocolPort[i]);
		} else {
			pArtNetNode->SetPortProtocol(i, (TPortProtocol) m_tArtNetParams.nProtocol);
		}
	}

	for (;i < (ARTNET_MAX_PORTS * ARTNET_MAX_PAGES); i++) {
		pArtNetNode->SetMergeMode(i, (TMerge) m_tArtNetParams.nMergeMode);
		pArtNetNode->SetPortProtocol(i, (TPortProtocol) m_tArtNetParams.nProtocol);
	}
}
