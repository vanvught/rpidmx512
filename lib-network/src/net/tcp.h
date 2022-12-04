/**
 * @file tcp.h
 *
 */
/* Copyright (C) 2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef TCP_H_
#define TCP_H_

#include <stdint.h>

#include "net_packets.h"

#include "../config/net_config.h"

#define TCP_RX_MSS						(TCP_DATA_SIZE)
#define TCP_RX_MAX_ENTRIES				(1U << 1) // Must always be a power of 2
#define TCP_RX_MAX_ENTRIES_MASK			(TCP_RX_MAX_ENTRIES - 1)
#define TCP_MAX_RX_WND 					(TCP_RX_MAX_ENTRIES * TCP_RX_MSS);

/**
 * Transmission control block (TCB)
 */
struct tcb {
	uint16_t local_port;

	uint16_t remotept;
	uint32_t remoteip;
	uint8_t remoteeth[ETH_ADDR_LEN];

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


struct send_info {
	uint32_t seq;
	uint32_t ack;
	uint8_t ctrl;
};

#endif /* TCP_H_ */
