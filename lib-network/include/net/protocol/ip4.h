/**
 * @file ip4.h
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
#ifndef NET_PROTOCOL_IP4_H_
#define NET_PROTOCOL_IP4_H_

#include <cstdint>

#include "net/protocol/ethernet.h"

#if !defined (PACKED)
# define PACKED __attribute__((packed))
#endif

enum IPv4_ADDR {
	IPv4_ADDR_LEN = 4
};

enum IPv4_PROTO {
	IPv4_PROTO_ICMP = 1,
	IPv4_PROTO_IGMP = 2,
	IPv4_PROTO_TCP = 6,
	IPv4_PROTO_UDP = 17
};

enum IPv4_FLAG {
	IPv4_FLAG_LF = 0x0000,
	IPv4_FLAG_MF = 0x2000,
	IPv4_FLAG_DF = 0x4000
};

struct ip4_header {
	uint8_t ver_ihl;				/*  1 */
	uint8_t tos;					/*  2 */
	uint16_t len;					/*  4 */
	uint16_t id;					/*  6 */
	uint16_t flags_froff;			/*  8 */
	uint8_t ttl;					/*  9 */
	uint8_t proto;					/* 10 */
	uint16_t chksum;				/* 12 */
	uint8_t src[IPv4_ADDR_LEN];		/* 16 */
	uint8_t dst[IPv4_ADDR_LEN];		/* 20 */
} PACKED;

struct t_ip4 {
	struct ether_header ether;
	struct ip4_header ip4;
} PACKED;

#endif /* NET_PROTOCOL_IP4_H_ */
