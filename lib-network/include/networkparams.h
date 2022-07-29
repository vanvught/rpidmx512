/**
 * @file networkparams.h
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

#ifndef NETWORKPARAMS_H_
#define NETWORKPARAMS_H_

#include <cstdint>

#include "network.h"

namespace networkparams {
namespace defaults {
static constexpr auto IS_DHCP_USED = true;
static constexpr auto DHCP_RETRY_TIME = 0;
static constexpr auto NTP_UTC_OFFSET = 0.0f;
}  // namespace defaults
}  // namespace networkparams

struct TNetworkParams {
	uint32_t nSetList;
	uint32_t nLocalIp;
	uint32_t nNetmask;
	uint32_t nGatewayIp;
	uint32_t nNameServerIp;
	bool bIsDhcpUsed;
	char aHostName[network::HOSTNAME_SIZE];
	uint32_t nNtpServerIp;
	float fNtpUtcOffset;
	uint8_t nDhcpRetryTime;
#if defined (ESP8266)
	char aSsid[34];
	char aPassword[34];
#endif
}__attribute__((packed));

#if !defined (ESP8266)
 static_assert(sizeof(struct TNetworkParams) <= 96, "struct TNetworkParams is too large");
#endif

struct NetworkParamsMask {
	static constexpr auto DHCP = (1U << 0);
	static constexpr auto IP_ADDRESS = (1U << 1);
	static constexpr auto NET_MASK = (1U << 2);
	static constexpr auto DEFAULT_GATEWAY = (1U << 3);
	static constexpr auto NAME_SERVER = (1U << 4);
	static constexpr auto HOSTNAME = (1U << 5);
	static constexpr auto NTP_SERVER = (1U << 6);
	static constexpr auto NTP_UTC_OFFSET = (1U << 7);
	static constexpr auto DHCP_RETRY_TIME = (1U << 8);
	static constexpr auto PTP_ENABLE = (1U << 9);
	static constexpr auto PTP_DOMAIN = (1U << 10);
#if defined (ESP8266)
	static constexpr auto SSID = (1U << 30);
	static constexpr auto PASSWORD = (1U << 31);
#endif
};

class NetworkParamsStore {
public:
	virtual ~NetworkParamsStore() {}

	virtual void Update(const struct TNetworkParams *pNetworkParams)=0;
	virtual void Copy(struct TNetworkParams *pNetworkParams)=0;
};

class NetworkParams {
public:
	NetworkParams(NetworkParamsStore *pNetworkParamsStore = nullptr);

	bool Load();
	void Load(const char *pBuffer, uint32_t nLength);

	void Builder(const struct TNetworkParams *ptNetworkParams, char *pBuffer, uint32_t nLength, uint32_t& nSize);
	void Save(char *pBuffer, uint32_t nLength, uint32_t& nSize);

	void Dump();

	bool isDhcpUsed() const {
		return m_tNetworkParams.bIsDhcpUsed;
	}

	uint8_t GetDhcpRetryTime() const {
		return m_tNetworkParams.nDhcpRetryTime;
	}

	uint32_t GetIpAddress() const {
		return m_tNetworkParams.nLocalIp;
	}

	uint32_t GetNetMask() const {
		return m_tNetworkParams.nNetmask;
	}

	uint32_t GetDefaultGateway() const {
		return m_tNetworkParams.nGatewayIp;
	}

	const char *GetHostName() const {
		return m_tNetworkParams.aHostName;
	}

	uint32_t GetNtpServer() const {
		if (!isMaskSet(NetworkParamsMask::NTP_SERVER)) {
			return 0;
		}
		return m_tNetworkParams.nNtpServerIp;
	}

	float GetNtpUtcOffset() const {
		if (!isMaskSet(NetworkParamsMask::NTP_UTC_OFFSET)) {
			return 0;
		}
		return m_tNetworkParams.fNtpUtcOffset;
	}

#if defined (ESP8266)
	uint32_t GetNameServer() const {
		return m_tNetworkParams.nNameServerIp;
	}

	const char *GetSSid() const {
		return m_tNetworkParams.aSsid;
	}

	const char *GetPassword() const {
		return m_tNetworkParams.aPassword;
	}
#endif

public:
    static void staticCallbackFunction(void *p, const char *s);

private:
    void callbackFunction(const char *s);
    bool isMaskSet(uint32_t nMask) const {
    	return (m_tNetworkParams.nSetList & nMask) == nMask;
    }

private:
	NetworkParamsStore *m_pNetworkParamsStore;
	static TNetworkParams m_tNetworkParams;
};

#endif /* NETWORKPARAMS_H_ */
