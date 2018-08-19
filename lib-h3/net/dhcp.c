/**
 * @file dhcp.h
 *
 */
/* Copyright (C) 2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#include <stdbool.h>
#include <assert.h>

#include "dhcp_internal.h"

#include "net/net.h"

#include "net_packets.h"
#include "net_debug.h"

#include "util.h"

#include "h3.h"

typedef union pcast32 {
		uint32_t u32;
		uint8_t u8[4];
} _pcast32;

// https://tools.ietf.org/html/rfc1541

struct t_dhcp_message {
	uint8_t  op;
	uint8_t  htype;
	uint8_t  hlen;
	uint8_t  hops;
	uint32_t xid;
	uint16_t secs;
	uint16_t flags;
	uint8_t  ciaddr[IPv4_ADDR_LEN];
	uint8_t  yiaddr[IPv4_ADDR_LEN];
	uint8_t  siaddr[IPv4_ADDR_LEN];
	uint8_t  giaddr[IPv4_ADDR_LEN];
	uint8_t  chaddr[16];
	uint8_t  sname[64];
	uint8_t  file[128];
	uint8_t  options[DHCP_OPT_SIZE];
} PACKED;

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

static struct t_dhcp_message s_dhcp_message ALIGNED;

static uint8_t s_dhcp_server_ip[IPv4_ADDR_LEN] ALIGNED = { 0, };
static uint8_t s_dhcp_allocated_ip[IPv4_ADDR_LEN] ALIGNED = { 0, };
static uint8_t s_dhcp_allocated_gw[IPv4_ADDR_LEN] ALIGNED = { 0, };
static uint8_t s_dhcp_allocated_netmask[IPv4_ADDR_LEN] ALIGNED = { 0, };

static void _message_init(const uint8_t *mac_address) {
	uint16_t i;

	uint8_t *p = (uint8_t *) &s_dhcp_message;

	for (i = 0; i < sizeof(struct t_dhcp_message); i++) {
		*p++ = 0;
	}

	s_dhcp_message.op = DHCP_OP_BOOTREQUEST;
	s_dhcp_message.htype = DHCP_HTYPE_10MB;	// This is the current default
	s_dhcp_message.hlen = ETH_ADDR_LEN;
	memcpy(s_dhcp_message.chaddr, mac_address, ETH_ADDR_LEN);

	s_dhcp_message.options[0] = (uint8_t) ((MAGIC_COOKIE & 0xFF000000) >> 24);
	s_dhcp_message.options[1] = (uint8_t) ((MAGIC_COOKIE & 0x00FF0000) >> 16);
	s_dhcp_message.options[2] = (uint8_t) ((MAGIC_COOKIE & 0x0000FF00) >> 8);
	s_dhcp_message.options[3] = (uint8_t) (MAGIC_COOKIE & 0x000000FF) >> 0;

	s_dhcp_message.options[4] = OPTIONS_MESSAGE_TYPE;
	s_dhcp_message.options[5] = 0x01;
	//s_dhcp_message.options[6] = DCHP_TYPE_DISCOVER;
}

static void _send_discover(int idx, const uint8_t *mac_address) {
	DEBUG_ENTRY

	uint16_t k = 6;

	s_dhcp_message.options[k++] = DCHP_TYPE_DISCOVER;

	s_dhcp_message.options[k++] = OPTIONS_CLIENT_IDENTIFIER;
	s_dhcp_message.options[k++] = 0x07;
	s_dhcp_message.options[k++] = 0x01;
	s_dhcp_message.options[k++] = mac_address[0];
	s_dhcp_message.options[k++] = mac_address[1];
	s_dhcp_message.options[k++] = mac_address[2];
	s_dhcp_message.options[k++] = mac_address[3];
	s_dhcp_message.options[k++] = mac_address[4];
	s_dhcp_message.options[k++] = mac_address[5];

	s_dhcp_message.options[k++] = OPTIONS_PARAM_REQUEST;
	s_dhcp_message.options[k++] = 0x06;	// length of request
	s_dhcp_message.options[k++] = OPTIONS_SUBNET_MASK;
	s_dhcp_message.options[k++] = OPTIONS_ROUTERS_ON_SUBNET;
	s_dhcp_message.options[k++] = OPTIONS_DNS;
	s_dhcp_message.options[k++] = OPTIONS_DOMAIN_NAME;
	s_dhcp_message.options[k++] = OPTIONS_DHCP_T1_VALUE;
	s_dhcp_message.options[k++] = OPTIONS_DHCP_T2_VALUE;
	s_dhcp_message.options[k++] = OPTIONS_END_OPTION;

	udp_send(idx, (uint8_t *)&s_dhcp_message, k + sizeof(struct t_dhcp_message) - DHCP_OPT_SIZE, IP_BROADCAST, DHCP_PORT_SERVER);

	DEBUG_EXIT
}

static void _send_request(int idx, const uint8_t *mac_address, const uint8_t *hostname) {
	DEBUG_ENTRY

	uint16_t i;
	uint16_t k = 6;

	s_dhcp_message.options[k++] = DCHP_TYPE_REQUEST;

	s_dhcp_message.options[k++] = OPTIONS_CLIENT_IDENTIFIER;
	s_dhcp_message.options[k++] = 0x07;
	s_dhcp_message.options[k++] = 0x01;
	s_dhcp_message.options[k++] = mac_address[0];
	s_dhcp_message.options[k++] = mac_address[1];
	s_dhcp_message.options[k++] = mac_address[2];
	s_dhcp_message.options[k++] = mac_address[3];
	s_dhcp_message.options[k++] = mac_address[4];
	s_dhcp_message.options[k++] = mac_address[5];

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
	for (i = 0; hostname[i] != 0; i++) {
		s_dhcp_message.options[k++] = hostname[i];
	}
	s_dhcp_message.options[k - (i + 1)] = i; // length of hostname

	s_dhcp_message.options[k++] = OPTIONS_PARAM_REQUEST;
	s_dhcp_message.options[k++] = 0x06;	// length of request
	s_dhcp_message.options[k++] = OPTIONS_SUBNET_MASK;
	s_dhcp_message.options[k++] = OPTIONS_ROUTERS_ON_SUBNET;
	s_dhcp_message.options[k++] = OPTIONS_DNS;
	s_dhcp_message.options[k++] = OPTIONS_DOMAIN_NAME;
	s_dhcp_message.options[k++] = OPTIONS_DHCP_T1_VALUE;
	s_dhcp_message.options[k++] = OPTIONS_DHCP_T2_VALUE;
	s_dhcp_message.options[k++] = OPTIONS_END_OPTION;

	udp_send(idx, (uint8_t *)&s_dhcp_message, k + sizeof(struct t_dhcp_message) - DHCP_OPT_SIZE, IP_BROADCAST, DHCP_PORT_SERVER);

	DEBUG_EXIT
}

static int _parse_response(int idx, const uint8_t *mac_address) {
	struct t_dhcp_message response;
	uint16_t size = 0;

	const uint32_t micros_timeout = H3_TIMER->AVS_CNT1 + (5 * 1000 * 1000); // 3 seconds

	while (H3_TIMER->AVS_CNT1 < micros_timeout) {
		net_handle();

		uint32_t from_ip;
		uint16_t from_port;

		size = udp_recv(idx, (uint8_t *)&response, sizeof(struct t_dhcp_message), &from_ip, &from_port);

		if ((size > 0) && (from_port == DHCP_PORT_SERVER)) {
			if (memcmp(response.chaddr, mac_address, ETH_ADDR_LEN) == 0) {
				break;
			}
		}
	}

	DEBUG_PRINTF("micros_timeout - H3_TIMER->AVS_CNT1=%d", micros_timeout - H3_TIMER->AVS_CNT1);

	uint8_t type = 0;
	uint8_t opt_len = 0;

	if (size > 0) {
		uint8_t *p = (uint8_t *) &response;
		p = p + sizeof(struct t_dhcp_message) - DHCP_OPT_SIZE + 4;
		uint8_t *e = (uint8_t *) &response + size;

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
            s_dhcp_allocated_ip[0] = response.yiaddr[0];
            s_dhcp_allocated_ip[1] = response.yiaddr[1];
            s_dhcp_allocated_ip[2] = response.yiaddr[2];
            s_dhcp_allocated_ip[3] = response.yiaddr[3];
		}

		return type;
	}

	return -1;
}

int dhcp_client(const uint8_t *mac_address, struct ip_info  *p_ip_info, const uint8_t *hostname) {
	DEBUG_ENTRY

	bool have_ip = false;
	int8_t retries = 3;

	_message_init(mac_address);

	int idx = udp_bind(DHCP_PORT_CLIENT);

	if (idx < 0) {
		return -1;
	}

	int type;

	while (!have_ip && (retries-- > 0)) {
		DEBUG_PRINTF("retries=%d", retries);

		_send_discover(idx, mac_address);

		if ((type =_parse_response(idx, mac_address)) < 0) {
			continue;
		}

		DEBUG_PRINTF("type=%d", type);

		if (type != DCHP_TYPE_OFFER) {
			continue;
		}

		DEBUG_PRINTF(IPSTR, s_dhcp_server_ip[0],s_dhcp_server_ip[1],s_dhcp_server_ip[2],s_dhcp_server_ip[3]);

		_send_request(idx, mac_address, hostname);

		if ((type =_parse_response(idx, mac_address)) < 0) {
			continue;
		}

		DEBUG_PRINTF("type=%d", type);

		if (type == DCHP_TYPE_ACK) {
			have_ip = true;
			break;
		}
	}

	(void) udp_unbind(DHCP_PORT_CLIENT);

	if (have_ip) {
		_pcast32 ip;

		memcpy(ip.u8, s_dhcp_allocated_ip, IPv4_ADDR_LEN);
		p_ip_info->ip.addr = ip.u32;

		memcpy(ip.u8, s_dhcp_allocated_gw, IPv4_ADDR_LEN);
		p_ip_info->gw.addr = ip.u32;

		memcpy(ip.u8, s_dhcp_allocated_netmask, IPv4_ADDR_LEN);
		p_ip_info->netmask.addr = ip.u32;
	}

	DEBUG_EXIT
	return have_ip ? 0 : -2;
}



