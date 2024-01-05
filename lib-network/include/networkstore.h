/**
 * @file networkstore.h
 *
 */
/* Copyright (C) 2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef NETWORKSTORE_H_
#define NETWORKSTORE_H_

#include <cstdint>
#include <cstddef>
#include <algorithm>

#include "networkparams.h"
#include "configstore.h"

class NetworkStore {
public:
	static void SaveIp(uint32_t nIp) {
		ConfigStore::Get()->Update(configstore::Store::NETWORK, offsetof(struct networkparams::Params, nLocalIp), &nIp, sizeof(uint32_t), networkparams::Mask::IP_ADDRESS);
	}

	static void SaveNetMask(uint32_t nNetMask) {
		ConfigStore::Get()->Update(configstore::Store::NETWORK, offsetof(struct networkparams::Params, nNetmask), &nNetMask, sizeof(uint32_t), networkparams::Mask::NET_MASK);
	}

	static void SaveGatewayIp(uint32_t nGatewayIp) {
		ConfigStore::Get()->Update(configstore::Store::NETWORK, offsetof(struct networkparams::Params, nGatewayIp), &nGatewayIp, sizeof(uint32_t), networkparams::Mask::DEFAULT_GATEWAY);
	}

	static void SaveHostName(const char *pHostName, uint32_t nLength) {
		nLength = std::min(nLength,static_cast<uint32_t>(network::HOSTNAME_SIZE));
		ConfigStore::Get()->Update(configstore::Store::NETWORK, offsetof(struct networkparams::Params, aHostName), pHostName, nLength, networkparams::Mask::HOSTNAME);
	}

	static void SaveDhcp(bool bIsDhcpUsed) {
		ConfigStore::Get()->Update(configstore::Store::NETWORK, offsetof(struct networkparams::Params, bIsDhcpUsed), &bIsDhcpUsed, sizeof(bool), networkparams::Mask::DHCP);
	}
};

#endif /* NETWORKSTORE_H_ */
