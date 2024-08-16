/**
 * @file arp.h
 *
 */
/* Copyright (C) 2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef NET_PROTOCOL_ARP_H_
#define NET_PROTOCOL_ARP_H_

#include "ip4_address.h"
#include "net/protocol/ethernet.h"
#include "net/protocol/ieee.h"

#if !defined (PACKED)
# define PACKED __attribute__((packed))
#endif

enum ARP_HARDWARE_TYPE {
	ARP_HWTYPE_ETHERNET = 1
};

enum ARP_PROTOCOL_TYPE {
	ARP_PRTYPE_IPv4 = ETHER_TYPE_IPv4
};

enum ARP_HARDWARE {
	ARP_HARDWARE_SIZE = ETH_ADDR_LEN
};

enum ARP_PROTOCOL {
	ARP_PROTOCOL_SIZE = IPv4_ADDR_LEN
};

enum ARP_OPCODE {
	ARP_OPCODE_RQST = 1,
	ARP_OPCODE_REPLY = 2
};

struct arp_packet {
	uint16_t hardware_type;			/*  2 */
	uint16_t protocol_type;			/*  4 */
	uint8_t hardware_size;			/*  5 */
	uint8_t protocol_size;			/*  6 */
	uint16_t opcode;				/*  8 */
	uint8_t sender_mac[ETH_ADDR_LEN];/*14 */
	uint8_t sender_ip[IPv4_ADDR_LEN];/* 18 */
	uint8_t target_mac[ETH_ADDR_LEN];/*24 */
	uint8_t target_ip[IPv4_ADDR_LEN];/* 28 */
	uint8_t padding[18];			/* 46 */ /* +14 = 60 */
} PACKED;

struct t_arp {
	struct ether_header ether;
	struct arp_packet arp;
} PACKED;

#endif /* NET_PROTOCOL_ARP_H_ */
