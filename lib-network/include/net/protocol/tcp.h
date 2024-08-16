/**
 * @file tcp.h
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

#ifndef NET_PROTOCOL_TCP_H_
#define NET_PROTOCOL_TCP_H_

#include <cstdint>

#include "net/protocol/ethernet.h"
#include "net/protocol/ip4.h"

#if !defined (PACKED)
# define PACKED __attribute__((packed))
#endif

struct t_tcp_packet {
	uint16_t srcpt;					/*  2 */
	uint16_t dstpt;					/*  4 */
	uint32_t seqnum;				/*  8 */
	uint32_t acknum;				/* 12 */
	uint8_t offset;					/* 13 */
	uint8_t control;				/* 14 */
	uint16_t window;				/* 16 */
	uint16_t checksum;				/* 18 */
	uint16_t urgent;				/* 20 */
#define TCP_HEADER_SIZE		20
#define TCP_OPTIONS_SIZE	40	/* Assuming maximum TCP options size is 40 bytes */
#define TCP_DATA_SIZE		(MTU_SIZE - TCP_HEADER_SIZE - sizeof(struct ip4_header) - TCP_OPTIONS_SIZE)
	uint8_t data[MTU_SIZE - TCP_HEADER_SIZE - sizeof(struct ip4_header)];
} PACKED;

struct t_tcp {
	struct ether_header ether;
	struct ip4_header ip4;
	struct t_tcp_packet tcp;
} PACKED;

#define IPv4_UDP_HEADERS_SIZE 			(sizeof(struct ip4_header) + UDP_HEADER_SIZE)			/* IP | UDP */

#endif /* NET_PROTOCOL_TCP_H_ */
