/**
 * @file mdns.cpp
 *
 */
/* Copyright (C) 2019-2022 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <cstdint>
#include <cstring>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cassert>

#include "mdns.h"

#include "network.h"
#include "hardware.h"

#include "debug.h"

#define MDNS_MULTICAST_ADDRESS	"224.0.0.251"
#define MDNS_TLD                ".local"
#define DNS_SD_SERVICE          "_services._dns-sd._udp.local"
#define MDNS_RESPONSE_TTL     	(120)    ///< (in seconds)
#define ANNOUNCE_TIMEOUT 		((MDNS_RESPONSE_TTL / 2) + (MDNS_RESPONSE_TTL / 4))

enum TDNSClasses {
	DNSClassInternet = 1
};

enum TDNSRecordTypes {
	DNSRecordTypeA = 1,		///< 0x01
	DNSRecordTypePTR = 12,	///< 0x0c
	DNSRecordTypeTXT = 16,	///< 0x10
	DNSRecordTypeSRV = 33	///< 0x21
};

enum TDNSCacheFlush {
	DNSCacheFlushTrue = 0x8000
};

enum TDNSOpCodes {
	DNSOpQuery = 0,
	DNSOpIQuery = 1,
	DNSOpStatus = 2,
	DNSOpNotify = 4,
	DNSOpUpdate = 5
};

struct TmDNSHeader {
	uint16_t xid;
	uint16_t nFlags;
	uint16_t queryCount;
	uint16_t answerCount;
	uint16_t authorityCount;
	uint16_t additionalCount;
} __attribute__((__packed__));

using namespace mdns;

uint32_t MDNS::s_nMulticastIp;
int32_t MDNS::s_nHandle = -1;
uint32_t MDNS::s_nRemoteIp;
uint16_t MDNS::s_nRemotePort;
uint16_t MDNS::s_nBytesReceived;
uint32_t MDNS::s_nLastAnnounceMillis;
uint32_t MDNS::s_nDNSServiceRecords;
ServiceRecord MDNS::s_ServiceRecords[SERVICE_RECORDS_MAX];
RecordData MDNS::s_AnswerLocalIp;
RecordData MDNS::s_ServiceRecordsData;
char *MDNS::s_pName;
uint8_t *MDNS::s_pBuffer;

static constexpr const char *get_protocol_name(Protocol nProtocol) {
	return nProtocol == Protocol::TCP ? "_tcp" MDNS_TLD : "_udp" MDNS_TLD;
}

MDNS::MDNS() {
	struct in_addr group_ip;
	static_cast<void>(inet_aton(MDNS_MULTICAST_ADDRESS, &group_ip));
	s_nMulticastIp = group_ip.s_addr;
}

void MDNS::Start() {
	assert(s_nHandle == -1);

	s_nHandle = Network::Get()->Begin(UDP_PORT);
	assert(s_nHandle != -1);

	Network::Get()->JoinGroup(s_nHandle, s_nMulticastIp);

	if (s_pName == nullptr) {
		SetName(Network::Get()->GetHostName());
	}

	CreateAnswerLocalIpAddress();

	Network::Get()->SendTo(s_nHandle, s_AnswerLocalIp.aBuffer, static_cast<uint16_t>(s_AnswerLocalIp.nSize), s_nMulticastIp, UDP_PORT);
	Network::Get()->SetDomainName(&MDNS_TLD[1]);
}

void MDNS::SetName(const char *pName) {
	assert(pName != nullptr);
	assert(strlen(pName) != 0);

	if (s_pName != nullptr) {
		delete[] s_pName;
	}

	s_pName = new char[strlen(pName) + sizeof(MDNS_TLD)];
	assert(s_pName != nullptr);

	strcpy(s_pName, pName);
	strcat(s_pName, MDNS_TLD);

	DEBUG_PUTS(s_pName);
}

void MDNS::CreateMDNSMessage(uint32_t nIndex) {
	DEBUG_ENTRY

	auto *pHeader = reinterpret_cast<struct TmDNSHeader*>(&s_ServiceRecordsData.aBuffer);

	pHeader->nFlags = __builtin_bswap16(0x8400);
	pHeader->queryCount = 0;
	pHeader->answerCount = __builtin_bswap16(4);
	pHeader->authorityCount = __builtin_bswap16(1);
	pHeader->additionalCount = __builtin_bswap16(0);

	auto *pData = reinterpret_cast<uint8_t*>(&s_ServiceRecordsData.aBuffer) + sizeof(struct TmDNSHeader);

	pData += CreateAnswerServiceSrv(nIndex, pData);
	pData += CreateAnswerServiceTxt(nIndex, pData);
	pData += CreateAnswerServiceDnsSd(nIndex, pData);
	pData += CreateAnswerServicePtr(nIndex, pData);

	memcpy(pData, &s_AnswerLocalIp.aBuffer[sizeof (struct TmDNSHeader)], s_AnswerLocalIp.nSize - sizeof (struct TmDNSHeader));
	pData += (s_AnswerLocalIp.nSize - sizeof (struct TmDNSHeader));

	s_ServiceRecordsData.nSize = static_cast<uint32_t>(pData - reinterpret_cast<uint8_t*>(pHeader));

//	DEBUG_dump(&s_ServiceRecordsData.aBuffer, static_cast<uint16_t>(s_ServiceRecordsData.nSize));

	DEBUG_EXIT
}

uint32_t MDNS::DecodeDNSNameNotation(const char *pDNSNameNotation, char *pString) {
	DEBUG_ENTRY

	const auto *pSrc = pDNSNameNotation;
	auto *pDst = pString;

	uint32_t nLenght = 0;
	uint32_t nSize = 0;
	auto isCompressed = false;

	while (*pSrc != 0) {
		nLenght = static_cast<uint8_t>(*pSrc++);
//		DEBUG_PRINTF("nLenght=%.2x", (int) nLenght);

		if (nLenght > 63) {
			if (!isCompressed) {
				nSize += 1;
			}

			isCompressed = true;
			const auto nCompressedOffset = ((nLenght & static_cast<uint32_t>(~(0xC0))) << 8) | ((*pSrc & 0xFF));
			nLenght =  s_pBuffer[nCompressedOffset];

//			DEBUG_PRINTF("--> nCompressedOffset=%.4x", (int) nCompressedOffset);

			pSrc = reinterpret_cast<char*>(&s_pBuffer[nCompressedOffset + 1]);

			for (uint32_t i = 0; i < nLenght; i++) {
				*pDst = *pSrc;
				pDst++;
				pSrc++;
			}

		} else if (nLenght > 0) {

			for (uint32_t i = 0; i < nLenght; i++) {
				*pDst = *pSrc;
				pDst++;
				pSrc++;
			}

			if (!isCompressed) {
				nSize += (nLenght + 1);
			}
		}

		if (*pSrc != 0) {
			*pDst++ = '.';
		}
	}

	*pDst = '\0';

	DEBUG_PRINTF("pString=[%s]:%u", pString, 1 + nSize);
	DEBUG_EXIT
	return 1 + nSize;
}

bool MDNS::AddServiceRecord(const char *pName, const char *pServName, uint16_t nPort, mdns::Protocol nProtocol, const char *pTextContent) {
	assert(pServName != nullptr);
	assert(nPort != 0);

	uint32_t i;

	for (i = 0; i < SERVICE_RECORDS_MAX; i++) {
		if (s_ServiceRecords[i].pName == nullptr) {
			s_ServiceRecords[i].nPort = nPort;
			s_ServiceRecords[i].nProtocol = nProtocol;

			if (pName == nullptr) {
				s_ServiceRecords[i].pName = new char[1 + strlen(Network::Get()->GetHostName()) + strlen(pServName)];
				assert(s_ServiceRecords[i].pName != nullptr);

				strcpy(s_ServiceRecords[i].pName, Network::Get()->GetHostName());
			} else {
				s_ServiceRecords[i].pName = new char[1 + strlen(pName) + strlen(pServName)];
				assert(s_ServiceRecords[i].pName != nullptr);

				strcpy(s_ServiceRecords[i].pName, pName);
			}

			strcat(s_ServiceRecords[i].pName, pServName);

			const char *p = FindFirstDotFromRight(pServName);

			s_ServiceRecords[i].pServName = new char[1 + strlen(p) + 12];
			assert(s_ServiceRecords[i].pServName != nullptr);

			strcpy(s_ServiceRecords[i].pServName, p);
			strcat(s_ServiceRecords[i].pServName, ".");
			strcat(s_ServiceRecords[i].pServName, get_protocol_name(nProtocol));

			if (pTextContent != nullptr) {
				s_ServiceRecords[i].pTextContent = new char[1 + strlen(pTextContent)];
				assert(s_ServiceRecords[i].pTextContent != nullptr);

				strcpy(s_ServiceRecords[i].pTextContent, pTextContent);
			}

			break;
		}
	}

	if (i == SERVICE_RECORDS_MAX) {
		assert(0);
		return false;
	}

	DEBUG_PRINTF("[%d].nPort = %d", i, s_ServiceRecords[i].nPort);
	DEBUG_PRINTF("[%d].nProtocol = [%s]", i, s_ServiceRecords[i].nProtocol == Protocol::TCP ? "TCP" : "UDP");
	DEBUG_PRINTF("[%d].pName = [%s]", i, s_ServiceRecords[i].pName);
	DEBUG_PRINTF("[%d].pServName = [%s]", i, s_ServiceRecords[i].pServName);
	DEBUG_PRINTF("[%d].pTextContent = [%s]", i, s_ServiceRecords[i].pTextContent != nullptr ? s_ServiceRecords[i].pTextContent : "><");

	CreateMDNSMessage(i);

	DEBUG_PRINTF("%d:%d %p -> %d " IPSTR, i, s_nHandle, reinterpret_cast<void *>(&s_ServiceRecordsData.aBuffer), s_ServiceRecordsData.nSize, IP2STR(s_nMulticastIp));

	Network::Get()->SendTo(s_nHandle, &s_ServiceRecordsData.aBuffer, static_cast<uint16_t>(s_ServiceRecordsData.nSize), s_nMulticastIp, UDP_PORT);
	return true;
}


const char *MDNS::FindFirstDotFromRight(const char *pString) const {
	const char *p = pString + strlen(pString);
	while (p > pString && *p-- != '.')
		;
	return &p[1];
}

uint32_t MDNS::WriteDnsName(const char *pSource, char *pDestination, bool bNullTerminated) {
	DEBUG_PUTS(pSource);

	const auto *pSrc = pSource;
	auto *pDst = pDestination;

	while (1 == 1) {
		auto *pLength = pDst++;
		const auto *pSrcStart = pSrc;

		while ((*pSrc != 0) && (*pSrc != '.')) {
			*pDst = *pSrc;
			pDst++;
			pSrc++;
		}

		*pLength = static_cast<char>(pSrc - pSrcStart);

		if (*pSrc == 0) {
			if (bNullTerminated) {
				*pDst++ = 0;
			}
			break;
		}

		pSrc++;
	}

	return static_cast<uint32_t>(pDst - pDestination);
}

void MDNS::CreateAnswerLocalIpAddress() {
	DEBUG_ENTRY

	auto *pHeader = reinterpret_cast<struct TmDNSHeader*>(&s_AnswerLocalIp.aBuffer);

	pHeader->nFlags = __builtin_bswap16(0x8400);
	pHeader->queryCount = 0;
	pHeader->answerCount = __builtin_bswap16(1);
	pHeader->authorityCount = 0;
	pHeader->additionalCount = 0;

	uint8_t *pData = reinterpret_cast<uint8_t*>(&s_AnswerLocalIp.aBuffer) + sizeof(struct TmDNSHeader);

	pData += WriteDnsName(s_pName, reinterpret_cast<char*>(pData));

	*reinterpret_cast<uint16_t*>(pData) = __builtin_bswap16(DNSRecordTypeA);
	pData += 2;
	*reinterpret_cast<uint16_t*>(pData) = __builtin_bswap16(DNSCacheFlushTrue | DNSClassInternet);
	pData += 2;
	*reinterpret_cast<uint32_t*>(pData) = __builtin_bswap32(MDNS_RESPONSE_TTL);
	pData += 4;
	*reinterpret_cast<uint16_t*>(pData) = __builtin_bswap16(4);	// Data length
	pData += 2;
	*reinterpret_cast<uint32_t*>(pData) = Network::Get()->GetIp();
	pData += 4;

	s_AnswerLocalIp.nSize = static_cast<uint32_t>(pData - reinterpret_cast<uint8_t*>(pHeader));

	DEBUG_EXIT
}

uint32_t MDNS::CreateAnswerServiceSrv(uint32_t nIndex, uint8_t *pDestination) {
	DEBUG_ENTRY

	auto *pDst = pDestination;

	pDst += WriteDnsName(s_ServiceRecords[nIndex].pName, reinterpret_cast<char*>(pDst), false);
	pDst += WriteDnsName(get_protocol_name(s_ServiceRecords[nIndex].nProtocol), reinterpret_cast<char*>(pDst));

	*reinterpret_cast<uint16_t*>(pDst) = __builtin_bswap16(DNSRecordTypeSRV);
	pDst += 2;
	*reinterpret_cast<uint16_t*>(pDst) = __builtin_bswap16(DNSCacheFlushTrue | DNSClassInternet);
	pDst += 2;
	*reinterpret_cast<uint32_t*>(pDst) = __builtin_bswap32(MDNS_RESPONSE_TTL);
	pDst += 4;
	*reinterpret_cast<uint16_t*>(pDst) = __builtin_bswap16(static_cast<uint16_t>(8 + strlen(s_pName)));
	pDst += 2;
	*reinterpret_cast<uint32_t*>(pDst) = 0; // Priority and Weight
	pDst += 4;
	*reinterpret_cast<uint16_t*>(pDst) = __builtin_bswap16(s_ServiceRecords[nIndex].nPort);
	pDst += 2;
	pDst += WriteDnsName(s_pName, reinterpret_cast<char*>(pDst));

	DEBUG_EXIT
	return static_cast<uint32_t>(pDst - pDestination);
}

uint32_t MDNS::CreateAnswerServiceTxt(uint32_t nIndex, uint8_t *pDestination) {
	DEBUG_ENTRY

	auto *pDst = pDestination;

	pDst += WriteDnsName(s_ServiceRecords[nIndex].pName, reinterpret_cast<char*>(pDst), false);
	pDst += WriteDnsName(get_protocol_name(s_ServiceRecords[nIndex].nProtocol), reinterpret_cast<char*>(pDst));

	*reinterpret_cast<uint16_t*>(pDst) = __builtin_bswap16(DNSRecordTypeTXT);
	pDst += 2;
	*reinterpret_cast<uint16_t*>(pDst) = __builtin_bswap16(DNSCacheFlushTrue | DNSClassInternet);
	pDst += 2;
	*reinterpret_cast<uint32_t*>(pDst) = __builtin_bswap32(MDNS_RESPONSE_TTL);
	pDst += 4;

	if (s_ServiceRecords[nIndex].pTextContent == nullptr) {
		*reinterpret_cast<uint16_t*>(pDst) = __builtin_bswap16(0x0001);	// Data length
		pDst += 2;
		*pDst = 0;														// Text length
		pDst++;
	} else {
		const uint32_t nSize = strlen(s_ServiceRecords[nIndex].pTextContent);
		*reinterpret_cast<uint16_t*>(pDst) = __builtin_bswap16(static_cast<uint16_t>(1 + nSize));	// Data length
		pDst += 2;
		*pDst = static_cast<uint8_t>(nSize);														// Text length
		pDst++;
		strcpy(reinterpret_cast<char*>(pDst), s_ServiceRecords[nIndex].pTextContent);
		pDst += nSize;
	}

	DEBUG_EXIT
	return static_cast<uint32_t>(pDst - pDestination);
}

uint32_t MDNS::CreateAnswerServicePtr(uint32_t nIndex, uint8_t *pDestination) {
	DEBUG_ENTRY

	auto *pDst = pDestination;

	pDst += WriteDnsName(s_ServiceRecords[nIndex].pServName, reinterpret_cast<char*>(pDst));

	*reinterpret_cast<uint16_t*>(pDst) = __builtin_bswap16(DNSRecordTypePTR);
	pDst += 2;
	*reinterpret_cast<uint16_t*>(pDst) = __builtin_bswap16(DNSClassInternet);
	pDst += 2;
	*reinterpret_cast<uint32_t*>(pDst) = __builtin_bswap32(MDNS_RESPONSE_TTL);
	pDst += 4;
	*reinterpret_cast<uint16_t*>(pDst) = __builtin_bswap16(static_cast<uint16_t>(13 + strlen(s_ServiceRecords[nIndex].pName)));
	pDst += 2;
	pDst += WriteDnsName(s_ServiceRecords[nIndex].pName, reinterpret_cast<char*>(pDst), false);
	pDst += WriteDnsName(get_protocol_name(s_ServiceRecords[nIndex].nProtocol), reinterpret_cast<char*>(pDst));

	DEBUG_EXIT
	return static_cast<uint32_t>(pDst - pDestination);
}

uint32_t MDNS::CreateAnswerServiceDnsSd(uint32_t nIndex, uint8_t *pDestination) {
	DEBUG_ENTRY

	auto *pDst = pDestination;

	pDst += WriteDnsName(DNS_SD_SERVICE, reinterpret_cast<char*>(pDst));

	*reinterpret_cast<uint16_t*>(pDst) = __builtin_bswap16(DNSRecordTypePTR);
	pDst += 2;
	*reinterpret_cast<uint16_t*>(pDst) = __builtin_bswap16(DNSClassInternet);
	pDst += 2;
	*reinterpret_cast<uint32_t*>(pDst) = __builtin_bswap32(MDNS_RESPONSE_TTL);
	pDst += 4;
	*reinterpret_cast<uint16_t*>(pDst) = __builtin_bswap16(static_cast<uint16_t>(2 + strlen(s_ServiceRecords[nIndex].pServName)));
	pDst += 2;
	pDst += WriteDnsName(s_ServiceRecords[nIndex].pServName, reinterpret_cast<char*>(pDst));

	DEBUG_EXIT
	return static_cast<uint32_t>(pDst - pDestination);
}

void MDNS::HandleRequest(uint16_t nQuestions) {
	DEBUG_ENTRY

	char DnsName[255];

	uint32_t nOffset = sizeof(struct TmDNSHeader);

	for (uint32_t i = 0; i < nQuestions; i++) {
		nOffset += DecodeDNSNameNotation(reinterpret_cast<const char*>(&s_pBuffer[nOffset]), DnsName);

		const auto nType = __builtin_bswap16(*reinterpret_cast<uint16_t*>(&s_pBuffer[nOffset]));
		nOffset += 2;

		const auto nClass = __builtin_bswap16(*reinterpret_cast<uint16_t*>(&s_pBuffer[nOffset])) & 0x7F;
		nOffset += 2;

		DEBUG_PRINTF("%s ==> Type : %d, Class: %d", DnsName, nType, nClass);

		if (nClass == DNSClassInternet) {
			DEBUG_PRINTF("%s:%s", s_pName, DnsName);

			if ((strcmp(s_pName, DnsName) == 0) && (nType == DNSRecordTypeA)) {
				Network::Get()->SendTo(s_nHandle, s_AnswerLocalIp.aBuffer, static_cast<uint16_t>(s_AnswerLocalIp.nSize), s_nMulticastIp, UDP_PORT);
			}

			const auto isDnsDs = (strcmp(DNS_SD_SERVICE, DnsName) == 0);

			for (uint32_t i = 0; i < SERVICE_RECORDS_MAX; i++) {
				if (s_ServiceRecords[i].pName != nullptr) {
					if ((isDnsDs || (strcmp(s_ServiceRecords[i].pServName, DnsName) == 0)) && (nType == DNSRecordTypePTR) ) {
						CreateMDNSMessage(i);
						Network::Get()->SendTo(s_nHandle, &s_ServiceRecordsData.aBuffer, static_cast<uint16_t>(s_ServiceRecordsData.nSize), s_nMulticastIp, UDP_PORT);
					}
				}
			}
		}

	}

	DEBUG_EXIT
}

void MDNS::Parse() {
	DEBUG_ENTRY

	auto *pmDNSHeader = reinterpret_cast<struct TmDNSHeader*>(s_pBuffer);
	const auto nFlags = __builtin_bswap16(pmDNSHeader->nFlags);

#ifndef NDEBUG
//	Dump(pmDNSHeader, nFlags);
#endif

	if ((((nFlags >> 15) & 1) == 0) && (((nFlags >> 14) & 0xf) == DNSOpQuery)) {
		if (pmDNSHeader->queryCount != 0) {
			HandleRequest(__builtin_bswap16(pmDNSHeader->queryCount));
		}
	}

	DEBUG_EXIT
}

void MDNS::Run() {
#if 0
	 uint32_t nNow = Hardware::Get()->Millis();
#endif

	s_nBytesReceived = Network::Get()->RecvFrom(s_nHandle, const_cast<const void **>(reinterpret_cast<void **>(&s_pBuffer)), &s_nRemoteIp, &s_nRemotePort);

	if ((s_nRemotePort == UDP_PORT) && (s_nBytesReceived > sizeof(struct TmDNSHeader))) {
		Parse();
	}

#if 0
	if (__builtin_expect(((nNow - s_nLastAnnounceMillis) > 1000 * ANNOUNCE_TIMEOUT), 0)) {
		DEBUG_PUTS("> Announce <");
		for (uint32_t i = 0; i < s_nDNSServiceRecords; i++) {
			if (s_ServiceRecords[i].pName != 0) {
				//Network::Get()->SendTo(m_nHandle, (uint8_t *)&m_aServiceRecordsData[i].aBuffer, m_aServiceRecordsData[i].nSize, m_nMulticastIp, UDP_PORT);
			}
		}

		s_nLastAnnounceMillis = nNow;
	}
#endif
}

#include <cstdio>

void MDNS::Print() {
	printf("mDNS\n");
	if (s_nHandle == -1) {
		printf(" Not running\n");
		return;
	}
	printf(" Name : %s\n", s_pName);
	for (uint32_t i = 0; i < SERVICE_RECORDS_MAX; i++) {
		if (s_ServiceRecords[i].pName != nullptr) {
			printf(" %s %d %s\n", s_ServiceRecords[i].pServName, s_ServiceRecords[i].nPort, s_ServiceRecords[i].pTextContent == nullptr ? "" : s_ServiceRecords[i].pTextContent);
		}
	}
}

#ifndef NDEBUG
void MDNS::Dump(const struct TmDNSHeader *pmDNSHeader, uint16_t nFlags) {
	Flags tmDNSFlags;

	tmDNSFlags.rcode = nFlags & 0xf;
	tmDNSFlags.cd = (nFlags >> 4) & 1;
	tmDNSFlags.ad = (nFlags >> 5) & 1;
	tmDNSFlags.zero = (nFlags) & 1;
	tmDNSFlags.ra = (nFlags >> 7) & 1;
	tmDNSFlags.rd = (nFlags >> 8) & 1;
	tmDNSFlags.tc = (nFlags >> 9) & 1;
	tmDNSFlags.aa = (nFlags >> 10) & 1;
	tmDNSFlags.opcode = (nFlags >> 14) & 0xf;
	tmDNSFlags.qr = (nFlags >> 15) & 1;

	const uint16_t nQuestions = __builtin_bswap16(pmDNSHeader->queryCount);
	const uint16_t nAnswers = __builtin_bswap16(pmDNSHeader->answerCount);
	const uint16_t nAuthority = __builtin_bswap16(pmDNSHeader->authorityCount);
	const uint16_t nAdditional = __builtin_bswap16(pmDNSHeader->additionalCount);

	printf("ID: %u\n", pmDNSHeader->xid);
	printf("Flags: \n");
	printf("      QR: %u", tmDNSFlags.qr);
	printf("  OPCODE: %u", tmDNSFlags.opcode);
	printf("      AA: %u", tmDNSFlags.aa);
	printf("      TC: %u", tmDNSFlags.tc);
	printf("      RD: %u", tmDNSFlags.rd);
	printf("      RA: %u", tmDNSFlags.ra);
	printf("       Z: %u", tmDNSFlags.zero);
	printf("      AD: %u", tmDNSFlags.ad);
	printf("      CD: %u", tmDNSFlags.cd);
	printf("   RCODE: %u\n", tmDNSFlags.rcode);
	printf("Questions : %u\n", nQuestions);
	printf("Answers   : %u\n", nAnswers);
	printf("Authority : %u\n", nAuthority);
	printf("Additional: %u\n", nAdditional);

}
#endif
