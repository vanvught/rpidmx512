/**
 * @file artnetrdm.h
 *
 */
/**
 * Art-Net Designed by and Copyright Artistic Licence Holdings Ltd.
 */
/* Copyright (C) 2017-2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include "artnetrdm.h"

#include "artnetnode.h"
#include "network.h"

#include "artnetnode_internal.h"

ArtNetRdm::~ArtNetRdm(void) {

}

void ArtNetNode::HandleTodControl(void) {
	const struct TArtTodControl *packet = (struct TArtTodControl *) &(m_ArtNetPacket.ArtPacket.ArtTodControl);
	const uint16_t portAddress = (uint16_t)(packet->Net << 8) | (uint16_t)(packet->Address);

	for (uint32_t i = 0; i < ARTNET_MAX_PORTS; i++) {
		if ((portAddress == m_OutputPorts[i].port.nPortAddress) && m_OutputPorts[i].bIsEnabled) {

			if (m_IsLightSetRunning[i] && (!m_IsRdmResponder)) {
				m_pLightSet->Stop(i);
			}

			if (packet->Command == 0x01) {	// AtcFlush
				m_pArtNetRdm->Full(i);
			}

			SendTod(i);

			if (m_IsLightSetRunning[i] && (!m_IsRdmResponder)) {
				m_pLightSet->Start(i);
			}
		}
	}
}

void ArtNetNode::HandleTodRequest(void) {
	const struct TArtTodRequest *packet = (struct TArtTodRequest *) &(m_ArtNetPacket.ArtPacket.ArtTodRequest);
	const uint16_t portAddress = (uint16_t)(packet->Net << 8) | (uint16_t)(packet->Address[0]);

	for (uint32_t i = 0; i < ARTNET_MAX_PORTS; i++) {
		if ((portAddress == m_OutputPorts[i].port.nPortAddress) && m_OutputPorts[i].bIsEnabled) {
			SendTod(i);
		}
	}
}

void ArtNetNode::SendTod(uint8_t nPortId) {
	assert(nPortId < ARTNET_MAX_PORTS);

	m_pTodData->Net = m_Node.NetSwitch[0];
	m_pTodData->Address = m_OutputPorts[nPortId].port.nDefaultAddress;

	const uint8_t discovered = m_pArtNetRdm->GetUidCount(nPortId);

	m_pTodData->UidTotalHi = 0;
	m_pTodData->UidTotalLo = discovered;
	m_pTodData->BlockCount = 0;
	m_pTodData->UidCount = discovered;
	m_pTodData->Port = 1 + nPortId;

	m_pArtNetRdm->Copy(nPortId, (uint8_t *) m_pTodData->Tod);

	const uint16_t length = (uint16_t) sizeof(struct TArtTodData) - (uint16_t) (sizeof m_pTodData->Tod) + (uint16_t) (discovered * 6);

	Network::Get()->SendTo(m_nHandle, (const uint8_t *) m_pTodData, (const uint16_t) length, m_Node.IPAddressBroadcast, (uint16_t) ARTNET_UDP_PORT);
}

void ArtNetNode::SetRdmHandler(ArtNetRdm *pArtNetTRdm, bool IsResponder) {
	assert(pArtNetTRdm != 0);

	if (pArtNetTRdm != 0) {
		m_pArtNetRdm = pArtNetTRdm;
		m_IsRdmResponder = IsResponder;

		m_pTodData = new TArtTodData;
		assert(m_pTodData != 0);

		if (m_pTodData != 0) {
			m_Node.Status1 |= STATUS1_RDM_CAPABLE;
			memset(m_pTodData, 0, sizeof(struct TArtTodData));

			memcpy(m_pTodData->Id, (const char *) NODE_ID, sizeof(m_pTodData->Id));
			m_pTodData->OpCode = OP_TODDATA;
			m_pTodData->ProtVerLo = ARTNET_PROTOCOL_REVISION;
			m_pTodData->RdmVer = 0x01; // Devices that support RDM STANDARD V1.0 set field to 0x01.
		}
	}
}

void ArtNetNode::HandleRdm(void) {
	struct TArtRdm *packet = (struct TArtRdm *) &(m_ArtNetPacket.ArtPacket.ArtRdm);
	const uint16_t portAddress = (uint16_t) (packet->Net << 8) | (uint16_t) (packet->Address);

	for (uint32_t i = 0; i < ARTNET_MAX_PORTS; i++) {
		if ((portAddress == m_OutputPorts[i].port.nPortAddress) && m_OutputPorts[i].bIsEnabled) {

			if (!m_IsRdmResponder) {
				if ((m_OutputPorts[i].tPortProtocol == PORT_ARTNET_SACN) && (m_pArtNet4Handler != 0)) {
					const uint8_t nMask = GO_OUTPUT_IS_MERGING | GO_DATA_IS_BEING_TRANSMITTED | GO_OUTPUT_IS_SACN;
					m_IsLightSetRunning[i] = (m_pArtNet4Handler->GetStatus(i) & nMask) != 0;
				}

				if (m_IsLightSetRunning[i]) {
					m_pLightSet->Stop(i); // Stop DMX if was running
				}
			}

			const uint8_t *response = (uint8_t *) m_pArtNetRdm->Handler(i, packet->RdmPacket);

			if (response != 0) {
				packet->RdmVer = 0x01;

				const uint8_t nMessageLength = response[2] + 1;
				memcpy((uint8_t *) packet->RdmPacket, &response[1], nMessageLength);

				const uint16_t nLength = (uint16_t) sizeof(struct TArtRdm) - (uint16_t) sizeof(packet->RdmPacket) + nMessageLength;

				Network::Get()->SendTo(m_nHandle, (const uint8_t *) packet, (const uint16_t) nLength, m_ArtNetPacket.IPAddressFrom, (uint16_t) ARTNET_UDP_PORT);
			} else {
				//printf("\n==> No response <==\n");
			}

			if (m_IsLightSetRunning[i] && (!m_IsRdmResponder)) {
				m_pLightSet->Start(i); // Start DMX if was running
			}
		}
	}
}
