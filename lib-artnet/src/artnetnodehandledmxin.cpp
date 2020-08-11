/**
 * @file artnetnodehandledmxin.cpp
 *
 */
/**
 * Art-Net Designed by and Copyright Artistic Licence Holdings Ltd.
 */
/* Copyright (C) 2019 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <string.h>
#include <cassert>

#include "artnetnode.h"
#include "artnet.h"
#include "artnetdmx.h"

#include "network.h"
#include "hardware.h"

#include "debug.h"

void ArtNetNode::SetDestinationIp(uint8_t nPortIndex, uint32_t nDestinationIp) {
	if (nPortIndex < ARTNET_NODE_MAX_PORTS_INPUT) {
		m_InputPorts[nPortIndex].nDestinationIp = nDestinationIp;
		DEBUG_PRINTF("m_nDestinationIp=" IPSTR ", Netmask=" IPSTR, IP2STR(m_InputPorts[nPortIndex].nDestinationIp), IP2STR(Network::Get()->GetNetmask()));
	}
}

void ArtNetNode::HandleDmxIn() {
	struct TArtDmx tArtDmx;

	memcpy(tArtDmx.Id, artnet::NODE_ID, sizeof m_PollReply.Id);
	tArtDmx.OpCode = OP_DMX;
	tArtDmx.ProtVerHi = 0;
	tArtDmx.ProtVerLo = ArtNet::PROTOCOL_REVISION;

	for (uint32_t i = 0; i < ArtNet::MAX_PORTS; i++) {
		uint32_t nUpdatesPerSecond;

		if (m_InputPorts[i].bIsEnabled){
			uint16_t nLength;
			const uint8_t *pDmxData = m_pArtNetDmx->Handler(i, nLength, nUpdatesPerSecond);

			if (pDmxData != nullptr) {
				tArtDmx.Sequence = 1 + m_InputPorts[i].nSequence++;
				tArtDmx.Physical = i;
				tArtDmx.PortAddress = m_InputPorts[i].port.nPortAddress;
				tArtDmx.LengthHi = (nLength & 0xFF00) >> 8;
				tArtDmx.Length = (nLength & 0xFF);

				memcpy(tArtDmx.Data, pDmxData, nLength);

				m_InputPorts[i].port.nStatus = GI_DATA_RECIEVED;

				Network::Get()->SendTo(m_nHandle, &tArtDmx, sizeof(struct TArtDmx), m_InputPorts[i].nDestinationIp, ArtNet::UDP_PORT);

				m_State.bIsReceivingDmx = true;
			} else {
				if ((m_InputPorts[i].port.nStatus & GO_DATA_IS_BEING_TRANSMITTED) == GO_DATA_IS_BEING_TRANSMITTED) {
					if (nUpdatesPerSecond == 0) {
						m_InputPorts[i].port.nStatus = m_InputPorts[i].port.nStatus & ~GI_DATA_RECIEVED;
						m_State.bIsReceivingDmx = false;
					}
				}
			}
		}
	}
}
