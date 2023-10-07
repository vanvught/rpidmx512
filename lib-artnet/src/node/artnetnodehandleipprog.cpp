/**
 * @file artnetnodehandleipprog.cpp
 *
 */
/**
 * Art-Net Designed by and Copyright Artistic Licence Holdings Ltd.
 */
/* Copyright (C) 2021-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "artnetnode.h"
#include "network.h"

#include "debug.h"

static constexpr uint8_t COMMAND_ENABLE_PROGRAMMING = (1U << 7);
static constexpr uint8_t COMMAND_ENABLE_DHCP		= ((1U << 6) | COMMAND_ENABLE_PROGRAMMING);
static constexpr uint8_t COMMAND_PROGRAM_GATEWAY	= ((1U << 4) | COMMAND_ENABLE_PROGRAMMING);
static constexpr uint8_t COMMAND_SET_TO_DEFAULT		= ((1U << 3) | COMMAND_ENABLE_PROGRAMMING);
static constexpr uint8_t COMMAND_PROGRAM_IPADDRESS	= ((1U << 2) | COMMAND_ENABLE_PROGRAMMING);
static constexpr uint8_t COMMAND_PROGRAM_SUBNETMASK	= ((1U << 1) | COMMAND_ENABLE_PROGRAMMING);

union uip {
	uint32_t u32;
	uint8_t u8[4];
} static ip;

void ArtNetNode::HandleIpProg() {
	DEBUG_ENTRY

	const auto *const pArtIpProg = reinterpret_cast<artnet::ArtIpProg *>(m_pReceiveBuffer);
	const auto nCommand = pArtIpProg->Command;
	auto *pArtIpProgReply = reinterpret_cast<artnet::ArtIpProgReply *>(m_pReceiveBuffer);
	const auto isDhcp = Network::Get()->IsDhcpUsed();

	pArtIpProgReply->OpCode = static_cast<uint16_t>(artnet::OpCodes::OP_IPPROGREPLY);

	if ((nCommand & COMMAND_ENABLE_DHCP) == COMMAND_ENABLE_DHCP) {
		Network::Get()->EnableDhcp();
	}

	if ((nCommand & COMMAND_SET_TO_DEFAULT) == COMMAND_SET_TO_DEFAULT) {
		Network::Get()->SetIp(0);
	}

	if ((nCommand & COMMAND_PROGRAM_IPADDRESS) == COMMAND_PROGRAM_IPADDRESS) {
		memcpy(ip.u8, &pArtIpProg->ProgIpHi, artnet::IP_SIZE);
		Network::Get()->SetIp(ip.u32);
	}

	if ((nCommand & COMMAND_PROGRAM_SUBNETMASK) == COMMAND_PROGRAM_SUBNETMASK) {
		memcpy(ip.u8, &pArtIpProg->ProgSmHi, artnet::IP_SIZE);
		Network::Get()->SetNetmask(ip.u32);
	}

	if ((nCommand & COMMAND_PROGRAM_GATEWAY) == COMMAND_PROGRAM_GATEWAY) {
		memcpy(ip.u8, &pArtIpProg->ProgGwHi, artnet::IP_SIZE);
		Network::Get()->SetGatewayIp(ip.u32);
	}

	if (Network::Get()->IsDhcpUsed()) {
		pArtIpProgReply->Status = (1U << 6);
	} else {
		pArtIpProgReply->Status = 0;
	}

	pArtIpProgReply->Spare2 = 0;

	auto isChanged = (isDhcp != Network::Get()->IsDhcpUsed());

	ip.u32 = Network::Get()->GetIp();
	isChanged |= (memcmp(&pArtIpProg->ProgIpHi, ip.u8, artnet::IP_SIZE) != 0);
	memcpy(&pArtIpProgReply->ProgIpHi, ip.u8, artnet::IP_SIZE);

	ip.u32 = Network::Get()->GetNetmask();
	isChanged |= (memcmp(&pArtIpProg->ProgSmHi, ip.u8, artnet::IP_SIZE) != 0);
	memcpy(&pArtIpProgReply->ProgSmHi, ip.u8, artnet::IP_SIZE);

	ip.u32 = Network::Get()->GetGatewayIp();
	isChanged |= (memcmp(&pArtIpProg->ProgGwHi, ip.u8, artnet::IP_SIZE) != 0);
	memcpy(&pArtIpProgReply->ProgGwHi, ip.u8, artnet::IP_SIZE);

	pArtIpProgReply->Spare7 = 0;
	pArtIpProgReply->Spare8 = 0;

	Network::Get()->SendTo(m_nHandle, m_pReceiveBuffer, sizeof(struct artnet::ArtIpProgReply), m_nIpAddressFrom, artnet::UDP_PORT);

	if (isChanged) {
		m_ArtPollReply.Status2 = static_cast<uint8_t>((m_ArtPollReply.Status2 & (~(artnet::Status2::IP_DHCP))) | (Network::Get()->IsDhcpUsed() ? artnet::Status2::IP_DHCP : artnet::Status2::IP_MANUALY));

		memcpy(m_ArtPollReply.IPAddress, &pArtIpProgReply->ProgIpHi, artnet::IP_SIZE);
#if (ARTNET_VERSION >= 4)
		memcpy(m_ArtPollReply.BindIp, &pArtIpProgReply->ProgIpHi, artnet::IP_SIZE);
#endif
		if (m_State.SendArtPollReplyOnChange) {
			SendPollRelply(0, m_nIpAddressFrom);
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
