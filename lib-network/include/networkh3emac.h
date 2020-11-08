/**
 * networkh3emac.h
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

#ifndef NETWORKH3EMAC_H_
#define NETWORKH3EMAC_H_

#include <stdint.h>

#include "network.h"

#include "networkparams.h"

extern "C" {
	void net_handle(void);
}

class NetworkH3emac final : public Network {
public:
	NetworkH3emac();
	~NetworkH3emac();

	void Init(NetworkParamsStore *pNetworkParamsStore = nullptr);

	void Shutdown() override;

	int32_t Begin(uint16_t nPort) override;
	int32_t End(uint16_t nPort) override;

	void MacAddressCopyTo(uint8_t *pMacAddress) override;

	void JoinGroup(int32_t nHandle, uint32_t nIp) override;
	void LeaveGroup(int32_t nHandle, uint32_t nIp) override;

	uint16_t RecvFrom(int32_t nHandle, void *pBuffer, uint16_t nLength, uint32_t *pFromIp, uint16_t *pFromPort) override;
	void SendTo(int32_t nHandle, const void *pBuffer, uint16_t nLength, uint32_t nToIp, uint16_t nRemotePort) override;

	void SetIp(uint32_t nIp) override;
	void SetNetmask(uint32_t nNetmask) override;
	void SetHostName(const char *pHostName) override;
	bool SetZeroconf() override;

	bool EnableDhcp() override; 

	void Run() {
		net_handle();
	}

private:
	void SetDefaultIp();
};

#endif /* NETWORKH3EMAC_H_ */
