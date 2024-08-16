/**
 * @file igmp.h
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

#ifndef NET_PROTOCOL_IGMP_H_
#define NET_PROTOCOL_IGMP_H_

#include <cstdint>

#include "net/protocol/ethernet.h"
#include "net/protocol/ip4.h"

#if !defined (PACKED)
# define PACKED __attribute__((packed))
#endif

enum IGMP_TYPE {
	IGMP_TYPE_QUERY = 0x11,
	IGMP_TYPE_REPORT = 0x16,
	IGMP_TYPE_LEAVE = 0x17
};

struct t_igmp_packet {
	uint8_t type;
	uint8_t max_resp_time;
	uint16_t checksum;
	uint8_t group_address[IPv4_ADDR_LEN];
} PACKED;

struct t_igmp {
	struct ether_header ether;
	struct ip4_header ip4;
	union {
		struct {
			uint32_t ip4_options;
			struct t_igmp_packet igmp;
		} report;
		struct t_igmp_packet igmp;
	} igmp;
} PACKED;

#define IPv4_IGMP_REPORT_HEADERS_SIZE 	(sizeof(struct t_igmp) - sizeof(struct ether_header))
#define IGMP_REPORT_PACKET_SIZE			(sizeof(struct t_igmp))

#endif /* NET_PROTOCOL_IGMP_H_ */
