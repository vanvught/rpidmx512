/**
 * @file networkbaremetalmacaddress.h
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

#ifndef NETWORKBAREMETALMACADDRESS_H_
#define NETWORKBAREMETALMACADDRESS_H_

#include <stdint.h>

#include "network.h"

class NetworkBaremetalMacAddress final : public Network {
public:
	NetworkBaremetalMacAddress();
	~NetworkBaremetalMacAddress() override;

	void MacAddressCopyTo(uint8_t *pMacAddress) override;

	// Dummy methods - not implemented virtual

	int32_t Begin(__attribute__((unused))  uint16_t nPort) override {
		return 0;
	}

	int32_t End(__attribute__((unused))  uint16_t nPort) override {
		return 0;
	}

	void JoinGroup(__attribute__((unused))  int32_t nHandle, __attribute__((unused))  uint32_t nIp) override {
	}

	void LeaveGroup(__attribute__((unused))  int32_t nHandle, __attribute__((unused))  uint32_t nIp) override {
	}

	uint16_t RecvFrom(__attribute__((unused)) int32_t nHandle, __attribute__((unused)) void *pBuffer, __attribute__((unused)) uint16_t nLength, __attribute__((unused)) uint32_t *pFromIp, __attribute__((unused)) uint16_t *pFromPort) override {
		return 0;
	}
	void SendTo(__attribute__((unused)) int32_t nHandle, __attribute__((unused)) const void *pBuffer, __attribute__((unused)) uint16_t nLength, __attribute__((unused)) uint32_t nToIp, __attribute__((unused)) uint16_t nRemotePort) override {
	}

	void SetIp(__attribute__((unused)) uint32_t nIp) override {
	}

	void SetNetmask(__attribute__((unused))  uint32_t nNetmask) override {
	}

	bool SetZeroconf() override {
		return false;
	}

	bool EnableDhcp() override {
		return false;
	}
};

#endif /* NETWORKBAREMETALMACADDRESS_H_ */
