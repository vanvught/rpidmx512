/**
 * @file networkparams.h
 *
 */
/* Copyright (C) 2017-2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
	uint32_t bSetList;
	uint32_t nLocalIp;
	uint32_t nNetmask;
	uint32_t nGatewayIp;
	uint32_t nNameServerIp;
	bool bIsDhcpUsed;
	uint8_t aHostName[NETWORK_HOSTNAME_SIZE];
};

class NetworkParamsStore {
public:
	virtual ~NetworkParamsStore(void);

	virtual void Update(const struct TNetworkParams *pNetworkParams)=0;
	virtual void Copy(struct TNetworkParams *pNetworkParams)=0;

private:

};

class NetworkParams {
public:
	NetworkParams(NetworkParamsStore *pNetworkParamsStore = 0);
	~NetworkParams(void);

	bool Load(void);
	void Dump(void);

	inline bool isDhcpUsed(void) {
		return m_tNetworkParams.bIsDhcpUsed;
	}

	inline uint32_t GetIpAddress(void) {
		return m_tNetworkParams.nLocalIp;
	}

	inline uint32_t GetNetMask(void) {
		return m_tNetworkParams.nNetmask;
	}

	inline uint32_t GetDefaultGateway(void) {
		return m_tNetworkParams.nGatewayIp;
	}

	inline uint32_t GetNameServer(void) {
		return m_tNetworkParams.nNameServerIp;
	}

	inline const uint8_t *GetHostName(void) {
		return m_tNetworkParams.aHostName;
	}

public:
	static uint32_t GetMaskDhcpUsed(void);
	static uint32_t GetMaskIpAddress(void);
	static uint32_t GetMaskNetMask(void);

private:
	bool isMaskSet(uint32_t nMask) const;

public:
    static void staticCallbackFunction(void *p, const char *s);

private:
    void callbackFunction(const char *s);

private:
    NetworkParamsStore 		*m_pNetworkParamsStore;
    struct TNetworkParams	m_tNetworkParams;
};

#endif /* NETWORKPARAMS_H_ */
