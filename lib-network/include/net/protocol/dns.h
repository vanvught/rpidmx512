/**
 * @file dns.h
 *
 */
/* Copyright (C) 2018-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef NET_PROTOCOL_DNS_H_
#define NET_PROTOCOL_DNS_H_

#include <cstdint>

#include "ip4_address.h"

namespace net { namespace dns {
static constexpr uint32_t SIZEOF_DNS_HDR = 12;

enum class Flag1 : uint8_t {
	FLAG1_RESPONSE			= 0x80,	///< query (0), or a response (1).
	FLAG1_OPCODE_STATUS		= 0x10,	///< a server status request (STATUS)
	FLAG1_OPCODE_IQUERY 	= 0x08,	///< an inverse query (IQUERY)
	FLAG1_OPCODE_STANDARD	= 0x00,	///< (RFC 6762, section 18.3)
	FLAG1_AUTHORATIVE		= 0x04,	///< Authoritative Answer
	FLAG1_TRUNC				= 0x02,	///< TrunCation
	FLAG1_RD				= 0x01	///< If RD is set, it directs the name server to pursue the query recursively.
};

static constexpr uint8_t operator| (Flag1 a, Flag1 b) {
	return static_cast<uint8_t>((static_cast<uint8_t>(a) | static_cast<uint8_t>(b)));
}

///< NS field TYPE used for "Resource Records"
enum class RRType : uint16_t {
	RRTYPE_A	= 1,	///< a host address
	RRTYPE_PTR	= 12,	///< a domain name pointer
	RRTYPE_TXT	= 16,	///< text strings
	RRTYPE_SRV	= 33,	///< service location
	RRTYPE_ALL	= 255	///< any type
};

///< DNS field CLASS used for "Resource Records"
enum class RRClass : uint16_t {
	RRCLASS_INTERNET = 1,		///< Internet
	RRCLASS_ANY      = 255, 	///< Any class
	RRCLASS_FLUSH    = 0x8000	///< Flush bit
};

static constexpr uint16_t operator| (RRClass a, RRClass b) {
	return static_cast<uint16_t>((static_cast<uint16_t>(a) | static_cast<uint16_t>(b)));
}

struct Header {
	uint16_t xid;
	uint8_t nFlag1;
	uint8_t nFlag2;
	uint16_t nQueryCount;
	uint16_t nAnswerCount;
	uint16_t nAuthorityCount;
	uint16_t nAdditionalCount;
} __attribute__((__packed__));

inline uint8_t dns_header_get_opcode(const Header *const header) {
	return ((header->nFlag1) >> 3) & 0xF;
}

/*
 * mDNS
 */

static constexpr uint32_t MULTICAST_MESSAGE_SIZE = 512;	///< The 1987 DNS specification [RFC1035] restricts DNS messages carried by UDP to no more than 512 bytes
static constexpr uint32_t MULTICAST_ADDRESS = network::convert_to_uint(224, 0, 0, 251);

} }  // namespace net::dns

#endif /* NET_PROTOCOL_DNS_H_ */
