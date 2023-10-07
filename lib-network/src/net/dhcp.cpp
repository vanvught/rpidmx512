/**
 * @file dhcp.cpp
 *
 */
/* Copyright (C) 2018-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <cassert>

#include "dhcp_internal.h"

#include "net.h"
#include "net_private.h"

#include "hardware.h"

#include "../../config/net_config.h"

namespace net {
namespace globals {
extern struct IpInfo ipInfo;
extern uint8_t macAddress[ETH_ADDR_LEN];
}  // namespace globals
}  // namespace net

typedef union pcast32 {
	uint32_t u32;
	uint8_t u8[4];
} _pcast32;

// https://tools.ietf.org/html/rfc1541

namespace dhcp {
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
	uint8_t options[DHCP_OPT_SIZE];
}PACKED;
}  // namespace dhcp

enum OPTIONS {
	OPTIONS_PAD_OPTION = 0,
	OPTIONS_SUBNET_MASK = 1,
	OPTIONS_ROUTERS_ON_SUBNET = 3,
	OPTIONS_DNS = 6,
	OPTIONS_HOSTNAME = 12,
	OPTIONS_DOMAIN_NAME = 15,
	OPTIONS_REQUESTED_IP = 50,
	OPTIONS_MESSAGE_TYPE = 53,
	OPTIONS_SERVER_IDENTIFIER = 54,
	OPTIONS_PARAM_REQUEST = 55,
	OPTIONS_DHCP_T1_VALUE = 58,
	OPTIONS_DHCP_T2_VALUE = 59,
	OPTIONS_CLIENT_IDENTIFIER = 61,
	OPTIONS_END_OPTION = 255
};

static dhcp::Message s_dhcp_message ALIGNED;

static uint8_t s_dhcp_server_ip[IPv4_ADDR_LEN] ALIGNED = { 0, };
static uint8_t s_dhcp_allocated_ip[IPv4_ADDR_LEN] ALIGNED = { 0, };
static uint8_t s_dhcp_allocated_gw[IPv4_ADDR_LEN] ALIGNED = { 0, };
static uint8_t s_dhcp_allocated_netmask[IPv4_ADDR_LEN] ALIGNED = { 0, };

static void message_init(const uint8_t *pMacAddress) {
	auto *p = reinterpret_cast<uint8_t *>(&s_dhcp_message);

	for (uint32_t i = 0; i < sizeof(dhcp::Message); i++) {
		*p++ = 0;
	}

	s_dhcp_message.op = DHCP_OP_BOOTREQUEST;
	s_dhcp_message.htype = DHCP_HTYPE_10MB;	// This is the current default
	s_dhcp_message.hlen = ETH_ADDR_LEN;
	memcpy(s_dhcp_message.chaddr, pMacAddress, ETH_ADDR_LEN);

	s_dhcp_message.options[0] = static_cast<uint8_t>((MAGIC_COOKIE & 0xFF000000) >> 24);
	s_dhcp_message.options[1] = static_cast<uint8_t>((MAGIC_COOKIE & 0x00FF0000) >> 16);
	s_dhcp_message.options[2] = static_cast<uint8_t>((MAGIC_COOKIE & 0x0000FF00) >> 8);
	s_dhcp_message.options[3] = static_cast<uint8_t>(MAGIC_COOKIE & 0x000000FF) >> 0;

	s_dhcp_message.options[4] = OPTIONS_MESSAGE_TYPE;
	s_dhcp_message.options[5] = 0x01;
}

static void send_discover(int nHandle, const uint8_t *pMacAddress) {
	DEBUG_ENTRY

	uint32_t k = 6;

	s_dhcp_message.options[k++] = DCHP_TYPE_DISCOVER;

	s_dhcp_message.options[k++] = OPTIONS_CLIENT_IDENTIFIER;
	s_dhcp_message.options[k++] = 0x07;
	s_dhcp_message.options[k++] = 0x01;
	s_dhcp_message.options[k++] = pMacAddress[0];
	s_dhcp_message.options[k++] = pMacAddress[1];
	s_dhcp_message.options[k++] = pMacAddress[2];
	s_dhcp_message.options[k++] = pMacAddress[3];
	s_dhcp_message.options[k++] = pMacAddress[4];
	s_dhcp_message.options[k++] = pMacAddress[5];

	s_dhcp_message.options[k++] = OPTIONS_PARAM_REQUEST;
	s_dhcp_message.options[k++] = 0x06;	// length of request
	s_dhcp_message.options[k++] = OPTIONS_SUBNET_MASK;
	s_dhcp_message.options[k++] = OPTIONS_ROUTERS_ON_SUBNET;
	s_dhcp_message.options[k++] = OPTIONS_DNS;
	s_dhcp_message.options[k++] = OPTIONS_DOMAIN_NAME;
	s_dhcp_message.options[k++] = OPTIONS_DHCP_T1_VALUE;
	s_dhcp_message.options[k++] = OPTIONS_DHCP_T2_VALUE;
	s_dhcp_message.options[k++] = OPTIONS_END_OPTION;

	udp_send(nHandle, reinterpret_cast<uint8_t *>(&s_dhcp_message), static_cast<uint16_t>(k + sizeof(dhcp::Message) - DHCP_OPT_SIZE), IP_BROADCAST, DHCP_PORT_SERVER);

	DEBUG_EXIT
}

static void _send_request(int idx, const uint8_t *pMacAddress, const char *pHostname) {
	DEBUG_ENTRY

	uint32_t i;
	uint32_t k = 6;

	s_dhcp_message.options[k++] = DCHP_TYPE_REQUEST;

	s_dhcp_message.options[k++] = OPTIONS_CLIENT_IDENTIFIER;
	s_dhcp_message.options[k++] = 0x07;
	s_dhcp_message.options[k++] = 0x01;
	s_dhcp_message.options[k++] = pMacAddress[0];
	s_dhcp_message.options[k++] = pMacAddress[1];
	s_dhcp_message.options[k++] = pMacAddress[2];
	s_dhcp_message.options[k++] = pMacAddress[3];
	s_dhcp_message.options[k++] = pMacAddress[4];
	s_dhcp_message.options[k++] = pMacAddress[5];

	s_dhcp_message.options[k++] = OPTIONS_REQUESTED_IP;
	s_dhcp_message.options[k++] = 0x04;
	s_dhcp_message.options[k++] = s_dhcp_allocated_ip[0];
	s_dhcp_message.options[k++] = s_dhcp_allocated_ip[1];
	s_dhcp_message.options[k++] = s_dhcp_allocated_ip[2];
	s_dhcp_message.options[k++] = s_dhcp_allocated_ip[3];

	s_dhcp_message.options[k++] = OPTIONS_SERVER_IDENTIFIER;
	s_dhcp_message.options[k++] = 0x04;
	s_dhcp_message.options[k++] = s_dhcp_server_ip[0];
	s_dhcp_message.options[k++] = s_dhcp_server_ip[1];
	s_dhcp_message.options[k++] = s_dhcp_server_ip[2];
	s_dhcp_message.options[k++] = s_dhcp_server_ip[3];

	s_dhcp_message.options[k++] = OPTIONS_HOSTNAME;
	s_dhcp_message.options[k++] = 0; // length of hostname
	for (i = 0; pHostname[i] != 0; i++) {
		s_dhcp_message.options[k++] = pHostname[i];
	}
	s_dhcp_message.options[k - (i + 1)] = static_cast<uint8_t>(i); // length of hostname

	s_dhcp_message.options[k++] = OPTIONS_PARAM_REQUEST;
	s_dhcp_message.options[k++] = 0x06;	// length of request
	s_dhcp_message.options[k++] = OPTIONS_SUBNET_MASK;
	s_dhcp_message.options[k++] = OPTIONS_ROUTERS_ON_SUBNET;
	s_dhcp_message.options[k++] = OPTIONS_DNS;
	s_dhcp_message.options[k++] = OPTIONS_DOMAIN_NAME;
	s_dhcp_message.options[k++] = OPTIONS_DHCP_T1_VALUE;
	s_dhcp_message.options[k++] = OPTIONS_DHCP_T2_VALUE;
	s_dhcp_message.options[k++] = OPTIONS_END_OPTION;

	udp_send(idx, reinterpret_cast<uint8_t *>(&s_dhcp_message), static_cast<uint16_t>(k + sizeof(dhcp::Message) - DHCP_OPT_SIZE), IP_BROADCAST, DHCP_PORT_SERVER);

	DEBUG_EXIT
}

static int parse_response(int nHandle, const uint8_t *pMacAddress) {
	uint8_t *pResponse;
	const auto nMillis = Hardware::Get()->Millis();
	uint32_t nSize = 0;

	do {
		net_handle();

		uint32_t nFromIp;
		uint16_t nFromPort;

		nSize = udp_recv2(nHandle, const_cast<const uint8_t **>(&pResponse), &nFromIp, &nFromPort);

		if ((nSize > 0) && (nFromPort == DHCP_PORT_SERVER)) {

			const auto *const pDhcpMessage = reinterpret_cast<dhcp::Message *>(pResponse);

			if (memcmp(pDhcpMessage->chaddr, pMacAddress, ETH_ADDR_LEN) == 0) {
				break;
			}
		}
	} while ((Hardware::Get()->Millis() - nMillis) < 500);

	DEBUG_PRINTF("timeout %u", Hardware::Get()->Millis() - nMillis);

	int type = 0;
	uint8_t opt_len = 0;

	if (nSize > 0) {
		auto *p = pResponse;
		p = p + sizeof(dhcp::Message) - DHCP_OPT_SIZE + 4;
		auto *e = pResponse + nSize;

		while (p < e) {
			switch (*p) {
			case OPTIONS_END_OPTION:
				p = e;
				break;
			case OPTIONS_PAD_OPTION:
				p++;
				break;
			case OPTIONS_MESSAGE_TYPE:
				p++;
				p++;
				type = *p++;
				break;
   			case OPTIONS_SUBNET_MASK:
   				p++;
   				p++;
   				s_dhcp_allocated_netmask[0] = *p++;
   				s_dhcp_allocated_netmask[1] = *p++;
   				s_dhcp_allocated_netmask[2] = *p++;
   				s_dhcp_allocated_netmask[3] = *p++;
   				break;
   			case OPTIONS_ROUTERS_ON_SUBNET:
   				p++;
   				opt_len = *p++;
   				s_dhcp_allocated_gw[0] = *p++;
   				s_dhcp_allocated_gw[1] = *p++;
   				s_dhcp_allocated_gw[2] = *p++;
   				s_dhcp_allocated_gw[3] = *p++;
   				p = p + (opt_len - 4);
   				break;
   			case OPTIONS_SERVER_IDENTIFIER :
   				p++;
   				opt_len = *p++;
   				s_dhcp_server_ip[0] = *p++;
   				s_dhcp_server_ip[1] = *p++;
   				s_dhcp_server_ip[2] = *p++;
   				s_dhcp_server_ip[3] = *p++;
   				break;
			default:
				p++;
				opt_len = *p++;
				p += opt_len;
				break;
			}
		}

		if (type == DCHP_TYPE_OFFER) {
			const auto *const pDhcpMessage = reinterpret_cast<dhcp::Message *>(pResponse);
            s_dhcp_allocated_ip[0] = pDhcpMessage->yiaddr[0];
            s_dhcp_allocated_ip[1] = pDhcpMessage->yiaddr[1];
            s_dhcp_allocated_ip[2] = pDhcpMessage->yiaddr[2];
            s_dhcp_allocated_ip[3] = pDhcpMessage->yiaddr[3];
		}

		return type;
	}

	return -1;
}

int dhcp_client(const char *pHostname) {
	DEBUG_ENTRY

	auto bHaveIp = false;
	int32_t retries = 20;

	message_init(net::globals::macAddress);

	auto nHandle = udp_begin(DHCP_PORT_CLIENT);

	if (nHandle < 0) {
		return -1;
	}

	net::globals::ipInfo.ip.addr = 0;
	ip_set_ip();

	while (!bHaveIp && (retries-- > 0)) {
		DEBUG_PRINTF("retries=%d", retries);

		send_discover(static_cast<uint8_t>(nHandle), net::globals::macAddress);

		int type;

		if ((type = parse_response(static_cast<uint8_t>(nHandle), net::globals::macAddress)) < 0) {
			continue;
		}

		DEBUG_PRINTF("type=%d", type);

		if (type != DCHP_TYPE_OFFER) {
			continue;
		}

		DEBUG_PRINTF(IPSTR, s_dhcp_server_ip[0],s_dhcp_server_ip[1],s_dhcp_server_ip[2],s_dhcp_server_ip[3]);

		_send_request(static_cast<uint8_t>(nHandle), net::globals::macAddress, pHostname);

		if ((type =parse_response(static_cast<uint8_t>(nHandle), net::globals::macAddress)) < 0) {
			continue;
		}

		DEBUG_PRINTF("type=%d", type);

		if (type == DCHP_TYPE_ACK) {
			bHaveIp = true;
			break;
		}
	}

	udp_end(DHCP_PORT_CLIENT);

	if (bHaveIp) {
		_pcast32 ip;

		memcpy(ip.u8, s_dhcp_allocated_ip, IPv4_ADDR_LEN);
		net::globals::ipInfo.ip.addr = ip.u32;

		memcpy(ip.u8, s_dhcp_allocated_gw, IPv4_ADDR_LEN);
		net::globals::ipInfo.gw.addr = ip.u32;

		memcpy(ip.u8, s_dhcp_allocated_netmask, IPv4_ADDR_LEN);
		net::globals::ipInfo.netmask.addr = ip.u32;
	}

	DEBUG_EXIT
	return bHaveIp ? 0 : -2;
}

void dhcp_client_release() {
	DEBUG_ENTRY

	auto nHandle = udp_begin(DHCP_PORT_CLIENT);

	if (nHandle < 0) {
		console_error("dhcp_client_release\n");
		return;
	}

	uint32_t k = 6;

	s_dhcp_message.options[k++] = DCHP_TYPE_RELEASE;

	s_dhcp_message.options[k++] = OPTIONS_SERVER_IDENTIFIER;
	s_dhcp_message.options[k++] = 0x04;
	s_dhcp_message.options[k++] = s_dhcp_server_ip[0];
	s_dhcp_message.options[k++] = s_dhcp_server_ip[1];
	s_dhcp_message.options[k++] = s_dhcp_server_ip[2];
	s_dhcp_message.options[k++] = s_dhcp_server_ip[3];

	s_dhcp_message.options[k++] = OPTIONS_END_OPTION;

	udp_send(static_cast<uint8_t>(nHandle), reinterpret_cast<uint8_t *>(&s_dhcp_message), static_cast<uint16_t>(k + sizeof(dhcp::Message) - DHCP_OPT_SIZE), IP_BROADCAST, DHCP_PORT_SERVER);
	udp_end(DHCP_PORT_CLIENT);

	DEBUG_EXIT
}
