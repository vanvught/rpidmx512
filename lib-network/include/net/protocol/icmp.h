/**
 * @file icmp.h
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

#ifndef NET_PROTOCOL_ICMP_H_
#define NET_PROTOCOL_ICMP_H_

#include <cstdint>

#include "net/protocol/ethernet.h"
#include "net/protocol/ip4.h"

#if !defined (PACKED)
# define PACKED __attribute__((packed))
#endif

enum ICMP_TYPE {
	ICMP_TYPE_ECHO_REPLY = 0,
	ICMP_TYPE_ECHO = 8
};

enum ICMP_CODE {
	ICMP_CODE_ECHO = 0
};

struct t_icmp_packet {
	uint8_t type;					/* 1 */
	uint8_t code;					/* 2 */
	uint16_t checksum;				/* 4 */
	uint8_t parameter[4];			/* 8 */
#define ICMP_HEADER_SIZE	8
#define ICMP_PAYLOAD_SIZE	(MTU_SIZE - ICMP_HEADER_SIZE - sizeof(struct ip4_header))
	uint8_t payload[ICMP_PAYLOAD_SIZE];
} PACKED;

struct t_icmp {
	struct ether_header ether;
	struct ip4_header ip4;
	struct t_icmp_packet icmp;
} PACKED;

#define IPv4_ICMP_HEADERS_SIZE 			(sizeof(struct t_icmp) - sizeof(struct ether_header))

#endif /* NET_PROTOCOL_ICMP_H_ */
