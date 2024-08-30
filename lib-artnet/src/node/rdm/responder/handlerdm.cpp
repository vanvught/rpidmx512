/**
 * @file handlerdm.h
 *
 */
/**
 * Art-Net Designed by and Copyright Artistic Licence Holdings Ltd.
 */
/* Copyright (C) 2017-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <cstdio>
#include <cassert>

#include "artnetnode.h"
#include "artnetrdmresponder.h"

#include "debug.h"

void ArtNetNode::HandleRdm() {
	auto *const pArtRdm = reinterpret_cast<artnet::ArtRdm *>(m_pReceiveBuffer);

	if (pArtRdm->RdmVer != 0x01) {
		DEBUG_EXIT
		return;
	}

	const auto portAddress = static_cast<uint16_t>((pArtRdm->Net << 8)) | static_cast<uint16_t>((pArtRdm->Address));

	for (uint32_t nPortIndex = 0; nPortIndex < artnetnode::MAX_PORTS; nPortIndex++) {
		if ((m_OutputPort[nPortIndex].GoodOutputB & artnet::GoodOutputB::RDM_DISABLED) == artnet::GoodOutputB::RDM_DISABLED) {
			continue;
		}

		if ((portAddress == m_Node.Port[nPortIndex].PortAddress) && (m_Node.Port[nPortIndex].direction == lightset::PortDir::OUTPUT)) {
			const auto *pRdmResponse = const_cast<uint8_t*>(m_pArtNetRdmResponder->Handler(nPortIndex, pArtRdm->RdmPacket));

			if (pRdmResponse != nullptr) {
				pArtRdm->RdmVer = 0x01;

				const auto nMessageLength = static_cast<uint16_t>(pRdmResponse[2] + 1);
				memcpy(pArtRdm->RdmPacket, &pRdmResponse[1], nMessageLength);

				const auto nLength = sizeof(struct artnet::ArtRdm) - sizeof(pArtRdm->RdmPacket) + nMessageLength;

				Network::Get()->SendTo(m_nHandle, m_pReceiveBuffer, nLength, m_nIpAddressFrom, artnet::UDP_PORT);
			} else {
				DEBUG_PUTS("No RDM response");
			}
		}
	}
}

void ArtNetNode::HandleTodControl() {
	DEBUG_ENTRY

	const auto *const pArtTodControl =  reinterpret_cast<artnet::ArtTodControl *>(m_pReceiveBuffer);
	const auto portAddress = static_cast<uint16_t>((pArtTodControl->Net << 8)) | static_cast<uint16_t>((pArtTodControl->Address));

	const uint32_t nPortIndex = 0;

	if ((m_OutputPort[nPortIndex].GoodOutputB & artnet::GoodOutputB::RDM_DISABLED) == artnet::GoodOutputB::RDM_DISABLED) {
		DEBUG_EXIT
		return;
	}

	if ((portAddress == m_Node.Port[nPortIndex].PortAddress) && (m_Node.Port[nPortIndex].direction == lightset::PortDir::OUTPUT)) {
		if (pArtTodControl->Command == 0x01) {	// AtcFlush
			SendTod(nPortIndex);
		}
	}

	DEBUG_EXIT
}

/**
 * Output Gateway always Directed Broadcasts this packet.
 */
void ArtNetNode::SendTod(uint32_t nPortIndex) {
	DEBUG_ENTRY
	DEBUG_PRINTF("nPortIndex=%u", nPortIndex);
	assert(nPortIndex < artnetnode::MAX_PORTS);

	auto *pTodData = &m_ArtTodPacket.ArtTodData;
	const auto nPage = nPortIndex;

	memcpy(pTodData->Id, artnet::NODE_ID, sizeof(pTodData->Id));
	pTodData->OpCode = static_cast<uint16_t>(artnet::OpCodes::OP_TODDATA);
	pTodData->ProtVerHi = 0;
	pTodData->ProtVerLo = artnet::PROTOCOL_REVISION;
	pTodData->RdmVer = 0x01; // Devices that support RDM STANDARD V1.0 set field to 0x01.

	const auto nDiscovered = static_cast<uint8_t>(nPortIndex == 0 ? 1 : 0);

	/**
	 * Physical Port = (BindIndex-1) * ArtPollReply- >NumPortsLo + ArtTodData->Port
	 * As most modern Art-Net gateways implement one universe per ArtPollReply,
	 * ArtTodData->Port will usually be set to a value of 1.
	 */
	pTodData->Port = static_cast<uint8_t>(1U + (nPortIndex & 0x3));
	pTodData->Spare1 = 0;
	pTodData->Spare2 = 0;
	pTodData->Spare3 = 0;
	pTodData->Spare4 = 0;
	pTodData->Spare5 = 0;
	pTodData->Spare6 = 0;
	pTodData->BindIndex = static_cast<uint8_t>(nPage + 1U); ///< ArtPollReplyData->BindIndex == ArtTodData- >BindIndex
	pTodData->Net = m_Node.Port[nPage].NetSwitch;
	pTodData->CommandResponse = 0; 							///< The packet contains the entire TOD or is the first packet in a sequence of packets that contains the entire TOD.
	pTodData->Address = m_Node.Port[nPortIndex].DefaultAddress;
	pTodData->UidTotalHi = 0;
	pTodData->UidTotalLo = nDiscovered;
	pTodData->BlockCount = 0;
	pTodData->UidCount = nDiscovered;

	m_pArtNetRdmResponder->TodCopy(nPortIndex, reinterpret_cast<uint8_t*>(pTodData->Tod));

	const auto nLength = sizeof(struct artnet::ArtTodData) - (sizeof(pTodData->Tod)) + (nDiscovered * 6U);

	Network::Get()->SendTo(m_nHandle, pTodData, nLength, Network::Get()->GetBroadcastIp(), artnet::UDP_PORT);

	DEBUG_EXIT
}

void ArtNetNode::HandleTodData() {
	DEBUG_ENTRY

	DEBUG_EXIT
}
