/**
 * @file networkbaremetalmacaddress.h
 *
 */
/* Copyright (C) 2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

class NetworkBaremetalMacAddress: public Network {
public:
	NetworkBaremetalMacAddress(void);
	~NetworkBaremetalMacAddress(void);

	void MacAddressCopyTo(uint8_t *pMacAddress);

	// Dummy
	inline void Begin(uint16_t nPort) {	}
	inline void End(void) {	}
	inline const char* GetHostName(void) { return ""; }
	inline void JoinGroup(uint32_t nIp) { }
	inline uint16_t RecvFrom(uint8_t *packet, uint16_t nSize, uint32_t *from_ip, uint16_t *from_port) { return 0; }
	inline void SendTo(const uint8_t *packet, uint16_t nSize, uint32_t to_ip, uint16_t remote_port) { }
	inline void SetIp(uint32_t nIp) { }
};

#endif /* NETWORKBAREMETALMACADDRESS_H_ */
