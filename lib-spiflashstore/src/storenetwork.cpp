/**
 * @file storenetwork.cpp
 *
 */
/* Copyright (C) 2018-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <algorithm>
#include <stdint.h>
#include <cassert>

#include "spiflashstore.h"

#include "networkparams.h"

#include "debug.h"

StoreNetwork *StoreNetwork::s_pThis = nullptr;

StoreNetwork::StoreNetwork() {
	DEBUG_ENTRY

	assert(s_pThis == nullptr);
	s_pThis = this;

	DEBUG_PRINTF("%p", reinterpret_cast<void *>(s_pThis));
	DEBUG_EXIT
}

void StoreNetwork::Update(const struct TNetworkParams *pNetworkParams) {
	DEBUG_ENTRY

	SpiFlashStore::Get()->Update(STORE_NETWORK, pNetworkParams, sizeof(struct TNetworkParams));

	DEBUG_EXIT
}

void StoreNetwork::Copy(struct TNetworkParams *pNetworkParams) {
	DEBUG_ENTRY

	SpiFlashStore::Get()->Copy(STORE_NETWORK, pNetworkParams, sizeof(struct TNetworkParams));

	DEBUG_EXIT
}

void StoreNetwork::SaveIp(uint32_t nIp) {
	DEBUG_ENTRY

//	DEBUG_PRINTF("offsetof=%d", __builtin_offsetof(struct TNetworkParams, nLocalIp));

	SpiFlashStore::Get()->Update(STORE_NETWORK, __builtin_offsetof(struct TNetworkParams, nLocalIp), &nIp, sizeof(uint32_t), NetworkParamsMask::IP_ADDRESS);

	DEBUG_EXIT
}

void StoreNetwork::SaveNetMask(uint32_t nNetMask) {
	DEBUG_ENTRY

//	DEBUG_PRINTF("offsetof=%d", __builtin_offsetof(struct TNetworkParams, nNetmask));

	SpiFlashStore::Get()->Update(STORE_NETWORK, __builtin_offsetof(struct TNetworkParams, nNetmask), &nNetMask, sizeof(uint32_t), NetworkParamsMask::NET_MASK);

	DEBUG_EXIT
}

void StoreNetwork::SaveHostName(const char *pHostName, uint32_t nLength) {
	DEBUG_ENTRY

//	DEBUG_PRINTF("offsetof=%d", __builtin_offsetof(struct TNetworkParams, aHostName));

	nLength = std::min(nLength,static_cast<uint32_t>(NETWORK_HOSTNAME_SIZE));

	SpiFlashStore::Get()->Update(STORE_NETWORK, __builtin_offsetof(struct TNetworkParams, aHostName), pHostName, nLength, NetworkParamsMask::HOSTNAME);

	DEBUG_EXIT
}

void StoreNetwork::SaveDhcp(bool bIsDhcpUsed) {
	DEBUG_ENTRY

//	DEBUG_PRINTF("offsetof=%d", __builtin_offsetof(struct TNetworkParams, bIsDhcpUsed));

	SpiFlashStore::Get()->Update(STORE_NETWORK, __builtin_offsetof(struct TNetworkParams, bIsDhcpUsed), &bIsDhcpUsed, sizeof(bool), NetworkParamsMask::DHCP);

	DEBUG_EXIT
}
