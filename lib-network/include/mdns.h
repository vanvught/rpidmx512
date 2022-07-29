/**
 * @file mdns.h
 *
 */
/* Copyright (C) 2019-2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef MDNS_H_
#define MDNS_H_

#include <cstdint>

#include "network.h"

#include "../config/apps_config.h"

namespace mdns {
struct Flags {
	uint32_t qr;
	uint32_t opcode;
	uint32_t aa;
	uint32_t tc;
	uint32_t rd;
	uint32_t ra;
	uint32_t zero;
	uint32_t ad;
	uint32_t cd;
	uint32_t rcode;
};

enum class Protocol : uint8_t {
	UDP, TCP
};

struct ServiceRecord {
	char *pName;
	char *pServName;
	char *pTextContent;
	uint16_t nPort;
	Protocol nProtocol;
};

struct RecordData {
	uint32_t nSize;
	uint8_t aBuffer[512];
};

static constexpr uint16_t UDP_PORT = 5353;
#if !defined (MDNS_SERVICE_RECORDS_MAX)
static constexpr auto SERVICE_RECORDS_MAX = 8;
#else
static constexpr auto SERVICE_RECORDS_MAX = MDNS_SERVICE_RECORDS_MAX;
#endif
}  // namespace mdns

class MDNS {
public:
	MDNS();
	~MDNS();

	void Start();
	void Stop() {
		Network::Get()->End(mdns::UDP_PORT);
		s_nHandle = -1;
	}

	void Run();

	void Print();

	void SetName(const char *pName);

	bool AddServiceRecord(const char* pName, const char *pServName, uint16_t nPort, mdns::Protocol nProtocol = mdns::Protocol::UDP, const char* pTextContent = nullptr);

private:
	void Parse();
	void HandleRequest(uint16_t nQuestions);

	uint32_t DecodeDNSNameNotation(const char *pDNSNameNotation, char *pString);

	uint32_t WriteDnsName(const char *pSource, char *pDestination, bool bNullTerminated = true);
	const char *FindFirstDotFromRight(const char *pString) const;

	void CreateAnswerLocalIpAddress();

	uint32_t CreateAnswerServiceSrv(uint32_t nIndex, uint8_t *pDestination);
	uint32_t CreateAnswerServiceTxt(uint32_t nIndex, uint8_t *pDestination);
	uint32_t CreateAnswerServicePtr(uint32_t nIndex, uint8_t *pDestination);
	uint32_t CreateAnswerServiceDnsSd(uint32_t nIndex, uint8_t *pDestination);

	void CreateMDNSMessage(uint32_t nIndex);

#ifndef NDEBUG
	void Dump(const struct TmDNSHeader *pmDNSHeader, uint16_t nFlags);
#endif

private:
	static uint32_t s_nMulticastIp;
	static int32_t s_nHandle;
	static uint32_t s_nRemoteIp;
	static uint16_t s_nRemotePort;
	static uint16_t s_nBytesReceived;
	static uint32_t s_nLastAnnounceMillis;
	static uint32_t s_nDNSServiceRecords;

	static mdns::ServiceRecord s_ServiceRecords[mdns::SERVICE_RECORDS_MAX];
	static mdns::RecordData s_AnswerLocalIp;
	static mdns::RecordData s_ServiceRecordsData;

	static char *s_pName;
	static uint8_t *s_pBuffer;
};

#endif /* MDNS_H_ */
