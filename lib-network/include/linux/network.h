/**
 * @file network.h
 *
 */
/* Copyright (C) 2017-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef LINUX_NETWORK_H_
#define LINUX_NETWORK_H_

#if defined(__linux__) || defined (__APPLE__)
#else
# error
#endif

#include <cstdint>
#include <cstring>
#include <net/if.h>

#include "networkparams.h"

namespace net {
typedef void (*UdpCallbackFunctionPtr)(const uint8_t *, uint32_t, uint32_t, uint16_t);
typedef void (*TcpCallbackFunctionPtr)(const int32_t, const uint8_t *, const uint32_t);
}  // namespace net

class Network {
public:
	Network(int argc, char **argv);
	~Network();

	void Print();

	void Shutdown() {}

	int32_t Begin(uint16_t nPort, net::UdpCallbackFunctionPtr callback = nullptr);
	int32_t End(uint16_t nPort);

	void MacAddressCopyTo(uint8_t *pMacAddress);

	void JoinGroup(int32_t nHandle, uint32_t nIp);
	void LeaveGroup(int32_t nHandle, uint32_t nIp);

	uint32_t RecvFrom(int32_t nHandle, void *pBuffer, uint32_t nLength, uint32_t *pFromIp, uint16_t *pFromPort);
	uint32_t RecvFrom(int32_t nHandle, const void **ppBuffer, uint32_t *pFromIp, uint16_t *pFromPort);
	void SendTo(int32_t nHandle, const void *pBuffer, uint32_t nLength, uint32_t nToIp, uint16_t nRemotePort);

	void SetIp(uint32_t nIp);
	void SetNetmask(uint32_t nNetmask);
	void SetGatewayIp(uint32_t nGatewayIp);

	void SetHostName(const char *pHostName);

	void SetDomainName(const char *pDomainName) {
		strncpy(m_aDomainName, pDomainName, net::DOMAINNAME_SIZE - 1);
		m_aDomainName[net::DOMAINNAME_SIZE - 1] = '\0';
	}

	uint32_t GetNameServer(const uint32_t nIndex) const {
		if (nIndex < net::NAMESERVERS_COUNT) {
			return m_nNameservers[nIndex];
		}

		return 0;
	}

	uint32_t GetNameServers() const {
		return net::NAMESERVERS_COUNT;
	}

	uint32_t GetSecondaryIp() const {
		return m_nLocalIp;
	}

	uint32_t GetIp() const {
		return m_nLocalIp;
	}

	const char *GetHostName() const {
		return m_aHostName;
	}

	const char *GetDomainName() const {
		return m_aDomainName;
	}

	bool SetZeroconf() {
		return false;
	}
	bool EnableDhcp() {
		return false;
	}

	uint32_t GetGatewayIp() const {
		return m_nGatewayIp;
	}

	uint32_t GetNetmask() const {
		return m_nNetmask;
	}

	uint32_t GetNetmaskCIDR() const {
		return static_cast<uint32_t>(__builtin_popcount(m_nNetmask));
	}

	uint32_t GetBroadcastIp() const {
		return m_nLocalIp | ~m_nNetmask;
	}

	bool IsDhcpCapable() const {
		return m_IsDhcpCapable;
	}

	bool IsDhcpUsed() const {
		return m_IsDhcpUsed;
	}

	bool IsZeroconfCapable() const {
		return m_IsZeroconfCapable;
	}

	bool IsZeroconfUsed() const {
		return m_IsZeroconfUsed;
	}

	char GetAddressingMode() {
		if (Network::Get()->IsZeroconfUsed()) {
			return  'Z';
		} else if (Network::Get()->IsDhcpKnown()) {
			if (Network::Get()->IsDhcpUsed()) {
				return 'D';
			} else {
				return 'S';
			}
		}

		return 'U';
	}

	 bool IsDhcpKnown() const {
#if defined (__CYGWIN__) || defined (__APPLE__)
		return false;
#else
		return true;
#endif
	}

	const char *GetIfName() const {
		return m_aIfName;
	}

	uint32_t GetIfIndex() const {
		return m_nIfIndex;
	}

	uint32_t GetNtpServerIp() const {
		return m_nNtpServerIp;
	}

	float GetNtpUtcOffset() const {
		return m_fNtpUtcOffset;
	}

	bool IsValidIp(uint32_t nIp) {
		return (m_nLocalIp & m_nNetmask) == (nIp & m_nNetmask);
	}

	static Network *Get() {
		return s_pThis;
	}

	void Run();

private:
	uint32_t GetDefaultGateway();
	bool IsDhclient(const char *pIfName);
	int IfGetByAddress(const char *pIp, char *pName, size_t nLength);
	int IfDetails(const char *pIfInterface);
#if defined(__APPLE__)
	bool OSxGetMacaddress(const char *pIfName, uint8_t *pMacAddress);
#endif

private:
	bool m_IsDhcpCapable { true };
	bool m_IsDhcpUsed { false };
	bool m_IsZeroconfCapable { true };
	bool m_IsZeroconfUsed { false };
	uint32_t m_nIfIndex { 1 };
	uint32_t m_nNtpServerIp { 0 };
	float m_fNtpUtcOffset { 0 };

	uint32_t m_nLocalIp { 0 };
	uint32_t m_nGatewayIp { 0 };
	uint32_t m_nNetmask { 0 };

	char m_aHostName[net::HOSTNAME_SIZE];
	char m_aDomainName[net::DOMAINNAME_SIZE];
	uint32_t m_nNameservers[net::NAMESERVERS_COUNT];
	uint8_t m_aNetMacaddr[net::MAC_SIZE];
	char m_aIfName[IFNAMSIZ];

	static Network *s_pThis;
};

#endif /* LINUX_NETWORK_H_ */
