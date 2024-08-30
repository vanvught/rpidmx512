/**
 * @file handlerdm.cpp
 *
 */
/**
 * Art-Net Designed by and Copyright Artistic Licence Holdings Ltd.
 */
/* Copyright (C) 2017-2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include "rdm.h"
#include "network.h"
#include "panel_led.h"

#include "debug.h"

/**
 * ArtTodControl is used to for an Output Gateway to flush its ToD and commence full discovery.
 * If the Output Gateway has physical DMX512 ports, discovery could take minutes.
 */
void ArtNetNode::HandleTodControl() {
	DEBUG_ENTRY

	const auto *const pArtTodControl = reinterpret_cast<artnet::ArtTodControl *>(m_pReceiveBuffer);

	if (pArtTodControl->Command == artnet::TodControlCommand::ATC_NONE) {
		DEBUG_EXIT
		return;
	}

	const auto portAddress = static_cast<uint16_t>((pArtTodControl->Net << 8)) | static_cast<uint16_t>((pArtTodControl->Address));

	for (uint32_t nPortIndex = 0; nPortIndex < artnetnode::MAX_PORTS; nPortIndex++) {
		if (portAddress != m_Node.Port[nPortIndex].PortAddress) {
			continue;
		}

		if ((m_Node.Port[nPortIndex].direction == lightset::PortDir::OUTPUT) &&
				((m_OutputPort[nPortIndex].GoodOutputB & artnet::GoodOutputB::RDM_DISABLED) != artnet::GoodOutputB::RDM_DISABLED)) {
			switch (pArtTodControl->Command) {
			case artnet::TodControlCommand::ATC_FLUSH:
				m_pArtNetRdmController->Full(nPortIndex);
				m_OutputPort[nPortIndex].GoodOutputB &= static_cast<uint8_t>(~artnet::GoodOutputB::DISCOVERY_NOT_RUNNING);
				break;
			case artnet::TodControlCommand::ATC_END:
				m_pArtNetRdmController->Stop(nPortIndex);
				m_OutputPort[nPortIndex].GoodOutputB |= artnet::GoodOutputB::DISCOVERY_NOT_RUNNING;
				break;
			case artnet::TodControlCommand::ATC_INCON:
				m_OutputPort[nPortIndex].GoodOutputB &= static_cast<uint8_t>(~artnet::GoodOutputB::DISCOVERY_DISABLED);
				break;
			case artnet::TodControlCommand::ATC_INCOFF:
				m_OutputPort[nPortIndex].GoodOutputB |= artnet::GoodOutputB::DISCOVERY_DISABLED;
				break;
			default:
				break;
			}
		} else if (m_Node.Port[nPortIndex].direction == lightset::PortDir::INPUT) {
			if (pArtTodControl->Command == artnet::TodControlCommand::ATC_FLUSH) {
				m_pArtNetRdmController->TodReset(nPortIndex);
			}
		}
	}

	DEBUG_EXIT
}

void ArtNetNode::HandleTodData() {
	DEBUG_ENTRY

	const auto *const pArtTodData = reinterpret_cast<artnet::ArtTodData *>(m_pReceiveBuffer);

	if (pArtTodData->RdmVer != 0x01) {
		DEBUG_EXIT
		return;
	}

	const auto portAddress = static_cast<uint16_t>((pArtTodData->Net << 8)) | static_cast<uint16_t>((pArtTodData->Address));

	for (uint32_t nPortIndex = 0; nPortIndex < artnetnode::MAX_PORTS; nPortIndex++) {
		if (m_Node.Port[nPortIndex].direction != lightset::PortDir::INPUT) {
			continue;
		}

		if (m_Node.Port[nPortIndex].PortAddress == portAddress) {
			DEBUG_PRINTF("nPortIndex=%u, portAddress=%u, pArtTodData->UidCount=%u",nPortIndex, portAddress, pArtTodData->UidCount);

			for (uint32_t nUidIndex = 0; nUidIndex < pArtTodData->UidCount; nUidIndex++) {
				const uint8_t *pUid = pArtTodData->Tod[nUidIndex];
				m_pArtNetRdmController->TodAddUid(nPortIndex, pUid);
			}
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

	const auto nDiscovered = static_cast<uint8_t>(m_pArtNetRdmController->GetUidCount(nPortIndex));

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

	m_pArtNetRdmController->TodCopy(nPortIndex, reinterpret_cast<uint8_t*>(pTodData->Tod));

	const auto nLength = sizeof(struct artnet::ArtTodData) - (sizeof(pTodData->Tod)) + (nDiscovered * 6U);

	Network::Get()->SendTo(m_nHandle, pTodData, static_cast<uint16_t>(nLength), Network::Get()->GetBroadcastIp(), artnet::UDP_PORT);

	DEBUG_EXIT
}

void ArtNetNode::SendTodRequest(uint32_t nPortIndex) {
	DEBUG_ENTRY
	assert(nPortIndex < artnetnode::MAX_PORTS);

	m_pArtNetRdmController->TodReset(nPortIndex);

	auto *pTodRequest = &m_ArtTodPacket.ArtTodRequest;
	const auto nPage = nPortIndex;

	memcpy(pTodRequest->Id, artnet::NODE_ID, sizeof(pTodRequest->Id));
	pTodRequest->OpCode = static_cast<uint16_t>(artnet::OpCodes::OP_TODREQUEST);
	pTodRequest->ProtVerHi = 0;
	pTodRequest->ProtVerLo = artnet::PROTOCOL_REVISION;
	pTodRequest->Spare1 = 0;
	pTodRequest->Spare2 = 0;
	pTodRequest->Spare3 = 0;
	pTodRequest->Spare4 = 0;
	pTodRequest->Spare5 = 0;
	pTodRequest->Spare6 = 0;
	pTodRequest->Spare7 = 0;
	pTodRequest->Net = m_Node.Port[nPage].NetSwitch;
	pTodRequest->Command = 0;
	pTodRequest->AddCount = 1;
	pTodRequest->Address[0] = m_Node.Port[nPortIndex].DefaultAddress;

	const auto nLength = sizeof(struct artnet::ArtTodRequest) - (sizeof(pTodRequest->Address)) + pTodRequest->AddCount;

	Network::Get()->SendTo(m_nHandle, pTodRequest, static_cast<uint16_t>(nLength), Network::Get()->GetBroadcastIp(), artnet::UDP_PORT);

	DEBUG_EXIT
}

void ArtNetNode::HandleRdm() {
	auto *const pArtRdm = reinterpret_cast<artnet::ArtRdm *>(m_pReceiveBuffer);

	if (pArtRdm->RdmVer != 0x01) {
		DEBUG_EXIT
		return;
	}

	const auto portAddress = static_cast<uint16_t>((pArtRdm->Net << 8)) | static_cast<uint16_t>((pArtRdm->Address));

	for (uint32_t nPortIndex = 0; nPortIndex < artnetnode::MAX_PORTS; nPortIndex++) {
		if (m_Node.Port[nPortIndex].PortAddress != portAddress) {
			continue;
		}

		if ((m_Node.Port[nPortIndex].direction == lightset::PortDir::OUTPUT) &&
		   ((m_OutputPort[nPortIndex].GoodOutputB & artnet::GoodOutputB::RDM_DISABLED) != artnet::GoodOutputB::RDM_DISABLED)) {
# if (ARTNET_VERSION >= 4)
			if (m_Node.Port[nPortIndex].protocol == artnet::PortProtocol::SACN) {
				constexpr auto nMask = artnet::GoodOutput::OUTPUT_IS_MERGING | artnet::GoodOutput::DATA_IS_BEING_TRANSMITTED | artnet::GoodOutput::OUTPUT_IS_SACN;
				m_OutputPort[nPortIndex].IsTransmitting = (GetGoodOutput4(nPortIndex) & nMask) != 0;
			}
# endif
			if (m_OutputPort[nPortIndex].IsTransmitting) {
				m_OutputPort[nPortIndex].IsTransmitting = false;
				m_pLightSet->Stop(nPortIndex); // Stop DMX if was running
			}

			m_OutputPort[nPortIndex].nIpRdm = m_nIpAddressFrom;

			auto *pRdmMessage = reinterpret_cast<const TRdmMessage *>(&pArtRdm->Address);

			pArtRdm->Address = E120_SC_RDM;
			Rdm::SendRaw(nPortIndex, &pArtRdm->Address, pRdmMessage->message_length + RDM_MESSAGE_CHECKSUM_SIZE);

#ifndef NDEBUG
			rdm::message_print(reinterpret_cast<const uint8_t *>(pRdmMessage));
#endif

#if defined(CONFIG_PANELLED_RDM_PORT)
			hal::panel_led_on(hal::panelled::PORT_A_RDM << nPortIndex);
#elif defined(CONFIG_PANELLED_RDM_NO_PORT)
			hal::panel_led_on(hal::panelled::RDM << nPortIndex);
#endif
		} else if (m_Node.Port[nPortIndex].direction == lightset::PortDir::INPUT) {
			auto *pRdmMessage = reinterpret_cast<const TRdmMessage *>(&pArtRdm->Address);

			if ((pRdmMessage->command_class == E120_GET_COMMAND_RESPONSE) || (pRdmMessage->command_class == E120_SET_COMMAND_RESPONSE)) {
				pArtRdm->Address = E120_SC_RDM;
				Rdm::SendRaw(nPortIndex, reinterpret_cast<const uint8_t *>(pRdmMessage), pRdmMessage->message_length + RDM_MESSAGE_CHECKSUM_SIZE);

#ifndef NDEBUG
				rdm::message_print(reinterpret_cast<const uint8_t *>(pRdmMessage));
#endif
			}

#if defined(CONFIG_PANELLED_RDM_PORT)
			hal::panel_led_on(hal::panelled::PORT_A_RDM << nPortIndex);
#elif defined(CONFIG_PANELLED_RDM_NO_PORT)
			hal::panel_led_on(hal::panelled::RDM << nPortIndex);
#endif
		}
	}
}
