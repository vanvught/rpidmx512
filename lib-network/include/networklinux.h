/**
 * @file networklinux.h
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

#ifndef NETWORKLINUX_H_
#define NETWORKLINUX_H_

#include <cstdint>

#include "network.h"

class NetworkLinux: public Network {
public:
	NetworkLinux();
	~NetworkLinux();

	int Init(const char *s);

	int32_t Begin(uint16_t nPort) override;
	int32_t End(uint16_t nPort) override;

	void MacAddressCopyTo(uint8_t *pMacAddress) override;

	void SetIp(uint32_t nIp) override;
	void SetNetmask(uint32_t nNetmask) override;
	void SetGatewayIp(uint32_t nGatewayIp) override;
	bool SetZeroconf() override {
		return false;
	}
	bool EnableDhcp() override {
		return false;
	}

	void SetHostName(const char *pHostName) override;

	void JoinGroup(int32_t nHandle, uint32_t nIp) override;
	void LeaveGroup(int32_t nHandle, uint32_t nIp) override;

	uint16_t RecvFrom(int32_t nHandle, void *pBuffer, uint16_t nLength, uint32_t *pFromIp, uint16_t *pFromPort) override;
	void SendTo(int32_t nHandle, const void *pBuffer, uint16_t nLength, uint32_t nToIp, uint16_t nRemotePort) override;

private:
	uint32_t GetDefaultGateway();
	bool IsDhclient(const char *pIfName);
	int IfGetByAddress(const char *pIp, char *pName, size_t nLength);
	int IfDetails(const char *pIfInterface);
#if defined(__APPLE__)
	bool OSxGetMacaddress(const char *pIfName, uint8_t *pMacAddress);
#endif
};

#endif /* NETWORKLINUX_H_ */
