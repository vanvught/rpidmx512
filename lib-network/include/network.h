/**
 * @file network.h
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

#ifndef NETWORK_H_
#define NETWORK_H_

#include <stdint.h>
#include <stdbool.h>

#define NETWORK_IP_SIZE			4
#define NETWORK_MAC_SIZE		6
#define NETWORK_HOSTNAME_SIZE	48

#ifndef IP2STR
#define IP2STR(addr) (uint8_t)(addr & 0xFF), (uint8_t)((addr >> 8) & 0xFF), (uint8_t)((addr >> 16) & 0xFF), (uint8_t)((addr >> 24) & 0xFF)
#define IPSTR "%d.%d.%d.%d"
#endif

#ifndef IP2STR3
#define IP2STR3(addr) (uint8_t)(addr[0]), (uint8_t)(addr[1]), (uint8_t)(addr[2]), (uint8_t)(addr[3])
#define IPSTR3 "%.3d.%.3d.%.3d.%.3d"
#endif

#ifndef MAC2STR
#define MAC2STR(mac) (int)(mac[0]),(int)(mac[1]),(int)(mac[2]),(int)(mac[3]), (int)(mac[4]), (int)(mac[5])
#define MACSTR "%.2x:%.2x:%.2x:%.2x:%.2x:%.2x"
#endif

class Network {
public:
	Network(void);
	virtual ~Network(void);

	virtual void Begin(uint16_t nPort)=0;
	virtual void End(void)=0;

	virtual const char* GetHostName(void)=0;
	virtual void MacAddressCopyTo(uint8_t *pMacAddress)=0;

	inline uint32_t GetIp(void) { return m_nLocalIp; }
	inline uint32_t GetNetmask(void) { return m_nNetmask; }
	inline uint32_t GetBroadcastIp(void) { return m_nBroadcastIp; }

	inline bool IsDhcpCapable(void) { return m_IsDhcpCapable; }
	inline bool IsDhcpUsed(void) { return m_IsDhcpUsed; }
	inline bool IsDhcpKnown(void) {
#if defined (__CYGWIN__) || defined (__APPLE__)
		return false;
#else
		return true;
#endif
	}

#if !defined(__circle__)
	virtual void JoinGroup(uint32_t ip)=0;
#endif

	virtual uint16_t RecvFrom(uint8_t *packet, uint16_t size, uint32_t *from_ip, uint16_t *from_port)=0;
	virtual void SendTo(const uint8_t *packet, uint16_t size, uint32_t to_ip, uint16_t remote_port)=0;

#if defined(__linux__)	||  defined (BARE_METAL)
	virtual void SetIp(uint32_t nIp)=0;
#endif

	void Print(void);

public:
	inline static Network* Get(void) { return s_pThis; }

protected:
	uint8_t m_aNetMacaddr[NETWORK_MAC_SIZE];
	uint32_t m_nLocalIp;
	uint32_t m_nGatewayIp;
	uint32_t m_nNetmask;
	uint32_t m_nBroadcastIp;
	bool m_IsDhcpCapable;
	bool m_IsDhcpUsed;

private:
	static Network *s_pThis;
};

#endif /* NETWORK_H_ */
