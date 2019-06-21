/**
 * @file networkparams.h
 *
 */
/* Copyright (C) 2017-2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#ifndef NETWORKPARAMS_H_
#define NETWORKPARAMS_H_

#include <stdbool.h>
#include <stdint.h>

#include "network.h"

struct TNetworkParams {
	uint32_t nSetList;
	uint32_t nLocalIp;
	uint32_t nNetmask;
	uint32_t nGatewayIp;
	uint32_t nNameServerIp;
	bool bIsDhcpUsed;
	uint8_t aHostName[NETWORK_HOSTNAME_SIZE];
	bool bResetEmac;
};

enum TNetworkParamsMask {
	NETWORK_PARAMS_MASK_DHCP = (1 << 0),
	NETWORK_PARAMS_MASK_IP_ADDRESS = (1 << 1),
	NETWORK_PARAMS_MASK_NET_MASK = (1 << 2),
	NETWORK_PARAMS_MASK_DEFAULT_GATEWAY = (1 << 3),
	NETWORK_PARAMS_MASK_NAME_SERVER = (1 << 4),
	NETWORK_PARAMS_MASK_HOSTNAME = (1 << 5),
	NETWORK_PARAMS_MASK_EMAC = (1 << 6)
};

class NetworkParamsStore {
public:
	virtual ~NetworkParamsStore(void);

	virtual void Update(const struct TNetworkParams *pNetworkParams)=0;
	virtual void Copy(struct TNetworkParams *pNetworkParams)=0;
};

class NetworkParams {
public:
	NetworkParams(NetworkParamsStore *pNetworkParamsStore = 0);
	~NetworkParams(void);

	bool Load(void);
	void Load(const char *pBuffer, uint32_t nLength);

	bool Builder(const struct TNetworkParams *ptNetworkParams, uint8_t* pBuffer, uint32_t nLength, uint32_t& nSize);
	bool Save(uint8_t* pBuffer, uint32_t nLength, uint32_t& nSize);

	void Dump(void);

	bool isDhcpUsed(void) {
		return m_tNetworkParams.bIsDhcpUsed;
	}

	uint32_t GetIpAddress(void) {
		return m_tNetworkParams.nLocalIp;
	}

	uint32_t GetNetMask(void) {
		return m_tNetworkParams.nNetmask;
	}

	const uint8_t *GetHostName(void) {
		return m_tNetworkParams.aHostName;
	}

	uint32_t GetDefaultGateway(void) {
		return m_tNetworkParams.nGatewayIp;
	}

	uint32_t GetNameServer(void) {
		return m_tNetworkParams.nNameServerIp;
	}

	// Allwinner H2+/H3 Only
	bool GetResetEmac(void) {
		return m_tNetworkParams.bResetEmac;
	}

public:
    static void staticCallbackFunction(void *p, const char *s);

private:
    void callbackFunction(const char *s);
	bool isMaskSet(uint32_t nMask) const;

private:
	NetworkParamsStore *m_pNetworkParamsStore;
	struct TNetworkParams m_tNetworkParams;
};

#endif /* NETWORKPARAMS_H_ */
