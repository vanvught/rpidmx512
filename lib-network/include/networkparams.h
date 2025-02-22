/**
 * @file networkparams.h
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

#ifndef NETWORKPARAMS_H_
#define NETWORKPARAMS_H_

#include <cstdint>

#include "net/ip4_address.h"
#include "network.h"
#include "configstore.h"

namespace networkparams {
namespace defaults {
static constexpr auto IS_DHCP_USED = true;
}  // namespace defaults

struct Params {
	uint32_t nSetList;
	uint32_t nLocalIp;
	uint32_t nNetmask;
	uint32_t nGatewayIp;
	uint32_t nNameServerIp;
	bool bUseStaticIp;
	char aHostName[net::HOSTNAME_SIZE];
	uint32_t nNtpServerIp;
#if defined (ESP8266)
	char aSsid[34];
	char aPassword[34];
#endif
}__attribute__((packed));

#if !defined (ESP8266)
 static_assert(sizeof(struct Params) <= configstore::STORE_SIZE[static_cast<uint32_t>(configstore::Store::NETWORK)]);
#endif

struct Mask {
	static constexpr auto NTP_SERVER = (1U << 6);
};

namespace store {
inline void Update(const struct networkparams::Params *pParams) {
	ConfigStore::Get()->Update(configstore::Store::NETWORK, pParams, sizeof(struct networkparams::Params));
}

inline void Copy(struct networkparams::Params *pParams) {
	ConfigStore::Get()->Copy(configstore::Store::NETWORK, pParams, sizeof(struct networkparams::Params));
}
}  // namespace store
}  // namespace networkparams

class NetworkParams {
public:
	NetworkParams();

	void Load();
	void Load(const char *pBuffer, uint32_t nLength);

	void Builder(const networkparams::Params *pParams, char *pBuffer, uint32_t nLength, uint32_t& nSize);
	void Save(char *pBuffer, uint32_t nLength, uint32_t& nSize) {
		Builder(nullptr, pBuffer, nLength, nSize);
	}

	bool isDhcpUsed() const {
		return !m_Params.bUseStaticIp;
	}

	uint32_t GetIpAddress() const {
		return m_Params.nLocalIp;
	}

	uint32_t GetNetMask() const {
		return m_Params.nNetmask;
	}

	uint32_t GetDefaultGateway() const {
		return m_Params.nGatewayIp;
	}

	const char *GetHostName() const {
		return m_Params.aHostName;
	}

	uint32_t GetNtpServer() const {
		if (!IsMaskSet(networkparams::Mask::NTP_SERVER)) {
			return 0;
		}
		return m_Params.nNtpServerIp;
	}

#if defined (ESP8266)
	uint32_t GetNameServer() const {
		return m_Params.nNameServerIp;
	}

	const char *GetSSid() const {
		return m_Params.aSsid;
	}

	const char *GetPassword() const {
		return m_Params.aPassword;
	}
#endif

public:
    static void StaticCallbackFunction(void *p, const char *s);

private:
	void Dump();
    void CallbackFunction(const char *s);
    bool IsMaskSet(uint32_t nMask) const {
    	return (m_Params.nSetList & nMask) == nMask;
    }

private:
	networkparams::Params m_Params;
};

#endif /* NETWORKPARAMS_H_ */
