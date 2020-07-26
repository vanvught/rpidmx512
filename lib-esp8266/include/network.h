/**
 * @file network.h
 *
 */
/* Copyright (C) 2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef NETWORK_H_
#define NETWORK_H_

#if !defined(ESP8266)
# error ESP8266 must be defined
#endif

#include <stdint.h>
#include <net/if.h>

enum TNetwork {
	NETWORK_IP_SIZE = 4,
	NETWORK_MAC_SIZE = 6,
	NETWORK_HOSTNAME_SIZE = 64,		/* including a terminating null byte. */
	NETWORK_DOMAINNAME_SIZE = 64	/* including a terminating null byte. */
};

#define IP2STR(addr) (addr & 0xFF), ((addr >> 8) & 0xFF), ((addr >> 16) & 0xFF), ((addr >> 24) & 0xFF)
#define IPSTR "%d.%d.%d.%d"

#define MAC2STR(mac) static_cast<int>(mac[0]),static_cast<int>(mac[1]),static_cast<int>(mac[2]),static_cast<int>(mac[3]), static_cast<int>(mac[4]), static_cast<int>(mac[5])
#define MACSTR "%.2x:%.2x:%.2x:%.2x:%.2x:%.2x"

class NetworkStore {
public:
	virtual ~NetworkStore() {}

	virtual void SaveIp(uint32_t nIp)=0;
	virtual void SaveNetMask(uint32_t nNetMask)=0;
	virtual void SaveHostName(const char *pHostName, uint32_t nLength)=0;
	virtual void SaveDhcp(bool bIsDhcpUsed)=0;
};

class Network {
public:
	Network();
	~Network() {

	}

	uint32_t GetIp() {
		return m_nLocalIp;
	}

	uint32_t GetNetmask() {
		return m_nNetmask;
	}

	 bool IsDhcpKnown() {
		return true;
	}

	bool IsDhcpUsed() {
		return m_IsDhcpUsed;
	}

	static Network *Get() {
		return s_pThis;
	}

protected:
	char m_aIfName[IFNAMSIZ];
	uint8_t m_aNetMacaddr[NETWORK_MAC_SIZE];
	uint32_t m_nLocalIp;
	uint32_t m_nGatewayIp;
	uint32_t m_nNetmask;
	char m_aHostName[NETWORK_HOSTNAME_SIZE];
	bool m_IsDhcpUsed;

private:
	static Network *s_pThis;
};

#endif /* NETWORK_H_ */
