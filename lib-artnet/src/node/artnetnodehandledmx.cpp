/**
 * @file artnetnodehandledmx.cpp
 *
 */
/**
 * Art-Net Designed by and Copyright Artistic Licence Holdings Ltd.
 */
/* Copyright (C) 2021-2022 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <cstdint>
#include <cstring>
#include <algorithm>
#include <cassert>

#include "artnetnode.h"
#include "artnet.h"

#include "lightsetdata.h"

using namespace artnet;

void ArtNetNode::UpdateMergeStatus(uint32_t nPortIndex) {
	if (!m_State.IsMergeMode) {
		m_State.IsMergeMode = true;
		m_State.IsChanged = true;
	}

	m_OutputPort[nPortIndex].genericPort.nStatus |= GoodOutput::OUTPUT_IS_MERGING;
}

void ArtNetNode::CheckMergeTimeouts(uint32_t nPortIndex) {
	const uint32_t nTimeOutAMillis = m_nCurrentPacketMillis - m_OutputPort[nPortIndex].sourceA.nMillis;

	if (nTimeOutAMillis > (artnet::MERGE_TIMEOUT_SECONDS * 1000U)) {
		m_OutputPort[nPortIndex].sourceA.nIp = 0;
		m_OutputPort[nPortIndex].genericPort.nStatus &= static_cast<uint8_t>(~GoodOutput::OUTPUT_IS_MERGING);
	}

	const uint32_t nTimeOutBMillis = m_nCurrentPacketMillis - m_OutputPort[nPortIndex].sourceB.nMillis;

	if (nTimeOutBMillis > (artnet::MERGE_TIMEOUT_SECONDS * 1000U)) {
		m_OutputPort[nPortIndex].sourceB.nIp = 0;
		m_OutputPort[nPortIndex].genericPort.nStatus &= static_cast<uint8_t>(~GoodOutput::OUTPUT_IS_MERGING);
	}

	auto bIsMerging = false;

	for (uint32_t i = 0; i < artnetnode::MAX_PORTS; i++) {
		bIsMerging |= ((m_OutputPort[i].genericPort.nStatus & GoodOutput::OUTPUT_IS_MERGING) != 0);
	}

	if (!bIsMerging) {
		m_State.IsChanged = true;
		m_State.IsMergeMode = false;
#if defined ( ENABLE_SENDDIAG )
		SendDiag("Leaving Merging Mode", artnet::DP_LOW);
#endif
	}
}

void ArtNetNode::HandleDmx() {
	const auto *pArtDmx = &(m_ArtNetPacket.ArtPacket.ArtDmx);

	auto nDmxSlots = static_cast<uint16_t>( ((pArtDmx->LengthHi << 8) & 0xff00) | pArtDmx->Length);
	nDmxSlots = std::min(nDmxSlots, artnet::DMX_LENGTH);

	for (uint32_t nPortIndex = 0; nPortIndex < artnetnode::MAX_PORTS; nPortIndex++) {

		if (m_OutputPort[nPortIndex].genericPort.bIsEnabled && (m_OutputPort[nPortIndex].protocol == PortProtocol::ARTNET) && (pArtDmx->PortAddress == m_OutputPort[nPortIndex].genericPort.nPortAddress)) {

			auto ipA = m_OutputPort[nPortIndex].sourceA.nIp;
			auto ipB = m_OutputPort[nPortIndex].sourceB.nIp;

			m_OutputPort[nPortIndex].genericPort.nStatus = m_OutputPort[nPortIndex].genericPort.nStatus | GoodOutput::DATA_IS_BEING_TRANSMITTED;

			if (m_State.IsMergeMode) {
				if (__builtin_expect((!m_State.bDisableMergeTimeout), 1)) {
					CheckMergeTimeouts(nPortIndex);
				}
			}

			if (ipA == 0 && ipB == 0) {
#if defined ( ENABLE_SENDDIAG )
				SendDiag("1. first packet recv on this port", artnet::DP_LOW);
#endif
				m_OutputPort[nPortIndex].sourceA.nIp = m_ArtNetPacket.IPAddressFrom;
				m_OutputPort[nPortIndex].sourceA.nMillis = m_nCurrentPacketMillis;
				lightset::Data::SetSourceA(nPortIndex, pArtDmx->Data, nDmxSlots);
			} else if (ipA == m_ArtNetPacket.IPAddressFrom && ipB == 0) {
#if defined ( ENABLE_SENDDIAG )
				SendDiag("2. continued transmission from the same ip (source A)", artnet::DP_LOW);
#endif
				m_OutputPort[nPortIndex].sourceA.nMillis = m_nCurrentPacketMillis;
				lightset::Data::SetSourceA(nPortIndex, pArtDmx->Data, nDmxSlots);
			} else if (ipA == 0 && ipB == m_ArtNetPacket.IPAddressFrom) {
#if defined ( ENABLE_SENDDIAG )
				SendDiag("3. continued transmission from the same ip (source B)", artnet::DP_LOW);
#endif
				m_OutputPort[nPortIndex].sourceB.nMillis = m_nCurrentPacketMillis;
				lightset::Data::SetSourceB(nPortIndex, pArtDmx->Data, nDmxSlots);
			} else if (ipA != m_ArtNetPacket.IPAddressFrom && ipB == 0) {
#if defined ( ENABLE_SENDDIAG )
				SendDiag("4. new source, start the merge", artnet::DP_LOW);
#endif
				m_OutputPort[nPortIndex].sourceB.nIp = m_ArtNetPacket.IPAddressFrom;
				m_OutputPort[nPortIndex].sourceB.nMillis = m_nCurrentPacketMillis;
				UpdateMergeStatus(nPortIndex);
				lightset::Data::MergeSourceB(nPortIndex, pArtDmx->Data, nDmxSlots, m_OutputPort[nPortIndex].mergeMode);
			} else if (ipA == 0 && ipB != m_ArtNetPacket.IPAddressFrom) {
#if defined ( ENABLE_SENDDIAG )
				SendDiag("5. new source, start the merge", artnet::DP_LOW);
#endif
				m_OutputPort[nPortIndex].sourceA.nIp = m_ArtNetPacket.IPAddressFrom;
				m_OutputPort[nPortIndex].sourceA.nMillis = m_nCurrentPacketMillis;
				UpdateMergeStatus(nPortIndex);
				lightset::Data::MergeSourceA(nPortIndex, pArtDmx->Data, nDmxSlots, m_OutputPort[nPortIndex].mergeMode);
			} else if (ipA == m_ArtNetPacket.IPAddressFrom && ipB != m_ArtNetPacket.IPAddressFrom) {
#if defined ( ENABLE_SENDDIAG )
				SendDiag("6. continue merge", artnet::DP_LOW);
#endif
				m_OutputPort[nPortIndex].sourceA.nMillis = m_nCurrentPacketMillis;
				UpdateMergeStatus(nPortIndex);
				lightset::Data::MergeSourceA(nPortIndex, pArtDmx->Data, nDmxSlots, m_OutputPort[nPortIndex].mergeMode);
			} else if (ipA != m_ArtNetPacket.IPAddressFrom && ipB == m_ArtNetPacket.IPAddressFrom) {
#if defined ( ENABLE_SENDDIAG )
				SendDiag("7. continue merge", artnet::DP_LOW);
#endif
				m_OutputPort[nPortIndex].sourceB.nMillis = m_nCurrentPacketMillis;
				UpdateMergeStatus(nPortIndex);
				lightset::Data::MergeSourceB(nPortIndex, pArtDmx->Data, nDmxSlots, m_OutputPort[nPortIndex].mergeMode);
			}
#ifndef NDEBUG
			else if (ipA == m_ArtNetPacket.IPAddressFrom && ipB == m_ArtNetPacket.IPAddressFrom) {
#if defined ( ENABLE_SENDDIAG )
				SendDiag("8. Source matches both buffers, this shouldn't be happening!", artnet::DP_LOW);
#endif
				return;
			} else if (ipA != m_ArtNetPacket.IPAddressFrom && ipB != m_ArtNetPacket.IPAddressFrom) {
#if defined ( ENABLE_SENDDIAG )
				SendDiag("9. More than two sources, discarding data", artnet::DP_LOW);
#endif
				return;
			}
#endif
			else {
#if defined ( ENABLE_SENDDIAG )
				SendDiag("0. No cases matched, this shouldn't happen!", artnet::DP_LOW);
#endif
				return;
			}

			if (!m_State.IsSynchronousMode) {
#if defined ( ENABLE_SENDDIAG )
				SendDiag("Send data", artnet::DP_LOW);
#endif
				lightset::Data::Output(m_pLightSet, nPortIndex);

				if (!m_OutputPort[nPortIndex].IsTransmitting) {
					m_pLightSet->Start(nPortIndex);
					m_State.IsChanged = true;
					m_OutputPort[nPortIndex].IsTransmitting = true;
				}
			}

			m_State.nReceivingDmx |= (1U << static_cast<uint8_t>(lightset::PortDir::OUTPUT));
		}
	}
}
