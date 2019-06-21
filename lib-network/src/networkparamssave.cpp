/**
 * @file networkparams.cpp
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

#include "networkparams.h"
#include "networkconst.h"

#include "propertiesbuilder.h"

#include "debug.h"

bool NetworkParams::Builder(const struct TNetworkParams *ptNetworkParams, uint8_t* pBuffer, uint32_t nLength, uint32_t& nSize) {
	DEBUG_ENTRY

	if (ptNetworkParams != 0) {
		memcpy(&m_tNetworkParams, ptNetworkParams, sizeof(struct TNetworkParams));
	} else {
		m_pNetworkParamsStore->Copy(&m_tNetworkParams);
	}

	PropertiesBuilder builder(NetworkConst::PARAMS_FILE_NAME, pBuffer, nLength);

	bool isAdded = builder.Add(NetworkConst::PARAMS_USE_DHCP, (uint32_t) m_tNetworkParams.bIsDhcpUsed, isMaskSet(NETWORK_PARAMS_MASK_DHCP));
	isAdded &= builder.AddIpAddress(NetworkConst::PARAMS_IP_ADDRESS, m_tNetworkParams.nLocalIp, isMaskSet(NETWORK_PARAMS_MASK_IP_ADDRESS));
	isAdded &= builder.AddIpAddress(NetworkConst::PARAMS_NET_MASK, m_tNetworkParams.nNetmask, isMaskSet(NETWORK_PARAMS_MASK_NET_MASK));
	isAdded &= builder.AddIpAddress(NetworkConst::PARAMS_DEFAULT_GATEWAY, m_tNetworkParams.nGatewayIp, isMaskSet(NETWORK_PARAMS_MASK_DEFAULT_GATEWAY));
	isAdded &= builder.Add(NetworkConst::PARAMS_HOSTNAME, (const char *)m_tNetworkParams.aHostName, isMaskSet(NETWORK_PARAMS_MASK_HOSTNAME));
	isAdded &= builder.Add(NetworkConst::PARAMS_RESET_EMAC, (uint32_t) m_tNetworkParams.bResetEmac, isMaskSet(NETWORK_PARAMS_MASK_EMAC));

	nSize = builder.GetSize();

	DEBUG_PRINTF("isAdded=%d, nSize=%d", isAdded, nSize);

	DEBUG_EXIT
	return isAdded;
}

bool NetworkParams::Save(uint8_t* pBuffer, uint32_t nLength, uint32_t& nSize) {
	DEBUG_ENTRY

	if (m_pNetworkParamsStore == 0) {
		nSize = 0;
		DEBUG_EXIT
		return false;
	}

	return Builder(0, pBuffer, nLength, nSize);
}
