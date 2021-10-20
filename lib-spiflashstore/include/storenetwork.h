/**
 * @file storenetwork.h
 *
 */
/* Copyright (C) 2018-2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef STORENETWORK_H_
#define STORENETWORK_H_

#include <algorithm>

#include "network.h"
#include "networkparams.h"
#include "spiflashstore.h"

class StoreNetwork final: public NetworkParamsStore, public NetworkStore {
public:
	StoreNetwork();

	void Update(const struct TNetworkParams *pNetworkParams) override {
		SpiFlashStore::Get()->Update(spiflashstore::Store::NETWORK, pNetworkParams, sizeof(struct TNetworkParams));
	}

	void Copy(struct TNetworkParams *pNetworkParams) override {
		SpiFlashStore::Get()->Copy(spiflashstore::Store::NETWORK, pNetworkParams, sizeof(struct TNetworkParams));
	}

	void SaveIp(uint32_t nIp) override {
		SpiFlashStore::Get()->Update(spiflashstore::Store::NETWORK, __builtin_offsetof(struct TNetworkParams, nLocalIp), &nIp, sizeof(uint32_t), NetworkParamsMask::IP_ADDRESS);
	}

	void SaveNetMask(uint32_t nNetMask) override {
		SpiFlashStore::Get()->Update(spiflashstore::Store::NETWORK, __builtin_offsetof(struct TNetworkParams, nNetmask), &nNetMask, sizeof(uint32_t), NetworkParamsMask::NET_MASK);
	}

	void SaveGatewayIp(uint32_t nGatewayIp) override {
		SpiFlashStore::Get()->Update(spiflashstore::Store::NETWORK, __builtin_offsetof(struct TNetworkParams, nGatewayIp), &nGatewayIp, sizeof(uint32_t), NetworkParamsMask::DEFAULT_GATEWAY);
	}

	void SaveHostName(const char *pHostName, uint32_t nLength) override {
		nLength = std::min(nLength,static_cast<uint32_t>(network::HOSTNAME_SIZE));
		SpiFlashStore::Get()->Update(spiflashstore::Store::NETWORK, __builtin_offsetof(struct TNetworkParams, aHostName), pHostName, nLength, NetworkParamsMask::HOSTNAME);
	}

	void SaveDhcp(bool bIsDhcpUsed) override {
		SpiFlashStore::Get()->Update(spiflashstore::Store::NETWORK, __builtin_offsetof(struct TNetworkParams, bIsDhcpUsed), &bIsDhcpUsed, sizeof(bool), NetworkParamsMask::DHCP);
	}

	static StoreNetwork *Get() {
		return s_pThis;
	}

private:
	static StoreNetwork *s_pThis;
};

#endif /* STORENETWORK_H_ */
