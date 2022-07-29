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

#include "artnetrdm.h"

#include "artnetnode.h"
#include "network.h"

#include "artnetnode_internal.h"

#include "debug.h"

using namespace artnet;

void ArtNetNode::SetRmd(uint32_t nPortIndex, bool bEnable) {
	DEBUG_ENTRY
	assert(nPortIndex < artnetnode::MAX_PORTS);

	const auto isChanged = (m_OutputPort[nPortIndex].isRdmEnabled != bEnable);

	m_OutputPort[nPortIndex].isRdmEnabled = bEnable;

	if (isChanged && (m_State.status == artnetnode::Status::ON)) {
		if (m_pArtNetStore != nullptr) {
			m_pArtNetStore->SaveRdmEnabled(nPortIndex, bEnable);
		}
		if (m_pArtNetDisplay != nullptr) {
			m_pArtNetDisplay->ShowRdmEnabled(nPortIndex, bEnable);
		}
	}

	DEBUG_EXIT
}

void ArtNetNode::HandleTodControl() {
	DEBUG_ENTRY

	const auto *pArtTodControl =  &(m_ArtNetPacket.ArtPacket.ArtTodControl);
	const auto portAddress = static_cast<uint16_t>((pArtTodControl->Net << 8)) | static_cast<uint16_t>((pArtTodControl->Address));

	for (uint32_t i = 0; i < artnetnode::MAX_PORTS; i++) {
		if (!m_OutputPort[i].isRdmEnabled) {
			continue;
		}

		if ((portAddress == m_OutputPort[i].genericPort.nPortAddress) && m_OutputPort[i].genericPort.bIsEnabled) {
			if (m_OutputPort[i].IsTransmitting && (!m_IsRdmResponder)) {
				m_pLightSet->Stop(i);
			}

			if (pArtTodControl->Command == 0x01) {	// AtcFlush
				m_pArtNetRdm->Full(i);
			}

			SendTod(i);

			if (m_OutputPort[i].IsTransmitting && (!m_IsRdmResponder)) {
				m_pLightSet->Start(i);
			}
		}
	}

	DEBUG_EXIT
}

void ArtNetNode::HandleTodRequest() {
	DEBUG_ENTRY

	const auto *pArtTodRequest = &(m_ArtNetPacket.ArtPacket.ArtTodRequest);
	const auto portAddress = static_cast<uint16_t>((pArtTodRequest->Net << 8)) | static_cast<uint16_t>((pArtTodRequest->Address[0]));

	for (uint32_t i = 0; i < artnetnode::MAX_PORTS; i++) {
		if (!m_OutputPort[i].isRdmEnabled) {
			continue;
		}

		if ((portAddress == m_OutputPort[i].genericPort.nPortAddress) && m_OutputPort[i].genericPort.bIsEnabled) {
			SendTod(i);
		}
	}

	DEBUG_EXIT
}

void ArtNetNode::SendTod(uint32_t nPortIndex) {
	DEBUG_ENTRY
	assert(nPortIndex < artnetnode::MAX_PORTS);

	auto pTodData = &(m_ArtNetPacket.ArtPacket.ArtTodData);
	const auto nPage = static_cast<uint8_t>(nPortIndex / artnetnode::PAGE_SIZE);

	pTodData->OpCode = OP_TODDATA;
	pTodData->RdmVer = 0x01; // Devices that support RDM STANDARD V1.0 set field to 0x01.

	const auto nDiscovered = static_cast<uint8_t>(m_pArtNetRdm->GetUidCount(nPortIndex));

	pTodData->Port = static_cast<uint8_t>(1U + (nPortIndex & 0x3));
	pTodData->Spare1 = 0;
	pTodData->Spare2 = 0;
	pTodData->Spare3 = 0;
	pTodData->Spare4 = 0;
	pTodData->Spare5 = 0;
	pTodData->Spare6 = 0;
	pTodData->BindIndex = static_cast<uint8_t>(nPage + 1);
	pTodData->Net = m_Node.NetSwitch[nPage];
	pTodData->CommandResponse = 0; // The packet contains the entire TOD or is the first packet in a sequence of packets that contains the entire TOD.
	pTodData->Address = m_OutputPort[nPortIndex].genericPort.nDefaultAddress;
	pTodData->UidTotalHi = 0;
	pTodData->UidTotalLo = nDiscovered;
	pTodData->BlockCount = 0;
	pTodData->UidCount = nDiscovered;

	m_pArtNetRdm->Copy(nPortIndex, reinterpret_cast<uint8_t*>(pTodData->Tod));

	const auto nLength = sizeof(struct TArtTodData) - (sizeof(pTodData->Tod)) + (nDiscovered * 6U);

	Network::Get()->SendTo(m_nHandle, pTodData, static_cast<uint16_t>(nLength), m_Node.IPAddressBroadcast, artnet::UDP_PORT);

	DEBUG_EXIT
}

void ArtNetNode::SetRdmHandler(ArtNetRdm *pArtNetTRdm, bool IsResponder) {
	DEBUG_ENTRY

	m_pArtNetRdm = pArtNetTRdm;

	if (pArtNetTRdm != nullptr) {
		m_IsRdmResponder = IsResponder;
		m_Node.Status1 |= Status1::RDM_CAPABLE;
	} else {
		m_Node.Status1 &= static_cast<uint8_t>(~Status1::RDM_CAPABLE);
	}

	DEBUG_EXIT
}

void ArtNetNode::HandleRdm() {
	DEBUG_ENTRY

	auto *pArtRdm = &(m_ArtNetPacket.ArtPacket.ArtRdm);
	const auto portAddress = static_cast<uint16_t>((pArtRdm->Net << 8)) | static_cast<uint16_t>((pArtRdm->Address));

	for (uint32_t i = 0; i < artnetnode::MAX_PORTS; i++) {
		if (!m_OutputPort[i].isRdmEnabled) {
			continue;
		}

		if ((portAddress == m_OutputPort[i].genericPort.nPortAddress) && m_OutputPort[i].genericPort.bIsEnabled) {
			if (!m_IsRdmResponder) {
				if ((m_OutputPort[i].protocol == PortProtocol::SACN) && (m_pArtNet4Handler != nullptr)) {
					constexpr auto nMask = GoodOutput::OUTPUT_IS_MERGING | GoodOutput::DATA_IS_BEING_TRANSMITTED | GoodOutput::OUTPUT_IS_SACN;
					m_OutputPort[i].IsTransmitting = (m_pArtNet4Handler->GetStatus(i) & nMask) != 0;
				}

				if (m_OutputPort[i].IsTransmitting) {
					m_pLightSet->Stop(i); // Stop DMX if was running
				}

			}

			const auto *pRdmResponse = const_cast<uint8_t*>(m_pArtNetRdm->Handler(i, pArtRdm->RdmPacket));

			if (pRdmResponse != nullptr) {
				pArtRdm->RdmVer = 0x01;

				const auto nMessageLength = static_cast<uint16_t>(pRdmResponse[2] + 1);
				memcpy(pArtRdm->RdmPacket, &pRdmResponse[1], nMessageLength);

				const auto nLength = sizeof(struct TArtRdm) - sizeof(pArtRdm->RdmPacket) + nMessageLength;

				Network::Get()->SendTo(m_nHandle, pArtRdm, static_cast<uint16_t>(nLength), m_ArtNetPacket.IPAddressFrom, artnet::UDP_PORT);
			} else {
				DEBUG_PUTS("No RDM response");
			}

			if (m_OutputPort[i].IsTransmitting && (!m_IsRdmResponder)) {
				m_pLightSet->Start(i); // Start DMX if was running
			}
		}
	}

	DEBUG_EXIT
}

void ArtNetNode::SetRdmUID(const uint8_t *pUid, bool bSupportsLLRP) {
	memcpy(m_Node.DefaultUidResponder, pUid, sizeof(m_Node.DefaultUidResponder));
	if (bSupportsLLRP) {
		m_Node.Status3 |= Status3::SUPPORTS_LLRP;
	} else {
		m_Node.Status3 &= static_cast<uint8_t>(~Status3::SUPPORTS_LLRP);
	}
}
