/**
 * @file mdns.h
 *
 */
/* Copyright (C) 2019-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef NET_APPS_MDNS_H_
#define NET_APPS_MDNS_H_

#include <cstdint>

#include "network.h"
#include "net/protocol/dns.h"

#include "../config/apps_config.h"

namespace mdns {
enum class Services {
	CONFIG, TFTP, HTTP, RDMNET_LLRP, NTP, MIDI, OSC, DDP, PP, LAST_NOT_USED
};

struct ServiceRecord {
	char *pName;
	char *pTextContent;
	uint16_t nTextContentLength;
	uint16_t nPort;
	mdns::Services services;
};
}  // namespace mdns

class MDNS {
public:
	MDNS();
	~MDNS();

	bool ServiceRecordAdd(const char *pName, const mdns::Services service, const char *pTextContent = nullptr, const uint16_t nPort = 0);
	bool ServiceRecordDelete(const mdns::Services service);

	void Print();

	void SendAnnouncement(const uint32_t nTTL);

	void Run() {
		s_nBytesReceived = Network::Get()->RecvFrom(s_nHandle, const_cast<const void **>(reinterpret_cast<void **>(&s_pReceiveBuffer)), &s_nRemoteIp, &s_nRemotePort);

		if (__builtin_expect((s_nBytesReceived < sizeof(struct net::dns::Header)), 1)) {
			return;
		}

		const auto *const pHeader = reinterpret_cast<net::dns::Header *>(s_pReceiveBuffer);
		const auto nFlag1 = pHeader->nFlag1;

		if ((nFlag1 >> 3) & 0xF) {
			return;
		}

		HandleQuestions(static_cast<uint32_t>(__builtin_bswap16(pHeader->nQueryCount)));
	}

	static MDNS *Get() {
		return s_pThis;
	}

private:
	void Parse();
	void HandleQuestions(const uint32_t nQuestions);
	void SendAnswerLocalIpAddress(const uint16_t nTransActionID, const uint32_t nTTL);
	void SendMessage(mdns::ServiceRecord const& serviceRecord, const uint16_t nTransActionID, const uint32_t nTTL);
	void SendTo(const uint32_t nLength);

private:
	static int32_t s_nHandle;
	static uint32_t s_nRemoteIp;
	static uint32_t s_nBytesReceived;
	static uint8_t *s_pReceiveBuffer;
	static uint16_t s_nRemotePort;

	static MDNS *s_pThis;
};

#endif /* NET_APPS_MDNS_H_ */
