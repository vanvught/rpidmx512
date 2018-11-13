/**
 * @file w5x00_join_group.c
 *
 */
/* Copyright (C) 2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include <stdint.h>
#include <stdio.h>
#include <assert.h>

#include "wizchip_conf.h"
#include "socket.h"

#include "debug.h"

#define IPSTR "%d.%d.%d.%d"
#define MACSTR "%.2x:%.2x:%.2x:%.2x:%.2x:%.2x"

int8_t w5x00_join_group(uint8_t sn, uint32_t ip, uint16_t port) {
	DEBUG_ENTRY
	DEBUG_PUTS(_WIZCHIP_ID_);

	uint8_t multicast_ip[4];
	uint8_t multicast_mac[6];

	multicast_ip[0] = ip & 0xFF;
	multicast_ip[1] = (ip >> 8) & 0xFF;
	multicast_ip[2] = (ip >> 16) & 0xFF;
	multicast_ip[3] = (ip >> 24) & 0xFF;

	multicast_mac[0] = 0x01;
	multicast_mac[1] = 0x00;
	multicast_mac[2] = 0x5E;
	multicast_mac[3] = multicast_ip[1] & 0x7F;
	multicast_mac[4] = multicast_ip[2];
	multicast_mac[5] = multicast_ip[3];

#ifndef NDEBUG
	printf("Multicast-Group H/W address : " MACSTR "\n", multicast_mac[0], multicast_mac[1],multicast_mac[2],multicast_mac[3],multicast_mac[4],multicast_mac[5]);
	printf("Multicast-Group IP address : " IPSTR "\n", multicast_ip[0], multicast_ip[1],multicast_ip[2],multicast_ip[3]);
	printf("Multicast-GroupPort number : %d\n", (int) port);
#endif

	/* set Multicast-Group information */
	setSn_DHAR(sn, multicast_mac);	/* set Multicast-Group H/W address */
	setSn_DIPR(sn, multicast_ip);	/* set Multicast-Group IP address */
	setSn_DPORT(sn, port);			/* set Multicast-GroupPort number */

	const uint8_t sd = _socket(sn, Sn_MR_UDP, port, SF_IO_NONBLOCK | Sn_MR_MULTI);

	if (sd < 0) {
		printf("_socket(0, Sn_MR_UDP, %d, SF_IO_NONBLOCK | Sn_MR_MULTI)=%d\n", (int)port, (int) sd);
	}

	return sd;

	DEBUG_EXIT
}
