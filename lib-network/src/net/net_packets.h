/**
 * @file net_packets.h
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

#ifndef NET_PACKETS_H_
#define NET_PACKETS_H_

#include <cstdint>

#if !defined (PACKED)
# define PACKED __attribute__((packed))
#endif

enum MTU {
	MTU_SIZE = 1500
};

enum ETHER_TYPE {
	ETHER_TYPE_IPv4 = 0x0800,
	ETHER_TYPE_ARP = 0x0806
};

enum ETH_ADDR {
	ETH_ADDR_LEN = 6
};

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

#define IPv4_BROADCAST	0xffffffff

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

enum IGMP_TYPE {
	IGMP_TYPE_QUERY = 0x11,
	IGMP_TYPE_REPORT = 0x16,
	IGMP_TYPE_LEAVE = 0x17
};

enum ICMP_TYPE {
	ICMP_TYPE_ECHO_REPLY = 0,
	ICMP_TYPE_ECHO = 8
};

enum ICMP_CODE {
	ICMP_CODE_ECHO = 0
};

struct ether_header {
	uint8_t dst[ETH_ADDR_LEN];		/*  6 */
	uint8_t src[ETH_ADDR_LEN];		/* 12 */
	uint16_t type;					/* 14 */
} PACKED;

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

struct t_igmp_packet {
	uint8_t type;
	uint8_t max_resp_time;
	uint16_t checksum;
	uint8_t group_address[IPv4_ADDR_LEN];
} PACKED;

struct t_icmp_packet {
	uint8_t type;					/* 1 */
	uint8_t code;					/* 2 */
	uint16_t checksum;				/* 4 */
	uint8_t parameter[4];			/* 8 */
#define ICMP_HEADER_SIZE	8
#define ICMP_PAYLOAD_SIZE	(MTU_SIZE - ICMP_HEADER_SIZE - sizeof(struct ip4_header))
	uint8_t payload[ICMP_PAYLOAD_SIZE];
} PACKED;

struct t_udp_packet {
	uint16_t source_port;			/* 2 */
	uint16_t destination_port;		/* 4 */
	uint16_t len;					/* 6 */
	uint16_t checksum;				/* 8 */
#define UDP_HEADER_SIZE		8
#define UDP_DATA_SIZE		(MTU_SIZE - UDP_HEADER_SIZE - sizeof(struct ip4_header))
	uint8_t data[UDP_DATA_SIZE];
}PACKED;

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
#define TCP_DATA_SIZE		(MTU_SIZE - TCP_HEADER_SIZE - sizeof(struct ip4_header) - 20U)
	uint8_t data[TCP_DATA_SIZE];
} PACKED;

struct t_arp {
	struct ether_header ether;
	struct arp_packet arp;
} PACKED;

struct t_ip4 {
	struct ether_header ether;
	struct ip4_header ip4;
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

struct t_icmp {
	struct ether_header ether;
	struct ip4_header ip4;
	struct t_icmp_packet icmp;
} PACKED;

struct t_udp {
	struct ether_header ether;
	struct ip4_header ip4;
	struct t_udp_packet udp;
} PACKED;

struct t_tcp {
	struct ether_header ether;
	struct ip4_header ip4;
	struct t_tcp_packet tcp;
} PACKED;

#define IPv4_UDP_HEADERS_SIZE 			(sizeof(struct ip4_header) + UDP_HEADER_SIZE)			/* IP | UDP */
#define UDP_PACKET_HEADERS_SIZE			(sizeof(struct ether_header) + IPv4_UDP_HEADERS_SIZE)	/* ETH | IP | UDP */

#define IPv4_IGMP_REPORT_HEADERS_SIZE 	(sizeof(struct t_igmp) - sizeof(struct ether_header))
#define IGMP_REPORT_PACKET_SIZE			(sizeof(struct t_igmp))

#define IPv4_ICMP_HEADERS_SIZE 			(sizeof(struct t_icmp) - sizeof(struct ether_header))

#endif /* NET_PACKETS_H_ */
