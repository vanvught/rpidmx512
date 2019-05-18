/**
 * @file artnetipprog.cpp
 *
 */
/**
 * Art-Net Designed by and Copyright Artistic Licence Holdings Ltd.
 */
/* Copyright (C) 2017-2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#include <assert.h>

#include "artnetipprog.h"

#include "artnetnode.h"
#include "network.h"

union uip {
	uint32_t u32;
	uint8_t u8[4];
} static ip;


ArtNetIpProg::~ArtNetIpProg(void) {
}

void ArtNetNode::SetIpProgHandler(ArtNetIpProg *pArtNetIpProg) {
	assert(pArtNetIpProg != 0);

	if(pArtNetIpProg != 0) {
		m_pArtNetIpProg = pArtNetIpProg;

		m_pIpProgReply = new TArtIpProgReply;
		assert(m_pIpProgReply != 0);

		if (m_pIpProgReply != 0) {
			memset(m_pIpProgReply, 0, sizeof(struct TArtIpProgReply));

			memcpy(m_pIpProgReply->Id, (const char *) NODE_ID, sizeof(m_pIpProgReply->Id));
			m_pIpProgReply->OpCode = OP_IPPROGREPLY;
			m_pIpProgReply->ProtVerLo = ARTNET_PROTOCOL_REVISION;
		} else {
			m_pArtNetIpProg = 0;
		}
	}
}

void ArtNetNode::HandleIpProg(void) {
	struct TArtIpProg *packet = (struct TArtIpProg *) &(m_ArtNetPacket.ArtPacket.ArtIpProg);

	m_pArtNetIpProg->Handler((const TArtNetIpProg *) &packet->Command, (TArtNetIpProgReply *) &m_pIpProgReply->ProgIpHi);

	Network::Get()->SendTo(m_nHandle, (const uint8_t *) m_pIpProgReply, (uint16_t) sizeof(struct TArtIpProgReply), m_ArtNetPacket.IPAddressFrom, (uint16_t) ARTNET_UDP_PORT);

	memcpy(ip.u8, &m_pIpProgReply->ProgIpHi, ARTNET_IP_SIZE);

	if (ip.u32 != m_Node.IPAddressLocal) {
		// Update Node network details
		m_Node.IPAddressLocal = Network::Get()->GetIp();
		m_Node.IPAddressBroadcast = m_Node.IPAddressLocal | ~(Network::Get()->GetNetmask());
		m_Node.Status2 = (m_Node.Status2 & ~(STATUS2_IP_DHCP)) | (Network::Get()->IsDhcpUsed() ? STATUS2_IP_DHCP : STATUS2_IP_MANUALY);
		// Update PollReply for new IPAddress
		memcpy(m_PollReply.IPAddress, &m_pIpProgReply->ProgIpHi, ARTNET_IP_SIZE);
		if (m_nVersion > 3) {
			memcpy(m_PollReply.BindIp, &m_pIpProgReply->ProgIpHi, ARTNET_IP_SIZE);
		}

		if (m_State.SendArtPollReplyOnChange) {
			SendPollRelply(true);
		}
	}
}
