/**
 * @file artnetnodehandledmx.cpp
 *
 */
/**
 * Art-Net Designed by and Copyright Artistic Licence Holdings Ltd.
 */
/* Copyright (C) 2021-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#if !defined(__clang__)
# pragma GCC push_options
# pragma GCC optimize ("O2")
# pragma GCC optimize ("no-tree-loop-distribute-patterns")
#endif

#include <cstdint>
#include <cstring>
#include <algorithm>
#include <cassert>

#include "artnetnode.h"
#include "artnet.h"

#include "lightsetdata.h"

void ArtNetNode::UpdateMergeStatus(const uint32_t nPortIndex) {
	if (!m_State.IsMergeMode) {
		m_State.IsMergeMode = true;
		m_State.IsChanged = true;
	}

	m_OutputPort[nPortIndex].GoodOutput |= artnet::GoodOutput::OUTPUT_IS_MERGING;
}

void ArtNetNode::CheckMergeTimeouts(const uint32_t nPortIndex) {
	const auto nTimeOutAMillis = m_nCurrentPacketMillis - m_OutputPort[nPortIndex].SourceA.nMillis;

	if (nTimeOutAMillis > (artnet::MERGE_TIMEOUT_SECONDS * 1000U)) {
		m_OutputPort[nPortIndex].SourceA.nIp = 0;
		m_OutputPort[nPortIndex].GoodOutput &= static_cast<uint8_t>(~artnet::GoodOutput::OUTPUT_IS_MERGING);
	}

	const auto nTimeOutBMillis = m_nCurrentPacketMillis - m_OutputPort[nPortIndex].SourceB.nMillis;

	if (nTimeOutBMillis > (artnet::MERGE_TIMEOUT_SECONDS * 1000U)) {
		m_OutputPort[nPortIndex].SourceB.nIp = 0;
		m_OutputPort[nPortIndex].GoodOutput &= static_cast<uint8_t>(~artnet::GoodOutput::OUTPUT_IS_MERGING);
	}

	auto bIsMerging = false;

	for (uint32_t i = 0; i < artnetnode::MAX_PORTS; i++) {
		bIsMerging |= ((m_OutputPort[i].GoodOutput & artnet::GoodOutput::OUTPUT_IS_MERGING) != 0);
	}

	if (!bIsMerging) {
		m_State.IsChanged = true;
		m_State.IsMergeMode = false;
		SendDiag(artnet::PriorityCodes::DIAG_LOW, "%u: Leaving Merging Mode", nPortIndex);
	}
}

void ArtNetNode::HandleDmx() {
	const auto *const pArtDmx = reinterpret_cast<artnet::ArtDmx *>(m_pReceiveBuffer);
	const auto nDmxSlots = std::min(static_cast<uint32_t>(((pArtDmx->LengthHi << 8) & 0xff00) | pArtDmx->Length), artnet::DMX_LENGTH);

	for (uint32_t nPortIndex = 0; nPortIndex < artnetnode::MAX_PORTS; nPortIndex++) {
		if ((m_Node.Port[nPortIndex].direction == lightset::PortDir::OUTPUT)
		 && (m_Node.Port[nPortIndex].protocol == artnet::PortProtocol::ARTNET)
		 && (m_Node.Port[nPortIndex].PortAddress == pArtDmx->PortAddress)) {

			m_OutputPort[nPortIndex].GoodOutput |= artnet::GoodOutput::DATA_IS_BEING_TRANSMITTED;

			if (m_State.IsMergeMode) {
				if (__builtin_expect((!m_State.bDisableMergeTimeout), 1)) {
					CheckMergeTimeouts(nPortIndex);
				}
			}

			const auto ipA = m_OutputPort[nPortIndex].SourceA.nIp;
			const auto ipB = m_OutputPort[nPortIndex].SourceB.nIp;
			const auto mergeMode = ((m_OutputPort[nPortIndex].GoodOutput & artnet::GoodOutput::MERGE_MODE_LTP) == artnet::GoodOutput::MERGE_MODE_LTP) ? lightset::MergeMode::LTP : lightset::MergeMode::HTP;

			if (__builtin_expect((ipA == 0 && ipB == 0), 0)) {							// Case 1.
				m_OutputPort[nPortIndex].SourceA.nIp = m_nIpAddressFrom;
				m_OutputPort[nPortIndex].SourceA.nMillis = m_nCurrentPacketMillis;
				m_OutputPort[nPortIndex].SourceA.nPhysical = pArtDmx->Physical;
				lightset::Data::SetSourceA(nPortIndex, pArtDmx->Data, nDmxSlots);
				SendDiag(artnet::PriorityCodes::DIAG_LOW, "%u:%u 1. First packet", nPortIndex, pArtDmx->Physical);
			} else if (ipA == m_nIpAddressFrom && ipB == 0) {							// Case 2.
				if (m_OutputPort[nPortIndex].SourceA.nPhysical == pArtDmx->Physical) {
					m_OutputPort[nPortIndex].SourceA.nMillis = m_nCurrentPacketMillis;
					lightset::Data::SetSourceA(nPortIndex, pArtDmx->Data, nDmxSlots);
					SendDiag(artnet::PriorityCodes::DIAG_LOW, "%u:%u 2. continued transmission from the same ip (source A)", nPortIndex, pArtDmx->Physical);
				} else if (m_OutputPort[nPortIndex].SourceB.nPhysical != pArtDmx->Physical) {
					m_OutputPort[nPortIndex].SourceB.nIp = m_nIpAddressFrom;
					m_OutputPort[nPortIndex].SourceB.nMillis = m_nCurrentPacketMillis;
					m_OutputPort[nPortIndex].SourceB.nPhysical = pArtDmx->Physical;
					UpdateMergeStatus(nPortIndex);
					lightset::Data::MergeSourceB(nPortIndex, pArtDmx->Data, nDmxSlots, mergeMode);
					SendDiag(artnet::PriorityCodes::DIAG_LOW, "%u:%u 2. New source from same ip (source B), start the merge", nPortIndex, pArtDmx->Physical);
				} else {
					SendDiag(artnet::PriorityCodes::DIAG_LOW, "%u:%u 2. More than two sources, discarding data", nPortIndex, pArtDmx->Physical);
					return;
				}
			} else if (ipA == 0 && ipB == m_nIpAddressFrom) {							// Case 3.
				if (m_OutputPort[nPortIndex].SourceB.nPhysical == pArtDmx->Physical) {
					m_OutputPort[nPortIndex].SourceB.nMillis = m_nCurrentPacketMillis;
					lightset::Data::SetSourceB(nPortIndex, pArtDmx->Data, nDmxSlots);
					SendDiag(artnet::PriorityCodes::DIAG_LOW, "%u:%u 3. continued transmission from the same ip (source B)", nPortIndex, pArtDmx->Physical);
				} else if (m_OutputPort[nPortIndex].SourceA.nPhysical != pArtDmx->Physical) {
					m_OutputPort[nPortIndex].SourceA.nIp = m_nIpAddressFrom;
					m_OutputPort[nPortIndex].SourceA.nMillis = m_nCurrentPacketMillis;
					m_OutputPort[nPortIndex].SourceA.nPhysical = pArtDmx->Physical;
					UpdateMergeStatus(nPortIndex);
					lightset::Data::MergeSourceA(nPortIndex, pArtDmx->Data, nDmxSlots, mergeMode);
					SendDiag(artnet::PriorityCodes::DIAG_LOW, "%u:%u 3. New source from same ip (source A), start the merge", nPortIndex, pArtDmx->Physical);
				} else {
					SendDiag(artnet::PriorityCodes::DIAG_LOW, "%u:%u 3. More than two sources, discarding data", nPortIndex, pArtDmx->Physical);
					return;
				}
			} else if (ipA != m_nIpAddressFrom && ipB == 0) {							// Case 4.
				m_OutputPort[nPortIndex].SourceB.nIp = m_nIpAddressFrom;
				m_OutputPort[nPortIndex].SourceB.nMillis = m_nCurrentPacketMillis;
				m_OutputPort[nPortIndex].SourceB.nPhysical = pArtDmx->Physical;
				UpdateMergeStatus(nPortIndex);
				lightset::Data::MergeSourceB(nPortIndex, pArtDmx->Data, nDmxSlots, mergeMode);
				SendDiag(artnet::PriorityCodes::DIAG_LOW, "%u:%u 4. new source, start the merge", nPortIndex, pArtDmx->Physical);
			} else if (ipA == 0 && ipB != m_nIpAddressFrom) {							// Case 5.
				m_OutputPort[nPortIndex].SourceA.nIp = m_nIpAddressFrom;
				m_OutputPort[nPortIndex].SourceA.nMillis = m_nCurrentPacketMillis;
				m_OutputPort[nPortIndex].SourceA.nPhysical = pArtDmx->Physical;
				UpdateMergeStatus(nPortIndex);
				lightset::Data::MergeSourceA(nPortIndex, pArtDmx->Data, nDmxSlots, mergeMode);
				SendDiag(artnet::PriorityCodes::DIAG_LOW, "%u:%u 5. new source, start the merge", nPortIndex, pArtDmx->Physical);
			} else if (ipA == m_nIpAddressFrom && ipB != m_nIpAddressFrom) {			// Case 6.
				if (m_OutputPort[nPortIndex].SourceA.nPhysical == pArtDmx->Physical) {
					m_OutputPort[nPortIndex].SourceA.nMillis = m_nCurrentPacketMillis;
					UpdateMergeStatus(nPortIndex);
					lightset::Data::MergeSourceA(nPortIndex, pArtDmx->Data, nDmxSlots, mergeMode);
					SendDiag(artnet::PriorityCodes::DIAG_LOW, "%u:%u 6. continue merge (Source A)", nPortIndex, pArtDmx->Physical);
				} else {
					SendDiag(artnet::PriorityCodes::DIAG_MED, "%u:%u 6. More than two sources, discarding data", nPortIndex, pArtDmx->Physical);
					return;
				}
			} else if (ipA != m_nIpAddressFrom && ipB == m_nIpAddressFrom) {			// Case 7.
				if (m_OutputPort[nPortIndex].SourceB.nPhysical == pArtDmx->Physical) {
					m_OutputPort[nPortIndex].SourceB.nMillis = m_nCurrentPacketMillis;
					UpdateMergeStatus(nPortIndex);
					lightset::Data::MergeSourceB(nPortIndex, pArtDmx->Data, nDmxSlots, mergeMode);
					SendDiag(artnet::PriorityCodes::DIAG_LOW, "%u:%u 7. continue merge (Source B)", nPortIndex, pArtDmx->Physical);
				} else {
					SendDiag(artnet::PriorityCodes::DIAG_MED, "%u:%u 7. More than two sources, discarding data", nPortIndex, pArtDmx->Physical);
					puts("WARN: 7. More than two sources, discarding data");
					return;
				}
			} else if (ipA == m_nIpAddressFrom && ipB == m_nIpAddressFrom) {			// Case 8.
				if (m_OutputPort[nPortIndex].SourceA.nPhysical == pArtDmx->Physical) {
					m_OutputPort[nPortIndex].SourceA.nMillis = m_nCurrentPacketMillis;
					UpdateMergeStatus(nPortIndex);
					lightset::Data::MergeSourceA(nPortIndex, pArtDmx->Data, nDmxSlots, mergeMode);
					SendDiag(artnet::PriorityCodes::DIAG_LOW, "%u:%u 8. Source matches both ip, merging Physical (SourceA)", nPortIndex, pArtDmx->Physical);
				} else if (m_OutputPort[nPortIndex].SourceB.nPhysical == pArtDmx->Physical) {
					m_OutputPort[nPortIndex].SourceB.nMillis = m_nCurrentPacketMillis;
					UpdateMergeStatus(nPortIndex);
					lightset::Data::MergeSourceB(nPortIndex, pArtDmx->Data, nDmxSlots, mergeMode);
					SendDiag(artnet::PriorityCodes::DIAG_LOW, "%u:%u 8. Source matches both ip, merging Physical (SourceB)", nPortIndex, pArtDmx->Physical);
				} else {
					SendDiag(artnet::PriorityCodes::DIAG_LOW, "%u:%u 8. Source matches both ip, more than two sources, discarding data", nPortIndex, pArtDmx->Physical);
					puts("WARN: 8. Source matches both ip, discarding data");
					return;
				}
			}
#ifndef NDEBUG
			else if (ipA != m_nIpAddressFrom && ipB != m_nIpAddressFrom) {				// Case 9.
				SendDiag(artnet::PriorityCodes::DIAG_LOW, "%u: 9. More than two sources, discarding data", nPortIndex);
				puts("WARN: 9. More than two sources, discarding data");
				return;
			}
#endif
			else {																		// Case 0.
				SendDiag(artnet::PriorityCodes::DIAG_HIGH, "%u: 0. No cases matched, this shouldn't happen!", nPortIndex);
#ifndef NDEBUG
				puts("ERROR: 0. No cases matched, this shouldn't happen!");
#endif
				return;
			}

			if ((m_State.IsSynchronousMode) && ((m_OutputPort[nPortIndex].GoodOutput & artnet::GoodOutput::OUTPUT_IS_MERGING) != artnet::GoodOutput::OUTPUT_IS_MERGING)) {
				lightset::Data::Set(m_pLightSet, nPortIndex);
				m_OutputPort[nPortIndex].IsDataPending = true;
				SendDiag(artnet::PriorityCodes::DIAG_LOW, "%u: Buffering data", nPortIndex);
			} else {
				lightset::Data::Output(m_pLightSet, nPortIndex);

				if (!m_OutputPort[nPortIndex].IsTransmitting) {
					m_pLightSet->Start(nPortIndex);
					m_State.IsChanged = true;
					m_OutputPort[nPortIndex].IsTransmitting = true;
				}

				SendDiag(artnet::PriorityCodes::DIAG_LOW, "%u: Send data", nPortIndex);
			}

			m_State.nReceivingDmx |= (1U << static_cast<uint8_t>(lightset::PortDir::OUTPUT));
		}
	}
}
