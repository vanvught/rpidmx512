/**
 * @file dhcp.h
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

#ifndef NET_PROTOCOL_DHCP_H
#define NET_PROTOCOL_DHCP_H

#include <cstdint>

#include "ip4.h"

namespace net::dhcp {
static constexpr uint32_t OPT_SIZE = 312;
static constexpr uint32_t MAGIC_COOKIE = 0x63825363; ///< You should not modify it number.

struct OpCode {
	static constexpr uint8_t BOOTREQUEST = 1;
	static constexpr uint8_t BOOTREPLY = 2;
};

struct HardwareType {
	static constexpr uint8_t HTYPE_10MB = 1;
	static constexpr uint8_t HTYPE_100MB = 2;
};

struct Type {
	static constexpr uint8_t DISCOVER = 1;
	static constexpr uint8_t OFFER = 2;
	static constexpr uint8_t REQUEST = 3;
	static constexpr uint8_t DECLINE = 4;
	static constexpr uint8_t ACK = 5;
	static constexpr uint8_t NAK = 6;
	static constexpr uint8_t RELEASE = 7;
	static constexpr uint8_t INFORM = 8;
};

struct Options {
	/* BootP options */
	static constexpr uint8_t OPTION_PAD_OPTION = 0;
	static constexpr uint8_t OPTION_SUBNET_MASK = 1;		///< RFC 2132 3.3
	static constexpr uint8_t OPTION_ROUTER = 3;
	static constexpr uint8_t OPTION_DNS_SERVER = 6;
	static constexpr uint8_t OPTION_HOSTNAME = 12;
	static constexpr uint8_t OPTION_DOMAIN_NAME = 15;
	static constexpr uint8_t OPTION_IP_TTL = 23;
	static constexpr uint8_t OPTION_MTU = 26;
	static constexpr uint8_t OPTION_BROADCAST = 28;
	static constexpr uint8_t OPTION_TCP_TTL = 37;
	static constexpr uint8_t OPTION_NTP = 42;
	static constexpr uint8_t OPTION_END = 255;
	/* DHCP options */
	static constexpr uint8_t OPTION_REQUESTED_IP		= 50; ///< RFC 2132 9.1, requested IP address
	static constexpr uint8_t OPTION_LEASE_TIME			= 51; ///< RFC 2132 9.2, time in seconds, in 4 bytes
	static constexpr uint8_t OPTION_OVERLOAD			= 52; ///< RFC2132 9.3, use file and/or sname field for options
	static constexpr uint8_t OPTION_MESSAGE_TYPE		= 53; ///< RFC 2132 9.6, important for DHCP
	static constexpr uint8_t OPTION_SERVER_IDENTIFIER	= 54; ///< RFC 2132 9.7, server IP address
	static constexpr uint8_t OPTION_PARAM_REQUEST 		= 55; ///< RFC 2132 9.8, requested option types
	static constexpr uint8_t OPTION_MAX_MSG_SIZE 		= 57; ///< RFC 2132 9.10, message size accepted >= 576
	static constexpr uint8_t OPTION_DHCP_T1_VALUE 		= 58; ///< T1 renewal time
	static constexpr uint8_t OPTION_DHCP_T2_VALUE 		= 59; ///< T2 renewal time
	static constexpr uint8_t OPTION_CLIENT_IDENTIFIER 	= 61;
};

enum class State: uint8_t {
	STATE_OFF			= 0,
	STATE_REQUESTING	= 1,
	STATE_INIT			= 2,
	STATE_REBOOTING		= 3,
	STATE_REBINDING		= 4,
	STATE_RENEWING		= 5,
	STATE_SELECTING		= 6,
	STATE_INFORMING		= 7,
	STATE_CHECKING		= 8,
	STATE_PERMANENT		= 9,
	STATE_BOUND			= 10,
	STATE_RELEASING		= 11,
	STATE_BACKING_OFF	= 12
};

struct Message {
	uint8_t op;
	uint8_t htype;
	uint8_t hlen;
	uint8_t hops;
	uint32_t xid;
	uint16_t secs;
	uint16_t flags;
	uint8_t ciaddr[IPv4_ADDR_LEN];
	uint8_t yiaddr[IPv4_ADDR_LEN];
	uint8_t siaddr[IPv4_ADDR_LEN];
	uint8_t giaddr[IPv4_ADDR_LEN];
	uint8_t chaddr[16];
	uint8_t sname[64];
	uint8_t file[128];
	uint8_t options[dhcp::OPT_SIZE];
} __attribute__((packed));
}  // namespace net::dhcp

#endif /* NET_PROTOCOL_DHCP_H */
