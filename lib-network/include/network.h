/**
 * @file network.h
 *
 */
/* Copyright (C) 2017-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <net/if.h>

#define IP2STR(addr) (addr & 0xFF), ((addr >> 8) & 0xFF), ((addr >> 16) & 0xFF), ((addr >> 24) & 0xFF)
#define IPSTR "%d.%d.%d.%d"

#define MAC2STR(mac) static_cast<int>(mac[0]),static_cast<int>(mac[1]),static_cast<int>(mac[2]),static_cast<int>(mac[3]), static_cast<int>(mac[4]), static_cast<int>(mac[5])
#define MACSTR "%.2x:%.2x:%.2x:%.2x:%.2x:%.2x"

enum class TDhcpMode {
	INACTIVE = 0x00,	///< The IP address was not obtained via DHCP
	ACTIVE = 0x01,		///< The IP address was obtained via DHCP
	UNKNOWN = 0x02		///< The system cannot determine if the address was obtained via DHCP
};

enum TNetwork {
	NETWORK_IP_SIZE = 4,
	NETWORK_MAC_SIZE = 6,
	NETWORK_HOSTNAME_SIZE = 64,		/* including a terminating null byte. */
	NETWORK_DOMAINNAME_SIZE = 64	/* including a terminating null byte. */
};

enum class DhcpClientStatus {
	IDLE,
	RENEW,
	GOT_IP,
	FAILED
};

class NetworkDisplay {
public:
	virtual ~NetworkDisplay() {
	}

	virtual void ShowIp()=0;
	virtual void ShowNetMask()=0;
	virtual void ShowHostName()=0;

	virtual void ShowDhcpStatus(DhcpClientStatus nStatus)=0;

	virtual void ShowShutdown()=0;
};

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
	virtual ~Network() {
	}

	void Print();

	virtual void Shutdown();

	virtual int32_t Begin(uint16_t nPort)=0;
	virtual int32_t End(uint16_t nPort)=0;

	virtual void MacAddressCopyTo(uint8_t *pMacAddress)=0;

	virtual void JoinGroup(int32_t nHandle, uint32_t nIp)=0;
	virtual void LeaveGroup(int32_t nHandle, uint32_t nIp)=0;

	virtual uint16_t RecvFrom(int32_t nHandle, void *pBuffer, uint16_t nLength, uint32_t *pFromIp, uint16_t *pFromPort)=0;
	virtual void SendTo(int32_t nHandle, const void *pBuffer, uint16_t nLength, uint32_t nToIp, uint16_t nRemotePort)=0;

	virtual void SetIp(uint32_t nIp)=0;
	virtual void SetNetmask(uint32_t nNetmask)=0;
	virtual bool SetZeroconf()=0;
	virtual bool EnableDhcp()=0;

	virtual void SetHostName(const char *pHostName);
	virtual void SetDomainName(const char *pDomainName);

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
	void SetQueuedDhcp();
	void SetQueuedZeroconf();
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
#if defined (__CYGWIN__) || defined (__APPLE__)
		return false;
#else
		return true;
#endif
	}

	TDhcpMode GetDhcpMode() const {
		if (IsDhcpKnown()) {
			if (m_IsDhcpUsed) {
				return TDhcpMode::ACTIVE;
			}

			return TDhcpMode::INACTIVE;
		}

		return TDhcpMode::UNKNOWN;
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

	void SetNetworkDisplay(NetworkDisplay *pNetworkDisplay) {
		m_pNetworkDisplay = pNetworkDisplay;
	}

	void SetNetworkStore(NetworkStore *pNetworkStore) {
		m_pNetworkStore = pNetworkStore;
	}

	static Network *Get() {
		return s_pThis;
	}

	static uint32_t CIDRToNetmask(uint8_t nCDIR);

protected:
	uint8_t m_aNetMacaddr[NETWORK_MAC_SIZE];
	uint32_t m_nLocalIp{0};
	uint32_t m_nGatewayIp{0};
	uint32_t m_nNetmask{0};
	bool m_IsDhcpCapable{true};
	bool m_IsDhcpUsed{false};
	bool m_IsZeroconfCapable{true};
	bool m_IsZeroconfUsed{false};
	char m_aHostName[NETWORK_HOSTNAME_SIZE];
	char m_aDomainName[NETWORK_DOMAINNAME_SIZE];
	char m_aIfName[IFNAMSIZ];
	uint32_t m_nIfIndex{1};
	uint32_t m_nNtpServerIp{0};
	float m_fNtpUtcOffset{0};

	NetworkDisplay *m_pNetworkDisplay{nullptr};
	NetworkStore *m_pNetworkStore{nullptr};

private:
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

#endif /* NETWORK_H_ */
