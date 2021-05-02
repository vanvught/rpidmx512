/**
 * @file mdns.cpp
 *
 */
/* Copyright (C) 2019-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <stdint.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cassert>

#include "mdns.h"

#include "network.h"
#include "hardware.h"

#include "debug.h"

#define MDNS_MULTICAST_ADDRESS	"224.0.0.251"
#define MDNS_PORT 				5353
#define MDNS_TLD                ".local"
#define DNS_SD_SERVICE          "_services._dns-sd._udp.local"
#define MDNS_RESPONSE_TTL     	(120)    ///< (in seconds)
#define ANNOUNCE_TIMEOUT 		((MDNS_RESPONSE_TTL / 2) + (MDNS_RESPONSE_TTL / 4))

#define BUFFER_SIZE				1024

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

MDNS::MDNS() {
	struct in_addr group_ip;
	static_cast<void>(inet_aton(MDNS_MULTICAST_ADDRESS, &group_ip));
	m_nMulticastIp = group_ip.s_addr;

	m_pBuffer = new uint8_t[BUFFER_SIZE];
	assert(m_pBuffer != nullptr);

	m_pOutBuffer = new uint8_t[BUFFER_SIZE];
	assert(m_pOutBuffer != nullptr);

	memset(&m_aServiceRecords, 0, sizeof(m_aServiceRecords));
}

MDNS::~MDNS() {
	delete[] m_pOutBuffer;
	m_pOutBuffer = nullptr;

	delete[] m_pBuffer;
	m_pBuffer = nullptr;
}

void MDNS::Start() {
	assert(m_nHandle == -1);

	m_nHandle = Network::Get()->Begin(MDNS_PORT);
	Network::Get()->JoinGroup(m_nHandle, m_nMulticastIp);

	if (m_pName == nullptr) {
		SetName(Network::Get()->GetHostName());
	}

	CreateAnswerLocalIpAddress();

	Network::Get()->SetDomainName(&MDNS_TLD[1]);
}

void MDNS::Stop() {
	Network::Get()->End(MDNS_PORT);
	m_nHandle = -1;
}

void MDNS::SetName(const char *pName) {
	assert(pName != nullptr);
	assert(strlen(pName) != 0);

	if (m_pName != nullptr) {
		delete[] m_pName;
	}

	m_pName = new char[strlen(pName) + sizeof(MDNS_TLD)];
	assert(m_pName != nullptr);

	strcpy(m_pName, pName);
	strcpy(m_pName + strlen(pName), MDNS_TLD);

	DEBUG_PUTS(m_pName);
}

void MDNS::CreateMDNSMessage(uint32_t nIndex) {
	DEBUG1_ENTRY

	auto *pHeader = reinterpret_cast<struct TmDNSHeader*>(&m_aServiceRecordsData[nIndex].aBuffer);

	pHeader->nFlags = __builtin_bswap16(0x8400);
	pHeader->queryCount = 0;
	pHeader->answerCount = __builtin_bswap16(4);
	pHeader->authorityCount = __builtin_bswap16(1);
	pHeader->additionalCount = __builtin_bswap16(0);

	auto *pData = reinterpret_cast<uint8_t*>(&m_aServiceRecordsData[nIndex].aBuffer) + sizeof(struct TmDNSHeader);

	pData += CreateAnswerServiceSrv(nIndex, pData);
	pData += CreateAnswerServiceTxt(nIndex, pData);
	pData += CreateAnswerServiceDnsSd(nIndex, pData);
	pData += CreateAnswerServicePtr(nIndex, pData);

	memcpy(pData, &m_tAnswerLocalIp.aBuffer[sizeof (struct TmDNSHeader)], m_tAnswerLocalIp.nSize - sizeof (struct TmDNSHeader));
	pData += (m_tAnswerLocalIp.nSize - sizeof (struct TmDNSHeader));

	m_aServiceRecordsData[nIndex].nSize =  static_cast<uint32_t>(pData - reinterpret_cast<uint8_t*>(pHeader));

	debug_dump(&m_aServiceRecordsData[nIndex].aBuffer, m_aServiceRecordsData[nIndex].nSize);

	DEBUG1_EXIT
}

uint32_t MDNS::DecodeDNSNameNotation(const char *pDNSNameNotation, char *pString) {
	DEBUG_ENTRY

	const char *pSrc = pDNSNameNotation;
	char *pDst = pString;

	uint32_t nLenght = 0;
	uint32_t nSize = 0;
	bool isCompressed = false;

	while (*pSrc != 0) {
		nLenght = static_cast<uint8_t>(*pSrc++);
//		DEBUG_PRINTF("nLenght=%.2x", (int) nLenght);

		if (nLenght > 63) {
			if (!isCompressed) {
				nSize += 1;
			}

			isCompressed = true;
			const uint32_t nCompressedOffset = ((nLenght & static_cast<uint32_t>(~(0xC0))) << 8) | ((*pSrc & 0xFF));
			nLenght =  m_pBuffer[nCompressedOffset];

//			DEBUG_PRINTF("--> nCompressedOffset=%.4x", (int) nCompressedOffset);

			pSrc = reinterpret_cast<char*>(&m_pBuffer[nCompressedOffset + 1]);

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

	DEBUG_EXIT
	return 1 + nSize;
}

bool MDNS::AddServiceRecord(const char *pName, const char *pServName, uint16_t nPort, const char *pTextContent) {
	DEBUG1_ENTRY

	assert(pServName != nullptr);
	assert(nPort != 0);

	uint32_t i;

	for (i = 0; i < SERVICE_RECORDS_MAX; i++) {
		if (m_aServiceRecords[i].pName == nullptr) {
			m_aServiceRecords[i].nPort = nPort;

			if (pName == nullptr) {
				m_aServiceRecords[i].pName = new char[1 + strlen(Network::Get()->GetHostName() + strlen(pServName))];
				assert(m_aServiceRecords[i].pName != nullptr);

				strcpy(m_aServiceRecords[i].pName, Network::Get()->GetHostName());
			} else {
				m_aServiceRecords[i].pName = new char[1 + strlen(pName) + strlen(pServName)];
				assert(m_aServiceRecords[i].pName != nullptr);

				strcpy(m_aServiceRecords[i].pName, pName);
			}

			strcat(m_aServiceRecords[i].pName, pServName);

			const char *p = FindFirstDotFromRight(pServName);

			m_aServiceRecords[i].pServName = new char[1 + strlen(p) + 12];
			assert(m_aServiceRecords[i].pServName != nullptr);

			strcpy(m_aServiceRecords[i].pServName, p);
			strcat(m_aServiceRecords[i].pServName, "._udp" MDNS_TLD);

			if (pTextContent != nullptr) {
				m_aServiceRecords[i].pTextContent = new char[1 + strlen(pTextContent)];
				assert(m_aServiceRecords[i].pTextContent != nullptr);

				strcpy(m_aServiceRecords[i].pTextContent, pTextContent);
			}

			break;
		}
	}

	if (i == SERVICE_RECORDS_MAX) {
		DEBUG1_EXIT
		return false;
	}

	DEBUG_PRINTF("[%d].nPort = %d", i, m_aServiceRecords[i].nPort = nPort);
	DEBUG_PRINTF("[%d].pName = [%s]", i, m_aServiceRecords[i].pName);
	DEBUG_PRINTF("[%d].pServName = [%s]", i, m_aServiceRecords[i].pServName);
	DEBUG_PRINTF("[%d].pTextContent = [%s]", i, m_aServiceRecords[i].pTextContent);

	CreateMDNSMessage(i);

	Network::Get()->SendTo(m_nHandle, &m_aServiceRecordsData[i].aBuffer, m_aServiceRecordsData[i].nSize, m_nMulticastIp, MDNS_PORT);

	DEBUG1_EXIT
	return true;
}


const char *MDNS::FindFirstDotFromRight(const char *pString) {
	const char *p = pString + strlen(pString);
	while (p > pString && *p-- != '.')
		;
	return &p[1];
}

uint32_t MDNS::WriteDnsName(const char *pSource, char *pDestination, bool bNullTerminated) {
	const char *pSrc = pSource;
	char *pDst = pDestination;

	while (1 == 1) {
		char *pLength = pDst++;
		const char *pSrcStart = pSrc;

		while ((*pSrc != 0) && (*pSrc != '.')) {
			*pDst = *pSrc;
			pDst++;
			pSrc++;
		}

		*pLength = (pSrc - pSrcStart);

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
	DEBUG1_ENTRY

	auto *pHeader = reinterpret_cast<struct TmDNSHeader*>(&m_tAnswerLocalIp.aBuffer);

	pHeader->nFlags = __builtin_bswap16(0x8400);
	pHeader->queryCount = 0;
	pHeader->answerCount = __builtin_bswap16(1);
	pHeader->authorityCount = 0;
	pHeader->additionalCount = 0;

	uint8_t *pData = reinterpret_cast<uint8_t*>(&m_tAnswerLocalIp.aBuffer) + sizeof(struct TmDNSHeader);

	pData += WriteDnsName(m_pName, reinterpret_cast<char*>(pData));

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

	m_tAnswerLocalIp.nSize = static_cast<uint32_t>(pData - reinterpret_cast<uint8_t*>(pHeader));

	DEBUG1_EXIT
}

uint32_t MDNS::CreateAnswerServiceSrv(uint32_t nIndex, uint8_t *pDestination) {
	DEBUG_ENTRY

	uint8_t *pDst = pDestination;

	pDst += WriteDnsName(m_aServiceRecords[nIndex].pName, reinterpret_cast<char*>(pDst), false);
	pDst += WriteDnsName("_udp" MDNS_TLD, reinterpret_cast<char*>(pDst));

	*reinterpret_cast<uint16_t*>(pDst) = __builtin_bswap16(DNSRecordTypeSRV);
	pDst += 2;
	*reinterpret_cast<uint16_t*>(pDst) = __builtin_bswap16(DNSCacheFlushTrue | DNSClassInternet);
	pDst += 2;
	*reinterpret_cast<uint32_t*>(pDst) = __builtin_bswap32(MDNS_RESPONSE_TTL);
	pDst += 4;
	*reinterpret_cast<uint16_t*>(pDst) = __builtin_bswap16(8 + strlen(m_pName));
	pDst += 2;
	*reinterpret_cast<uint32_t*>(pDst) = 0; // Priority and Weight
	pDst += 4;
	*reinterpret_cast<uint16_t*>(pDst) = __builtin_bswap16(m_aServiceRecords[nIndex].nPort);
	pDst += 2;
	pDst += WriteDnsName(m_pName, reinterpret_cast<char*>(pDst));

	DEBUG_EXIT
	return static_cast<uint32_t>(pDst - pDestination);
}

uint32_t MDNS::CreateAnswerServiceTxt(uint32_t nIndex, uint8_t *pDestination) {
	DEBUG_ENTRY

	uint8_t *pDst = pDestination;

	pDst += WriteDnsName(m_aServiceRecords[nIndex].pName, reinterpret_cast<char*>(pDst), false);
	pDst += WriteDnsName("_udp" MDNS_TLD, reinterpret_cast<char*>(pDst));

	*reinterpret_cast<uint16_t*>(pDst) = __builtin_bswap16(DNSRecordTypeTXT);
	pDst += 2;
	*reinterpret_cast<uint16_t*>(pDst) = __builtin_bswap16(DNSCacheFlushTrue | DNSClassInternet);
	pDst += 2;
	*reinterpret_cast<uint32_t*>(pDst) = __builtin_bswap32(MDNS_RESPONSE_TTL);
	pDst += 4;

	if (m_aServiceRecords[nIndex].pTextContent == nullptr) {
		*reinterpret_cast<uint16_t*>(pDst) = __builtin_bswap16(0x0001);	// Data length
		pDst += 2;
		*pDst = 0;														// Text length
		pDst++;
	} else {
		const uint32_t nSize = strlen(m_aServiceRecords[nIndex].pTextContent);
		*reinterpret_cast<uint16_t*>(pDst) = __builtin_bswap16(1 + nSize);	// Data length
		pDst += 2;
		*pDst = nSize;														// Text length
		pDst++;
		strcpy(reinterpret_cast<char*>(pDst), m_aServiceRecords[nIndex].pTextContent);
		pDst += nSize;
	}

	DEBUG_EXIT
	return static_cast<uint32_t>(pDst - pDestination);
}

uint32_t MDNS::CreateAnswerServicePtr(uint32_t nIndex, uint8_t *pDestination) {
	DEBUG_ENTRY

	uint8_t *pDst = pDestination;

	pDst += WriteDnsName(m_aServiceRecords[nIndex].pServName, reinterpret_cast<char*>(pDst));

	*reinterpret_cast<uint16_t*>(pDst) = __builtin_bswap16(DNSRecordTypePTR);
	pDst += 2;
	*reinterpret_cast<uint16_t*>(pDst) = __builtin_bswap16(DNSClassInternet);
	pDst += 2;
	*reinterpret_cast<uint32_t*>(pDst) = __builtin_bswap32(MDNS_RESPONSE_TTL);
	pDst += 4;
	*reinterpret_cast<uint16_t*>(pDst) = __builtin_bswap16(13 + strlen(m_aServiceRecords[nIndex].pName));
	pDst += 2;
	pDst += WriteDnsName(m_aServiceRecords[nIndex].pName, reinterpret_cast<char*>(pDst), false);
	pDst += WriteDnsName("_udp" MDNS_TLD, reinterpret_cast<char*>(pDst));

	DEBUG_EXIT
	return static_cast<uint32_t>(pDst - pDestination);
}

uint32_t MDNS::CreateAnswerServiceDnsSd(uint32_t nIndex, uint8_t *pDestination) {
	DEBUG_ENTRY

	uint8_t *pDst = pDestination;

	pDst += WriteDnsName(DNS_SD_SERVICE, reinterpret_cast<char*>(pDst));

	*reinterpret_cast<uint16_t*>(pDst) = __builtin_bswap16(DNSRecordTypePTR);
	pDst += 2;
	*reinterpret_cast<uint16_t*>(pDst) = __builtin_bswap16(DNSClassInternet);
	pDst += 2;
	*reinterpret_cast<uint32_t*>(pDst) = __builtin_bswap32(MDNS_RESPONSE_TTL);
	pDst += 4;
	*reinterpret_cast<uint16_t*>(pDst) = __builtin_bswap16(2 + strlen(m_aServiceRecords[nIndex].pServName));
	pDst += 2;
	pDst += WriteDnsName(m_aServiceRecords[nIndex].pServName, reinterpret_cast<char*>(pDst));

	DEBUG_EXIT
	return static_cast<uint32_t>(pDst - pDestination);
}

void MDNS::HandleRequest(uint16_t nQuestions) {
	DEBUG_ENTRY

	char DnsName[255];

	uint32_t nOffset = sizeof(struct TmDNSHeader);

	for (uint32_t i = 0; i < nQuestions; i++) {
		nOffset += DecodeDNSNameNotation(reinterpret_cast<const char*>(&m_pBuffer[nOffset]), DnsName);

		const uint16_t nType = __builtin_bswap16(*reinterpret_cast<uint16_t*>(&m_pBuffer[nOffset]));
		nOffset += 2;

		const uint16_t nClass = __builtin_bswap16(*reinterpret_cast<uint16_t*>(&m_pBuffer[nOffset])) & 0x7F;
		nOffset += 2;

		DEBUG_PRINTF("%s ==> Type : %d, Class: %d", DnsName, nType, nClass);

		if (nClass == DNSClassInternet) {
			if ((strcmp(m_pName, DnsName) == 0) && (nType == DNSRecordTypeA)) {
				Network::Get()->SendTo(m_nHandle, m_tAnswerLocalIp.aBuffer, m_tAnswerLocalIp.nSize, m_nMulticastIp, MDNS_PORT);
			}

			const bool isDnsDs = (strcmp(DNS_SD_SERVICE, DnsName) == 0);

			for (uint32_t i = 0; i < SERVICE_RECORDS_MAX; i++) {
				if (m_aServiceRecords[i].pName != nullptr) {
					if ((isDnsDs || (strcmp(m_aServiceRecords[i].pServName, DnsName) == 0)) && (nType == DNSRecordTypePTR) ) {
						Network::Get()->SendTo(m_nHandle, &m_aServiceRecordsData[i].aBuffer, m_aServiceRecordsData[i].nSize, m_nMulticastIp, MDNS_PORT);
					}
				}
			}
		}

	}

	DEBUG_EXIT
}

void MDNS::Parse() {
	DEBUG_ENTRY

	auto *pmDNSHeader = reinterpret_cast<struct TmDNSHeader*>(m_pBuffer);
	const uint16_t nFlags = __builtin_bswap16(pmDNSHeader->nFlags);

#ifndef NDEBUG
	Dump(pmDNSHeader, nFlags);
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

	m_nBytesReceived = Network::Get()->RecvFrom(m_nHandle, m_pBuffer, BUFFER_SIZE, &m_nRemoteIp, &m_nRemotePort);

	if ((m_nRemotePort == MDNS_PORT) && (m_nBytesReceived > sizeof(struct TmDNSHeader))) {
		debug_dump(m_pBuffer, m_nBytesReceived);

		if (m_nBytesReceived < BUFFER_SIZE) {
			Parse();
		} else {
			DEBUG_PUTS("!>>> m_nBytesReceived == BUFFER_SIZE <<<!");
		}

	}

#if 0
	if (__builtin_expect(((nNow - m_nLastAnnounceMillis) > 1000 * ANNOUNCE_TIMEOUT), 0)) {
		DEBUG_PUTS("> Announce <");
		for (uint32_t i = 0; i < m_nDNSServiceRecords; i++) {
			if (m_aServiceRecords[i].pName != 0) {
				//Network::Get()->SendTo(m_nHandle, (uint8_t *)&m_aServiceRecordsData[i].aBuffer, m_aServiceRecordsData[i].nSize, m_nMulticastIp, MDNS_PORT);
			}
		}

		m_nLastAnnounceMillis = nNow;
	}
#endif
}

#ifndef NDEBUG
void MDNS::Dump(const struct TmDNSHeader *pmDNSHeader, uint16_t nFlags) {
	TmDNSFlags tmDNSFlags;

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
