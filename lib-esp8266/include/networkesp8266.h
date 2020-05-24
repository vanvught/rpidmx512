/**
 * @file networkesp8266.h
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

#ifndef NETWORKESP8266_H_
#define NETWORKESP8266_H_

#include <stdint.h>

#include "network.h"

class NetworkESP8266: public Network {
public:
	NetworkESP8266(void);
	~NetworkESP8266(void);

	void Init(void);

	int32_t Begin(uint16_t nPort);
	int32_t End(uint16_t nPort);

	void MacAddressCopyTo(uint8_t *pMacAddress);

	void JoinGroup(int32_t nHandle, uint32_t nIp);
	void LeaveGroup(__attribute__((unused)) int32_t nHandle, __attribute__((unused)) uint32_t nIp) {
		// Not supported
	}

	uint16_t RecvFrom(int32_t nHandle, void *pBuffer, uint16_t nLength, uint32_t *pFromIp, uint16_t *pFromPort);
	void SendTo(int32_t nHandle, const void *pBuffer, uint16_t nLength, uint32_t nToIp, uint16_t nRemotePort);

	void Print(void) {

	}

private:
	bool m_IsInitDone;
};

#endif /* NETWORKESP8266_H_ */
