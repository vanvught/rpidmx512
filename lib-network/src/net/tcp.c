/**
 * @file tcp.c
 *
 */
/* Copyright (C) 2021-2022 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#if (__GNUC__ < 10)
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wconversion"
# pragma GCC diagnostic ignored "-Wsign-conversion"
#endif

#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

#include "tcp.h"

#include "net.h"
#include "net_packets.h"
#include "net_platform.h"
#include "net_debug.h"

#include "c/millis.h"

#ifndef ALIGNED
# define ALIGNED __attribute__ ((aligned (4)))
#endif

#ifndef MIN
# define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

extern int console_error(const char*);
extern void emac_eth_send(void*, int);
extern uint16_t net_chksum(void*, uint32_t);

extern struct ip_info g_ip_info;
extern uint8_t g_mac_address[ETH_ADDR_LEN];

struct queue_entry {
	uint8_t data[TCP_RX_MSS];
	uint16_t size;
};

struct queue {
	uint16_t queue_head;
	uint16_t queue_tail;
	struct queue_entry entries[TCP_RX_MAX_ENTRIES];
};

static struct queue s_recv_queue[TCP_MAX_CONNECTIONS_ALLOWED] SECTION_NETWORK ALIGNED;
static struct tcb s_tcb[TCP_MAX_CONNECTIONS_ALLOWED] SECTION_NETWORK ALIGNED;
static uint16_t s_id SECTION_NETWORK ALIGNED;
static struct t_tcp s_tcp SECTION_NETWORK ALIGNED;

#if !defined (NDEBUG)
static const char *state_name[] = {
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

static uint8_t _new_state(struct tcb *p_tcb, uint8_t state, const char *func, const char *file, unsigned line) {
	assert(p_tcb->state < sizeof state_name / sizeof state_name[0]);
	assert(state < sizeof state_name / sizeof state_name[0]);

	printf("%s() %s, line %i: %s -> %s\n", func, file, line, state_name[p_tcb->state], state_name[state]);

	return p_tcb->state = state;
}

static void _unexpected_state(unsigned state, unsigned line) {
	printf("Unexpected state %s at line %u\n", state_name[state], line);
}

# define NEW_STATE(state)		_new_state(l_tcb, state, __func__, __FILE__, __LINE__)
# define UNEXPECTED_STATE()		_unexpected_state (l_tcb->state, __LINE__)
# define CLIENT_NOT_IMPLEMENTED	assert(0)
#else
# define NEW_STATE(STATE)		(l_tcb->state = STATE)
# define UNEXPECTED_STATE()		((void)0)
# define CLIENT_NOT_IMPLEMENTED	((void)0)
#endif

/*
 * RFC 793, Page 16
 */

enum {
	CONTROL_URG = 0x20,/* Urgent Pointer field significant */
	CONTROL_ACK = 0x10,/* Acknowledgment field significant */
	CONTROL_PSH = 0x08,/* Acknowledgment */
	CONTROL_RST = 0x04,/* Reset the connection */
	CONTROL_SYN = 0x02,/* Synchronize sequence numbers */
	CONTROL_FIN = 0x01 /* No more data from sender */
};

/*
 * RFC 793, Page 18
 */

enum {
	OPTION_KIND_END = 0,/* End of option list */
	OPTION_KIND_NOP = 1,/* No-Operation */
	OPTION_KIND_MSS = 2 /* Maximum Segment Size */
};

#define OPTION_MSS_LEN  4 /* length of MSS option */

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

/* Modulo 32 sequence number arithmetic */
/* Kindly copied from
 * https://github.com/rsta2/circle/blob/master/lib/net/tcpconnection.cpp#L102
 */
#define lt(x, y)		((int) ((uint32_t) (x) - (uint32_t) (y)) < 0)
#define le(x, y)		((int) ((uint32_t) (x) - (uint32_t) (y)) <= 0)
#define gt(x, y) 		lt (y, x)
#define ge(x, y) 		le (y, x)

#define bw(l, x, h)		(lt ((l), (x)) && lt ((x), (h)))	/* between */
#define bwl(l, x, h)	(le ((l), (x)) && lt ((x), (h)))	/*	low border inclusive */
#define bwh(l, x, h)	(lt ((l), (x)) && le ((x), (h)))	/*	high border inclusive */
#define bwlh(l, x, h)	(le ((l), (x)) && le ((x), (h)))	/*	both borders inclusive */

typedef union pcast32 {
	uint32_t u32;
	uint8_t u8[4];
} _pcast32;

static uint32_t _get_seqnum(struct t_tcp *p_tcp) {
	_pcast32 src;
	memcpy(src.u8, &p_tcp->tcp.seqnum, 4);
	return src.u32;
}

static uint32_t _get_acknum(struct t_tcp *p_tcp) {
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

static uint32_t _get_iss(void) {
	return millis();
}

static void _init_tcb(struct tcb *l_tcb, uint16_t local_port) {
	memset(l_tcb, 0, sizeof(struct tcb));
	l_tcb->ISS = _get_iss();

	l_tcb->local_port = local_port;
	l_tcb->RCV.WND = TCP_MAX_RX_WND;

	l_tcb->SND.UNA = l_tcb->ISS;
	l_tcb->SND.NXT = l_tcb->ISS;
	l_tcb->SND.WL2 = l_tcb->ISS;

	NEW_STATE(STATE_LISTEN);
}

__attribute__((cold)) void tcp_init(void) {
	DEBUG_ENTRY

	_pcast32 src;

	/* Ethernet */
	memcpy(s_tcp.ether.src, g_mac_address, ETH_ADDR_LEN);
	s_tcp.ether.type = __builtin_bswap16(ETHER_TYPE_IPv4);
	/* IPv4 */
	src.u32 = g_ip_info.ip.addr;
	memcpy(s_tcp.ip4.src, src.u8, IPv4_ADDR_LEN);
	s_tcp.ip4.ver_ihl = 0x45;
	s_tcp.ip4.tos = 0;
	s_tcp.ip4.flags_froff = __builtin_bswap16(IPv4_FLAG_DF);
	s_tcp.ip4.ttl = 64;
	s_tcp.ip4.proto = IPv4_PROTO_TCP;

	debug_dump(&s_tcp, 34);

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

#define TCP_PSEUDO_LEN  12

static uint16_t _chksum(struct t_tcp *p_tcp, struct tcb *p_tcb, uint16_t length) {
	struct tcpPseudo *pseu;
	uint8_t buf[TCP_PSEUDO_LEN];
	uint16_t sum;

	/* Store current data before TCP header in temporary buffer */
	pseu = (struct tcpPseudo*) ((uint8_t *)&p_tcp->tcp - TCP_PSEUDO_LEN);
	memcpy(buf, pseu, TCP_PSEUDO_LEN);

	/* Generate TCP psuedo header */
	memcpy(pseu->srcIp, &g_ip_info.ip.addr, IPv4_ADDR_LEN);
	memcpy(pseu->dstIp, &p_tcb->remoteip, IPv4_ADDR_LEN);
	pseu->zero = 0;
	pseu->proto = IPv4_PROTO_TCP;
	pseu->length = __builtin_bswap16(length);

	sum = net_chksum(pseu, (uint32_t)length + TCP_PSEUDO_LEN);

	/* Restore data before TCP header from temporary buffer */
	memcpy(pseu, buf, TCP_PSEUDO_LEN);

	return sum;
}

static void _tcp_send_package(struct tcb *p_tcb, struct send_info *info) {
	uint8_t *data;
	uint32_t i = 0;
	uint8_t data_offset = 5; /*  Data Offset:  4 bits
	The number of 32 bit words in the TCP Header.  This indicates where
    the data begins.  The TCP header (even one including options) is an
    integral number of 32 bits long. */
	uint8_t header_length;
	uint16_t tcplen;

	assert(data_offset * 4 == TCP_HEADER_SIZE);

	if (info->ctrl & CONTROL_SYN) {
		data_offset++;
	}

	header_length = data_offset * 4;
	tcplen = header_length + p_tcb->TX.size;

	/* Ethernet */
	memcpy(s_tcp.ether.dst, p_tcb->remoteeth, ETH_ADDR_LEN);
	/* IPv4 */
	s_tcp.ip4.id = s_id++;
	s_tcp.ip4.len = __builtin_bswap16(tcplen + sizeof(struct ip4_header));
	memcpy(s_tcp.ip4.dst, &p_tcb->remoteip, IPv4_ADDR_LEN);
	s_tcp.ip4.chksum = 0;
	s_tcp.ip4.chksum = net_chksum((void*) &s_tcp.ip4, 20);
	// TCP
	s_tcp.tcp.srcpt = p_tcb->local_port;
	s_tcp.tcp.dstpt = p_tcb->remotept;
	s_tcp.tcp.seqnum = info->seq;
	s_tcp.tcp.acknum = info->ack;
	s_tcp.tcp.offset = data_offset << 4;
	s_tcp.tcp.control = info->ctrl;
	s_tcp.tcp.window =  p_tcb->RCV.WND;
	s_tcp.tcp.urgent = p_tcb->SND.UP;
	s_tcp.tcp.checksum = 0;

	data = (uint8_t *)&s_tcp.tcp.data;

	/* Add options */
	if (info->ctrl & CONTROL_SYN) {
		*data++ = OPTION_KIND_MSS;
		*data++ = OPTION_MSS_LEN;
		*((uint16_t*) data) = __builtin_bswap16(TCP_RX_MSS);
	}

	DEBUG_PRINTF("SEQ=%u, ACK=%u, tcplen=%u, data_offset=%u, p_tcb->TX.size=%u", s_tcp.tcp.seqnum, s_tcp.tcp.acknum, tcplen, data_offset, p_tcb->TX.size);

	if (p_tcb->TX.data != NULL) {
		for (i = 0; i < p_tcb->TX.size; i++) {
			*data++ = p_tcb->TX.data[i];
		}
	}

	s_tcp.tcp.srcpt = __builtin_bswap16(s_tcp.tcp.srcpt);
	s_tcp.tcp.dstpt = __builtin_bswap16(s_tcp.tcp.dstpt);
	_bswap32(&s_tcp);
	s_tcp.tcp.window = __builtin_bswap16(s_tcp.tcp.window);
	s_tcp.tcp.urgent = __builtin_bswap16(s_tcp.tcp.urgent);

	s_tcp.tcp.checksum = _chksum(&s_tcp, p_tcb, tcplen);

	emac_eth_send((void*) &s_tcp, (int) tcplen + sizeof(struct ip4_header) + sizeof(struct ether_header));
}

/*
 * TCP Send RST
 */

static void _send_reset(struct t_tcp *p_tcp, struct tcb *p_tcb) {
	DEBUG_ENTRY

	struct send_info info;
	uint16_t datalen;

	if (p_tcp->tcp.control & CONTROL_RST) {
		return;
	}

	info.ctrl = CONTROL_RST;

	if (p_tcp->tcp.control & CONTROL_ACK) {
		info.seq = _get_acknum(p_tcp);
	} else {
		info.seq = 0;
		info.ctrl |= CONTROL_ACK;
	}

	datalen = 0;

	if (p_tcp->tcp.control & CONTROL_SYN) {
		datalen++;
	}

	if (p_tcp->tcp.control & CONTROL_FIN) {
		datalen++;
	}

	info.ack = _get_seqnum(p_tcp) + datalen;

	_tcp_send_package(p_tcb, &info);

	DEBUG_EXIT
}

__attribute__((hot)) void tcp_handle(struct t_tcp *p_tcp) {
	struct send_info info;
	struct tcb *l_tcb;
	_pcast32 src;
	uint16_t connection_index;
	uint32_t SEG_SEQ;
	uint32_t SEG_ACK;
	uint16_t SEG_WND;
	uint16_t SEG_LEN;

	const uint16_t tcplen = __builtin_bswap16(p_tcp->ip4.len) - (uint16_t) sizeof(struct ip4_header);
	const uint16_t data_offset = offset2octets(p_tcp->tcp.offset);
	const uint16_t data_length = tcplen - data_offset;

	/*
	 * Page 65 SEGMENT ARRIVES
	 */

	p_tcp->tcp.srcpt = __builtin_bswap16(p_tcp->tcp.srcpt);
	p_tcp->tcp.dstpt = __builtin_bswap16(p_tcp->tcp.dstpt);
	_bswap32(p_tcp);
	p_tcp->tcp.window = __builtin_bswap16(p_tcp->tcp.window);
	p_tcp->tcp.urgent = __builtin_bswap16(p_tcp->tcp.urgent);

	DEBUG_PRINTF(IPSTR ":%d[%d] -> %d", p_tcp->ip4.src[0], p_tcp->ip4.src[1], p_tcp->ip4.src[2], p_tcp->ip4.src[3], p_tcp->tcp.dstpt, p_tcp->tcp.srcpt, tcplen);

	/* find a TCB */

	for (connection_index = 0; connection_index < TCP_MAX_CONNECTIONS_ALLOWED; connection_index++) {
		l_tcb = &s_tcb[connection_index];

		if (l_tcb->local_port == p_tcp->tcp.dstpt) {
			if (l_tcb->state == STATE_LISTEN) {
				break;
			} else {
				memcpy(src.u8, p_tcp->ip4.src, IPv4_ADDR_LEN);
				if ((l_tcb->remotept == p_tcp->tcp.srcpt) && (l_tcb->remoteip == src.u32)) {
					break;
				}
//				else {
//					memcpy(src.u8, p_tcp->ip4.src, IPv4_ADDR_LEN);
//					/* Do not use current TCB */
//					struct tcb t_tcb;
//					memset(&t_tcb, 0, sizeof(struct tcb));
//					t_tcb.local_port = p_tcp->tcp.dstpt;
//					t_tcb.remotept = p_tcp->tcp.srcpt;
//					memcpy(src.u8, p_tcp->ip4.src, IPv4_ADDR_LEN);
//					t_tcb.remoteip = src.u32;
//					memcpy(t_tcb.remoteeth, p_tcp->ether.src, ETH_ADDR_LEN);
//					_send_reset(p_tcp, &t_tcb);
//					return;
//				}
			}
		}
	}

	if (connection_index == TCP_MAX_CONNECTIONS_ALLOWED) {
		DEBUG_PUTS("/* There is no TCB */");
//		struct tcb t_tcb;
//		memset(&t_tcb, 0, sizeof(struct tcb));
//		t_tcb.local_port = p_tcp->tcp.dstpt;
//		t_tcb.remotept = p_tcp->tcp.srcpt;
//		memcpy(src.u8, p_tcp->ip4.src, IPv4_ADDR_LEN);
//		t_tcb.remoteip = src.u32;
//		memcpy(t_tcb.remoteeth, p_tcp->ether.src, ETH_ADDR_LEN);
//		_send_reset(p_tcp, &t_tcb);
		return;
	}

	DEBUG_PRINTF("%u:[%s] %c%c%c%c%c%c SEQ=%u, ACK=%u, tcplen=%u, data_offset=%u, data_length=%u",
			connection_index,
			state_name[l_tcb->state],
			p_tcp->tcp.control & CONTROL_URG ? 'U' : '-',
			p_tcp->tcp.control & CONTROL_ACK ? 'A' : '-',
			p_tcp->tcp.control & CONTROL_PSH ? 'P' : '-',
			p_tcp->tcp.control & CONTROL_RST ? 'R' : '-',
			p_tcp->tcp.control & CONTROL_SYN ? 'S' : '-',
			p_tcp->tcp.control & CONTROL_FIN ? 'F' : '-',
			_get_seqnum(p_tcp),
			_get_acknum(p_tcp),
			tcplen,
			data_offset,
			data_length);

	SEG_LEN = data_length;
	SEG_ACK = _get_acknum(p_tcp);
	SEG_SEQ = _get_seqnum(p_tcp);
	SEG_WND = p_tcp->tcp.window;

	switch (l_tcb->state) {
	case STATE_LISTEN: {
		/* An incoming RST should be ignored. */
		if (p_tcp->tcp.control & CONTROL_RST) {
			return;
		}
		/* Any acknowledgment is bad if it arrives on a connection still in
		 * the LISTEN state.  An acceptable reset segment should be formed
		 * for any arriving ACK-bearing segment. */
		if (p_tcp->tcp.control & CONTROL_ACK) {
			_send_reset(p_tcp, l_tcb);
			return;
		}
		if (p_tcp->tcp.control & CONTROL_SYN) {
			l_tcb->remotept = p_tcp->tcp.srcpt;
			memcpy(src.u8, p_tcp->ip4.src, IPv4_ADDR_LEN);
			l_tcb->remoteip = src.u32;
			memcpy(l_tcb->remoteeth, p_tcp->ether.src, ETH_ADDR_LEN);
			/* If the SEG.PRC is less than the TCB.PRC then continue. */ //TODO
			l_tcb->RCV.NXT = SEG_SEQ + 1;
			l_tcb->IRS = SEG_SEQ;

			l_tcb->SND.WND = SEG_WND;	//TODO Where in RFC?
			l_tcb->SND.WL1 = SEG_SEQ;	//TODO Where in RFC?
			l_tcb->SND.WL2 = SEG_ACK;	//TODO Where in RFC?

			/* <SEQ=ISS><ACK=RCV.NXT><CTL=SYN,ACK> */
			info.seq = l_tcb->ISS;
			info.ack = l_tcb->RCV.NXT;
			info.ctrl = CONTROL_SYN | CONTROL_ACK;

			_tcp_send_package(l_tcb, &info);

			l_tcb->SND.NXT = l_tcb->ISS + 1;
			l_tcb->SND.UNA = l_tcb->ISS;

			NEW_STATE(STATE_SYN_RECEIVED);
			return;
		}
	}
		break;
	case STATE_SYN_RECEIVED:
	case STATE_ESTABLISHED:
	case STATE_FIN_WAIT_1:
	case STATE_FIN_WAIT_2:
	case STATE_CLOSE_WAIT:
	case STATE_CLOSING:
	case STATE_LAST_ACK:
	case STATE_TIME_WAIT: {
		/* first check sequence number *//* Page 69 */
		bool is_acceptable = false;

		DEBUG_PRINTF("RCV.WND=%u, SEG_LEN=%u, RCV.NXT=%u, SEG_SEQ=%u", l_tcb->RCV.WND, SEG_LEN, l_tcb->RCV.NXT, SEG_SEQ);

		if (l_tcb->RCV.WND > 0) {
			if (SEG_LEN == 0) {
				if (bwl(l_tcb->RCV.NXT, SEG_SEQ, l_tcb->RCV.NXT + l_tcb->RCV.WND)) {
					is_acceptable = true;
				}
			} else {
				/* RCV.WND > 0 RCV.WND = 0 */
				if ( bwl(l_tcb->RCV.NXT, SEG_SEQ, l_tcb->RCV.NXT + l_tcb->RCV.WND)
				  || bwl (l_tcb->RCV.NXT, SEG_SEQ + SEG_LEN-1, l_tcb->RCV.NXT+l_tcb->RCV.WND)) {
					is_acceptable = true;
				}
			}
		} else {
			/* RCV.WND = 0 */
			if (SEG_LEN == 0) {
				/*  SEG.SEQ = RCV.NXT */
				if (SEG_SEQ == l_tcb->RCV.NXT) {
					is_acceptable = true;
				}
			}
		}

		DEBUG_PRINTF("is_acceptable=%d", is_acceptable);

		if (!is_acceptable) {
			/* If an incoming segment is not acceptable, an acknowledgment
			 * should be sent in reply (unless the RST bit is set, if so drop
			 * the segment and return):
			 */
			if (p_tcp->tcp.control & CONTROL_RST) {
				_init_tcb(l_tcb, l_tcb->local_port);
				NEW_STATE(STATE_LISTEN);
				return;
			}
			/*  <SEQ=SND.NXT><ACK=RCV.NXT><CTL=ACK> */
			info.seq = l_tcb->SND.NXT;
			info.ack = l_tcb->RCV.NXT;
			info.ctrl = CONTROL_ACK;

			_tcp_send_package(l_tcb, &info);
			return;
		}

		/* second check the RST bit, *//* Page 70 */
		if (p_tcp->tcp.control & CONTROL_RST) {
			switch (l_tcb->state) {
			case STATE_SYN_RECEIVED:
				_init_tcb(l_tcb, l_tcb->local_port);
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
				_init_tcb(l_tcb, l_tcb->local_port);
				break;
			case STATE_CLOSING:
			case STATE_LAST_ACK:
			case STATE_TIME_WAIT:
				/* If the RST bit is set then, enter the CLOSED state, delete the
				 * TCB, and return. */
				_init_tcb(l_tcb, l_tcb->local_port);
				break;
			default:
				assert(0);
				break;
			}
			return;
		}

		/* third check security and precedence */
		/* Nothing todo here */

		/* fourth, check the SYN bit, *//* Page 71 */
		if (p_tcp->tcp.control & CONTROL_SYN) {
			/* RFC 1122 section 4.2.2.20 (e) */
			if (l_tcb->state == STATE_SYN_RECEIVED) {
				NEW_STATE(STATE_LISTEN);
				return;
			}

			_send_reset(p_tcp, l_tcb);
		}

		/*  fifth check the ACK field, *//* Page 72 */
		if (!(p_tcp->tcp.control & CONTROL_ACK)) {
			/* if the ACK bit is off drop the segment and return */
			return;
		}

		switch (l_tcb->state) {
		case STATE_SYN_RECEIVED:
			/*  If SND.UNA =< SEG.ACK =< SND.NXT then enter ESTABLISHED state
			 *and continue processing. */
			if (bwlh (l_tcb->SND.UNA, SEG_ACK, l_tcb->SND.NXT)) {
				/* RFC 1122 section 4.2.2.20 (f) */
				l_tcb->SND.WND = SEG_WND;
				l_tcb->SND.WL1 = SEG_SEQ;
				l_tcb->SND.WL2 = SEG_ACK;

				l_tcb->SND.UNA = SEG_ACK;		// got ACK for SYN

				NEW_STATE(STATE_ESTABLISHED);
				return;
			} else {
				/* <SEQ=SEG.ACK><CTL=RST> */
				_send_reset(p_tcp, l_tcb);
			}
			break;
		case STATE_ESTABLISHED:
		case STATE_FIN_WAIT_1:
		case STATE_FIN_WAIT_2:
		case STATE_CLOSE_WAIT:
		case STATE_CLOSING:
			DEBUG_PRINTF("SND.UNA=%u, SEG_ACK=%u, SND.NXT=%u", l_tcb->SND.UNA, SEG_ACK, l_tcb->SND.NXT);

			if (bwh(l_tcb->SND.UNA, SEG_ACK, l_tcb->SND.NXT)) {

				uint32_t nBytesAck = SEG_ACK - l_tcb->SND.UNA;
				l_tcb->SND.UNA = SEG_ACK;

				if (SEG_ACK == l_tcb->SND.NXT) {
					DEBUG_PUTS("/* all segments are acknowledged */");
				}

				if (l_tcb->state == STATE_FIN_WAIT_1 || l_tcb->state == STATE_CLOSING) {
					nBytesAck--;
					DEBUG_PUTS("/* acknowledged FIN does not count */");
				}

				if (l_tcb->state == STATE_ESTABLISHED && nBytesAck == 1) {
					nBytesAck--;
				}

				/* update send window */
				if ( lt(l_tcb->SND.WL1, SEG_SEQ) || (l_tcb->SND.WL1 == SEG_SEQ && le(l_tcb->SND.WL2, SEG_ACK))) {
					l_tcb->SND.WND = SEG_WND;
					l_tcb->SND.WL1 = SEG_SEQ;
					l_tcb->SND.WL2 = SEG_ACK;
				}
			} else if (le(SEG_ACK, l_tcb->SND.UNA)) { /* RFC 1122 section 4.2.2.20 (g) */
				DEBUG_PUTS("/* ignore duplicate ACK */");
				if (bwlh(l_tcb->SND.UNA, SEG_ACK, l_tcb->SND.NXT)) {
					// ... but update send window
					if ( lt(l_tcb->SND.WL1, SEG_SEQ) || (l_tcb->SND.WL1 == SEG_SEQ && le(l_tcb->SND.WL2, SEG_ACK))) {
						l_tcb->SND.WND = SEG_WND;
						l_tcb->SND.WL1 = SEG_SEQ;
						l_tcb->SND.WL2 = SEG_ACK;
					}
				}
			} else if (gt(SEG_ACK, l_tcb->SND.NXT)) {
				DEBUG_PRINTF("SEG_ACK=%u, SND.NXT=%u", SEG_ACK,l_tcb->SND.NXT);

				info.seq = l_tcb->SND.NXT;
				info.ack = l_tcb->RCV.NXT;
				info.ctrl = CONTROL_ACK;

				_tcp_send_package(l_tcb, &info);
				return;
			}
			break;
		case STATE_LAST_ACK:
			if (SEG_ACK == l_tcb->SND.NXT) { /* if our FIN is now acknowledged */
				_init_tcb(l_tcb, l_tcb->local_port);
			}
			break;
		case STATE_TIME_WAIT:
			if (SEG_ACK == l_tcb->SND.NXT) { /* if our FIN is now acknowledged */
				info.seq = l_tcb->SND.NXT;
				info.ack = l_tcb->RCV.NXT;
				info.ctrl = CONTROL_ACK;

				_tcp_send_package(l_tcb, &info);
				CLIENT_NOT_IMPLEMENTED;
			}
			break;
		default:
			UNEXPECTED_STATE();
			break;
		}

		/*sixth, check the URG bit,*/
		/* Nothing todo here */

		/* seventh, process the segment text, */
		switch (l_tcb->state) {
		case STATE_ESTABLISHED:
		case STATE_FIN_WAIT_1:
		case STATE_FIN_WAIT_2:
			if (data_length > 0) {
				if (SEG_SEQ == l_tcb->RCV.NXT) {
					const uint32_t entry = s_recv_queue[connection_index].queue_head;
					struct queue_entry *p_queue_entry = &s_recv_queue[connection_index].entries[entry];
					memcpy(p_queue_entry->data, (uint8_t*) ((uint8_t*) &p_tcp->tcp + data_offset), data_length);
					p_queue_entry->size = data_length;

					l_tcb->RCV.NXT += data_length;
					l_tcb->RCV.WND -= TCP_DATA_SIZE;

					info.seq = l_tcb->SND.NXT;
					info.ack = l_tcb->RCV.NXT;
					info.ctrl = CONTROL_ACK;

					_tcp_send_package(l_tcb, &info);

					s_recv_queue[connection_index].queue_head = (s_recv_queue[connection_index].queue_head + 1) & TCP_RX_MAX_ENTRIES_MASK;
				} else {
					info.seq = l_tcb->SND.NXT;
					info.ack = l_tcb->RCV.NXT;
					info.ctrl = CONTROL_ACK;

					_tcp_send_package(l_tcb, &info);
					DEBUG_PUTS("Out of order");
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

		if ((l_tcb->state == STATE_CLOSED) || (l_tcb->state == STATE_LISTEN)  || (l_tcb->state == STATE_SYN_SENT) ) {
			return;
		}

		if (!(p_tcp->tcp.control & CONTROL_FIN)) {
			return ;
		}

		/*
		 If the FIN bit is set, signal the user "connection closing" and
		 return any pending RECEIVEs with same message, advance RCV.NXT
		 over the FIN, and send an acknowledgment for the FIN.  Note that
		 FIN implies PUSH for any segment text not yet delivered to the
		 user.
		 */

		l_tcb->RCV.NXT = l_tcb->RCV.NXT + 1;
		info.seq = l_tcb->SND.NXT;
		info.ack = l_tcb->RCV.NXT;
		info.ctrl = CONTROL_ACK;

		_tcp_send_package(l_tcb, &info);

		switch (l_tcb->state) {
		case STATE_SYN_RECEIVED:
		case STATE_ESTABLISHED:
			NEW_STATE(STATE_CLOSE_WAIT);
			break;
		case STATE_FIN_WAIT_1:
			/*
			 If our FIN has been ACKed (perhaps in this segment), then
			 enter TIME-WAIT, start the time-wait timer, turn off the other
			 timers; otherwise enter the CLOSING state.
			 */
			if (SEG_ACK == l_tcb->SND.NXT) { /* if our FIN is now acknowledged */
				NEW_STATE(STATE_TIME_WAIT);
				CLIENT_NOT_IMPLEMENTED;
			} else {
				NEW_STATE(STATE_CLOSING);
			}
			break;
		case STATE_FIN_WAIT_2:
			/*
			 Enter the TIME-WAIT state.  Start the time-wait timer, turn off the other timers.
			 */
			NEW_STATE(STATE_TIME_WAIT);
			CLIENT_NOT_IMPLEMENTED;
			break;
		case STATE_CLOSE_WAIT:
			/* Remain in the CLOSE-WAIT state. */
		case STATE_CLOSING:
			/* Remain in the CLOSING state. */
		case STATE_LAST_ACK:
			/* Remain in the LAST-ACK state. */
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

// -->

int tcp_begin(uint16_t local_port) {
	int i;
	DEBUG_PRINTF("local_port=%u", local_port);

	for (i = 0; i < TCP_MAX_CONNECTIONS_ALLOWED; i++) {
		if (s_tcb[i].local_port == local_port) {
			if (s_tcb[i].state == STATE_CLOSED) {
				/* Create a new transmission control block (TCB)
				 * to hold connection state information. */
				break;
			}
		}

		if (s_tcb[i].local_port == 0) {
			break;
		}
	}

	if (i == TCP_MAX_CONNECTIONS_ALLOWED) {
		console_error("tcp_begin: too many connections");
		return -1;
	}

	s_tcb[i].local_port = local_port;

	DEBUG_PRINTF("i=%d, local_port=%d[%x]", i, local_port, local_port);

	/* create transmission control block (TCB) */
	_init_tcb(&s_tcb[i], local_port);
	return i;
}

void tcp_write(int handle, const uint8_t *buffer, uint16_t length) {
	struct send_info info;
	struct tcb *l_tcb = &s_tcb[handle];

	length = MIN(length, TCP_DATA_SIZE);

	l_tcb->TX.data = (uint8_t *)buffer;
	l_tcb->TX.size = length;

	info.seq = l_tcb->SND.NXT;
	info.ack = l_tcb->RCV.NXT;
	info.ctrl = CONTROL_ACK | CONTROL_PSH;

	_tcp_send_package(l_tcb, &info);

	l_tcb->TX.data = NULL;
	l_tcb->TX.size = 0;

	l_tcb->SND.NXT += length;
}

uint16_t tcp_read(int handle, const uint8_t **p) {
	struct send_info info;
	uint16_t size;
	assert(handle >= 0);
	assert(handle < TCP_MAX_CONNECTIONS_ALLOWED);

	struct tcb *l_tcb = &s_tcb[handle];

	if (l_tcb->state == STATE_CLOSE_WAIT) {
		info.seq = l_tcb->SND.NXT;
		info.ack = l_tcb->RCV.NXT;
		info.ctrl = CONTROL_FIN | CONTROL_ACK;

		_tcp_send_package(l_tcb, &info);

		NEW_STATE(STATE_LAST_ACK);

		l_tcb->SND.NXT++;
		return 0;
	}

	if (s_recv_queue[handle].queue_head == s_recv_queue[handle].queue_tail) {
		return 0;
	}

	const uint32_t entry = s_recv_queue[handle].queue_tail;
	const struct queue_entry *p_queue_entry = &s_recv_queue[handle].entries[entry];

	*p = p_queue_entry->data;
	size = p_queue_entry->size;

	l_tcb->RCV.WND += TCP_DATA_SIZE;

	s_recv_queue[handle].queue_tail = (s_recv_queue[handle].queue_tail + 1) & TCP_RX_MAX_ENTRIES_MASK;

	return size;
}

// <---
