/**
 * @file udp.h
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

#ifndef NET_PROTOCOL_UDP_H_
#define NET_PROTOCOL_UDP_H_

#include <cstdint>

#include "net/protocol/ethernet.h"
#include "net/protocol/ip4.h"

#if !defined (PACKED)
# define PACKED __attribute__((packed))
#endif

struct t_udp_packet {
	uint16_t source_port;			/* 2 */
	uint16_t destination_port;		/* 4 */
	uint16_t len;					/* 6 */
	uint16_t checksum;				/* 8 */
#define UDP_HEADER_SIZE		8
#define UDP_DATA_SIZE		(MTU_SIZE - UDP_HEADER_SIZE - sizeof(struct ip4_header))
	uint8_t data[UDP_DATA_SIZE];
}PACKED;

struct t_udp {
	struct ether_header ether;
	struct ip4_header ip4;
	struct t_udp_packet udp;
} PACKED;

#define UDP_PACKET_HEADERS_SIZE			(sizeof(struct ether_header) + IPv4_UDP_HEADERS_SIZE)	/* ETH | IP | UDP */

#endif /* NET_PROTOCOL_UDP_H_ */
