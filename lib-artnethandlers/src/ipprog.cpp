/**
 * @file ipprog.cpp
 *
 */
/* Copyright (C) 2018-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <stddef.h>
#ifndef NDEBUG
 #include <stdio.h>
#endif
#include <string.h>
#include <cassert>

#include "artnetipprog.h"
#include "artnet.h"

#include "ipprog.h"

#include "network.h"

union uip {
	uint32_t u32;
	uint8_t u8[4];
} static ip_union;

IpProg::IpProg() {
}

void IpProg::Handler(const struct TArtNetIpProg *pArtNetIpProg, struct TArtNetIpProgReply *pArtNetIpProgReply) {
	// Ip
	ip_union.u32 = Network::Get()->GetIp();
	memcpy(&pArtNetIpProgReply->ProgIpHi, ip_union.u8, ArtNet::IP_SIZE);
	// Netmask
	ip_union.u32 = Network::Get()->GetNetmask();
	memcpy(&pArtNetIpProgReply->ProgSmHi, ip_union.u8, ArtNet::IP_SIZE);
	// Port
	pArtNetIpProgReply->ProgPortHi = (ArtNet::UDP_PORT >> 8);
	pArtNetIpProgReply->ProgPortLo = ArtNet::UDP_PORT & 0xFF;
	//
	pArtNetIpProgReply->Filler = 0;
	// Gateway
	ip_union.u32 = Network::Get()->GetGatewayIp();
	memcpy(&pArtNetIpProgReply->ProgGwHi, ip_union.u8, ArtNet::IP_SIZE);

#ifndef NDEBUG
	printf("IpProg::Handler, Command = %d\n", pArtNetIpProg->Command);
	printf("\tIP : " IPSTR "\n", IP2STR(Network::Get()->GetIp()));
	printf("\tNetmask : " IPSTR "\n", IP2STR(Network::Get()->GetNetmask()));
	printf("\tGateway : " IPSTR "\n", IP2STR(Network::Get()->GetGatewayIp()));
#endif

	if (pArtNetIpProg->Command == 0) {
		// Nothing to do
	}

	if ((pArtNetIpProg->Command & IPPROG_COMMAND_ENABLE_DHCP) == IPPROG_COMMAND_ENABLE_DHCP) {
		if (!Network::Get()->EnableDhcp()) {
			pArtNetIpProgReply->Status = 0; // DHCP Disabled;
		} else {
			pArtNetIpProgReply->Status = (1 << 6); // DHCP Enabled

			ip_union.u32 = Network::Get()->GetIp();
			memcpy(&pArtNetIpProgReply->ProgIpHi, ip_union.u8, ArtNet::IP_SIZE);
			ip_union.u32 = Network::Get()->GetNetmask();
			memcpy(&pArtNetIpProgReply->ProgSmHi, ip_union.u8, ArtNet::IP_SIZE);
			ip_union.u32 = Network::Get()->GetGatewayIp();
			memcpy(&pArtNetIpProgReply->ProgGwHi, ip_union.u8, ArtNet::IP_SIZE);
		}
	}

	if ((pArtNetIpProg->Command & IPPROG_COMMAND_SET_TO_DEFAULT) == IPPROG_COMMAND_SET_TO_DEFAULT) {
		Network::Get()->SetIp(0);

		ip_union.u32 = Network::Get()->GetIp();
		memcpy(&pArtNetIpProgReply->ProgIpHi, ip_union.u8, ArtNet::IP_SIZE);
		ip_union.u32 = Network::Get()->GetNetmask();
		memcpy(&pArtNetIpProgReply->ProgSmHi, ip_union.u8, ArtNet::IP_SIZE);
		ip_union.u32 = Network::Get()->GetGatewayIp();
		memcpy(&pArtNetIpProgReply->ProgGwHi, ip_union.u8, ArtNet::IP_SIZE);

		pArtNetIpProgReply->Status = 0; // DHCP Disabled;
	}

	if ((pArtNetIpProg->Command & IPPROG_COMMAND_PROGRAM_IPADDRESS) == IPPROG_COMMAND_PROGRAM_IPADDRESS) {
		// Get IPAddress from IpProg
		memcpy(ip_union.u8, &pArtNetIpProg->ProgIpHi, ArtNet::IP_SIZE);

		Network::Get()->SetIp(ip_union.u32);

		// Set IPAddress in IpProgReply
		memcpy(&pArtNetIpProgReply->ProgIpHi, &pArtNetIpProg->ProgIpHi, ArtNet::IP_SIZE);
		pArtNetIpProgReply->Status = 0; // DHCP Disabled;
	}

	if ((pArtNetIpProg->Command & IPPROG_COMMAND_PROGRAM_SUBNETMASK) == IPPROG_COMMAND_PROGRAM_SUBNETMASK) {
		// Get SubnetMask from IpProg
		memcpy(ip_union.u8, &pArtNetIpProg->ProgSmHi, ArtNet::IP_SIZE);

		Network::Get()->SetNetmask(ip_union.u32);

		// Set SubnetMask in IpProgReply
		memcpy(&pArtNetIpProgReply->ProgSmHi, &pArtNetIpProg->ProgSmHi, ArtNet::IP_SIZE);
	}

	if ((pArtNetIpProg->Command & IPPROG_COMMAND_PROGRAM_GATEWAY) == IPPROG_COMMAND_PROGRAM_GATEWAY) {
		// FIXME Remove when Gateway supported
	}

#ifndef NDEBUG
	printf("\tIP : " IPSTR "\n", IP2STR(Network::Get()->GetIp()));
	printf("\tNetmask : " IPSTR "\n", IP2STR(Network::Get()->GetNetmask()));
	printf("\tGateway : " IPSTR "\n", IP2STR(Network::Get()->GetGatewayIp()));
#endif
}
