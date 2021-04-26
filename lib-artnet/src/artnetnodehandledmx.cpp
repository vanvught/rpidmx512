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

#include <stdint.h>
#include <string.h>
#include <algorithm>
#include <cassert>

#include "artnetnode.h"
#include "artnet.h"

using namespace artnet;

bool ArtNetNode::IsDmxDataChanged(uint8_t nPortId, const uint8_t *pData, uint16_t nLength) {
	assert(pData != nullptr);

	auto isChanged = false;
	const auto *pSrc = pData;
	auto *pDst = m_OutputPorts[nPortId].data;

	if (nLength != m_OutputPorts[nPortId].nLength) {
		m_OutputPorts[nPortId].nLength = nLength;

		for (uint32_t i = 0; i < nLength; i++) {
			*pDst++ = *pSrc++;
		}

		return true;
	}

	for (uint32_t i = 0; i < nLength; i++) {
		if (*pDst != *pSrc) {
			isChanged = true;
		}
		*pDst++ = *pSrc++;
	}

	return isChanged;
}

bool ArtNetNode::IsMergedDmxDataChanged(uint8_t nPortId, const uint8_t *pData, uint16_t nLength) {
	assert(pData != nullptr);

	auto isChanged = false;

	if (!m_State.IsMergeMode) {
		m_State.IsMergeMode = true;
		m_State.IsChanged = true;
	}

	m_OutputPorts[nPortId].port.nStatus |= GO_OUTPUT_IS_MERGING;

	if (m_OutputPorts[nPortId].mergeMode == Merge::HTP) {

		if (nLength != m_OutputPorts[nPortId].nLength) {
			m_OutputPorts[nPortId].nLength = nLength;
			for (uint32_t i = 0; i < nLength; i++) {
				uint8_t data = std::max(m_OutputPorts[nPortId].dataA[i], m_OutputPorts[nPortId].dataB[i]);
				m_OutputPorts[nPortId].data[i] = data;
			}
			return true;
		}

		for (uint32_t i = 0; i < nLength; i++) {
			uint8_t data = std::max(m_OutputPorts[nPortId].dataA[i], m_OutputPorts[nPortId].dataB[i]);
			if (data != m_OutputPorts[nPortId].data[i]) {
				m_OutputPorts[nPortId].data[i] = data;
				isChanged = true;
			}
		}

		return isChanged;
	} else {
		return IsDmxDataChanged(nPortId, pData, nLength);
	}
}

void ArtNetNode::CheckMergeTimeouts(uint8_t nPortId) {
	const uint32_t nTimeOutAMillis = m_nCurrentPacketMillis - m_OutputPorts[nPortId].nMillisA;

	if (nTimeOutAMillis > (artnet::MERGE_TIMEOUT_SECONDS * 1000)) {
		m_OutputPorts[nPortId].ipA = 0;
		m_OutputPorts[nPortId].port.nStatus &= (~GO_OUTPUT_IS_MERGING);
	}

	const uint32_t nTimeOutBMillis = m_nCurrentPacketMillis - m_OutputPorts[nPortId].nMillisB;

	if (nTimeOutBMillis > (artnet::MERGE_TIMEOUT_SECONDS * 1000)) {
		m_OutputPorts[nPortId].ipB = 0;
		m_OutputPorts[nPortId].port.nStatus &= (~GO_OUTPUT_IS_MERGING);
	}

	bool bIsMerging = false;

	for (uint32_t i = 0; i < (ArtNet::MAX_PORTS * m_nPages); i++) {
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

	auto data_length = (static_cast<uint32_t>(pArtDmx->LengthHi << 8) & 0xff00) | pArtDmx->Length;
	data_length = std::min(data_length, ArtNet::DMX_LENGTH);

	for (uint32_t i = 0; i < (ArtNet::MAX_PORTS * m_nPages); i++) {

		if (m_OutputPorts[i].bIsEnabled && (m_OutputPorts[i].tPortProtocol == PortProtocol::ARTNET) && (pArtDmx->PortAddress == m_OutputPorts[i].port.nPortAddress)) {

			uint32_t ipA = m_OutputPorts[i].ipA;
			uint32_t ipB = m_OutputPorts[i].ipB;

			bool sendNewData = false;

			m_OutputPorts[i].port.nStatus = m_OutputPorts[i].port.nStatus | GO_DATA_IS_BEING_TRANSMITTED;

			if (m_State.IsMergeMode) {
				if (__builtin_expect((!m_State.bDisableMergeTimeout), 1)) {
					CheckMergeTimeouts(i);
				}
			}

			if (ipA == 0 && ipB == 0) {
#if defined ( ENABLE_SENDDIAG )
				SendDiag("1. first packet recv on this port", ARTNET_DP_LOW);
#endif
				m_OutputPorts[i].ipA = m_ArtNetPacket.IPAddressFrom;
				m_OutputPorts[i].nMillisA = m_nCurrentPacketMillis;
				memcpy(&m_OutputPorts[i].dataA, pArtDmx->Data, data_length);
				sendNewData = IsDmxDataChanged(i, pArtDmx->Data, data_length);
			} else if (ipA == m_ArtNetPacket.IPAddressFrom && ipB == 0) {
#if defined ( ENABLE_SENDDIAG )
				SendDiag("2. continued transmission from the same ip (source A)", ARTNET_DP_LOW);
#endif
				m_OutputPorts[i].nMillisA = m_nCurrentPacketMillis;
				memcpy(&m_OutputPorts[i].dataA, pArtDmx->Data, data_length);
				sendNewData = IsDmxDataChanged(i, pArtDmx->Data, data_length);
			} else if (ipA == 0 && ipB == m_ArtNetPacket.IPAddressFrom) {
#if defined ( ENABLE_SENDDIAG )
				SendDiag("3. continued transmission from the same ip (source B)", ARTNET_DP_LOW);
#endif
				m_OutputPorts[i].nMillisB = m_nCurrentPacketMillis;
				memcpy(&m_OutputPorts[i].dataB, pArtDmx->Data, data_length);
				sendNewData = IsDmxDataChanged(i, pArtDmx->Data, data_length);
			} else if (ipA != m_ArtNetPacket.IPAddressFrom && ipB == 0) {
#if defined ( ENABLE_SENDDIAG )
				SendDiag("4. new source, start the merge", ARTNET_DP_LOW);
#endif
				m_OutputPorts[i].ipB = m_ArtNetPacket.IPAddressFrom;
				m_OutputPorts[i].nMillisB = m_nCurrentPacketMillis;
				memcpy(&m_OutputPorts[i].dataB, pArtDmx->Data, data_length);
				sendNewData = IsMergedDmxDataChanged(i, m_OutputPorts[i].dataB, data_length);
			} else if (ipA == 0 && ipB != m_ArtNetPacket.IPAddressFrom) {
#if defined ( ENABLE_SENDDIAG )
				SendDiag("5. new source, start the merge", ARTNET_DP_LOW);
#endif
				m_OutputPorts[i].ipA = m_ArtNetPacket.IPAddressFrom;
				m_OutputPorts[i].nMillisA = m_nCurrentPacketMillis;
				memcpy(&m_OutputPorts[i].dataA, pArtDmx->Data, data_length);
				sendNewData = IsMergedDmxDataChanged(i, m_OutputPorts[i].dataA, data_length);
			} else if (ipA == m_ArtNetPacket.IPAddressFrom && ipB != m_ArtNetPacket.IPAddressFrom) {
#if defined ( ENABLE_SENDDIAG )
				SendDiag("6. continue merge", ARTNET_DP_LOW);
#endif
				m_OutputPorts[i].nMillisA = m_nCurrentPacketMillis;
				memcpy(&m_OutputPorts[i].dataA, pArtDmx->Data, data_length);
				sendNewData = IsMergedDmxDataChanged(i, m_OutputPorts[i].dataA, data_length);
			} else if (ipA != m_ArtNetPacket.IPAddressFrom && ipB == m_ArtNetPacket.IPAddressFrom) {
#if defined ( ENABLE_SENDDIAG )
				SendDiag("7. continue merge", ARTNET_DP_LOW);
#endif
				m_OutputPorts[i].nMillisB = m_nCurrentPacketMillis;
				memcpy(&m_OutputPorts[i].dataB, pArtDmx->Data, data_length);
				sendNewData = IsMergedDmxDataChanged(i, m_OutputPorts[i].dataB, data_length);
			} else if (ipA == m_ArtNetPacket.IPAddressFrom && ipB == m_ArtNetPacket.IPAddressFrom) {
#if defined ( ENABLE_SENDDIAG )
				SendDiag("8. Source matches both buffers, this shouldn't be happening!", ARTNET_DP_LOW);
#endif
				return;
			} else if (ipA != m_ArtNetPacket.IPAddressFrom && ipB != m_ArtNetPacket.IPAddressFrom) {
#if defined ( ENABLE_SENDDIAG )
				SendDiag("9. More than two sources, discarding data", ARTNET_DP_LOW);
#endif
				return;
			} else {
#if defined ( ENABLE_SENDDIAG )
				SendDiag("0. No cases matched, this shouldn't happen!", ARTNET_DP_LOW);
#endif
				return;
			}

			if (sendNewData || m_bDirectUpdate) {
				if (!m_State.IsSynchronousMode) {
#if defined ( ENABLE_SENDDIAG )
					SendDiag("Send new data", ARTNET_DP_LOW);
#endif
					m_pLightSet->SetData(i, m_OutputPorts[i].data, m_OutputPorts[i].nLength);

					if(!m_IsLightSetRunning[i]) {
						m_pLightSet->Start(i);
						m_State.IsChanged |= (!m_IsLightSetRunning[i]);
						m_IsLightSetRunning[i] = true;
					}
				} else {
#if defined ( ENABLE_SENDDIAG )
					SendDiag("DMX data pending", ARTNET_DP_LOW);
#endif
					m_OutputPorts[i].IsDataPending = sendNewData;
				}
			} else {
#if defined ( ENABLE_SENDDIAG )
				SendDiag("Data not changed", ARTNET_DP_LOW);
#endif
			}

			m_State.bIsReceivingDmx = true;
		}
	}
}
