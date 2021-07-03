/**
 * @file artnetnodehandledmx.cpp
 *
 */
/**
 * Art-Net Designed by and Copyright Artistic Licence Holdings Ltd.
 */
/* Copyright (C) 2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

using namespace artnet;

void ArtNetNode::MergeDmxData(uint32_t nPortIndex, const uint8_t *pData, uint32_t nLength) {
	assert(pData != nullptr);

	if (!m_State.IsMergeMode) {
		m_State.IsMergeMode = true;
		m_State.IsChanged = true;
	}

	m_OutputPorts[nPortIndex].port.nStatus |= GO_OUTPUT_IS_MERGING;
	m_OutputPorts[nPortIndex].nLength = nLength;

	if (m_OutputPorts[nPortIndex].mergeMode == Merge::HTP) {
		for (uint32_t i = 0; i < nLength; i++) {
			auto data = std::max(m_OutputPorts[nPortIndex].dataA[i], m_OutputPorts[nPortIndex].dataB[i]);
			m_OutputPorts[nPortIndex].data[i] = data;
		}

		return;
	}

	memcpy(m_OutputPorts[nPortIndex].data, pData, nLength);
}

void ArtNetNode::CheckMergeTimeouts(uint32_t nPortIndex) {
	const uint32_t nTimeOutAMillis = m_nCurrentPacketMillis - m_OutputPorts[nPortIndex].nMillisA;

	if (nTimeOutAMillis > (artnet::MERGE_TIMEOUT_SECONDS * 1000U)) {
		m_OutputPorts[nPortIndex].ipA = 0;
		m_OutputPorts[nPortIndex].port.nStatus &= static_cast<uint8_t>(~GO_OUTPUT_IS_MERGING);
	}

	const uint32_t nTimeOutBMillis = m_nCurrentPacketMillis - m_OutputPorts[nPortIndex].nMillisB;

	if (nTimeOutBMillis > (artnet::MERGE_TIMEOUT_SECONDS * 1000U)) {
		m_OutputPorts[nPortIndex].ipB = 0;
		m_OutputPorts[nPortIndex].port.nStatus &= static_cast<uint8_t>(~GO_OUTPUT_IS_MERGING);
	}

	bool bIsMerging = false;

	for (uint32_t i = 0; i < (ArtNet::PORTS * m_nPages); i++) {
		bIsMerging |= ((m_OutputPorts[i].port.nStatus & GO_OUTPUT_IS_MERGING) != 0);
	}

	if (!bIsMerging) {
		m_State.IsChanged = true;
		m_State.IsMergeMode = false;
#if defined ( ENABLE_SENDDIAG )
		SendDiag("Leaving Merging Mode", ARTNET_DP_LOW);
#endif
	}
}

void ArtNetNode::HandleDmx() {
	const auto *pArtDmx = &(m_ArtNetPacket.ArtPacket.ArtDmx);

	auto nDmxSlots = static_cast<uint16_t>( ((pArtDmx->LengthHi << 8) & 0xff00) | pArtDmx->Length);
	nDmxSlots = std::min(nDmxSlots, ArtNet::DMX_LENGTH);

	for (uint8_t nPortIndex = 0; nPortIndex < (ArtNet::PORTS * m_nPages); nPortIndex++) {

		if (m_OutputPorts[nPortIndex].bIsEnabled && (m_OutputPorts[nPortIndex].tPortProtocol == PortProtocol::ARTNET) && (pArtDmx->PortAddress == m_OutputPorts[nPortIndex].port.nPortAddress)) {

			auto ipA = m_OutputPorts[nPortIndex].ipA;
			auto ipB = m_OutputPorts[nPortIndex].ipB;

			m_OutputPorts[nPortIndex].port.nStatus = m_OutputPorts[nPortIndex].port.nStatus | GO_DATA_IS_BEING_TRANSMITTED;

			if (m_State.IsMergeMode) {
				if (__builtin_expect((!m_State.bDisableMergeTimeout), 1)) {
					CheckMergeTimeouts(nPortIndex);
				}
			}

			if (ipA == 0 && ipB == 0) {
#if defined ( ENABLE_SENDDIAG )
				SendDiag("1. first packet recv on this port", ARTNET_DP_LOW);
#endif
				m_OutputPorts[nPortIndex].ipA = m_ArtNetPacket.IPAddressFrom;
				m_OutputPorts[nPortIndex].nMillisA = m_nCurrentPacketMillis;
				memcpy(&m_OutputPorts[nPortIndex].dataA, pArtDmx->Data, nDmxSlots);
				m_OutputPorts[nPortIndex].nLength = nDmxSlots;
				memcpy(m_OutputPorts[nPortIndex].data, pArtDmx->Data, nDmxSlots);
			} else if (ipA == m_ArtNetPacket.IPAddressFrom && ipB == 0) {
#if defined ( ENABLE_SENDDIAG )
				SendDiag("2. continued transmission from the same ip (source A)", ARTNET_DP_LOW);
#endif
				m_OutputPorts[nPortIndex].nMillisA = m_nCurrentPacketMillis;
				memcpy(&m_OutputPorts[nPortIndex].dataA, pArtDmx->Data, nDmxSlots);
				m_OutputPorts[nPortIndex].nLength = nDmxSlots;
				memcpy(m_OutputPorts[nPortIndex].data, pArtDmx->Data, nDmxSlots);
			} else if (ipA == 0 && ipB == m_ArtNetPacket.IPAddressFrom) {
#if defined ( ENABLE_SENDDIAG )
				SendDiag("3. continued transmission from the same ip (source B)", ARTNET_DP_LOW);
#endif
				m_OutputPorts[nPortIndex].nMillisB = m_nCurrentPacketMillis;
				memcpy(&m_OutputPorts[nPortIndex].dataB, pArtDmx->Data, nDmxSlots);
				m_OutputPorts[nPortIndex].nLength = nDmxSlots;
				memcpy(m_OutputPorts[nPortIndex].data, pArtDmx->Data, nDmxSlots);
			} else if (ipA != m_ArtNetPacket.IPAddressFrom && ipB == 0) {
#if defined ( ENABLE_SENDDIAG )
				SendDiag("4. new source, start the merge", ARTNET_DP_LOW);
#endif
				m_OutputPorts[nPortIndex].ipB = m_ArtNetPacket.IPAddressFrom;
				m_OutputPorts[nPortIndex].nMillisB = m_nCurrentPacketMillis;
				memcpy(&m_OutputPorts[nPortIndex].dataB, pArtDmx->Data, nDmxSlots);
				MergeDmxData(nPortIndex, m_OutputPorts[nPortIndex].dataB, nDmxSlots);
			} else if (ipA == 0 && ipB != m_ArtNetPacket.IPAddressFrom) {
#if defined ( ENABLE_SENDDIAG )
				SendDiag("5. new source, start the merge", ARTNET_DP_LOW);
#endif
				m_OutputPorts[nPortIndex].ipA = m_ArtNetPacket.IPAddressFrom;
				m_OutputPorts[nPortIndex].nMillisA = m_nCurrentPacketMillis;
				memcpy(&m_OutputPorts[nPortIndex].dataA, pArtDmx->Data, nDmxSlots);
				MergeDmxData(nPortIndex, m_OutputPorts[nPortIndex].dataA, nDmxSlots);
			} else if (ipA == m_ArtNetPacket.IPAddressFrom && ipB != m_ArtNetPacket.IPAddressFrom) {
#if defined ( ENABLE_SENDDIAG )
				SendDiag("6. continue merge", ARTNET_DP_LOW);
#endif
				m_OutputPorts[nPortIndex].nMillisA = m_nCurrentPacketMillis;
				memcpy(&m_OutputPorts[nPortIndex].dataA, pArtDmx->Data, nDmxSlots);
				MergeDmxData(nPortIndex, m_OutputPorts[nPortIndex].dataA, nDmxSlots);
			} else if (ipA != m_ArtNetPacket.IPAddressFrom && ipB == m_ArtNetPacket.IPAddressFrom) {
#if defined ( ENABLE_SENDDIAG )
				SendDiag("7. continue merge", ARTNET_DP_LOW);
#endif
				m_OutputPorts[nPortIndex].nMillisB = m_nCurrentPacketMillis;
				memcpy(&m_OutputPorts[nPortIndex].dataB, pArtDmx->Data, nDmxSlots);
				MergeDmxData(nPortIndex, m_OutputPorts[nPortIndex].dataB, nDmxSlots);
			}
#ifndef NDEBUG
			else if (ipA == m_ArtNetPacket.IPAddressFrom && ipB == m_ArtNetPacket.IPAddressFrom) {
#if defined ( ENABLE_SENDDIAG )
				SendDiag("8. Source matches both buffers, this shouldn't be happening!", ARTNET_DP_LOW);
#endif
				return;
			} else if (ipA != m_ArtNetPacket.IPAddressFrom && ipB != m_ArtNetPacket.IPAddressFrom) {
#if defined ( ENABLE_SENDDIAG )
				SendDiag("9. More than two sources, discarding data", ARTNET_DP_LOW);
#endif
				return;
			}
#endif
			else {
#if defined ( ENABLE_SENDDIAG )
				SendDiag("0. No cases matched, this shouldn't happen!", ARTNET_DP_LOW);
#endif
				return;
			}

			if (!m_State.IsSynchronousMode) {
#if defined ( ENABLE_SENDDIAG )
				SendDiag("Send data", ARTNET_DP_LOW);
#endif
				m_pLightSet->SetData(nPortIndex, m_OutputPorts[nPortIndex].data, m_OutputPorts[nPortIndex].nLength);

				if (!m_IsLightSetRunning[nPortIndex]) {
					m_pLightSet->Start(nPortIndex);
					m_State.IsChanged |= (!m_IsLightSetRunning[nPortIndex]);
					m_IsLightSetRunning[nPortIndex] = true;
				}
			}

			m_State.bIsReceivingDmx = true;
		}
	}
}
