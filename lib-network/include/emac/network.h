/**
 * @file network.h
 *
 */
/* Copyright (C) 2017-2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef EMAC_NETWORK_H_
#define EMAC_NETWORK_H_

#if defined (NO_EMAC) || defined (ESP8266)
# error
#endif

#include <cstdint>
#include <cstring>
#include <net/if.h>

#include "networkparams.h"

#include "../src/net/net.h"

extern "C" {
	void net_handle(void);
}

#define HAVE_NET_HANDLE

class Network {
public:
	Network();
	~Network() {}

	void Init(NetworkParamsStore *pNetworkParamsStore = nullptr);

	void Print();

	void Shutdown();

	int32_t Begin(uint16_t nPort);
	int32_t End(uint16_t nPort);

	void MacAddressCopyTo(uint8_t *pMacAddress);

	void JoinGroup(int32_t nHandle, uint32_t nIp);
	void LeaveGroup(int32_t nHandle, uint32_t nIp);

	uint16_t RecvFrom(int32_t nHandle, void *pBuffer, uint16_t nLength, uint32_t *from_ip, uint16_t *from_port) {
		return udp_recv(static_cast<uint8_t>(nHandle), reinterpret_cast<uint8_t*>(pBuffer), nLength, from_ip, from_port);
	}

	uint16_t RecvFrom(int32_t nHandle, const void **ppBuffer, uint32_t *pFromIp, uint16_t *pFromPort) {
		return udp_recv2(static_cast<uint8_t>(nHandle), reinterpret_cast<const uint8_t **>(ppBuffer), pFromIp, pFromPort);
	}

	void SendTo(int32_t nHandle, const void *pBuffer, uint16_t nLength, uint32_t to_ip, uint16_t remote_port) {
		udp_send(static_cast<uint8_t>(nHandle), reinterpret_cast<const uint8_t*>(pBuffer), nLength, to_ip, remote_port);
	}

	/*
	 * Experimental TCP
	 */

	int32_t TcpBegin(uint16_t nLocalPort);
	uint16_t TcpRead(const int32_t nHandle, const uint8_t **ppBuffer);
	void TcpWrite(const int32_t nHandle, const uint8_t *pBuffer, uint16_t nLength);
	int32_t TcpEnd(const int32_t nHandle);

	void SetIp(uint32_t nIp);
	void SetNetmask(uint32_t nNetmask);
	void SetGatewayIp(uint32_t nGatewayIp);
	bool SetZeroconf();
	bool EnableDhcp();

	void SetHostName(const char *pHostName);

	void SetDomainName(const char *pDomainName) {
		strncpy(m_aDomainName, pDomainName, network::DOMAINNAME_SIZE - 1);
		m_aDomainName[network::DOMAINNAME_SIZE - 1] = '\0';
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

	void SetQueuedStaticIp(uint32_t nLocalIp = 0, uint32_t nNetmask = 0);
	void SetQueuedDhcp() {
		m_QueuedConfig.nMask |= QueuedConfig::DHCP;
	}
	void SetQueuedZeroconf() {
		m_QueuedConfig.nMask |= QueuedConfig::ZEROCONF;
	}

	bool ApplyQueuedConfig();

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
		return true;
	}

	network::dhcp::Mode GetDhcpMode() const {
		if (m_IsDhcpUsed) {
			return network::dhcp::Mode::ACTIVE;
		}

		return network::dhcp::Mode::INACTIVE;
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

	void SetNetworkStore(NetworkStore *pNetworkStore) {
		m_pNetworkStore = pNetworkStore;
	}

	bool IsValidIp(uint32_t nIp) {
		return (m_nLocalIp & m_nNetmask) == (nIp & m_nNetmask);
	}

	static uint32_t CIDRToNetmask(uint8_t nCDIR) {
		if (nCDIR != 0) {
			const auto nNetmask = __builtin_bswap32(static_cast<uint32_t>(~0x0) << (32 - nCDIR));
			return nNetmask;
		}

		return 0;
	}

	void Run() {
		net_handle();
	}

	static Network *Get() {
		return s_pThis;
	}

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

	char m_aHostName[network::HOSTNAME_SIZE];
	char m_aDomainName[network::DOMAINNAME_SIZE];
	uint8_t m_aNetMacaddr[network::MAC_SIZE];
	char m_aIfName[IFNAMSIZ];

	NetworkStore *m_pNetworkStore { nullptr };
	NetworkDisplay m_NetworkDisplay;

	void SetDefaultIp();

	struct QueuedConfig {
		static constexpr uint32_t NONE = 0;
		static constexpr uint32_t STATIC_IP = (1U << 0);
		static constexpr uint32_t NET_MASK = (1U << 1);
		static constexpr uint32_t DHCP = (1U << 2);
		static constexpr uint32_t ZEROCONF = (1U << 3);
		uint32_t nMask = QueuedConfig::NONE;
		uint32_t nLocalIp = 0;
		uint32_t nNetmask = 0;
	};

	QueuedConfig m_QueuedConfig;

    bool isQueuedMaskSet(uint32_t nMask) {
    	return (m_QueuedConfig.nMask & nMask) == nMask;
    }

	static Network *s_pThis;
};

#endif /* EMAC_NETWORK_H_ */
