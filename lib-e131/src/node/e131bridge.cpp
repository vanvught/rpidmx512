/**
 * @file e131bridge.cpp
 *
 */
/* Copyright (C) 2016-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cassert>

#include "e131bridge.h"

#include "e117const.h"

#if defined (E131_HAVE_DMXIN)
# include "dmx.h"
#endif

#include "lightset.h"
#include "lightsetdata.h"

#include "hardware.h"
#include "network.h"

#include "panel_led.h"

#include "debug.h"

E131Bridge *E131Bridge::s_pThis = nullptr;

E131Bridge::E131Bridge() {
	DEBUG_ENTRY

	assert(s_pThis == nullptr);
	s_pThis = this;

	memset(&m_Bridge, 0, sizeof(struct e131bridge::Bridge));

	for (auto& port : m_Bridge.Port) {
		port.direction = lightset::PortDir::DISABLE;
	}

	memset(&m_State, 0, sizeof(e131bridge::State));
	m_State.nPriority = e131::priority::LOWEST;
	m_State.failsafe = lightset::FailSafe::HOLD;

	for (uint32_t i = 0; i < e131bridge::MAX_PORTS; i++) {
		memset(&m_OutputPort[i], 0, sizeof(e131bridge::OutputPort));
		memset(&m_InputPort[i], 0, sizeof(e131bridge::InputPort));
		m_InputPort[i].nPriority = 100;
	}

#if defined (E131_HAVE_DMXIN) || defined (NODE_SHOWFILE)
	char aSourceName[e131::SOURCE_NAME_LENGTH];
	uint8_t nLength;
	snprintf(aSourceName, e131::SOURCE_NAME_LENGTH, "%.48s %s", Network::Get()->GetHostName(), Hardware::Get()->GetBoardName(nLength));
	SetSourceName(aSourceName);

	Hardware::Get()->GetUuid(m_Cid);
#endif

	m_nHandle = Network::Get()->Begin(e131::UDP_PORT);
	assert(m_nHandle != -1);

	DEBUG_EXIT
}

E131Bridge::~E131Bridge() {
	DEBUG_ENTRY

	DEBUG_EXIT
}

void E131Bridge::Start() {
#if defined (E131_HAVE_DMXIN)
	const auto nIpMulticast = network::convert_to_uint(239, 255, 0, 0);
	m_DiscoveryIpAddress = nIpMulticast | ((e131::universe::DISCOVERY & static_cast<uint32_t>(0xFF)) << 24) | ((e131::universe::DISCOVERY & 0xFF00) << 8);
	FillDataPacket();
	FillDiscoveryPacket();

	for (uint32_t nPortIndex = 0; nPortIndex < e131bridge::MAX_PORTS; nPortIndex++) {
		if (m_Bridge.Port[nPortIndex].direction == lightset::PortDir::INPUT) {
			Dmx::Get()->SetPortDirection(nPortIndex, dmx::PortDirection::INP, true);
		}
	}

	SetLocalMerging();
#endif

#if defined (OUTPUT_HAVE_STYLESWITCH)
	/*
	 * Make sure that the supported LightSet OutputSyle is correctly set
	 */
	if (m_pLightSet != nullptr) {
		for (uint32_t nPortIndex = 0; nPortIndex < e131bridge::MAX_PORTS; nPortIndex++) {
			if (m_Bridge.Port[nPortIndex].direction == lightset::PortDir::OUTPUT) {
				SetOutputStyle(nPortIndex, GetOutputStyle(nPortIndex));
			}
		}
	}
#endif

	m_State.status = e131bridge::Status::ON;
	Hardware::Get()->SetMode(hardware::ledblink::Mode::NORMAL);
}

void E131Bridge::Stop() {
	m_State.IsNetworkDataLoss = true;

	for (uint32_t nPortIndex = 0; nPortIndex < e131bridge::MAX_PORTS; nPortIndex++) {
		if (m_pLightSet != nullptr) {
			m_pLightSet->Stop(nPortIndex);
		}
		lightset::Data::ClearLength(nPortIndex);
	}

#if defined (E131_HAVE_DMXIN)
	for (uint32_t nPortIndex = 0; nPortIndex < e131bridge::MAX_PORTS; nPortIndex++) {
		if (m_Bridge.Port[nPortIndex].direction == lightset::PortDir::INPUT) {
			Dmx::Get()->SetPortDirection(nPortIndex, dmx::PortDirection::INP, false);
		}
	}
#endif

	m_State.status = e131bridge::Status::OFF;
	Hardware::Get()->SetMode(hardware::ledblink::Mode::OFF_OFF);
}

void E131Bridge::SetSynchronizationAddress(bool bSourceA, bool bSourceB, uint16_t nSynchronizationAddress) {
	DEBUG_ENTRY
	DEBUG_PRINTF("bSourceA=%d, bSourceB=%d, nSynchronizationAddress=%d", bSourceA, bSourceB, nSynchronizationAddress);

	assert(nSynchronizationAddress != 0);

	uint16_t *pSynchronizationAddressSource;

	if (bSourceA) {
		pSynchronizationAddressSource = &m_State.nSynchronizationAddressSourceA;
	} else if (bSourceB) {
		pSynchronizationAddressSource = &m_State.nSynchronizationAddressSourceB;
	} else {
		DEBUG_EXIT
		return; // Just make the compiler happy
	}

	if (*pSynchronizationAddressSource == 0) {
		*pSynchronizationAddressSource = nSynchronizationAddress;
		DEBUG_PUTS("SynchronizationAddressSource == 0");
	} else if (*pSynchronizationAddressSource != nSynchronizationAddress) {
		// e131bridge::MAX_PORTS forces to check all ports
		LeaveUniverse(e131bridge::MAX_PORTS, *pSynchronizationAddressSource);
		*pSynchronizationAddressSource = nSynchronizationAddress;
		DEBUG_PUTS("SynchronizationAddressSource != nSynchronizationAddress");
	} else {
		DEBUG_PUTS("Already received SynchronizationAddress");
		DEBUG_EXIT
		return;
	}

	Network::Get()->JoinGroup(m_nHandle, e131::universe_to_multicast_ip(nSynchronizationAddress));

	DEBUG_EXIT
}

void E131Bridge::LeaveUniverse(uint32_t nPortIndex, uint16_t nUniverse) {
	DEBUG_ENTRY
	DEBUG_PRINTF("nPortIndex=%d, nUniverse=%d", nPortIndex, nUniverse);

	for (uint32_t i = 0; i < e131bridge::MAX_PORTS; i++) {
		DEBUG_PRINTF("\tm_OutputPort[%d].nUniverse=%d", i, m_Bridge.Port[i].nUniverse);

		if (i == nPortIndex) {
			DEBUG_PUTS("continue");
			continue;
		}

		if (m_Bridge.Port[i].nUniverse == nUniverse) {
			DEBUG_EXIT
			return;
		}
	}

	Network::Get()->LeaveGroup(m_nHandle, e131::universe_to_multicast_ip(nUniverse));

	DEBUG_EXIT
}

void E131Bridge::SetLocalMerging() {
	DEBUG_ENTRY

	for (uint32_t nInputPortIndex = 0; nInputPortIndex < e131bridge::MAX_PORTS; nInputPortIndex++) {
		if ((m_Bridge.Port[nInputPortIndex].direction == lightset::PortDir::OUTPUT) || (m_Bridge.Port[nInputPortIndex].nUniverse == 0))  {
			continue;
		}

		m_Bridge.Port[nInputPortIndex].bLocalMerge = false;

		for (uint32_t nOutputPortIndex = 0; nOutputPortIndex < e131bridge::MAX_PORTS; nOutputPortIndex++) {
			if (m_Bridge.Port[nOutputPortIndex].direction == lightset::PortDir::INPUT) {
				continue;
			}

			DEBUG_PRINTF("nInputPortIndex=%u %u, nOutputPortIndex=%u %u",
					nInputPortIndex,
					m_Bridge.Port[nInputPortIndex].nUniverse,
					nOutputPortIndex,
					m_Bridge.Port[nOutputPortIndex].nUniverse);

			if (m_Bridge.Port[nInputPortIndex].nUniverse == m_Bridge.Port[nOutputPortIndex].nUniverse) {

				if (!m_Bridge.Port[nOutputPortIndex].bLocalMerge) {
					m_OutputPort[nOutputPortIndex].sourceA.nIp = Network::Get()->GetIp();
					DEBUG_PUTS("Local merge Source A");
				} else {
					m_OutputPort[nOutputPortIndex].sourceB.nIp = Network::Get()->GetIp();
					DEBUG_PUTS("Local merge Source B");
				}

				DEBUG_PUTS("");
				m_Bridge.Port[nInputPortIndex].bLocalMerge = true;
				m_Bridge.Port[nOutputPortIndex].bLocalMerge = true;
			}
		}
	}

	for (uint32_t nPortIndex = 0; nPortIndex < e131bridge::MAX_PORTS; nPortIndex++) {
		DEBUG_PRINTF("nPortIndex=%u, bLocalMerge=%c", nPortIndex, m_Bridge.Port[nPortIndex].bLocalMerge ? 'Y' : 'N');
	}

	DEBUG_EXIT
}

void E131Bridge::SetUniverse(const uint32_t nPortIndex, const lightset::PortDir portDir, const uint16_t nUniverse) {
	DEBUG_ENTRY
	DEBUG_PRINTF("nPortIndex=%u, dir=%s, nUniverse=%u", nPortIndex, lightset::get_direction(portDir), nUniverse);

	assert(nPortIndex < e131bridge::MAX_PORTS);
	assert(portDir <= lightset::PortDir::DISABLE);
	assert((nUniverse >= e131::universe::DEFAULT) && (nUniverse <= e131::universe::MAX));

	if (portDir == lightset::PortDir::DISABLE) {
		if (m_Bridge.Port[nPortIndex].direction == lightset::PortDir::OUTPUT) {
			assert(m_State.nEnableOutputPorts > 1);
			m_State.nEnableOutputPorts = static_cast<uint8_t>(m_State.nEnableOutputPorts - 1);
			LeaveUniverse(nPortIndex, nUniverse);
		}

#if defined (E131_HAVE_DMXIN)
		if (m_Bridge.Port[nPortIndex].direction == lightset::PortDir::INPUT) {
			assert(m_State.nEnabledInputPorts > 1);
			m_State.nEnabledInputPorts = static_cast<uint8_t>(m_State.nEnabledInputPorts - 1);
		}
#endif

		m_Bridge.Port[nPortIndex].direction = lightset::PortDir::DISABLE;

		DEBUG_EXIT
		return;
	}

#if defined (E131_HAVE_DMXIN)
	if (portDir == lightset::PortDir::INPUT) {
		if (m_Bridge.Port[nPortIndex].direction == lightset::PortDir::INPUT) {
			if (m_Bridge.Port[nPortIndex].nUniverse == nUniverse) {
				DEBUG_EXIT
				return;
			}
		} else {
			m_State.nEnabledInputPorts = static_cast<uint8_t>(m_State.nEnabledInputPorts + 1);
			assert(m_State.nEnabledInputPorts <= e131bridge::MAX_PORTS);
		}

		m_Bridge.Port[nPortIndex].direction = lightset::PortDir::INPUT;
		m_Bridge.Port[nPortIndex].nUniverse = nUniverse;
		m_InputPort[nPortIndex].nMulticastIp = e131::universe_to_multicast_ip(nUniverse);

		DEBUG_EXIT
		return;
	}
#endif

	if (portDir == lightset::PortDir::OUTPUT) {
		if (m_Bridge.Port[nPortIndex].direction == lightset::PortDir::OUTPUT) {
			if (m_Bridge.Port[nPortIndex].nUniverse == nUniverse) {
				DEBUG_EXIT
				return;
			} else {
				LeaveUniverse(nPortIndex, m_Bridge.Port[nPortIndex].nUniverse);
			}
		} else {
			m_State.nEnableOutputPorts = static_cast<uint8_t>(m_State.nEnableOutputPorts + 1);
			assert(m_State.nEnableOutputPorts <= e131bridge::MAX_PORTS);
		}

		Network::Get()->JoinGroup(m_nHandle, e131::universe_to_multicast_ip(nUniverse));

		m_Bridge.Port[nPortIndex].direction = lightset::PortDir::OUTPUT;
		m_Bridge.Port[nPortIndex].nUniverse = nUniverse;

	}
}

void E131Bridge::UpdateMergeStatus(const uint32_t nPortIndex) {
	if (!m_State.IsMergeMode) {
		m_State.IsMergeMode = true;
		m_State.IsChanged = true;
	}

	m_OutputPort[nPortIndex].IsMerging = true;
}

void E131Bridge::CheckMergeTimeouts(uint32_t nPortIndex) {
	assert(nPortIndex < e131bridge::MAX_PORTS);

	const auto timeOutA = m_nCurrentPacketMillis - m_OutputPort[nPortIndex].sourceA.nMillis;

	if (timeOutA > (e131::MERGE_TIMEOUT_SECONDS * 1000U)) {
		m_OutputPort[nPortIndex].sourceA.nIp = 0;
		memset(m_OutputPort[nPortIndex].sourceA.cid, 0, e131::CID_LENGTH);
		m_OutputPort[nPortIndex].IsMerging = false;
	}

	const auto timeOutB = m_nCurrentPacketMillis - m_OutputPort[nPortIndex].sourceB.nMillis;

	if (timeOutB > (e131::MERGE_TIMEOUT_SECONDS * 1000U)) {
		m_OutputPort[nPortIndex].sourceB.nIp = 0;
		memset(m_OutputPort[nPortIndex].sourceB.cid, 0, e131::CID_LENGTH);
		m_OutputPort[nPortIndex].IsMerging = false;
	}

	auto bIsMerging = false;

	for (uint32_t i = 0; i < e131bridge::MAX_PORTS; i++) {
		bIsMerging |= m_OutputPort[i].IsMerging;
	}

	if (!bIsMerging) {
		m_State.IsChanged = true;
		m_State.IsMergeMode = false;
	}
}

bool E131Bridge::IsPriorityTimeOut(uint32_t nPortIndex) const {
	assert(nPortIndex < e131bridge::MAX_PORTS);

	const auto timeOutA = m_nCurrentPacketMillis - m_OutputPort[nPortIndex].sourceA.nMillis;
	const auto timeOutB = m_nCurrentPacketMillis - m_OutputPort[nPortIndex].sourceB.nMillis;

	if ( (m_OutputPort[nPortIndex].sourceA.nIp != 0) && (m_OutputPort[nPortIndex].sourceB.nIp != 0) ) {
		if ( (timeOutA < (e131::PRIORITY_TIMEOUT_SECONDS * 1000U)) || (timeOutB < (e131::PRIORITY_TIMEOUT_SECONDS * 1000U)) ) {
			return false;
		} else {
			return true;
		}
	} else if ( (m_OutputPort[nPortIndex].sourceA.nIp != 0) && (m_OutputPort[nPortIndex].sourceB.nIp == 0) ) {
		if (timeOutA > (e131::PRIORITY_TIMEOUT_SECONDS * 1000U)) {
			return true;
		}
	} else if ( (m_OutputPort[nPortIndex].sourceA.nIp == 0) && (m_OutputPort[nPortIndex].sourceB.nIp != 0) ) {
		if (timeOutB > (e131::PRIORITY_TIMEOUT_SECONDS * 1000U)) {
			return true;
		}
	}

	return false;
}

bool E131Bridge::isIpCidMatch(const e131bridge::Source *const source) const {
	if (source->nIp != m_nIpAddressFrom) {
		return false;
	}

	const auto *const pRaw = reinterpret_cast<TE131RawPacket *>(m_pReceiveBuffer);

	if (memcmp(source->cid, pRaw->RootLayer.Cid, e131::CID_LENGTH) != 0) {
		return false;
	}

	return true;
}

void E131Bridge::HandleDmx() {
	const auto *const pData = reinterpret_cast<TE131DataPacket *>(m_pReceiveBuffer);
	const auto *const pDmxData = &pData->DMPLayer.PropertyValues[1];
	const auto nDmxSlots = __builtin_bswap16(pData->DMPLayer.PropertyValueCount) - 1U;

	for (uint32_t nPortIndex = 0; nPortIndex < e131bridge::MAX_PORTS; nPortIndex++) {
		if (m_Bridge.Port[nPortIndex].direction == lightset::PortDir::OUTPUT) {
			// Frame layer
			// 8.2 Association of Multicast Addresses and Universe
			// Note: The identity of the universe shall be determined by the universe number in the
			// packet and not assumed from the multicast address.
			if (pData->FrameLayer.Universe != __builtin_bswap16(m_Bridge.Port[nPortIndex].nUniverse)) {
				continue;
			}

			auto *pSourceA = &m_OutputPort[nPortIndex].sourceA;
			auto *pSourceB = &m_OutputPort[nPortIndex].sourceB;

			const auto ipA = pSourceA->nIp;
			const auto ipB = pSourceB->nIp;

			const auto isSourceA = isIpCidMatch(pSourceA);
			const auto isSourceB = isIpCidMatch(pSourceB);

			// 6.9.2 Sequence Numbering
			// Having first received a packet with sequence number A, a second packet with sequence number B
			// arrives. If, using signed 8-bit binary arithmetic, B – A is less than or equal to 0, but greater than -20 then
			// the packet containing sequence number B shall be deemed out of sequence and discarded
			if (isSourceA) {
				const auto diff = static_cast<int8_t>(pData->FrameLayer.SequenceNumber - pSourceA->nSequenceNumberData);
				pSourceA->nSequenceNumberData = pData->FrameLayer.SequenceNumber;
				if ((diff <= 0) && (diff > -20)) {
					continue;
				}
			} else if (isSourceB) {
				const auto diff = static_cast<int8_t>(pData->FrameLayer.SequenceNumber - pSourceB->nSequenceNumberData);
				pSourceB->nSequenceNumberData = pData->FrameLayer.SequenceNumber;
				if ((diff <= 0) && (diff > -20)) {
					continue;
				}
			}

			// This bit, when set to 1, indicates that the data in this packet is intended for use in visualization or media
			// server preview applications and shall not be used to generate live output.
			if ((pData->FrameLayer.Options & e131::OptionsMask::PREVIEW_DATA) != 0) {
				continue;
			}

			// Upon receipt of a packet containing this bit set to a value of 1, receiver shall enter network data loss condition.
			// Any property values in these packets shall be ignored.
			if ((pData->FrameLayer.Options & e131::OptionsMask::STREAM_TERMINATED) != 0) {
				if (isSourceA || isSourceB) {
					SetNetworkDataLossCondition(isSourceA, isSourceB);
				}
				continue;
			}

			if (m_State.IsMergeMode) {
				if (__builtin_expect((!m_State.bDisableMergeTimeout), 1)) {
					CheckMergeTimeouts(nPortIndex);
				}
			}

			if (pData->FrameLayer.Priority < m_State.nPriority ){
				if (!IsPriorityTimeOut(nPortIndex)) {
					continue;
				}
				m_State.nPriority = pData->FrameLayer.Priority;
			} else if (pData->FrameLayer.Priority > m_State.nPriority) {
				m_OutputPort[nPortIndex].sourceA.nIp = 0;
				m_OutputPort[nPortIndex].sourceB.nIp = 0;
				m_State.IsMergeMode = false;
				m_State.nPriority = pData->FrameLayer.Priority;
			}

			if ((ipA == 0) && (ipB == 0)) {
//				printf("1. First package from Source\n");
				pSourceA->nIp = m_nIpAddressFrom;
				pSourceA->nSequenceNumberData = pData->FrameLayer.SequenceNumber;
				memcpy(pSourceA->cid, pData->RootLayer.Cid, 16);
				pSourceA->nMillis = m_nCurrentPacketMillis;
				lightset::Data::SetSourceA(nPortIndex, pDmxData, nDmxSlots);
			} else if (isSourceA && (ipB == 0)) {
//				printf("2. Continue package from SourceA\n");
				pSourceA->nSequenceNumberData = pData->FrameLayer.SequenceNumber;
				pSourceA->nMillis = m_nCurrentPacketMillis;
				lightset::Data::SetSourceA(nPortIndex, pDmxData, nDmxSlots);
			} else if ((ipA == 0) && isSourceB) {
//				printf("3. Continue package from SourceB\n");
				pSourceB->nSequenceNumberData = pData->FrameLayer.SequenceNumber;
				pSourceB->nMillis = m_nCurrentPacketMillis;
				lightset::Data::SetSourceB(nPortIndex, pDmxData, nDmxSlots);
			} else if (!isSourceA && (ipB == 0)) {
//				printf("4. New ip, start merging\n");
				pSourceB->nIp = m_nIpAddressFrom;
				pSourceB->nSequenceNumberData = pData->FrameLayer.SequenceNumber;
				memcpy(pSourceB->cid, pData->RootLayer.Cid, 16);
				pSourceB->nMillis = m_nCurrentPacketMillis;
				UpdateMergeStatus(nPortIndex);
				lightset::Data::MergeSourceB(nPortIndex, pDmxData, nDmxSlots, m_OutputPort[nPortIndex].mergeMode);
			} else if ((ipA == 0) && !isSourceB) {
//				printf("5. New ip, start merging\n");
				pSourceA->nIp = m_nIpAddressFrom;
				pSourceA->nSequenceNumberData = pData->FrameLayer.SequenceNumber;
				memcpy(pSourceA->cid, pData->RootLayer.Cid, 16);
				pSourceA->nMillis = m_nCurrentPacketMillis;
				UpdateMergeStatus(nPortIndex);
				lightset::Data::MergeSourceA(nPortIndex, pDmxData, nDmxSlots, m_OutputPort[nPortIndex].mergeMode);
			} else if (isSourceA && !isSourceB) {
//				printf("6. Continue merging\n");
				pSourceA->nSequenceNumberData = pData->FrameLayer.SequenceNumber;
				pSourceA->nMillis = m_nCurrentPacketMillis;
				UpdateMergeStatus(nPortIndex);
				lightset::Data::MergeSourceA(nPortIndex, pDmxData, nDmxSlots, m_OutputPort[nPortIndex].mergeMode);
			} else if (!isSourceA && isSourceB) {
//				printf("7. Continue merging\n");
				pSourceB->nSequenceNumberData = pData->FrameLayer.SequenceNumber;
				pSourceB->nMillis = m_nCurrentPacketMillis;
				UpdateMergeStatus(nPortIndex);
				lightset::Data::MergeSourceB(nPortIndex, pDmxData, nDmxSlots, m_OutputPort[nPortIndex].mergeMode);
			}
#ifndef NDEBUG
			else if (isSourceA && isSourceB) {
				puts("WARN: 8. Source matches both ip, discarding data");
				return;
			} else if (!isSourceA && !isSourceB) {
				puts("WARN: 9. More than two sources, discarding data");
				return;
			}
			else {
				puts("ERROR: 0. No cases matched, this shouldn't happen!");
				return;
			}
#endif
			// This bit indicates whether to lock or revert to an unsynchronized state when synchronization is lost
			// (See Section 11 on Universe Synchronization and 11.1 for discussion on synchronization states).
			// When set to 0, components that had been operating in a synchronized state shall not update with any
			// new packets until synchronization resumes. When set to 1, once synchronization has been lost,
			// components that had been operating in a synchronized state need not wait for a new
			// E1.31 Synchronization Packet in order to update to the next E1.31 Data Packet.
			if ((pData->FrameLayer.Options & e131::OptionsMask::FORCE_SYNCHRONIZATION) == 0) {
				// 6.3.3.1 Synchronization Address Usage in an E1.31 Synchronization Packet
				// An E1.31 Synchronization Packet is sent to synchronize the E1.31 data on a specific universe number.
				// A Synchronization Address of 0 is thus meaningless, and shall not be transmitted.
				// Receivers shall ignore E1.31 Synchronization Packets containing a Synchronization Address of 0.
				if (pData->FrameLayer.SynchronizationAddress != 0) {
					if (!m_State.IsForcedSynchronized) {
						if (!(isSourceA || isSourceB)) {
							SetSynchronizationAddress((pSourceA->nIp != 0), (pSourceB->nIp != 0), __builtin_bswap16(pData->FrameLayer.SynchronizationAddress));
						} else {
							SetSynchronizationAddress(isSourceA, isSourceB, __builtin_bswap16(pData->FrameLayer.SynchronizationAddress));
						}
						m_State.IsForcedSynchronized = true;
						m_State.IsSynchronized = true;
					}
				}
			} else {
				m_State.IsForcedSynchronized = false;
			}

			const auto doUpdate = ((!m_State.IsSynchronized) || (m_State.bDisableSynchronize));

			if (doUpdate) {
				lightset::Data::Output(m_pLightSet, nPortIndex);

				if (!m_OutputPort[nPortIndex].IsTransmitting) {
					m_pLightSet->Start(nPortIndex);
					m_OutputPort[nPortIndex].IsTransmitting = true;
					m_State.IsChanged = true;
				}
			} else {
				lightset::Data::Set(m_pLightSet, nPortIndex);
				m_OutputPort[nPortIndex].IsDataPending = true;
			}

			m_State.nReceivingDmx |= (1U << static_cast<uint8_t>(lightset::PortDir::OUTPUT));
		}
	}
}

void E131Bridge::SetNetworkDataLossCondition(bool bSourceA, bool bSourceB) {
	DEBUG_ENTRY
	DEBUG_PRINTF("%d %d", bSourceA, bSourceB);

	m_State.IsChanged = true;
	auto doFailsafe = false;

	if (bSourceA && bSourceB) {
		m_State.IsNetworkDataLoss = true;
		m_State.IsMergeMode = false;
		m_State.IsSynchronized = false;
		m_State.IsForcedSynchronized = false;
		m_State.nPriority = e131::priority::LOWEST;

		for (uint32_t i = 0; i < e131bridge::MAX_PORTS; i++) {
			if (m_OutputPort[i].IsTransmitting) {
				doFailsafe = true;
				m_OutputPort[i].sourceA.nIp = 0;
				memset(m_OutputPort[i].sourceA.cid, 0, e131::CID_LENGTH);
				m_OutputPort[i].sourceB.nIp = 0;
				memset(m_OutputPort[i].sourceB.cid, 0, e131::CID_LENGTH);
				lightset::Data::ClearLength(i);
				m_OutputPort[i].IsTransmitting = false;
				m_OutputPort[i].IsMerging = false;
			}
		}
	} else {
		for (uint32_t i = 0; i < e131bridge::MAX_PORTS; i++) {
			if (m_OutputPort[i].IsTransmitting) {
				if ((bSourceA) && (m_OutputPort[i].sourceA.nIp != 0)) {
					m_OutputPort[i].sourceA.nIp = 0;
					memset(m_OutputPort[i].sourceA.cid, 0, e131::CID_LENGTH);
					m_OutputPort[i].IsMerging = false;
				}

				if ((bSourceB) && (m_OutputPort[i].sourceB.nIp != 0)) {
					m_OutputPort[i].sourceB.nIp = 0;
					memset(m_OutputPort[i].sourceB.cid, 0, e131::CID_LENGTH);
					m_OutputPort[i].IsMerging = false;
				}

				if (!m_State.IsMergeMode) {
					doFailsafe = true;
					lightset::Data::ClearLength(i);
					m_OutputPort[i].IsTransmitting = false;
				}
			}
		}
	}

	if (doFailsafe) {
		switch (m_State.failsafe) {
		case lightset::FailSafe::HOLD:
			break;
		case lightset::FailSafe::OFF:
			m_pLightSet->Blackout(true);
			break;
		case lightset::FailSafe::ON:
			m_pLightSet->FullOn();
			break;
		default:
			DEBUG_PRINTF("m_State.failsafe=%u", static_cast<uint32_t>(m_State.failsafe));
			assert(0);
			__builtin_unreachable();
			break;
		}
	}

	Hardware::Get()->SetMode(hardware::ledblink::Mode::NORMAL);
	hal::panel_led_off(hal::panelled::SACN);

	m_State.nReceivingDmx &= static_cast<uint8_t>(~(1U << static_cast<uint8_t>(lightset::PortDir::OUTPUT)));

#if defined (E131_HAVE_DMXIN)
	SetLocalMerging();
#endif

	DEBUG_EXIT
}

bool E131Bridge::IsValidRoot() {
	const auto *const pRaw = reinterpret_cast<TE131RawPacket *>(m_pReceiveBuffer);
	// 5 E1.31 use of the ACN Root Layer Protocol
	// Receivers shall discard the packet if the ACN Packet Identifier is not valid.
	if (memcmp(pRaw->RootLayer.ACNPacketIdentifier, E117Const::ACN_PACKET_IDENTIFIER, e117::PACKET_IDENTIFIER_LENGTH) != 0) {
		return false;
	}
	
	if (pRaw->RootLayer.Vector != __builtin_bswap32(e131::vector::root::DATA)
			 && (pRaw->RootLayer.Vector != __builtin_bswap32(e131::vector::root::EXTENDED)) ) {
		return false;
	}

	return true;
}

bool E131Bridge::IsValidDataPacket() {
	const auto *const pData = reinterpret_cast<TE131DataPacket *>(m_pReceiveBuffer);
	// The DMP Layer's Vector shall be set to 0x02, which indicates a DMP Set Property message by
	// transmitters. Receivers shall discard the packet if the received value is not 0x02.
	if (pData->DMPLayer.Vector != e131::vector::dmp::SET_PROPERTY) {
		return false;
	}
	// Transmitters shall set the DMP Layer's Address Type and Data Type to 0xa1. Receivers shall discard the
	// packet if the received value is not 0xa1.
	if (pData->DMPLayer.Type != 0xa1) {
		return false;
	}
	// Transmitters shall set the DMP Layer's First Property Address to 0x0000. Receivers shall discard the
	// packet if the received value is not 0x0000.
	if (pData->DMPLayer.FirstAddressProperty != __builtin_bswap16(0x0000)) {
		return false;
	}
	// Transmitters shall set the DMP Layer's Address Increment to 0x0001. Receivers shall discard the packet if
	// the received value is not 0x0001.
	if (pData->DMPLayer.AddressIncrement != __builtin_bswap16(0x0001)) {
		return false;
	}

	return true;
}

void E131Bridge::Process() {
	m_State.IsNetworkDataLoss = false;
	m_nPreviousPacketMillis = m_nCurrentPacketMillis;

	if (m_State.IsSynchronized && !m_State.IsForcedSynchronized) {
		if ((m_nCurrentPacketMillis - m_State.SynchronizationTime) >= static_cast<uint32_t>(e131::NETWORK_DATA_LOSS_TIMEOUT_SECONDS * 1000)) {
			m_State.IsSynchronized = false;
		}
	}

	bool isActive = false;

	if (m_pLightSet != nullptr) {
		const auto *const pRaw = reinterpret_cast<TE131RawPacket *>(m_pReceiveBuffer);
		const auto nRootVector = __builtin_bswap32(pRaw->RootLayer.Vector);

		if (nRootVector == e131::vector::root::DATA) {
			if (IsValidDataPacket()) {
				HandleDmx();
				isActive = true;
			}
		} else if (nRootVector == e131::vector::root::EXTENDED) {
			const auto nFramingVector = __builtin_bswap32(pRaw->FrameLayer.Vector);
			if (nFramingVector == e131::vector::extended::SYNCHRONIZATION) {
				HandleSynchronization();
				isActive = true;
			}
		} else {
			DEBUG_PRINTF("Not supported Root Vector : 0x%x", nRootVector);
		}
	}

#if defined (E131_HAVE_DMXIN)
	HandleDmxIn();
	SendDiscoveryPacket();
#endif

	// The hardware::ledblink::Mode::FAST is for RDM Identify (Art-Net 4)
	if (m_bEnableDataIndicator && (Hardware::Get()->GetMode() != hardware::ledblink::Mode::FAST)) {
		if (m_State.nReceivingDmx != 0) {
			Hardware::Get()->SetMode(hardware::ledblink::Mode::DATA);
		} else {
			Hardware::Get()->SetMode(hardware::ledblink::Mode::NORMAL);
		}
	}

	if (isActive) {
		hal::panel_led_on(hal::panelled::SACN);
	} else {
		hal::panel_led_off(hal::panelled::SACN);
	}
}
