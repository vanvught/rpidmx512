/**
 * @file artnetnodehandleipprog.cpp
 *
 */
/**
 * Art-Net Designed by and Copyright Artistic Licence Holdings Ltd.
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

#include <cstring>
#include <cassert>
#if defined (__linux__)
# include <unistd.h>
#endif

#include "artnetnode.h"
#include "network.h"

#include "debug.h"

#define IPPROG_COMMAND_NONE					0
#define IPPROG_COMMAND_ENABLE_PROGRAMMING	(1 << 7)
#define IPPROG_COMMAND_ENABLE_DHCP			((1 << 6) | IPPROG_COMMAND_ENABLE_PROGRAMMING)
#define IPPROG_COMMAND_PROGRAM_GATEWAY		((1 << 4) | IPPROG_COMMAND_ENABLE_PROGRAMMING)	///< Not documented!
#define IPPROG_COMMAND_SET_TO_DEFAULT		((1 << 3) | IPPROG_COMMAND_ENABLE_PROGRAMMING)
#define IPPROG_COMMAND_PROGRAM_IPADDRESS	((1 << 2) | IPPROG_COMMAND_ENABLE_PROGRAMMING)
#define IPPROG_COMMAND_PROGRAM_SUBNETMASK	((1 << 1) | IPPROG_COMMAND_ENABLE_PROGRAMMING)
#define IPPROG_COMMAND_PROGRAM_PORT			((1 << 0) | IPPROG_COMMAND_ENABLE_PROGRAMMING)

using namespace artnet;

union uip {
	uint32_t u32;
	uint8_t u8[4];
} static ip;

void ArtNetNode::HandleIpProg() {
	DEBUG_ENTRY
#if defined (__linux__)
	if (getuid() != 0) {
		DEBUG_EXIT
		return;
	}
#endif

	const auto *pArtIpProg = &(m_ArtNetPacket.ArtPacket.ArtIpProg);
	const auto nCommand = pArtIpProg->Command;
	auto *pArtIpProgReply = &(m_ArtNetPacket.ArtPacket.ArtIpProgReply);
	const auto isDhcp = Network::Get()->IsDhcpUsed();

	pArtIpProgReply->OpCode = OP_IPPROGREPLY;

	if ((nCommand & IPPROG_COMMAND_ENABLE_DHCP) == IPPROG_COMMAND_ENABLE_DHCP) {
		Network::Get()->EnableDhcp();
	}

	if ((nCommand & IPPROG_COMMAND_SET_TO_DEFAULT) == IPPROG_COMMAND_SET_TO_DEFAULT) {
		Network::Get()->SetIp(0);
	}

	if ((nCommand & IPPROG_COMMAND_PROGRAM_IPADDRESS) == IPPROG_COMMAND_PROGRAM_IPADDRESS) {
		memcpy(ip.u8, &pArtIpProg->ProgIpHi, ArtNet::IP_SIZE);
		Network::Get()->SetIp(ip.u32);
	}

	if ((nCommand & IPPROG_COMMAND_PROGRAM_SUBNETMASK) == IPPROG_COMMAND_PROGRAM_SUBNETMASK) {
		memcpy(ip.u8, &pArtIpProg->ProgSmHi, ArtNet::IP_SIZE);
		Network::Get()->SetNetmask(ip.u32);
	}

	if ((nCommand & IPPROG_COMMAND_PROGRAM_GATEWAY) == IPPROG_COMMAND_PROGRAM_GATEWAY) {
		memcpy(ip.u8, &pArtIpProg->ProgGwHi, ArtNet::IP_SIZE);
		Network::Get()->SetGatewayIp(ip.u32);
	}

	if (Network::Get()->IsDhcpUsed()) {
		pArtIpProgReply->Status = (1 << 6);
	} else {
		pArtIpProgReply->Status = 0;
	}

	pArtIpProgReply->Spare2 = 0;

	auto isChanged = (isDhcp != Network::Get()->IsDhcpUsed());

	ip.u32 = Network::Get()->GetIp();
	isChanged |= (memcmp(&pArtIpProg->ProgIpHi, ip.u8, ArtNet::IP_SIZE) != 0);
	memcpy(&pArtIpProgReply->ProgIpHi, ip.u8, ArtNet::IP_SIZE);

	ip.u32 = Network::Get()->GetNetmask();
	isChanged |= (memcmp(&pArtIpProg->ProgSmHi, ip.u8, ArtNet::IP_SIZE) != 0);
	memcpy(&pArtIpProgReply->ProgSmHi, ip.u8, ArtNet::IP_SIZE);

	ip.u32 = Network::Get()->GetGatewayIp();
	isChanged |= (memcmp(&pArtIpProg->ProgGwHi, ip.u8, ArtNet::IP_SIZE) != 0);
	memcpy(&pArtIpProgReply->ProgGwHi, ip.u8, ArtNet::IP_SIZE);

	pArtIpProgReply->Spare7 = 0;
	pArtIpProgReply->Spare8 = 0;

	Network::Get()->SendTo(m_nHandle, &(m_ArtNetPacket.ArtPacket.ArtIpProgReply), sizeof(struct TArtIpProgReply), m_ArtNetPacket.IPAddressFrom, ArtNet::UDP_PORT);

	if (isChanged) {
		// Update Node network details
		m_Node.IPAddressLocal = Network::Get()->GetIp();
		m_Node.IPAddressBroadcast = m_Node.IPAddressLocal | ~(Network::Get()->GetNetmask());
		m_Node.Status2 = static_cast<uint8_t>((m_Node.Status2 & (~(Status2::IP_DHCP))) | (Network::Get()->IsDhcpUsed() ? Status2::IP_DHCP : Status2::IP_MANUALY));
		// Update PollReply for new IPAddress
		memcpy(m_PollReply.IPAddress, &pArtIpProgReply->ProgIpHi, ArtNet::IP_SIZE);
		if (ArtNet::VERSION > 3) {
			memcpy(m_PollReply.BindIp, &pArtIpProgReply->ProgIpHi, ArtNet::IP_SIZE);
		}

		if (m_State.SendArtPollReplyOnChange) {
			SendPollRelply(true);
		}

#ifndef NDEBUG
		DEBUG_PUTS("Changed:");
		Network::Get()->Print();
#endif
	}
#ifndef NDEBUG
	else {
		DEBUG_PUTS("No changes");
	}
#endif

	DEBUG_EXIT
}
