/**
 * @file w5x00_netinfo.c
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

#include "w5x00.h"

#define IPSTR "%d.%d.%d.%d"
#define MACSTR "%.2x:%.2x:%.2x:%.2x:%.2x:%.2x"

void w5x00_netinfo_set(wiz_NetInfo *netinfo, const uint8_t *pMacAddress, const struct ip_info *pIpInfo) {
	assert(netinfo != 0);
	assert(pMacAddress != 0);
	assert(pIpInfo != 0);

	unsigned i;
	for (i = 0; i < 6; i++) {
		netinfo->mac[i] = pMacAddress[i];
	}

	netinfo->ip[0] = pIpInfo->ip.addr & 0xFF;
	netinfo->ip[1] = (pIpInfo->ip.addr >> 8) & 0xFF;
	netinfo->ip[2] = (pIpInfo->ip.addr >> 16) & 0xFF;
	netinfo->ip[3] = (pIpInfo->ip.addr >> 24) & 0xFF;

	netinfo->sn[0] = pIpInfo->netmask.addr & 0xFF;
	netinfo->sn[1] = (pIpInfo->netmask.addr >> 8) & 0xFF;
	netinfo->sn[2] = (pIpInfo->netmask.addr >> 16) & 0xFF;
	netinfo->sn[3] = (pIpInfo->netmask.addr >> 24) & 0xFF;

	netinfo->gw[0] = pIpInfo->gw.addr & 0xFF;
	netinfo->gw[1] = (pIpInfo->gw.addr >> 8) & 0xFF;
	netinfo->gw[2] = (pIpInfo->gw.addr >> 16) & 0xFF;
	netinfo->gw[3] = (pIpInfo->gw.addr >> 24) & 0xFF;
}

void w5x00_netinfo_dump(const wiz_NetInfo *netinfo) {
	assert(netinfo != 0);
#ifndef NDEBUG
	char chip_id[16] = { '\0' };
	ctlwizchip(CW_GET_ID, chip_id);

	printf("%s\n", chip_id);
	printf(" Interface  : " IPSTR "\n", netinfo->ip[0], netinfo->ip[1],netinfo->ip[2],netinfo->ip[3]);
	printf(" Netmask    : " IPSTR "\n", netinfo->sn[0], netinfo->sn[1],netinfo->sn[2],netinfo->sn[3]);
	printf(" Gateway    : " IPSTR "\n", netinfo->gw[0], netinfo->gw[1],netinfo->gw[2],netinfo->gw[3]);
	printf(" MacAddress : " MACSTR "\n", netinfo->mac[0], netinfo->mac[1],netinfo->mac[2],netinfo->mac[3],netinfo->mac[4],netinfo->mac[5]);
	printf(" DHCP       : %s\n", netinfo->dhcp == NETINFO_DHCP ? "Yes" : "No");
#endif
}
