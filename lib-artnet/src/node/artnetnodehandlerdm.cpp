/**
 * @file artnetnodehandlerdm.h
 *
 */
/**
 * Art-Net Designed by and Copyright Artistic Licence Holdings Ltd.
 */
/* Copyright (C) 2017-2022 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include "artnetnode_internal.h"
#include "artnetrdm.h"

#include "network.h"

#include "debug.h"

void ArtNetNode::SetRmd(uint32_t nPortIndex, bool bEnable) {
	DEBUG_ENTRY
	assert(nPortIndex < artnetnode::MAX_PORTS);

	const auto isChanged = (m_OutputPort[nPortIndex].isRdmEnabled != bEnable);

	m_OutputPort[nPortIndex].isRdmEnabled = bEnable;

	if (isChanged && (m_State.status == artnetnode::Status::ON)) {
		if (m_pArtNetStore != nullptr) {
			m_pArtNetStore->SaveRdmEnabled(nPortIndex, bEnable);
		}

		artnet::display_rdm_enabled(nPortIndex, bEnable);
	}

	DEBUG_EXIT
}

//TODO Make RDM discovery a state-machine.

/**
 * ArtTodControl is used to for an Output Gateway to flush its ToD and commence full discovery.
 * If the Output Gateway has physical DMX512 ports, discovery could take minutes.
 */
void ArtNetNode::HandleTodControl() {
	DEBUG_ENTRY

	const auto *pArtTodControl =  &(m_ArtNetPacket.ArtPacket.ArtTodControl);
	const auto portAddress = static_cast<uint16_t>((pArtTodControl->Net << 8)) | static_cast<uint16_t>((pArtTodControl->Address));

	for (uint32_t nPortIndex = 0; nPortIndex < artnetnode::MAX_PORTS; nPortIndex++) {
		if (!m_OutputPort[nPortIndex].isRdmEnabled) {
			continue;
		}

		if ((portAddress == m_OutputPort[nPortIndex].genericPort.nPortAddress) && m_OutputPort[nPortIndex].genericPort.bIsEnabled) {
			if (m_OutputPort[nPortIndex].IsTransmitting && (!m_IsRdmResponder)) {
				m_pLightSet->Stop(nPortIndex);
			}

			if (pArtTodControl->Command == 0x01) {	// AtcFlush
				m_pArtNetRdm->Full(nPortIndex);
			}

			SendTod(nPortIndex);

			if (m_OutputPort[nPortIndex].IsTransmitting && (!m_IsRdmResponder)) {
				m_pLightSet->Start(nPortIndex);
			}
		}
	}

	DEBUG_EXIT
}

/**
 * An Output Gateway must not interpret receipt of an ArtTodRequest
 * as an instruction to perform full RDM Discovery on the DMX512 physical layer;
 * it is just a request to send the ToD back to the controller.
 */
void ArtNetNode::HandleTodRequest() {
	DEBUG_ENTRY

	const auto *pArtTodRequest = &(m_ArtNetPacket.ArtPacket.ArtTodRequest);
	const auto nAddCount = pArtTodRequest->AddCount & 0x1f;

	for (auto nCount = 0; nCount < nAddCount; nCount++) {
		const auto portAddress = static_cast<uint16_t>((pArtTodRequest->Net << 8)) | static_cast<uint16_t>((pArtTodRequest->Address[nCount]));

		for (uint32_t nPortIndex = 0; nPortIndex < artnetnode::MAX_PORTS; nPortIndex++) {
			if (!m_OutputPort[nPortIndex].isRdmEnabled) {
				continue;
			}

			if ((portAddress == m_OutputPort[nPortIndex].genericPort.nPortAddress) && m_OutputPort[nPortIndex].genericPort.bIsEnabled) {
				SendTod(nPortIndex);
			}
		}
	}

	DEBUG_EXIT
}

void ArtNetNode::HandleTodData() {
	DEBUG_ENTRY

	const auto *pArtTodData = &(m_ArtNetPacket.ArtPacket.ArtTodData);

	if (pArtTodData->RdmVer != 0x01) {
		DEBUG_EXIT
		return;
	}

	const auto portAddress = static_cast<uint16_t>((pArtTodData->Net << 8)) | static_cast<uint16_t>((pArtTodData->Address));

	for (uint32_t nPortIndex = 0; nPortIndex < artnetnode::MAX_PORTS; nPortIndex++) {
		if (!m_InputPort[nPortIndex].genericPort.bIsEnabled) {
			continue;
		}

		if (m_InputPort[nPortIndex].genericPort.nPortAddress == portAddress) {
			DEBUG_PRINTF("nPortIndex=%u, portAddress=%u, pArtTodData->UidCount=%u",nPortIndex, portAddress, pArtTodData->UidCount);

			for (uint32_t nUidIndex = 0; nUidIndex < pArtTodData->UidCount; nUidIndex++) {
				const uint8_t *pUid = pArtTodData->Tod[nUidIndex];
				m_pArtNetRdm->TodAddUid(nPortIndex, pUid);
			}
		}
	}

	DEBUG_EXIT
}

/**
 * An Output Gateway will send the ArtTodData packet in the following circumstances:
 * - Upon power on or decice reset. 
 * - In response to an ArtTodRequest if the Port-Address matches.
 * - In response to an ArtTodControl if the Port-Address matches.
 * - When their ToD changes due to the addition or deletion of a UID.
 * - At the end of full RDM discovery.
 */
void ArtNetNode::SendTod(uint32_t nPortIndex) {
	DEBUG_ENTRY
	assert(nPortIndex < artnetnode::MAX_PORTS);

	auto pTodData = &(m_ArtNetPacket.ArtPacket.ArtTodData);
	const auto nPage = nPortIndex / artnetnode::PAGE_SIZE;

	memcpy(pTodData->Id, artnet::NODE_ID, sizeof(pTodData->Id));
	pTodData->OpCode = OP_TODDATA;
	pTodData->ProtVerHi = 0;
	pTodData->ProtVerLo = artnet::PROTOCOL_REVISION;
	pTodData->RdmVer = 0x01; // Devices that support RDM STANDARD V1.0 set field to 0x01.

	const auto nDiscovered = static_cast<uint8_t>(m_pArtNetRdm->GetUidCount(nPortIndex));

	pTodData->Port = static_cast<uint8_t>(1U + (nPortIndex & 0x3));
	pTodData->Spare1 = 0;
	pTodData->Spare2 = 0;
	pTodData->Spare3 = 0;
	pTodData->Spare4 = 0;
	pTodData->Spare5 = 0;
	pTodData->Spare6 = 0;
	pTodData->BindIndex = static_cast<uint8_t>(nPage + 1U);
	pTodData->Net = m_Node.NetSwitch[nPage];
	pTodData->CommandResponse = 0; // The packet contains the entire TOD or is the first packet in a sequence of packets that contains the entire TOD.
	pTodData->Address = m_OutputPort[nPortIndex].genericPort.nDefaultAddress;
	pTodData->UidTotalHi = 0;
	pTodData->UidTotalLo = nDiscovered;
	pTodData->BlockCount = 0;
	pTodData->UidCount = nDiscovered;

	m_pArtNetRdm->TodCopy(nPortIndex, reinterpret_cast<uint8_t*>(pTodData->Tod));

	const auto nLength = sizeof(struct TArtTodData) - (sizeof(pTodData->Tod)) + (nDiscovered * 6U);

	Network::Get()->SendTo(m_nHandle, pTodData, static_cast<uint16_t>(nLength), m_Node.IPAddressBroadcast, artnet::UDP_PORT);

	DEBUG_EXIT
}

void ArtNetNode::SendTodRequest(uint32_t nPortIndex) {
	DEBUG_ENTRY
	assert(nPortIndex < artnetnode::MAX_PORTS);

	m_pArtNetRdm->TodReset(nPortIndex);

	auto *pTodRequest = &(m_ArtNetPacket.ArtPacket.ArtTodRequest);
	const auto nPage = nPortIndex / artnetnode::PAGE_SIZE;

	memcpy(pTodRequest->Id, artnet::NODE_ID, sizeof(pTodRequest->Id));
	pTodRequest->OpCode = OP_TODREQUEST;
	pTodRequest->ProtVerHi = 0;
	pTodRequest->ProtVerLo = artnet::PROTOCOL_REVISION;
	pTodRequest->Spare1 = 0;
	pTodRequest->Spare2 = 0;
	pTodRequest->Spare3 = 0;
	pTodRequest->Spare4 = 0;
	pTodRequest->Spare5 = 0;
	pTodRequest->Spare6 = 0;
	pTodRequest->Spare7 = 0;
	pTodRequest->Net = m_Node.NetSwitch[nPage];
	pTodRequest->Command = 0;
	pTodRequest->AddCount = 1;
	pTodRequest->Address[0] = m_InputPort[nPortIndex].genericPort.nDefaultAddress;

	const auto nLength = sizeof(struct TArtTodRequest) - (sizeof(pTodRequest->Address)) + pTodRequest->AddCount;

	Network::Get()->SendTo(m_nHandle, pTodRequest, static_cast<uint16_t>(nLength), m_Node.IPAddressBroadcast, artnet::UDP_PORT);

	DEBUG_EXIT
}

void ArtNetNode::SetRdmHandler(ArtNetRdm *pArtNetTRdm, bool IsResponder) {
	DEBUG_ENTRY

	m_pArtNetRdm = pArtNetTRdm;

	if (m_pArtNetRdm != nullptr) {
		m_IsRdmResponder = IsResponder;
		m_Node.Status1 |= artnet::Status1::RDM_CAPABLE;
	} else {
		m_Node.Status1 &= static_cast<uint8_t>(~artnet::Status1::RDM_CAPABLE);
	}

	DEBUG_EXIT
}

void ArtNetNode::HandleRdm() {
	DEBUG_ENTRY

	auto *pArtRdm = &(m_ArtNetPacket.ArtPacket.ArtRdm);

	if (pArtRdm->RdmVer != 0x01) {
		DEBUG_EXIT
		return;
	}

	const auto portAddress = static_cast<uint16_t>((pArtRdm->Net << 8)) | static_cast<uint16_t>((pArtRdm->Address));

	// Output ports
	for (uint32_t nPortIndex = 0; nPortIndex < artnetnode::MAX_PORTS; nPortIndex++) {
		if (!m_OutputPort[nPortIndex].isRdmEnabled) {
			continue;
		}

		if ((portAddress == m_OutputPort[nPortIndex].genericPort.nPortAddress) && m_OutputPort[nPortIndex].genericPort.bIsEnabled) {
#if defined	(RDM_CONTROLLER)
			if ((m_OutputPort[nPortIndex].protocol == artnet::PortProtocol::SACN) && (m_pArtNet4Handler != nullptr)) {
				constexpr auto nMask = artnet::GoodOutput::OUTPUT_IS_MERGING | artnet::GoodOutput::DATA_IS_BEING_TRANSMITTED | artnet::GoodOutput::OUTPUT_IS_SACN;
				m_OutputPort[nPortIndex].IsTransmitting = (m_pArtNet4Handler->GetStatus(nPortIndex) & nMask) != 0;
			}

			if (m_OutputPort[nPortIndex].IsTransmitting) {
				m_pLightSet->Stop(nPortIndex); // Stop DMX if was running
			}
#endif
			const auto *pRdmResponse = const_cast<uint8_t*>(m_pArtNetRdm->Handler(nPortIndex, pArtRdm->RdmPacket));

			if (pRdmResponse != nullptr) {
				pArtRdm->RdmVer = 0x01;

				const auto nMessageLength = static_cast<uint16_t>(pRdmResponse[2] + 1);
				memcpy(pArtRdm->RdmPacket, &pRdmResponse[1], nMessageLength);

				const auto nLength = sizeof(struct TArtRdm) - sizeof(pArtRdm->RdmPacket) + nMessageLength;

				Network::Get()->SendTo(m_nHandle, pArtRdm, static_cast<uint16_t>(nLength), m_ArtNetPacket.IPAddressFrom, artnet::UDP_PORT);
			} else {
				DEBUG_PUTS("No RDM response");
			}

#if defined	(RDM_CONTROLLER)
			if (m_OutputPort[nPortIndex].IsTransmitting) {
				m_pLightSet->Start(nPortIndex); // Start DMX if was running
			}
#endif
		}
	}

#if defined	(RDM_CONTROLLER)
	// Input ports
	for (uint32_t nPortIndex = 0; nPortIndex < artnetnode::MAX_PORTS; nPortIndex++) {
		if (!m_InputPort[nPortIndex].genericPort.bIsEnabled) {
			continue;
		}

		if (m_InputPort[nPortIndex].genericPort.nPortAddress == portAddress) {
			DEBUG_PRINTF("nPortIndex=%u, portAddress=%u", nPortIndex, portAddress);
			artnet::rdm_send(nPortIndex, pArtRdm->RdmPacket);
		}
	}
#endif

	DEBUG_EXIT
}

void ArtNetNode::SetRdmUID(const uint8_t *pUid, bool bSupportsLLRP) {
	memcpy(m_Node.DefaultUidResponder, pUid, sizeof(m_Node.DefaultUidResponder));

	if (bSupportsLLRP) {
		m_Node.Status3 |= artnet::Status3::SUPPORTS_LLRP;
	} else {
		m_Node.Status3 &= static_cast<uint8_t>(~artnet::Status3::SUPPORTS_LLRP);
	}
}
