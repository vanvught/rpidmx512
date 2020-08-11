/**
 * @file artnetipprog.cpp
 *
 */
/**
 * Art-Net Designed by and Copyright Artistic Licence Holdings Ltd.
 */
/* Copyright (C) 2017-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <string.h>
#include <cassert>

#include "artnetipprog.h"

#include "artnetnode.h"
#include "network.h"

union uip {
	uint32_t u32;
	uint8_t u8[4];
} static ip;

void ArtNetNode::SetIpProgHandler(ArtNetIpProg *pArtNetIpProg) {
	assert(pArtNetIpProg != nullptr);

	if(pArtNetIpProg != nullptr) {
		m_pArtNetIpProg = pArtNetIpProg;

		m_pIpProgReply = new TArtIpProgReply;
		assert(m_pIpProgReply != nullptr);

		if (m_pIpProgReply != nullptr) {
			memset(m_pIpProgReply, 0, sizeof(struct TArtIpProgReply));

			memcpy(m_pIpProgReply->Id, artnet::NODE_ID, sizeof(m_pIpProgReply->Id));
			m_pIpProgReply->OpCode = OP_IPPROGREPLY;
			m_pIpProgReply->ProtVerLo = ArtNet::PROTOCOL_REVISION;
		} else {
			m_pArtNetIpProg = nullptr;
		}
	}
}

void ArtNetNode::HandleIpProg() {
	struct TArtIpProg *packet = &(m_ArtNetPacket.ArtPacket.ArtIpProg);

	m_pArtNetIpProg->Handler(reinterpret_cast<const TArtNetIpProg*>(&packet->Command), reinterpret_cast<TArtNetIpProgReply*>(&m_pIpProgReply->ProgIpHi));

	Network::Get()->SendTo(m_nHandle, m_pIpProgReply, sizeof(struct TArtIpProgReply), m_ArtNetPacket.IPAddressFrom, ArtNet::UDP_PORT);

	memcpy(ip.u8, &m_pIpProgReply->ProgIpHi, ArtNet::IP_SIZE);

	if (ip.u32 != m_Node.IPAddressLocal) {
		// Update Node network details
		m_Node.IPAddressLocal = Network::Get()->GetIp();
		m_Node.IPAddressBroadcast = m_Node.IPAddressLocal | ~(Network::Get()->GetNetmask());
		m_Node.Status2 = (m_Node.Status2 & (~(ArtNetStatus2::IP_DHCP))) | (Network::Get()->IsDhcpUsed() ? ArtNetStatus2::IP_DHCP : ArtNetStatus2::IP_MANUALY);
		// Update PollReply for new IPAddress
		memcpy(m_PollReply.IPAddress, &m_pIpProgReply->ProgIpHi, ArtNet::IP_SIZE);
		if (m_nVersion > 3) {
			memcpy(m_PollReply.BindIp, &m_pIpProgReply->ProgIpHi, ArtNet::IP_SIZE);
		}

		if (m_State.SendArtPollReplyOnChange) {
			SendPollRelply(true);
		}
	}
}
