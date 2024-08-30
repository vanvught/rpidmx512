/**
 * @file tcp.cpp
 *
 */
/* Copyright (C) 2021-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

/*
 * Disclaimer: This code has implemented the server side only.
 * Server side states:
 * LISTEN -> ESTABLISHED -> CLOSE_WAIT -> LAST_ACK -> CLOSED:LISTEN
 */

#if defined (DEBUG_NET_TCP)
# undef NDEBUG
#endif

#pragma GCC diagnostic push
#if (__GNUC__ < 10)
# pragma GCC diagnostic ignored "-Wconversion"
# pragma GCC diagnostic ignored "-Wsign-conversion"
#endif
#pragma GCC push_options
#pragma GCC optimize ("O2")
#pragma GCC optimize ("no-tree-loop-distribute-patterns")

#include <cstdint>
#include <cstring>
#include <algorithm>
#include <cassert>

#include "../config/net_config.h"

#include "net.h"
#include "net/protocol/tcp.h"

#include "net_memcpy.h"
#include "net_private.h"

#include "hardware.h"

#include "debug.h"

namespace net {
#define TCP_RX_MSS						(TCP_DATA_SIZE)
#define TCP_RX_MAX_ENTRIES				(1U << 1) // Must always be a power of 2
#define TCP_RX_MAX_ENTRIES_MASK			(TCP_RX_MAX_ENTRIES - 1)
#define TCP_MAX_RX_WND 					(TCP_RX_MAX_ENTRIES * TCP_RX_MSS);
#define TCP_TX_MSS						(TCP_DATA_SIZE)

/**
 * Transmission control block (TCB)
 */
struct tcb {
	uint8_t localIp[IPv4_ADDR_LEN];
	uint8_t remoteIp[IPv4_ADDR_LEN];

	uint16_t nLocalPort;
	uint16_t nRemotePort;

	uint8_t remoteEthAddr[ETH_ADDR_LEN];

	/* Send Sequence Variables */
	struct {
		uint32_t UNA; 	/* send unacknowledged */
		uint32_t NXT;	/* send next */
		uint32_t WND;	/* send window */
		uint16_t UP;	/* send urgent pointer */
		uint32_t WL1;	/* segment sequence number used for last window update */
		uint32_t WL2;	/* segment acknowledgment number used for last window */
	} SND;

	uint32_t ISS;		/* initial send sequence number */

	struct {
		uint32_t Recent; /* holds a timestamp to be echoed in TSecr whenever a segment is sent */
	} TS;

	uint16_t SendMSS;

	struct {
		uint8_t *data;
		uint16_t size;
	} TX;

	/* Receive Sequence Variables */
	struct {
		uint32_t NXT; 	/* receive next */
		uint16_t WND; 	/* receive window */
		uint16_t UP; 	/* receive urgent pointer */
	} RCV;

	uint32_t IRS;		/* initial receive sequence number */

	uint8_t state;
};

struct SendInfo {
	uint32_t SEQ;
	uint32_t ACK;
	uint8_t CTL;
};

struct QueueEntry {
	uint8_t data[TCP_RX_MSS];
	uint16_t nSize;
	uint16_t nHandleConnection;
};

struct ReceiveQueue {
	uint16_t nHead;
	uint16_t nTail;
	QueueEntry Entries[TCP_RX_MAX_ENTRIES];
};

struct Port {
	tcb TCB[TCP_MAX_TCBS_ALLOWED];
	ReceiveQueue receiveQueue;
	uint16_t nLocalPort;
};

static struct Port s_Port[TCP_MAX_PORTS_ALLOWED] SECTION_NETWORK ALIGNED;
static uint16_t s_id SECTION_NETWORK ALIGNED;
static struct t_tcp s_tcp SECTION_NETWORK ALIGNED;

#if !defined (NDEBUG)
static const char *s_aStateName[] = {
		"CLOSED",
		"LISTEN",
		"SYN-SENT",
		"SYN-RECEIVED",
		"ESTABLISHED",
		"FIN-WAIT-1",
		"FIN-WAIT-2",
		"CLOSE-WAIT",
		"CLOSING",
		"LAST-ACK",
		"TIME-WAIT"
};

static uint8_t new_state(struct tcb *p_tcb, uint8_t state, const char *func, const char *file, unsigned line) {
	assert(p_tcb->state < sizeof s_aStateName / sizeof s_aStateName[0]);
	assert(state < sizeof s_aStateName / sizeof s_aStateName[0]);

	printf("%s() %s, line %i: %s -> %s\n", func, file, line, s_aStateName[p_tcb->state], s_aStateName[state]);

	return p_tcb->state = state;
}

static void unexpected_state(const uint32_t nState, const uint32_t nLine) {
	printf("Unexpected state %s at line %u\n", s_aStateName[nState], nLine);
}

# define NEW_STATE(tcb, STATE)	new_state(tcb, STATE, __func__, __FILE__, __LINE__)
# define UNEXPECTED_STATE()		unexpected_state (pTCB->state, __LINE__)
# define CLIENT_NOT_IMPLEMENTED	assert(0)
#else
static void NEW_STATE(struct tcb *pTcb, const uint8_t state) {
	pTcb->state = state;
}
# define UNEXPECTED_STATE()		((void)0)
# define CLIENT_NOT_IMPLEMENTED	((void)0)
#endif

/**
 * https://www.rfc-editor.org/rfc/rfc9293.html#name-header-format
 */

enum Control: uint8_t {
	URG = 0x20,	///< Urgent Pointer field significant
	ACK = 0x10,	///< Acknowledgment field significant
	PSH = 0x08,	///< Acknowledgment
	RST = 0x04,	///< Reset the connection
	SYN = 0x02,	///< Synchronize sequence numbers
	FIN = 0x01 	///< No more data from sender
};

/**
 * https://www.rfc-editor.org/rfc/rfc9293.html#name-specific-option-definitions
 * Mandatory Option Set: https://www.rfc-editor.org/rfc/rfc9293.html#table-1
 */

enum Option {
	KIND_END = 0,		///< End of option list
	KIND_NOP = 1,		///< No-Operation
	KIND_MSS = 2,		///< Maximum Segment Size
	KIND_TIMESTAMP = 8	///< RFC 7323 Timestamp value, Timestamp echo reply (2*4 byte)
};

static constexpr auto OPTION_MSS_LENGTH = 4U;
static constexpr auto OPTION_TIMESTAMP_LENGTH = 10U;

/*
 * RFC 793, Page 21
 */

enum {
	STATE_CLOSED, /* Is fictional because it represents the state when there is no TCB,
	 and therefore, no connection. */
	STATE_LISTEN, /* Represents waiting for a connection request from any
	 remote TCP and port. */
	STATE_SYN_SENT, /* Represents waiting for a matching connection request
	 after having sent a connection request. */
	STATE_SYN_RECEIVED, /* Represents waiting for a confirming connection
	 request acknowledgment after having both received and sent
	 a connection request. */
	STATE_ESTABLISHED, /* Represents an open connection, data received can be
	 delivered to the user.  The normal state for the data transfer phase
	 of the connection. */
	STATE_FIN_WAIT_1, /* Represents waiting for a connection termination request
	 from the remote TCP, or an acknowledgment of the connection
	 termination request previously sent. */
	STATE_FIN_WAIT_2,/* Represents waiting for a connection termination request
	 from the remote TCP.*/
	STATE_CLOSE_WAIT,/* Represents waiting for a connection termination request
	 from the local user. */
	STATE_CLOSING,/* Represents waiting for a connection termination request
	 acknowledgment from the remote TCP. */
	STATE_LAST_ACK, /* represents waiting for an acknowledgment of the
	 connection termination request previously sent to the remote TCP
	 (which includes an acknowledgment of its connection termination
	 request). */
	STATE_TIME_WAIT /* Represents waiting for enough time to pass to be sure
	 the remote TCP received the acknowledgment of its connection
	 termination request. */
};

#define offset2octets(x)  (((x) >> 4) * 4)

static constexpr bool SEQ_LT(const uint32_t x, const uint32_t y) {
	return static_cast<int32_t>(x - y) < 0;
}

static constexpr bool SEQ_LEQ(const uint32_t x, const uint32_t y) {
	return static_cast<int32_t>(x - y) <= 0;
}

static constexpr bool SEQ_GT(const uint32_t x, const uint32_t y) {
	return static_cast<int32_t>(x - y) > 0;
}

//static constexpr bool SEQ_GEQ(const uint32_t x, const uint32_t y) {
//	return static_cast<int32_t>(x - y) >= 0;
//}
//
//static constexpr bool SEQ_BETWEEN(const uint32_t l, const uint32_t x, const uint32_t h) {
//	return SEQ_LT(l, x) && SEQ_LT(x, h);
//}

static constexpr bool  SEQ_BETWEEN_L(const uint32_t l, const uint32_t x, const uint32_t h)	{
	return SEQ_LEQ(l, x) && SEQ_LT(x, h);	// low border inclusive
}

static constexpr bool  SEQ_BETWEEN_H(const uint32_t l, const uint32_t x, const uint32_t h) {
	return SEQ_LT(l, x) && SEQ_LEQ(x, h);	// high border inclusive
}

static constexpr bool SEQ_BETWEEN_LH(const uint32_t l, const uint32_t x, const uint32_t h)	{
	return SEQ_LEQ(l, x) && SEQ_LEQ(x, h);	// both borders inclusive
}

typedef union pcast32 {
	uint32_t u32;
	uint8_t u8[4];
} _pcast32;

static uint32_t _get_seqnum(struct t_tcp *const p_tcp) {
	_pcast32 src;
	memcpy(src.u8, &p_tcp->tcp.seqnum, 4);
	return src.u32;
}

static uint32_t _get_acknum(struct t_tcp *const p_tcp) {
	_pcast32 src;
	memcpy(src.u8, &p_tcp->tcp.acknum, 4);
	return src.u32;
}

static void _bswap32(struct t_tcp *p_tcp) {
	_pcast32 src;

	memcpy(src.u8, &p_tcp->tcp.acknum, 4);
	src.u32 = __builtin_bswap32(src.u32);
	memcpy(&p_tcp->tcp.acknum, src.u8, 4);

	memcpy(src.u8, &p_tcp->tcp.seqnum, 4);
	src.u32 = __builtin_bswap32(src.u32);
	memcpy(&p_tcp->tcp.seqnum, src.u8, 4);
}

static void _init_tcb(struct tcb *pTcb, const uint16_t nLocalPort) {
	memset(pTcb, 0, sizeof(struct tcb));

	pTcb->nLocalPort = nLocalPort;

	pTcb->ISS = Hardware::Get()->Millis();

	pTcb->RCV.WND = TCP_MAX_RX_WND;

	pTcb->SND.UNA = pTcb->ISS;
	pTcb->SND.NXT = pTcb->ISS;
	pTcb->SND.WL2 = pTcb->ISS;

	NEW_STATE(pTcb, STATE_LISTEN);
}

__attribute__((cold)) void tcp_init() {
	DEBUG_ENTRY

	/* Ethernet */
	std::memcpy(s_tcp.ether.src, net::globals::netif_default.hwaddr, ETH_ADDR_LEN);
	s_tcp.ether.type = __builtin_bswap16(ETHER_TYPE_IPv4);
	/* IPv4 */
	s_tcp.ip4.ver_ihl = 0x45;
	s_tcp.ip4.tos = 0;
	s_tcp.ip4.flags_froff = __builtin_bswap16(IPv4_FLAG_DF);
	s_tcp.ip4.ttl = 64;
	s_tcp.ip4.proto = IPv4_PROTO_TCP;

	DEBUG_EXIT
}

void tcp_shutdown() {
	DEBUG_ENTRY

	DEBUG_EXIT
}

/* TCP Checksum Pseudo Header */
struct tcpPseudo {
	uint8_t srcIp[IPv4_ADDR_LEN];
	uint8_t dstIp[IPv4_ADDR_LEN];
	uint8_t zero;
	uint8_t proto;
	uint16_t length;
};

static constexpr auto TCP_PSEUDO_LEN = 12;

static uint16_t _chksum(struct t_tcp *pTcp, const struct tcb *pTcb, uint16_t nLength) {
	uint8_t buf[TCP_PSEUDO_LEN];
	uint16_t nSum;

	// Store current data before TCP header in temporary buffer
	auto *pseu = reinterpret_cast<struct tcpPseudo *>(reinterpret_cast<uint8_t *>(&pTcp->tcp) - TCP_PSEUDO_LEN);
	memcpy(buf, pseu, TCP_PSEUDO_LEN);

	// Generate TCP psuedo header
	std::memcpy(pseu->srcIp, pTcb->localIp, IPv4_ADDR_LEN);
	std::memcpy(pseu->dstIp, pTcb->remoteIp, IPv4_ADDR_LEN);
	pseu->zero = 0;
	pseu->proto = IPv4_PROTO_TCP;
	pseu->length = __builtin_bswap16(nLength);

	nSum = net_chksum(pseu, static_cast<uint32_t>(nLength + TCP_PSEUDO_LEN));

	// Restore data before TCP header from temporary buffer
	memcpy(pseu, buf, TCP_PSEUDO_LEN);

	return nSum;
}

static void send_package(const struct tcb *pTcb, const struct SendInfo &sendInfo) {
	uint32_t nDataOffset = 5; /*  Data Offset:  4 bits
	The number of 32 bit words in the TCP Header.  This indicates where
    the data begins.  The TCP header (even one including options) is an
    integral number of 32 bits long. */
	assert(nDataOffset * 4 == TCP_HEADER_SIZE);

	if (sendInfo.CTL & Control::SYN) {
		nDataOffset++;
	}

	nDataOffset += 3; // Option::KIND_TIMESTAMP

	const auto nHeaderLength = nDataOffset * 4;
	const auto tcplen = nHeaderLength + pTcb->TX.size;

	/* Ethernet */
	std::memcpy(s_tcp.ether.dst, pTcb->remoteEthAddr, ETH_ADDR_LEN);
	/* IPv4 */
	s_tcp.ip4.id = s_id++;
	s_tcp.ip4.len = __builtin_bswap16(static_cast<uint16_t>(tcplen + sizeof(struct ip4_header)));
	std::memcpy(s_tcp.ip4.src, pTcb->localIp, IPv4_ADDR_LEN);
	std::memcpy(s_tcp.ip4.dst, pTcb->remoteIp, IPv4_ADDR_LEN);
	s_tcp.ip4.chksum = 0;
#if !defined (CHECKSUM_BY_HARDWARE)
	s_tcp.ip4.chksum = net_chksum(reinterpret_cast<void *>(&s_tcp.ip4), 20);
#endif
	// TCP
	s_tcp.tcp.srcpt = pTcb->nLocalPort;
	s_tcp.tcp.dstpt = pTcb->nRemotePort;
	s_tcp.tcp.seqnum = sendInfo.SEQ;
	s_tcp.tcp.acknum = sendInfo.ACK;
	s_tcp.tcp.offset = static_cast<uint8_t>(nDataOffset << 4);
	s_tcp.tcp.control = sendInfo.CTL;
	s_tcp.tcp.window =  pTcb->RCV.WND;
	s_tcp.tcp.urgent = pTcb->SND.UP;
	s_tcp.tcp.checksum = 0;

	auto *pData = reinterpret_cast<uint8_t *>(&s_tcp.tcp.data);

	/* Add options */
	if (sendInfo.CTL & Control::SYN) {
		*pData++ = Option::KIND_MSS;
		*pData++ = OPTION_MSS_LENGTH;
		*(reinterpret_cast<uint16_t*>(pData)) = __builtin_bswap16(TCP_RX_MSS);
		pData += 2;
	}

	*pData++ = Option::KIND_NOP;
	*pData++ = Option::KIND_NOP;
	*pData++ = Option::KIND_TIMESTAMP;
	*pData++ = 10;
	auto nMillis = __builtin_bswap32(Hardware::Get()->Millis());
	memcpy(pData, &nMillis, 4);
	pData += 4;
	memcpy(pData, &pTcb->TS.Recent, 4);
	pData += 4;

	DEBUG_PRINTF("SEQ=%u, ACK=%u, tcplen=%u, data_offset=%u, p_tcb->TX.size=%u", s_tcp.tcp.seqnum, s_tcp.tcp.acknum, tcplen, nDataOffset, pTcb->TX.size);

	if (pTcb->TX.data != nullptr) {
		for (auto i = 0; i < pTcb->TX.size; i++) {
			*pData++ = pTcb->TX.data[i];
		}
	}

	s_tcp.tcp.srcpt = __builtin_bswap16(s_tcp.tcp.srcpt);
	s_tcp.tcp.dstpt = __builtin_bswap16(s_tcp.tcp.dstpt);
	_bswap32(&s_tcp);
	s_tcp.tcp.window = __builtin_bswap16(s_tcp.tcp.window);
	s_tcp.tcp.urgent = __builtin_bswap16(s_tcp.tcp.urgent);

	s_tcp.tcp.checksum = _chksum(&s_tcp, pTcb, static_cast<uint16_t>(tcplen));

	emac_eth_send(reinterpret_cast<void *>(&s_tcp), static_cast<int>(tcplen + sizeof(struct ip4_header) + sizeof(struct ether_header)));
}

static void send_reset(struct t_tcp *pTcp, const struct tcb *pTcb) {
	DEBUG_ENTRY

	if (pTcp->tcp.control & Control::RST) {
		return;
	}

	struct SendInfo info;
	info.CTL = Control::RST;

	if (pTcp->tcp.control & Control::ACK) {
		info.SEQ = _get_acknum(pTcp);
	} else {
		info.SEQ = 0;
		info.CTL |= Control::ACK;
	}

	uint32_t nDataLength = 0;

	if (pTcp->tcp.control & Control::SYN) {
		nDataLength++;
	}

	if (pTcp->tcp.control & Control::FIN) {
		nDataLength++;
	}

	info.ACK = _get_seqnum(pTcp) + nDataLength;

	send_package(pTcb, info);

	DEBUG_EXIT
}

struct Options {
	uint8_t nKind;
	uint8_t nLength;
	uint8_t Data;
};

static void scan_options(struct t_tcp *pTcp, struct tcb *pTcb, const int32_t nDataOffset) {
	const auto *pTcpHeaderEnd = reinterpret_cast<uint8_t *>(&pTcp->tcp) +  nDataOffset;

	auto *pOptions = reinterpret_cast<struct Options *>(pTcp->tcp.data);

	while (reinterpret_cast<uint8_t *>(pOptions + 2) <= pTcpHeaderEnd) {
		switch (pOptions->nKind) {
		case Option::KIND_END:
			return;
			break;
		case Option::KIND_NOP:
			pOptions = reinterpret_cast<struct Options *>(reinterpret_cast<uint8_t *>(pOptions) + 1);
			break;
		case Option::KIND_MSS:
			if ((pOptions->nLength == OPTION_MSS_LENGTH) && ((reinterpret_cast<uint8_t *>(pOptions) + OPTION_MSS_LENGTH) <= pTcpHeaderEnd)) {
				const auto *p = &pOptions->Data;
				auto nMSS = (p[0] << 8) + p[1];
				// RFC 1122 section 4.2.2.6
				nMSS = std::min(static_cast<int32_t>(nMSS + 20), static_cast<int32_t>(TCP_TX_MSS)) - TCP_HEADER_SIZE; // - IP_OPTION_SIZE;
				pTcb->SendMSS = static_cast<uint16_t>(nMSS);
			}
			pOptions = reinterpret_cast<struct Options *>(reinterpret_cast<uint8_t *>(pOptions) + pOptions->nLength);
			break;
		case Option::KIND_TIMESTAMP:	// RFC 7323  3.  TCP Timestamps Option
			if ((pOptions->nLength == OPTION_TIMESTAMP_LENGTH) && ((reinterpret_cast<uint8_t *>(pOptions) + OPTION_TIMESTAMP_LENGTH) <= pTcpHeaderEnd)) {
				_pcast32 tsval;
				memcpy(tsval.u8, &pOptions->Data, 4);
#ifndef NDEBUG
				auto bIgnore = true;
#endif
				if (pTcp->tcp.control & Control::SYN) {
					pTcb->TS.Recent = tsval.u32;
#ifndef NDEBUG
					bIgnore = false;
#endif
				} else if ((__builtin_bswap32(tsval.u32) > __builtin_bswap32(pTcb->TS.Recent))) { //TODO
					pTcb->TS.Recent = tsval.u32;
#ifndef NDEBUG
					bIgnore = false;
#endif
				}

				DEBUG_PRINTF("TSVal=%u [ignore:%c]", __builtin_bswap32(tsval.u32), bIgnore ? 'Y' : 'N');
			}
			pOptions = reinterpret_cast<struct Options *>(reinterpret_cast<uint8_t *>(pOptions) + pOptions->nLength);
			break;
		default:
			pOptions = reinterpret_cast<struct Options *>(reinterpret_cast<uint8_t *>(pOptions) + pOptions->nLength);
			break;
		}
	}
}

__attribute__((hot)) void tcp_run() {
	for (auto& port : s_Port) {
		for (auto& tcb : port.TCB) {
			if (tcb.state == STATE_CLOSE_WAIT) {
				SendInfo info;
				info.SEQ = tcb.SND.NXT;
				info.ACK = tcb.RCV.NXT;
				info.CTL = Control::FIN | Control::ACK;

				send_package(&tcb, info);

				NEW_STATE(&tcb, STATE_LAST_ACK);

				tcb.SND.NXT++;
			}
		}
	}
}

static bool find_matching_tcb(const t_tcp *pTcp, const uint32_t nIndexPort, uint32_t& nIndexTCB) {
    for (nIndexTCB = 0; nIndexTCB < TCP_MAX_TCBS_ALLOWED; nIndexTCB++) {
        auto *pTCB = &s_Port[nIndexPort].TCB[nIndexTCB];

        if (pTCB->state == STATE_LISTEN) {
            continue;
        }

        if (pTCB->nRemotePort == pTcp->tcp.srcpt && memcmp(pTCB->remoteIp, pTcp->ip4.src, IPv4_ADDR_LEN) == 0) {
            return true;
        }
    }

    return false;
}

static bool find_available_tcb(const uint32_t nIndexPort, uint32_t& nIndexTCB) {
    for (nIndexTCB = 0; nIndexTCB < TCP_MAX_TCBS_ALLOWED; nIndexTCB++) {
        auto *pTCB = &s_Port[nIndexPort].TCB[nIndexTCB];

        if (pTCB->state == STATE_LISTEN) {
            DEBUG_PUTS("pTCB->state == STATE_LISTEN");
            return true;
        }
    }

    return false;
}

static void find_tcb(const t_tcp *pTcp, uint32_t& nIndexPort, uint32_t& nIndexTCB) {
    // Search each port for a match with the destination port
    for (nIndexPort = 0; nIndexPort < TCP_MAX_PORTS_ALLOWED; nIndexPort++) {
        if (s_Port[nIndexPort].nLocalPort != pTcp->tcp.dstpt) {
            continue;
        }

        // Search for an existing active TCB matching the source IP and port
        if (find_matching_tcb(pTcp, nIndexPort, nIndexTCB)) {
            DEBUG_EXIT
            return;
        }

        // If no matching TCB, find an available TCB in listening state
        if (find_available_tcb(nIndexPort, nIndexTCB)) {
            DEBUG_EXIT
            return;
        }

        // If no available TCB, trigger retransmission
        DEBUG_PUTS("MAX_TCB_ALLOWED -> Force retransmission");
        DEBUG_EXIT
        return;
    }
}

/**
 * https://www.rfc-editor.org/rfc/rfc9293.html#name-segment-arrives
 */
__attribute__((hot)) void tcp_handle(struct t_tcp *pTcp) {
	const auto tcplen = static_cast<uint16_t>(__builtin_bswap16(pTcp->ip4.len) - sizeof(struct ip4_header));

	pTcp->tcp.srcpt = __builtin_bswap16(pTcp->tcp.srcpt);
	pTcp->tcp.dstpt = __builtin_bswap16(pTcp->tcp.dstpt);

	DEBUG_PRINTF(IPSTR ":%d[%d] -> %d", pTcp->ip4.src[0], pTcp->ip4.src[1], pTcp->ip4.src[2], pTcp->ip4.src[3], pTcp->tcp.dstpt, pTcp->tcp.srcpt, tcplen);

	uint32_t nIndexPort = 0;
	uint32_t nIndexTCB = 0;
	find_tcb(pTcp, nIndexPort, nIndexTCB);
	DEBUG_PRINTF("nIndexPort=%u, nIndexTCB=%u", nIndexPort, nIndexTCB);

	const auto nDataOffset = offset2octets(pTcp->tcp.offset);

	// https://www.rfc-editor.org/rfc/rfc9293.html#name-closed-state
	// CLOSED (i.e., TCB does not exist)
	if (nIndexPort == TCP_MAX_PORTS_ALLOWED) {
		struct tcb TCB;

		std::memset(&TCB, 0, sizeof(struct tcb));

		TCB.nLocalPort = pTcp->tcp.dstpt;
		std::memcpy(TCB.localIp, pTcp->ip4.dst, IPv4_ADDR_LEN);

		TCB.nRemotePort = pTcp->tcp.srcpt;
		std::memcpy(TCB.remoteIp, pTcp->ip4.src, IPv4_ADDR_LEN);
		std::memcpy(TCB.remoteEthAddr, pTcp->ether.src, ETH_ADDR_LEN);

		_bswap32(pTcp);

		scan_options(pTcp, &TCB, nDataOffset);
		send_reset(pTcp, &TCB);

		DEBUG_PUTS("TCP_MAX_PORTS_ALLOWED");
		DEBUG_EXIT
		return;
	}

	const auto nDataLength = static_cast<uint16_t>(tcplen - nDataOffset);

	_bswap32(pTcp);
	pTcp->tcp.window = __builtin_bswap16(pTcp->tcp.window);
	pTcp->tcp.urgent = __builtin_bswap16(pTcp->tcp.urgent);

	SendInfo sendInfo;

	const auto SEG_LEN = nDataLength;
	const auto SEG_ACK = _get_acknum(pTcp);
	const auto SEG_SEQ = _get_seqnum(pTcp);
	const auto SEG_WND = pTcp->tcp.window;

	auto *pTCB = &s_Port[nIndexPort].TCB[nIndexTCB];

	DEBUG_PRINTF("%u:%u:[%s] %c%c%c%c%c%c SEQ=%u, ACK=%u, tcplen=%u, data_offset=%u, data_length=%u",
			nIndexPort,
			nIndexTCB,
			s_aStateName[pTCB->state],
			pTcp->tcp.control & Control::URG ? 'U' : '-',
			pTcp->tcp.control & Control::ACK ? 'A' : '-',
			pTcp->tcp.control & Control::PSH ? 'P' : '-',
			pTcp->tcp.control & Control::RST ? 'R' : '-',
			pTcp->tcp.control & Control::SYN ? 'S' : '-',
			pTcp->tcp.control & Control::FIN ? 'F' : '-',
			_get_seqnum(pTcp),
			_get_acknum(pTcp),
			tcplen,
			nDataOffset,
			nDataLength);

	scan_options(pTcp, pTCB, nDataOffset);

	// https://www.rfc-editor.org/rfc/rfc9293.html#name-listen-state
	if (pTCB->state == STATE_LISTEN) {
		std::memcpy(pTCB->localIp, pTcp->ip4.dst, IPv4_ADDR_LEN);

		pTCB->nRemotePort = pTcp->tcp.srcpt;
		std::memcpy(pTCB->remoteIp, pTcp->ip4.src, IPv4_ADDR_LEN);
		std::memcpy(pTCB->remoteEthAddr, pTcp->ether.src, ETH_ADDR_LEN);

		// First, check for a RST
		// An incoming RST should be ignored.
		if (pTcp->tcp.control & Control::RST) {

			DEBUG_EXIT
			return;
		}

		// Second, check for an ACK
		// Any acknowledgment is bad if it arrives on a connection still in the LISTEN state.
		// RST -> <SEQ=SEG.ACK><CTL=RST>
		if (pTcp->tcp.control & Control::ACK) {
			send_reset(pTcp, pTCB);

			DEBUG_PUTS("pTcp->tcp.control & Control::ACK");
			DEBUG_EXIT
			return;
		}

		// Third, check for a SYN
		// We skip security check
		if (pTcp->tcp.control & Control::SYN) {
			// Set RCV.NXT to SEG.SEQ+1, IRS is set to SEG.SEQ
			pTCB->RCV.NXT = SEG_SEQ + 1;
			pTCB->IRS = SEG_SEQ;

			// <SEQ=ISS><ACK=RCV.NXT><CTL=SYN,ACK>
			sendInfo.SEQ = pTCB->ISS;
			sendInfo.ACK = pTCB->RCV.NXT;
			sendInfo.CTL = Control::SYN | Control::ACK;
			send_package(pTCB, sendInfo);

			// SND.NXT is set to ISS+1 and SND.UNA to ISS. The connection state should be changed to SYN-RECEIVED.
			pTCB->SND.NXT = pTCB->ISS + 1;
			pTCB->SND.UNA = pTCB->ISS;

			NEW_STATE(pTCB, STATE_SYN_RECEIVED);
			DEBUG_EXIT
			return;
		}

		// Fourth, other data or control
		DEBUG_PUTS("This should not be reached.");
		DEBUG_EXIT
		return;
	}

	// We skip SYN-SENT STATE as we are server only.

	///< https://www.rfc-editor.org/rfc/rfc9293.html#name-other-states
	switch (pTCB->state) {
	case STATE_SYN_RECEIVED:
	case STATE_ESTABLISHED:
	case STATE_FIN_WAIT_1:
	case STATE_FIN_WAIT_2:
	case STATE_CLOSE_WAIT:
	case STATE_CLOSING:
	case STATE_LAST_ACK:
	case STATE_TIME_WAIT: {
		// There are four cases for the acceptability test for an incoming segment
		auto isAcceptable = false;

		DEBUG_PRINTF("RCV.WND=%u, SEG_LEN=%u, RCV.NXT=%u, SEG_SEQ=%u", pTCB->RCV.WND, SEG_LEN, pTCB->RCV.NXT, SEG_SEQ);

		if (pTCB->RCV.WND > 0) {
			if (SEG_LEN == 0) {
				// Case 2: SEG_LEN = 0 RCV.WND > 0 -> RCV.NXT =< SEG.SEQ < RCV.NXT+RCV.WND
				if (SEQ_BETWEEN_L(pTCB->RCV.NXT, SEG_SEQ, pTCB->RCV.NXT + pTCB->RCV.WND)) {
					isAcceptable = true;
				}
			} else {
				// Case 4: SEG_LEN > 0 RCV.WND > 0 ->
				// RCV.NXT =< SEG.SEQ < RCV.NXT+RCV.WND
				// or
				// RCV.NXT =< SEG.SEQ+SEG.LEN-1 < RCV.NXT+RCV.WND
				if ( SEQ_BETWEEN_L(pTCB->RCV.NXT, SEG_SEQ, pTCB->RCV.NXT + pTCB->RCV.WND)
				  || SEQ_BETWEEN_L(pTCB->RCV.NXT, SEG_SEQ + SEG_LEN-1, pTCB->RCV.NXT+pTCB->RCV.WND)) {
					isAcceptable = true;
				}
			}
		} else {
			// Case 1: SEG_LEN = 0 RCV.WND = 0 -> SEG.SEQ = RCV.NXT
			if (SEG_LEN == 0) {
				if (SEG_SEQ == pTCB->RCV.NXT) {
					isAcceptable = true;
				}
			}
			// Case 3: SEG_LEN > 0 RCV.WND =0 -> not acceptable
		}

		DEBUG_PRINTF("isAcceptable=%d", isAcceptable);

		if (!isAcceptable) {
			// If an incoming segment is not acceptable, an acknowledgment should be sent in reply
			// (unless the RST bit is set, if so drop the segment and return)
			// <SEQ=SND.NXT><ACK=RCV.NXT><CTL=ACK>
			if (pTcp->tcp.control & Control::RST) {
				_init_tcb(pTCB, pTCB->nLocalPort);
				DEBUG_EXIT
				return;
			}

			sendInfo.SEQ = pTCB->SND.NXT;
			sendInfo.ACK = pTCB->RCV.NXT;
			sendInfo.CTL = Control::ACK;
			send_package(pTCB, sendInfo);

			DEBUG_EXIT
			return;
		}

		// second check the RST bit, *//* Page 70 */
		if (pTcp->tcp.control & Control::RST) {
			switch (pTCB->state) {
			case STATE_SYN_RECEIVED:
				_init_tcb(pTCB, pTCB->nLocalPort);
				break;
			case STATE_ESTABLISHED:
			case STATE_FIN_WAIT_1:
			case STATE_FIN_WAIT_2:
			case STATE_CLOSE_WAIT:
				/* If the RST bit is set then, any outstanding RECEIVEs and SEND
				 * should receive "reset" responses.  All segment queues should be
				 * flushed.  Users should also receive an unsolicited general
				 * "connection reset" signal.  Enter the CLOSED state, delete the
				 * TCB, and return. */
				_init_tcb(pTCB, pTCB->nLocalPort);
				break;
			case STATE_CLOSING:
			case STATE_LAST_ACK:
			case STATE_TIME_WAIT:
				/* If the RST bit is set then, enter the CLOSED state, delete the
				 * TCB, and return. */
				_init_tcb(pTCB, pTCB->nLocalPort);
				break;
			default:
				assert(0);
				break;
			}
			return;
		}

		// third check security and precedence. No code needed here

		/* fourth, check the SYN bit, *//* Page 71 */
		if (pTcp->tcp.control & Control::SYN) {
			// RFC 1122 section 4.2.2.20 (e)
			if (pTCB->state == STATE_SYN_RECEIVED) {
				_init_tcb(pTCB, pTCB->nLocalPort);
				return;
			}

			send_reset(pTcp, pTCB);

			DEBUG_PUTS("pTcp->tcp.control & Control::SYN");
		}

		/*  fifth check the ACK field, *//* Page 72 */
		if (!(pTcp->tcp.control & Control::ACK)) {
			// if the ACK bit is off drop the segment and return
			return;
		}

		switch (pTCB->state) {
		case STATE_SYN_RECEIVED:
			/*  If SND.UNA =< SEG.ACK =< SND.NXT then enter ESTABLISHED state
			 *and continue processing. */
			if (SEQ_BETWEEN_LH (pTCB->SND.UNA, SEG_ACK, pTCB->SND.NXT)) {
				// RFC 1122 section 4.2.2.20 (f)
				pTCB->SND.WND = SEG_WND;
				pTCB->SND.WL1 = SEG_SEQ;
				pTCB->SND.WL2 = SEG_ACK;

				pTCB->SND.UNA = SEG_ACK;		// got ACK for SYN

				NEW_STATE(pTCB, STATE_ESTABLISHED);
				return;
			} else {
				// <SEQ=SEG.ACK><CTL=RST>
				send_reset(pTcp, pTCB);

				DEBUG_PUTS("<SEQ=SEG.ACK><CTL=RST>");
			}
			break;
		case STATE_ESTABLISHED:
		case STATE_FIN_WAIT_1:
		case STATE_FIN_WAIT_2:
		case STATE_CLOSE_WAIT:
		case STATE_CLOSING:
			DEBUG_PRINTF("SND.UNA=%u, SEG_ACK=%u, SND.NXT=%u", pTCB->SND.UNA, SEG_ACK, pTCB->SND.NXT);

			if (SEQ_BETWEEN_H(pTCB->SND.UNA, SEG_ACK, pTCB->SND.NXT)) {

				auto nBytesAck = SEG_ACK - pTCB->SND.UNA;
				pTCB->SND.UNA = SEG_ACK;

				if (SEG_ACK == pTCB->SND.NXT) {
					DEBUG_PUTS("/* all segments are acknowledged */");
				}

				if (pTCB->state == STATE_FIN_WAIT_1 || pTCB->state == STATE_CLOSING) {
					nBytesAck--;
					DEBUG_PUTS("/* acknowledged FIN does not count */");
				}

				if (pTCB->state == STATE_ESTABLISHED && nBytesAck == 1) {
					nBytesAck--;
				}

				// update send window
				if ( SEQ_LT(pTCB->SND.WL1, SEG_SEQ) || (pTCB->SND.WL1 == SEG_SEQ && SEQ_LEQ(pTCB->SND.WL2, SEG_ACK))) {
					pTCB->SND.WND = SEG_WND;
					pTCB->SND.WL1 = SEG_SEQ;
					pTCB->SND.WL2 = SEG_ACK;
				}
			} else if (SEQ_LEQ(SEG_ACK, pTCB->SND.UNA)) { /* RFC 1122 section 4.2.2.20 (g) */
				DEBUG_PUTS("/* ignore duplicate ACK */");
				if (SEQ_BETWEEN_LH(pTCB->SND.UNA, SEG_ACK, pTCB->SND.NXT)) {
					// ... but update send window
					if ( SEQ_LT(pTCB->SND.WL1, SEG_SEQ) || (pTCB->SND.WL1 == SEG_SEQ && SEQ_LEQ(pTCB->SND.WL2, SEG_ACK))) {
						pTCB->SND.WND = SEG_WND;
						pTCB->SND.WL1 = SEG_SEQ;
						pTCB->SND.WL2 = SEG_ACK;
					}
				}
			} else if (SEQ_GT(SEG_ACK, pTCB->SND.NXT)) {
				DEBUG_PRINTF("SEG_ACK=%u, SND.NXT=%u", SEG_ACK,pTCB->SND.NXT);

				sendInfo.SEQ = pTCB->SND.NXT;
				sendInfo.ACK = pTCB->RCV.NXT;
				sendInfo.CTL = Control::ACK;

				send_package(pTCB, sendInfo);
				return;
			}
			break;
		case STATE_LAST_ACK:
			if (SEG_ACK == pTCB->SND.NXT) { 	// if our FIN is now acknowledged
				_init_tcb(pTCB, pTCB->nLocalPort);
			}
			break;
		case STATE_TIME_WAIT:
			if (SEG_ACK == pTCB->SND.NXT) {		// if our FIN is now acknowledged
				sendInfo.SEQ = pTCB->SND.NXT;
				sendInfo.ACK = pTCB->RCV.NXT;
				sendInfo.CTL = Control::ACK;

				send_package(pTCB, sendInfo);
				CLIENT_NOT_IMPLEMENTED;
			}
			break;
		default:
			UNEXPECTED_STATE();
			break;
		}

		// sixth, check the URG bit. No code needed here

		// seventh, process the segment text
		switch (pTCB->state) {
		case STATE_ESTABLISHED:
		case STATE_FIN_WAIT_1:
		case STATE_FIN_WAIT_2:
			if (nDataLength > 0) {
				if (SEG_SEQ == pTCB->RCV.NXT) {
					auto *pQueue = &s_Port[nIndexPort].receiveQueue;
					auto *pQueueEntry = &pQueue->Entries[pQueue->nHead];

					pQueueEntry->nHandleConnection = static_cast<uint16_t>(nIndexTCB);
					memcpy(pQueueEntry->data, reinterpret_cast<uint8_t *>(&pTcp->tcp) + nDataOffset, nDataLength);
					pQueueEntry->nSize = nDataLength;

					pTCB->RCV.NXT += nDataLength;
					pTCB->RCV.WND -= static_cast<uint16_t>(TCP_DATA_SIZE);

					sendInfo.SEQ = pTCB->SND.NXT;
					sendInfo.ACK = pTCB->RCV.NXT;
					sendInfo.CTL = Control::ACK;

					send_package(pTCB, sendInfo);

					pQueue->nHead = (pQueue->nHead + 1) & TCP_RX_MAX_ENTRIES_MASK;
				} else {
					sendInfo.SEQ = pTCB->SND.NXT;
					sendInfo.ACK = pTCB->RCV.NXT;
					sendInfo.CTL = Control::ACK;

					send_package(pTCB, sendInfo);

					DEBUG_PUTS("Out of order");
					DEBUG_EXIT
					return;
				}
			}
			break;
		default:
			break;
		}

		/* eighth, check the FIN bit, *//* Page 75 */
		/*
		 Do not process the FIN if the state is CLOSED, LISTEN or SYN-SENT
		 since the SEG.SEQ cannot be validated; drop the segment and return.
		 */

		if ((pTCB->state == STATE_CLOSED) || (pTCB->state == STATE_LISTEN)  || (pTCB->state == STATE_SYN_SENT) ) {
			DEBUG_EXIT
			return;
		}

		if (!(pTcp->tcp.control & Control::FIN)) {
			DEBUG_EXIT
			return ;
		}

		/*
		 If the FIN bit is set, signal the user "connection closing" and
		 return any pending RECEIVEs with same message, advance RCV.NXT
		 over the FIN, and send an acknowledgment for the FIN.  Note that
		 FIN implies PUSH for any segment text not yet delivered to the
		 user.
		 */

		pTCB->RCV.NXT = pTCB->RCV.NXT + 1;
		sendInfo.SEQ = pTCB->SND.NXT;
		sendInfo.ACK = pTCB->RCV.NXT;
		sendInfo.CTL = Control::ACK;

		send_package(pTCB, sendInfo);

		switch (pTCB->state) {
		case STATE_SYN_RECEIVED:
		case STATE_ESTABLISHED:
			NEW_STATE(pTCB, STATE_CLOSE_WAIT);
			break;
		case STATE_FIN_WAIT_1:
			/*
			 If our FIN has been ACKed (perhaps in this segment), then
			 enter TIME-WAIT, start the time-wait timer, turn off the other
			 timers; otherwise enter the CLOSING state.
			 */
			if (SEG_ACK == pTCB->SND.NXT) { /* if our FIN is now acknowledged */
				NEW_STATE(pTCB, STATE_TIME_WAIT);
				CLIENT_NOT_IMPLEMENTED;
			} else {
				NEW_STATE(pTCB, STATE_CLOSING);
			}
			break;
		case STATE_FIN_WAIT_2:
			// Enter the TIME-WAIT state.  Start the time-wait timer, turn off the other timers.
			NEW_STATE(pTCB, STATE_TIME_WAIT);
			CLIENT_NOT_IMPLEMENTED;
			break;
		case STATE_CLOSE_WAIT:
			// Remain in the CLOSE-WAIT state.
		case STATE_CLOSING:
			// Remain in the CLOSING state.
		case STATE_LAST_ACK:
			// Remain in the LAST-ACK state.
			break;
		case STATE_TIME_WAIT:
			CLIENT_NOT_IMPLEMENTED;
			break;
		default:
			assert(0);
			break;
		}

	}
	break;
	default:
		assert(0);
		break;
	}
}

// --> Public API's

int tcp_begin(const uint16_t nLocalPort) {
	DEBUG_PRINTF("nLocalPort=%u", nLocalPort);

	for (int i = 0; i < TCP_MAX_PORTS_ALLOWED; i++) {
		if (s_Port[i].nLocalPort == nLocalPort) {
			return i;
		}

		if (s_Port[i].nLocalPort == 0) {
			s_Port[i].nLocalPort = nLocalPort;

			for (uint32_t nIndexTCB = 0; nIndexTCB < TCP_MAX_TCBS_ALLOWED; nIndexTCB++) {
				// create transmission control block's (TCB)
				_init_tcb(&s_Port[i].TCB[nIndexTCB], nLocalPort);
			}

			DEBUG_PRINTF("i=%d, nLocalPort=%d[%x]", i, nLocalPort, nLocalPort);
			return i;
		}
	}

#ifndef NDEBUG
	console_error("tcp_begin\n");
#endif
	return -1;

}

uint16_t tcp_read(const int32_t nHandleListen, const uint8_t **pData, uint32_t &nHandleConnection) {
	assert(nHandleListen >= 0);
	assert(nHandleListen < TCP_MAX_PORTS_ALLOWED);

	auto *pQueue = &s_Port[nHandleListen].receiveQueue;

	if (__builtin_expect((pQueue->nHead == pQueue->nTail), 1)) {
		return 0;
	}

	const auto nEntry = pQueue->nTail;
	const auto *const pQueueEntry = &pQueue->Entries[nEntry];

	nHandleConnection = pQueueEntry->nHandleConnection;
	*pData = pQueueEntry->data;

	auto *pTCB = &s_Port[nHandleListen].TCB[nHandleConnection];

	pTCB->RCV.WND += TCP_DATA_SIZE;

	pQueue->nTail = (pQueue->nTail + 1) & TCP_RX_MAX_ENTRIES_MASK;

	return pQueueEntry->nSize;
}

static void _write(struct tcb *pTCB, const uint8_t *pBuffer, const uint32_t nLength, const bool isLastSegment) {
	assert(nLength != 0);
	assert(nLength <= static_cast<uint32_t>(TCP_DATA_SIZE));

	DEBUG_PRINTF("nLength=%u, pTCB->SND.WND=%u", nLength, pTCB->SND.WND);

    if (nLength > pTCB->SND.WND) {
    	console_error("Retry or queue the data for later transmission\n");
        // TODO retry or queue the data for later transmission.
        return;
    }

	pTCB->TX.data = const_cast<uint8_t *>(pBuffer);
	pTCB->TX.size = nLength;

	struct SendInfo info;
	info.SEQ = pTCB->SND.NXT;
	info.ACK = pTCB->RCV.NXT;
	info.CTL = Control::ACK;
	if (isLastSegment) {
		info.CTL |= Control::PSH;
	}

	send_package(pTCB, info);

	pTCB->TX.data = nullptr;
	pTCB->TX.size = 0;

    pTCB->SND.NXT += nLength;
    pTCB->SND.WND -= nLength;
}

void tcp_write(const int32_t nHandleListen, const uint8_t *pBuffer, uint32_t nLength, uint32_t nHandleConnection) {
	assert(nHandleListen >= 0);
	assert(nHandleListen < TCP_MAX_PORTS_ALLOWED);
	assert(pBuffer != nullptr);
	assert(nHandleConnection < TCP_MAX_TCBS_ALLOWED);

	auto *pTCB = &s_Port[nHandleListen].TCB[nHandleConnection];
	assert(pTCB != nullptr);

	const auto *p = pBuffer;

	while (nLength > 0) {
		const auto nWriteLength = (nLength > TCP_DATA_SIZE) ? TCP_DATA_SIZE : nLength;
		const bool isLastSegment = (nLength < TCP_DATA_SIZE);
		_write(pTCB, p, nWriteLength, isLastSegment);
		p += nWriteLength;
		nLength -= nWriteLength;
	}
}
}  // namespace net
// <---
