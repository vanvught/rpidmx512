/**
 * @file mdns.cpp
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

#if defined (DEBUG_NET_APPS_MDNS)
# if defined (NDEBUG)
#  undef NDEBUG
# endif
#endif

#include <cstdint>
#include <cstring>
#include <cstdio>
#include <algorithm>
#include <cassert>

#include "net/apps/mdns.h"
#include "net/protocol/ip4.h"
#include "net/protocol/iana.h"

#include "network.h"
#include "hardware.h"

#include "debug.h"

namespace mdns {
#if !defined (MDNS_SERVICE_RECORDS_MAX)
static constexpr auto SERVICE_RECORDS_MAX = 8;
#else
static constexpr auto SERVICE_RECORDS_MAX = MDNS_SERVICE_RECORDS_MAX;
#endif

static constexpr uint32_t MDNS_RESPONSE_TTL = 3600;		///< (in seconds)

static constexpr size_t DOMAIN_MAXLEN = 256;
static constexpr size_t LABEL_MAXLEN = 63;
static constexpr size_t TXT_MAXLEN = 256;

static constexpr char DOMAIN_LOCAL[]		= { 5 , 'l', 'o', 'c', 'a', 'l' , 0};
#if defined (CONFIG_MDNS_DOMAIN_REVERSE)
static constexpr char DOMAIN_REVERSE[]		= { 7 , 'i','n','-','a','d','d','r', 4 , 'a','r','p','a', 0};
#endif
static constexpr char DOMAIN_UDP[]			= { 4 , '_', 'u', 'd', 'p',};
static constexpr char DOMAIN_TCP[]			= { 4 , '_', 't', 'c', 'p' };
static constexpr char DOMAIN_CONFIG[]		= { 7 , '_','c','o','n','f','i','g'};
static constexpr char DOMAIN_TFTP[]			= { 5 , '_','t','f','t','p'};
static constexpr char DOMAIN_HTTP[]			= { 5 , '_','h','t','t','p'};
static constexpr char DOMAIN_RDMNET_LLRP[]	= { 12, '_','r','d','m','n','e','t','-','l','l','r','p'};
static constexpr char DOMAIN_NTP[]			= { 4 , '_','n','t','p'};
static constexpr char DOMAIN_MIDI[]			= { 11, '_','a','p','p','l','e','-','m','i','d','i'};
static constexpr char DOMAIN_OSC[]			= { 4 , '_','o','s','c'};
static constexpr char DOMAIN_DDP[]			= { 4 , '_','d','d','p'};
static constexpr char DOMAIN_PP[]			= { 3 , '_','p','p'};

enum class HostReply : uint32_t {
	A = 0x01, PTR = 0x02
};

enum class ServiceReply : uint32_t {
	TYPE_PTR = 0x10, NAME_PTR = 0x20, SRV = 0x40, TXT = 0x80
};

enum class OpCodes {
	Query = 0, IQuery = 1, Status = 2, Notify = 4, Update = 5
};

enum class Protocols {
	UDP, TCP
};

struct Service {
	const char *pDomain;
	const uint16_t nLength;
	const Protocols protocols;
	const uint16_t nPortDefault;
};

static Service s_Services[] {
		{DOMAIN_CONFIG		, sizeof(DOMAIN_CONFIG)		, Protocols::UDP, 0x2905 },
		{DOMAIN_TFTP		, sizeof(DOMAIN_TFTP)		, Protocols::UDP, 69 },
		{DOMAIN_HTTP		, sizeof(DOMAIN_HTTP)		, Protocols::TCP, 80 },
		{DOMAIN_RDMNET_LLRP	, sizeof(DOMAIN_RDMNET_LLRP), Protocols::UDP, 5569 },
		{DOMAIN_NTP			, sizeof(DOMAIN_NTP)		, Protocols::UDP, 123 },
		{DOMAIN_MIDI		, sizeof(DOMAIN_MIDI)		, Protocols::UDP, 5004 },
		{DOMAIN_OSC			, sizeof(DOMAIN_OSC)		, Protocols::UDP, 0 },
		{DOMAIN_DDP			, sizeof(DOMAIN_DDP)		, Protocols::UDP, 4048 },
		{DOMAIN_PP			, sizeof(DOMAIN_PP)			, Protocols::UDP, 5078 }
};

struct Domain {
	uint8_t aName[DOMAIN_MAXLEN];
	uint16_t nLength;

	void AddLabel(const char *pLabel, const size_t nLabelLength) {
		assert(!(nLabelLength > LABEL_MAXLEN));
		assert(!(nLabelLength > 0 && (1 + nLabelLength + nLength >= DOMAIN_MAXLEN)));
		assert(!(nLabelLength == 0 && (1U + nLength > DOMAIN_MAXLEN)));

		aName[nLength] = static_cast<uint8_t>(nLabelLength);
		nLength++;

		memcpy(&aName[nLength], pLabel, nLabelLength);
		nLength += static_cast<uint16_t>(nLabelLength);
	}

	void AddProtocol(const mdns::Protocols protocols) {
		if (protocols == mdns::Protocols::UDP) {
			memcpy(&aName[nLength], DOMAIN_UDP, sizeof(DOMAIN_UDP));
			nLength += static_cast<uint16_t>(sizeof(DOMAIN_UDP));
			return;
		}

		memcpy(&aName[nLength], DOMAIN_TCP, sizeof(DOMAIN_TCP));
		nLength += static_cast<uint16_t>(sizeof(DOMAIN_TCP));
	}

	void AddDotLocal() {
		memcpy(&aName[nLength], DOMAIN_LOCAL, sizeof(DOMAIN_LOCAL));
		nLength += static_cast<uint16_t>(sizeof(DOMAIN_LOCAL));
	}

	void Print(const bool bNewLine = false) {
		putchar(' ');
		auto const *pName = aName;
		while (*pName && (pName < &aName[nLength])) {
			auto nLength = static_cast<size_t>(*pName);
			pName++;
			printf("%.*s.", static_cast<int>(nLength), pName);
			pName += nLength;
		}

		if (bNewLine) {
			putchar('\n');
		}
	}

	friend bool operator== (Domain const &DomainA, Domain const &DomainB) {
		if (DomainA.nLength != DomainB.nLength) {
			return false;
		}

		auto const *pNameA = DomainA.aName;
		auto const *pNameB = DomainB.aName;

		while(*pNameA && *pNameB && (pNameA < &DomainA.aName[DomainA.nLength])) {
			if (*pNameA != *pNameB) {
				return false;
			}

			auto nLength = static_cast<size_t>(*pNameA);
			pNameA++;
			pNameB++;

			if (strncasecmp(reinterpret_cast<const char *>(pNameA), reinterpret_cast<const char *>(pNameB), nLength) != 0) {
				return false;
			}

			pNameA += nLength;
			pNameB += nLength;
		}

		return true;
	}
};

static constexpr Domain DOMAIN_DNSSD {
	{ 9 , '_','s','e','r','v','i','c','e','s', 7 ,'_','d','n','s','-','s','d', 4 , '_', 'u', 'd', 'p', 5 , 'l', 'o', 'c', 'a', 'l' , 0},
	10 + 8 + 5 + 6 + 1
};

static ServiceRecord s_ServiceRecords[mdns::SERVICE_RECORDS_MAX];
static HostReply s_HostReplies;
static ServiceReply s_ServiceReplies;
static uint8_t s_RecordsData[net::dns::MULTICAST_MESSAGE_SIZE];
static bool s_isUnicast;
static bool s_bLegacyQuery;

}  // namespace mdns

static constexpr mdns::HostReply operator| (mdns::HostReply a, mdns::HostReply b) {
	return static_cast<mdns::HostReply>((static_cast<uint32_t>(a) | static_cast<uint32_t>(b)));
}

static constexpr mdns::HostReply operator& (mdns::HostReply a, mdns::HostReply b) {
	return static_cast<mdns::HostReply>((static_cast<uint32_t>(a) & static_cast<uint32_t>(b)));
}

static constexpr mdns::ServiceReply operator| (mdns::ServiceReply a, mdns::ServiceReply b) {
	return static_cast<mdns::ServiceReply>((static_cast<uint8_t>(a) | static_cast<uint8_t>(b)));
}

static constexpr mdns::ServiceReply operator& (mdns::ServiceReply a, mdns::ServiceReply b) {
	return static_cast<mdns::ServiceReply>((static_cast<uint32_t>(a) & static_cast<uint32_t>(b)));
}

int32_t MDNS::s_nHandle;
uint32_t MDNS::s_nRemoteIp;
uint16_t MDNS::s_nRemotePort;
uint32_t MDNS::s_nBytesReceived;
uint8_t *MDNS::s_pReceiveBuffer;
MDNS *MDNS::s_pThis;

using namespace mdns;

namespace network {
void mdns_announcement() {
	DEBUG_ENTRY

	assert(MDNS::Get() != nullptr);
	MDNS::Get()->SendAnnouncement(MDNS_RESPONSE_TTL);

	DEBUG_ENTRY
}

void mdns_shutdown() {
	DEBUG_ENTRY

	assert(MDNS::Get() != nullptr);
	MDNS::Get()->SendAnnouncement(0);

	DEBUG_ENTRY
}
}  // namespace network

static void create_service_domain(Domain& domain, ServiceRecord const& serviceRecord, const bool bIncludeName) {
	DEBUG_ENTRY

	domain.nLength = 0;

	if (bIncludeName) {
		if (serviceRecord.pName != nullptr) {
			domain.AddLabel(serviceRecord.pName, strlen(serviceRecord.pName));
		} else {
			domain.AddLabel(Network::Get()->GetHostName(), strlen(Network::Get()->GetHostName()));
		}
	}

	const auto nIndex = static_cast<uint32_t>(serviceRecord.services);

	memcpy(&domain.aName[domain.nLength], s_Services[nIndex].pDomain, s_Services[nIndex].nLength);
	domain.nLength += s_Services[nIndex].nLength;

	domain.AddProtocol(s_Services[nIndex].protocols);
	domain.AddDotLocal();

	DEBUG_EXIT
}

static void create_host_domain(Domain &domain) {
	domain.nLength = 0;
	domain.AddLabel(Network::Get()->GetHostName(), strlen(Network::Get()->GetHostName()));
	domain.AddDotLocal();
}

#if defined (CONFIG_MDNS_DOMAIN_REVERSE)
static void create_reverse_domain(Domain &domain) {
	DEBUG_ENTRY

	domain.nLength = 0;
	auto nIp = Network::Get()->GetIp();
	const auto *pIp = reinterpret_cast<const uint8_t *>(&nIp);

	for (int32_t i = IPv4_ADDR_LEN - 1; i >= 0; i--) {
		char buffer[3];
		uint32_t nLength = 1;

		auto d = pIp[i];

		const auto h = d / 100U;

		if (h != 0) {
			nLength = 3;
			buffer[0] = '0' + static_cast<char>(h);
			d -= static_cast<uint8_t>(h * 100U);
		}

		const auto t = d / 10U;

		if (t != 0) {
			nLength = std::max(static_cast<uint32_t>(2), nLength);
		} else {
			nLength = std::max(static_cast<uint32_t>(1), nLength);
		}

		buffer[1] = '0' + static_cast<char>(t);
		buffer[2] = '0' + static_cast<char>(d % 10U);

		domain.AddLabel(&buffer[3 - nLength], nLength);
	}

	memcpy(&domain.aName[domain.nLength], DOMAIN_REVERSE, sizeof(DOMAIN_REVERSE));
	domain.nLength += static_cast<uint16_t>(sizeof(DOMAIN_REVERSE));

	DEBUG_EXIT
}
#endif

/*
 * https://opensource.apple.com/source/mDNSResponder/mDNSResponder-26.2/mDNSCore/mDNS.c.auto.html
 * mDNSlocal const mDNSu8 *FindCompressionPointer(const mDNSu8 *const base, const mDNSu8 *const end, const mDNSu8 *const domname)
 */
uint8_t *find_compression_pointer(const uint8_t *const base, const uint8_t *const end, const uint8_t *const domname) {
	const auto *result = end - *domname - 1;

	while (result >= base) {
		if (result[0] == domname[0] && result[1] == domname[1]) {
			const auto *name = domname;
			const auto *targ = result;

			while (targ + *name < end) {
				// First see if this label matches
				int i;

				for (i = 0; i <= *name; i++) {
					if (targ[i] != name[i]) {
						break;
					}
				}

				if (i <= *name) {
					break;	// If label did not match, bail out
				}

				targ += 1 + *name;	// Else, did match, so advance target pointer
				name += 1 + *name;	// and proceed to check next label

				if (*name == 0 && *targ == 0) {	// If no more labels, we found a match!
					return const_cast<uint8_t *>(result);
				}

				if (*name == 0) {	// If no more labels to match, we failed, so bail out
					break;
				}

				// The label matched, so now follow the pointer (if appropriate) and then see if the next label matches
				if (targ[0] < 0x40) {
					continue;	// If length value, continue to check next label
				}

				if (targ[0] < 0xC0) {
					break;	// If 40-BF, not valid
				}

				if (targ + 1 >= end) {
					break;	// Second byte not present!
				}

				const uint8_t *pointertarget = base + ((static_cast<uint16_t>(targ[0] & 0x3F)) << 8) + targ[1];

				if (targ < pointertarget) {
					break;	// Pointertarget must point *backwards* in the packet
				}

				if (pointertarget[0] >= 0x40) {
					break;	// Pointertarget must point to a valid length byte
				}

				targ = pointertarget;
			}
		}

		result--;// We failed to match at this search position, so back up the tentative result pointer and try again
	}

	return nullptr;
}

/*
 * https://opensource.apple.com/source/mDNSResponder/mDNSResponder-26.2/mDNSCore/mDNS.c.auto.html
 * mDNSlocal mDNSu8 *putDomainNameAsLabels(const DNSMessage *const msg, mDNSu8 *ptr, const mDNSu8 *const limit, const domainname *const name)
 */
static uint8_t *put_domain_name_as_labels(uint8_t *ptr, Domain const &domain) {
	const uint8_t *const base = s_RecordsData;
	const auto *np = domain.aName;
	uint8_t *pointer = nullptr;
	const auto *const searchlimit = ptr;

	while (*np) {
		pointer = find_compression_pointer(base, searchlimit, np);

		if (pointer != nullptr) {
			auto nOffset = static_cast<uint16_t>(pointer - base);
			*ptr++ = static_cast<uint8_t>(0xC0 | (nOffset >> 8));
			*ptr++ = static_cast<uint8_t>(nOffset);
			return ptr;
		} else {
			auto len = *np++;
			*ptr++ = len;
			for (uint32_t i = 0; i < len; i++) {
				*ptr++ = *np++;
			}
		}
	}

	*ptr++ = 0;
	return ptr;
}

static uint8_t *add_question(uint8_t *pDestination, const Domain& domain, const net::dns::RRType type, const bool bFlush) {
	auto *pDst = put_domain_name_as_labels(pDestination, domain);

	*reinterpret_cast<volatile uint16_t*>(pDst) = __builtin_bswap16(static_cast<uint16_t>(type));
	pDst += 2;
	*reinterpret_cast<uint16_t*>(pDst) = __builtin_bswap16((bFlush ? net::dns::RRClass::RRCLASS_FLUSH : static_cast<net::dns::RRClass>(0)) | net::dns::RRClass::RRCLASS_INTERNET);
	pDst += 2;

	return pDst;
}

static uint32_t add_answer_srv(mdns::ServiceRecord const& serviceRecord, uint8_t *pDestination, const uint32_t nTTL) {
	DEBUG_ENTRY

	Domain domain;
	create_service_domain(domain, serviceRecord, true);

	auto *pDst = add_question(pDestination, domain, net::dns::RRType::RRTYPE_SRV, true);

	*reinterpret_cast<uint32_t*>(pDst) = __builtin_bswap32(nTTL);
	pDst += 4;
	auto *lengtPointer = pDst;
	pDst += 2;
	*reinterpret_cast<uint32_t*>(pDst) = 0; // Priority and Weight
	pDst += 4;
	*reinterpret_cast<uint16_t*>(pDst) = serviceRecord.nPort;
	pDst += 2;
	auto *pBegin = pDst;

	create_host_domain(domain);
	pDst = put_domain_name_as_labels(pDst, domain);

	*reinterpret_cast<uint16_t*>(lengtPointer) = __builtin_bswap16(static_cast<uint16_t>(6U + pDst - pBegin));

	DEBUG_EXIT
	return static_cast<uint32_t>(pDst - pDestination);
}

static uint32_t add_answer_txt(mdns::ServiceRecord const& serviceRecord, uint8_t *pDestination, const uint32_t nTTL) {
	DEBUG_ENTRY

	Domain domain;
	create_service_domain(domain, serviceRecord, true);

	auto *pDst = add_question(pDestination, domain, net::dns::RRType::RRTYPE_TXT, true);

	*reinterpret_cast<uint32_t*>(pDst) = __builtin_bswap32(nTTL);
	pDst += 4;

	if (serviceRecord.pTextContent == nullptr) {
		*reinterpret_cast<uint16_t*>(pDst) = __builtin_bswap16(0x0001);	// Data length
		pDst += 2;
		*pDst = 0;														// Text length
		pDst++;
	} else {
		const auto nSize = serviceRecord.nTextContentLength;
		*reinterpret_cast<uint16_t*>(pDst) = __builtin_bswap16(static_cast<uint16_t>(1U + nSize));	// Data length
		pDst += 2;
		*pDst = static_cast<uint8_t>(nSize);														// Text length
		pDst++;
		memcpy(reinterpret_cast<char*>(pDst), serviceRecord.pTextContent, serviceRecord.nTextContentLength);
		pDst += nSize;
	}

	DEBUG_EXIT
	return static_cast<uint32_t>(pDst - pDestination);
}

static uint32_t add_answer_ptr(mdns::ServiceRecord const& serviceRecord, uint8_t *pDestination, const uint32_t nTTL) {
	DEBUG_ENTRY

	Domain domain;
	create_service_domain(domain, serviceRecord, false);

	auto *pDst = add_question(pDestination, domain, net::dns::RRType::RRTYPE_PTR, false);

	*reinterpret_cast<uint32_t*>(pDst) = __builtin_bswap32(nTTL);
	pDst += 4;
	auto *lengtPointer = pDst;
	pDst += 2;
	auto *pBegin = pDst;

	create_service_domain(domain, serviceRecord, true);
	pDst = put_domain_name_as_labels(pDst, domain);

	*reinterpret_cast<uint16_t*>(lengtPointer) = __builtin_bswap16(static_cast<uint16_t>(pDst - pBegin));

	DEBUG_EXIT
	return static_cast<uint32_t>(pDst - pDestination);
}

static uint32_t add_answer_dnsd_ptr(mdns::ServiceRecord const& serviceRecord, uint8_t *pDestination, const uint32_t nTTL) {
	DEBUG_ENTRY

	auto *pDst = add_question(pDestination, DOMAIN_DNSSD, net::dns::RRType::RRTYPE_PTR, false);

	*reinterpret_cast<uint32_t*>(pDst) = __builtin_bswap32(nTTL);
	pDst += 4;
	auto *lengtPointer = pDst;
	pDst += 2;
	auto *pBegin = pDst;

	Domain domain;

	create_service_domain(domain, serviceRecord, false);
	pDst = put_domain_name_as_labels(pDst, domain);

	*reinterpret_cast<uint16_t*>(lengtPointer) = __builtin_bswap16(static_cast<uint16_t>(pDst - pBegin));

	DEBUG_EXIT
	return static_cast<uint32_t>(pDst - pDestination);
}

static uint32_t add_answer_a(uint8_t *pDestination, const uint32_t nTTL) {
	DEBUG_ENTRY

	Domain domain;
	create_host_domain(domain);

	auto *pDst = add_question(pDestination, domain, net::dns::RRType::RRTYPE_A, true);

	*reinterpret_cast<uint32_t*>(pDst) = __builtin_bswap32(nTTL);
	pDst += 4;
	*reinterpret_cast<uint16_t*>(pDst) = __builtin_bswap16(4);	// Data length
	pDst += 2;
	*reinterpret_cast<uint32_t*>(pDst) = Network::Get()->GetIp();
	pDst += 4;

	DEBUG_EXIT
	return static_cast<uint32_t>(pDst - pDestination);
}

#if defined (CONFIG_MDNS_DOMAIN_REVERSE)
static uint32_t add_answer_hostv4_ptr(uint8_t *pDestination, const uint32_t nTTL) {
	DEBUG_ENTRY

	Domain domain;
	create_reverse_domain(domain);

	auto *pDst = add_question(pDestination, domain, net::dns::RRType::RRTYPE_PTR, true);

	*reinterpret_cast<uint32_t*>(pDst) = __builtin_bswap32(nTTL);
	pDst += 4;
	auto *lengtPointer = pDst;
	pDst += 2;
	auto *pBegin = pDst;

	create_host_domain(domain);
	pDst = put_domain_name_as_labels(pDst, domain);

	*reinterpret_cast<uint16_t*>(lengtPointer) = __builtin_bswap16(static_cast<uint16_t>(pDst - pBegin));

	DEBUG_EXIT
	return static_cast<uint32_t>(pDst - pDestination);
}
#endif

/*
 * https://opensource.apple.com/source/mDNSResponder/mDNSResponder-26.2/mDNSCore/mDNS.c.auto.html
 * mDNSlocal const mDNSu8 *getDomainName(const DNSMessage *const msg, const mDNSu8 *ptr, const mDNSu8 *const end, domainname *const name)
 *
 * Routine to fetch an FQDN from the DNS message, following compression pointers if necessary.
 */
const uint8_t *get_domain_name(const uint8_t *const msg, const uint8_t *ptr, const uint8_t *const end, uint8_t *const name) {
	const uint8_t *nextbyte = nullptr;					// Record where we got to before we started following pointers
	uint8_t *np = name;									// Name pointer
	const uint8_t *const limit = np + DOMAIN_MAXLEN;	// Limit so we don't overrun buffer

	if (ptr < reinterpret_cast<const uint8_t*>(msg) || ptr >= end) {
		DEBUG_PUTS("Illegal ptr not within packet boundaries");
		return nullptr;
	}

	*np = 0;// Tentatively place the root label here (may be overwritten if we have more labels)

	while (1)						// Read sequence of labels
	{
		const auto len = *ptr++;	// Read length of this label
		if (len == 0)
			break;		// If length is zero, that means this name is complete
		switch (len & 0xC0) {
		int i;
		uint16_t offset;

	case 0x00:
		if (ptr + len >= end)// Remember: expect at least one more byte for the root label
				{
			DEBUG_PUTS("Malformed domain name (overruns packet end)");
			return nullptr;
		}
		if (np + 1 + len >= limit)// Remember: expect at least one more byte for the root label
				{
			DEBUG_PUTS("Malformed domain name (more than 255 characters)");
			return nullptr;
		}
		*np++ = len;
		for (i = 0; i < len; i++)
			*np++ = *ptr++;
		*np = 0;// Tentatively place the root label here (may be overwritten if we have more labels)
		break;

	case 0x40:
		DEBUG_PRINTF("Extended EDNS0 label types 0x40 not supported in name %.*s", static_cast<int>(len), name);
		return nullptr;

	case 0x80:
		DEBUG_PRINTF("Illegal label length 0x80 in domain name %.*s", static_cast<int>(len), name);
		return nullptr;

	case 0xC0:
		offset = static_cast<uint16_t>(((static_cast<uint16_t>(len & 0x3F)) << 8) | *ptr++);
		if (!nextbyte)
			nextbyte = ptr;	// Record where we got to before we started following pointers
		ptr = reinterpret_cast<const uint8_t *>(msg) + offset;
		if (ptr < reinterpret_cast<const uint8_t *>(msg) || ptr >= end) {
			DEBUG_PUTS("Illegal compression pointer not within packet boundaries");
			return nullptr;
		}
		if (*ptr & 0xC0) {
			DEBUG_PUTS("Compression pointer must point to real label");
			return nullptr;
		}
		break;
		}
	}

	if (nextbyte)
		return (nextbyte);
	else
		return (ptr);
}

void MDNS::SendAnswerLocalIpAddress(const uint16_t nTransActionID, const uint32_t nTTL) {
	DEBUG_ENTRY

	uint32_t nAnswers = 0;
	uint8_t *pDst = reinterpret_cast<uint8_t *>(&s_RecordsData) + sizeof(struct net::dns::Header);

#if defined (CONFIG_MDNS_DOMAIN_REVERSE)
	if ((HostReply::PTR & s_HostReplies) == HostReply::PTR) {
		if (s_bLegacyQuery) {
			Domain domain;
			create_reverse_domain(domain);
			pDst = add_question(pDst, domain, net::dns::RRType::RRTYPE_PTR, false);
		}
	}
#endif

	if ((HostReply::A & s_HostReplies) == HostReply::A) {
		nAnswers++;
		pDst += add_answer_a(pDst, nTTL);
	}
#if defined (CONFIG_MDNS_DOMAIN_REVERSE)
	if ((HostReply::PTR & s_HostReplies) == HostReply::PTR) {
		nAnswers++;
		pDst += add_answer_hostv4_ptr(pDst, nTTL);
	}
#endif

	auto *pHeader = reinterpret_cast< net::dns::Header *>(&s_RecordsData);

	pHeader->xid = nTransActionID;
	pHeader->nFlag1 = net::dns::Flag1::FLAG1_RESPONSE | net::dns::Flag1::FLAG1_AUTHORATIVE;
	pHeader->nFlag2 = 0;
#if defined (CONFIG_MDNS_DOMAIN_REVERSE)
	pHeader->nQueryCount = __builtin_bswap16(static_cast<uint16_t>(s_bLegacyQuery));
#else
	pHeader->nQueryCount = 0;
#endif
	pHeader->nAnswerCount = __builtin_bswap16(static_cast<uint16_t>(nAnswers));
	pHeader->nAuthorityCount = 0;
	pHeader->nAdditionalCount = 0;

	const auto nSize = static_cast<uint16_t>(pDst - reinterpret_cast<uint8_t *>(pHeader));
	SendTo(nSize);

	DEBUG_EXIT
}

MDNS::MDNS() {
	assert(s_pThis == nullptr);
	s_pThis = this;

	for (auto &record : s_ServiceRecords) {
		record.services = Services::LAST_NOT_USED;
	}

	s_nHandle = Network::Get()->Begin(net::iana::IANA_PORT_MDNS);
	assert(s_nHandle != -1);

	Network::Get()->JoinGroup(s_nHandle, net::dns::MULTICAST_ADDRESS);
	Network::Get()->SetDomainName(&DOMAIN_LOCAL[1]);

	SendAnnouncement(MDNS_RESPONSE_TTL);
}

MDNS::~MDNS() {
	SendAnnouncement(0);

	for (auto &record : s_ServiceRecords) {
		if (record.pName != nullptr) {
			delete[] record.pName;
		}

		if (record.pTextContent != nullptr) {
			delete[] record.pTextContent;
		}
	}

	Network::Get()->LeaveGroup(s_nHandle, net::dns::MULTICAST_ADDRESS);
	Network::Get()->End(net::iana::IANA_PORT_MDNS);
	s_nHandle = -1;
}

void MDNS::SendAnnouncement(const uint32_t nTTL) {
	DEBUG_ENTRY

	s_nRemotePort = net::iana::IANA_PORT_MDNS; //FIXME Hack ;-)
	s_HostReplies = HostReply::A;

	SendAnswerLocalIpAddress(0, nTTL);

	for (auto &record : s_ServiceRecords) {
		if (record.services < Services::LAST_NOT_USED) {
			s_ServiceReplies = ServiceReply::TYPE_PTR
							 | ServiceReply::NAME_PTR
							 | ServiceReply::SRV
							 | ServiceReply::TXT;
			SendMessage(record, 0, nTTL);
		}
	}

	DEBUG_EXIT
}

bool MDNS::ServiceRecordAdd(const char *pName, const mdns::Services services, const char *pTextContent, const uint16_t nPort) {
	DEBUG_ENTRY
	assert(services < mdns::Services::LAST_NOT_USED);

	for (auto &record : s_ServiceRecords) {
		if (record.services == Services::LAST_NOT_USED) {
			if (pName != nullptr) {
				const auto nLength = std::min(LABEL_MAXLEN, strlen(pName));
				if (nLength == 0) {
					assert(0);
					return false;
				}

				record.pName = new char[1 +  nLength];

				assert(record.pName != nullptr);
				memcpy(record.pName, pName, nLength);
				record.pName[nLength] = '\0';
			}

			record.services = services;

			if (nPort == 0) {
				record.nPort = __builtin_bswap16(s_Services[static_cast<uint32_t>(services)].nPortDefault);
			} else {
				record.nPort = __builtin_bswap16(nPort);
			}

			if (pTextContent != nullptr) {
				const auto nLength = std::min(TXT_MAXLEN, strlen(pTextContent));
				record.pTextContent = new char[nLength];

				assert(record.pTextContent != nullptr);
				memcpy(record.pTextContent, pTextContent, nLength);

				record.nTextContentLength = static_cast<uint16_t>(nLength);
			}

			s_nRemotePort = net::iana::IANA_PORT_MDNS; //FIXME Hack ;-)

			s_ServiceReplies = ServiceReply::TYPE_PTR
					| ServiceReply::NAME_PTR
					| ServiceReply::SRV
					| ServiceReply::TXT;

			SendMessage(record, 0, MDNS_RESPONSE_TTL);
			return true;
		}
	}

	assert(0);
	return false;
}

bool MDNS::ServiceRecordDelete(const mdns::Services service) {
	DEBUG_ENTRY
	assert(service < mdns::Services::LAST_NOT_USED);

	for (auto &record : s_ServiceRecords) {
		if (record.services == service) {
			SendMessage(record, 0, 0);

			if (record.pName != nullptr) {
				delete[] record.pName;
			}

			if (record.pTextContent != nullptr) {
				delete[] record.pTextContent;
			}

			DEBUG_EXIT
			return true;
		}
	}

	DEBUG_EXIT
	return false;
}

void MDNS::SendTo(const uint32_t nLength) {
	if (!s_isUnicast) {
		Network::Get()->SendTo(s_nHandle, s_RecordsData, nLength, net::dns::MULTICAST_ADDRESS, net::iana::IANA_PORT_MDNS);
		return;
	}

	Network::Get()->SendTo(s_nHandle, s_RecordsData, nLength, s_nRemoteIp, s_nRemotePort);
}

void MDNS::SendMessage(mdns::ServiceRecord const& serviceRecord, const uint16_t nTransActionID, const uint32_t nTT) {
	DEBUG_ENTRY

	uint32_t nAnswers = 0;
	auto *pDst = reinterpret_cast<uint8_t *>(&s_RecordsData) + sizeof(struct net::dns::Header);

	if ((s_ServiceReplies & ServiceReply::TYPE_PTR) == ServiceReply::TYPE_PTR) {
		nAnswers++;
		pDst += add_answer_dnsd_ptr(serviceRecord, pDst, nTT);
	}

	if ((s_ServiceReplies & ServiceReply::NAME_PTR) == ServiceReply::NAME_PTR) {
		nAnswers++;
		pDst += add_answer_ptr(serviceRecord, pDst, nTT);
	}

	if ((s_ServiceReplies & ServiceReply::SRV) == ServiceReply::SRV) {
		nAnswers++;
		pDst += add_answer_srv(serviceRecord, pDst, nTT);
	}

	if ((s_ServiceReplies & ServiceReply::TXT) == ServiceReply::TXT) {
		nAnswers++;
		pDst += add_answer_txt(serviceRecord, pDst, nTT);
	}

	pDst += add_answer_a(pDst, nTT);

	auto *pHeader = reinterpret_cast<net::dns::Header *>(&s_RecordsData);

	pHeader->xid = nTransActionID;
	pHeader->nFlag1 = net::dns::Flag1::FLAG1_RESPONSE | net::dns::Flag1::FLAG1_AUTHORATIVE;
	pHeader->nFlag2 = 0;
	pHeader->nQueryCount = 0;
	pHeader->nAnswerCount = __builtin_bswap16(static_cast<uint16_t>(nAnswers));
	pHeader->nAuthorityCount = __builtin_bswap16(1);
	pHeader->nAdditionalCount = __builtin_bswap16(0);

	const auto nSize = static_cast<uint16_t>(pDst - reinterpret_cast<uint8_t*>(pHeader));
	SendTo(nSize);

	DEBUG_EXIT
}

void MDNS::HandleQuestions(const uint32_t nQuestions) {
	DEBUG_ENTRY
	DEBUG_PRINTF("nQuestions=%u", nQuestions);

	s_HostReplies = static_cast<mdns::HostReply>(0);
	s_isUnicast = (s_nRemotePort != net::iana::IANA_PORT_MDNS);
	s_bLegacyQuery = s_isUnicast && (nQuestions == 1);

	const auto nTransactionID = s_bLegacyQuery ? *reinterpret_cast<uint16_t *>(&s_pReceiveBuffer[0]) : static_cast<uint16_t>(0);

	uint32_t nOffset = sizeof(struct net::dns::Header);

	for (uint32_t i = 0; i < nQuestions; i++) {
		Domain resourceDomain;

		auto *pResult = get_domain_name(s_pReceiveBuffer, &s_pReceiveBuffer[nOffset], &s_pReceiveBuffer[s_nBytesReceived], resourceDomain.aName);
		if (pResult == nullptr) {
			DEBUG_EXIT
			return;
		}

		resourceDomain.nLength = static_cast<uint16_t>(pResult - &s_pReceiveBuffer[nOffset]);
		nOffset += resourceDomain.nLength;

		const auto nType = static_cast<net::dns::RRType>(__builtin_bswap16(*reinterpret_cast<uint16_t*>(&s_pReceiveBuffer[nOffset])));
		nOffset += 2;

		const auto nClass = static_cast<net::dns::RRClass>(__builtin_bswap16(*reinterpret_cast<uint16_t *>(&s_pReceiveBuffer[nOffset])) & 0x7F);
		nOffset += 2;

#ifndef NDEBUG
		resourceDomain.Print();
		printf(" ==> Type : %d, Class: %d\n", static_cast<int>(nType), static_cast<int>(nClass));
#endif

		if ((nClass != net::dns::RRClass::RRCLASS_INTERNET) && (nClass != net::dns::RRClass::RRCLASS_ANY)) {
			continue;
		}

		/*
		 * Check host
		 */

		Domain domainHost;

		if ((nType == net::dns::RRType::RRTYPE_A) || (nType == net::dns::RRType::RRTYPE_ALL)) {
			DEBUG_PUTS("");
			create_host_domain(domainHost);

			if (domainHost == resourceDomain) {
				s_HostReplies = s_HostReplies | HostReply::A;
			}
		}

#if defined (CONFIG_MDNS_DOMAIN_REVERSE)
		if (nType == net::dns::RRType::RRTYPE_PTR || nType == net::dns::RRType::RRTYPE_ALL) {
			DEBUG_PUTS("");
			create_reverse_domain(domainHost);

			if (domainHost == resourceDomain) {
				s_HostReplies = s_HostReplies | HostReply::PTR;
			}

		}
#endif

		for (auto &record : s_ServiceRecords) {
			if (record.services < Services::LAST_NOT_USED) {
				/*
				 * Check service
				 */

				s_ServiceReplies = static_cast<mdns::ServiceReply>(0);
				Domain serviceDomain;

				if (nType == net::dns::RRType::RRTYPE_PTR || nType == net::dns::RRType::RRTYPE_ALL) {
					if (DOMAIN_DNSSD == resourceDomain) {
						s_ServiceReplies = s_ServiceReplies | ServiceReply::TYPE_PTR;
					}

					create_service_domain(serviceDomain, record, false);

					if (serviceDomain == resourceDomain) {
						s_ServiceReplies = s_ServiceReplies | ServiceReply::NAME_PTR;
					}
				}

				create_service_domain(serviceDomain, record, true);

				if (serviceDomain == resourceDomain) {
					if ((nType == net::dns::RRType::RRTYPE_SRV) || (nType == net::dns::RRType::RRTYPE_ALL)) {
						s_ServiceReplies = s_ServiceReplies | ServiceReply::SRV;
					}

					if ((nType == net::dns::RRType::RRTYPE_TXT) || (nType == net::dns::RRType::RRTYPE_ALL)) {
						s_ServiceReplies = s_ServiceReplies | ServiceReply::TXT;
					}
				}

				if (s_ServiceReplies != static_cast<mdns::ServiceReply>(0)) {
					SendMessage(record, nTransactionID, MDNS_RESPONSE_TTL);
				}
			}
		}
	}

	if (s_HostReplies != static_cast<mdns::HostReply>(0)) {
		DEBUG_PUTS("");
		SendAnswerLocalIpAddress(nTransactionID, MDNS_RESPONSE_TTL);
	}

	DEBUG_EXIT
}

void MDNS::Print() {
	printf("mDNS\n");

	Domain domain;

	create_host_domain(domain);
	domain.Print(true);

	for (auto &record : s_ServiceRecords) {
		if (record.services < Services::LAST_NOT_USED) {
			create_service_domain(domain, record, false);
			domain.Print();
			printf(" %d %.*s\n", __builtin_bswap16(record.nPort), record.nTextContentLength, record.pTextContent == nullptr ? "" : record.pTextContent);
		}
	}
}
