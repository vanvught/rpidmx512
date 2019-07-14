/**
 * @file e131bridge.cpp
 *
 */
/* Copyright (C) 2016-2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <assert.h>

#include "debug.h"

#if !defined(BARE_METAL)
 #include <arpa/inet.h>
#endif

#ifndef MIN
 #define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifndef MAX
 #define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

#include "e131bridge.h"

#include "lightset.h"

#include "hardware.h"
#include "network.h"
#include "ledblink.h"

static const uint8_t DEVICE_SOFTWARE_VERSION[] = { 1, 11 };
static const uint8_t ACN_PACKET_IDENTIFIER[E131_PACKET_IDENTIFIER_LENGTH] = { 0x41, 0x53, 0x43, 0x2d, 0x45, 0x31, 0x2e, 0x31, 0x37, 0x00, 0x00, 0x00 }; ///< 5.3 ACN Packet Identifier

E131Bridge::E131Bridge(void) :
	m_nHandle(-1),
	m_pLightSet(0),
	m_bDirectUpdate(false),
	m_bEnableDataIndicator(true),
	m_nCurrentPacketMillis(0),
	m_nPreviousPacketMillis(0)
{
	assert(Hardware::Get() != 0);
	assert(Network::Get() != 0);

	for (uint32_t i = 0; i < E131_MAX_PORTS; i++) {
		memset(&m_OutputPort[i], 0, sizeof(struct TE131OutputPort));
		m_OutputPort[i].nUniverse = E131_UNIVERSE_DEFAULT;
		m_OutputPort[i].mergeMode = E131_MERGE_HTP;
	}

	memset(&m_State, 0, sizeof(struct TE131BridgeState));
	m_State.IsNetworkDataLoss = true;
	m_State.IsMergeMode = false;
	m_State.IsSynchronized = false;
	m_State.IsForcedSynchronized = false;
	m_State.nPriority = E131_PRIORITY_LOWEST;
}

E131Bridge::~E131Bridge(void) {
	Stop();
}

void E131Bridge::Start(void) {
	assert(m_pLightSet != 0);
	assert(m_nHandle != -1);

	m_nHandle = Network::Get()->Begin(E131_DEFAULT_PORT);
	LedBlink::Get()->SetMode(LEDBLINK_MODE_NORMAL);
}

void E131Bridge::Stop(void) {
	m_State.IsNetworkDataLoss = true;

	for (uint32_t i = 0; i < E131_MAX_PORTS; i++) {
		m_pLightSet->Stop(i);
		m_OutputPort[i].length = 0;
		m_OutputPort[i].IsDataPending = false;
	}
}

const uint8_t *E131Bridge::GetSoftwareVersion(void) {
	return DEVICE_SOFTWARE_VERSION;
}

void E131Bridge::SetOutput(LightSet *pLightSet) {
	assert(pLightSet != 0);

	m_pLightSet = pLightSet;
}

uint32_t E131Bridge::UniverseToMulticastIp(uint16_t nUniverse) const {
	struct in_addr group_ip;
	(void) inet_aton("239.255.0.0", &group_ip);

	const uint32_t nMulticastIp = group_ip.s_addr
			| ((uint32_t) (((uint32_t) nUniverse & (uint32_t) 0xFF) << 24))
			| ((uint32_t) (((uint32_t) nUniverse & (uint32_t) 0xFF00) << 8));

	DEBUG_PRINTF(IPSTR, IP2STR(nMulticastIp));

	return nMulticastIp;
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
		// E131_MAX_PORTS forces to check all ports
		LeaveUniverse(E131_MAX_PORTS, *pSynchronizationAddressSource);
		*pSynchronizationAddressSource = nSynchronizationAddress;
		DEBUG_PUTS("SynchronizationAddressSource != nSynchronizationAddress");
	} else {
		DEBUG_PUTS("Already received SynchronizationAddress");
		DEBUG_EXIT
		return;
	}

	Network::Get()->JoinGroup(m_nHandle, UniverseToMulticastIp(nSynchronizationAddress));

	DEBUG_EXIT
}

void E131Bridge::LeaveUniverse(uint8_t nPortIndex, uint16_t nUniverse) {
	DEBUG_ENTRY
	DEBUG_PRINTF("nPortIndex=%d, nUniverse=%d", nPortIndex, nUniverse);

	for (uint32_t i = 0; i < E131_MAX_PORTS; i++) {
		DEBUG_PRINTF("\tnm_OutputPort[%d].nUniverse=%d", i, m_OutputPort[i].nUniverse);

		if (i == nPortIndex) {
			continue;
		}
		if (m_OutputPort[i].nUniverse == nUniverse) {
			DEBUG_EXIT
			return;
		}
	}

	Network::Get()->LeaveGroup(m_nHandle, UniverseToMulticastIp(nUniverse));

	DEBUG_EXIT
}

void E131Bridge::SetUniverse(uint8_t nPortIndex, TE131PortDir dir, uint16_t nUniverse) {
	assert(nPortIndex < E131_MAX_PORTS);
	assert(dir <= E131_DISABLE_PORT);
	assert((nUniverse >= E131_UNIVERSE_DEFAULT) && (nUniverse <= E131_UNIVERSE_MAX));

	if (dir == E131_INPUT_PORT) {
		// Not supported. We have output ports only.
		return;
	}

	if (dir == E131_DISABLE_PORT) {
		if (m_OutputPort[nPortIndex].bIsEnabled) {
			m_OutputPort[nPortIndex].bIsEnabled = false;
			m_State.nActivePorts = m_State.nActivePorts - 1;
			LeaveUniverse(nPortIndex, nUniverse);
		}
		return;
	}

	if (m_OutputPort[nPortIndex].bIsEnabled) {
		if (m_OutputPort[nPortIndex].nUniverse == nUniverse) {
			return;
		} else {
			LeaveUniverse(nPortIndex, nUniverse);
		}
	} else {
		m_State.nActivePorts = m_State.nActivePorts + 1;
		assert(m_State.nActivePorts <= E131_MAX_PORTS);
		m_OutputPort[nPortIndex].bIsEnabled = true;
	}

	Network::Get()->JoinGroup(m_nHandle, UniverseToMulticastIp(nUniverse));

	m_OutputPort[nPortIndex].nUniverse = nUniverse;
}

bool E131Bridge::GetUniverse(uint8_t nPortIndex, uint16_t &nUniverse) const{
	assert(nPortIndex < E131_MAX_PORTS);

	nUniverse = m_OutputPort[nPortIndex].nUniverse;

	return m_OutputPort[nPortIndex].bIsEnabled;
}

void E131Bridge::SetMergeMode(uint8_t nPortIndex, TE131Merge tE131Merge) {
	assert(nPortIndex < E131_MAX_PORTS);

	m_OutputPort[nPortIndex].mergeMode = tE131Merge;
}

TE131Merge E131Bridge::GetMergeMode(uint8_t nPortIndex) const {
	assert(nPortIndex < E131_MAX_PORTS);

	return m_OutputPort[nPortIndex].mergeMode;
}

bool E131Bridge::IsDmxDataChanged(uint8_t nPortIndex, const uint8_t *pData, uint16_t nLength) {
	assert(nPortIndex < E131_MAX_PORTS);
	assert(pData != 0);

	bool isChanged = false;

	uint8_t *src = (uint8_t *)pData;
	uint8_t *dst = (uint8_t *)m_OutputPort[nPortIndex].data;

	if (nLength != m_OutputPort[nPortIndex].length) {
		m_OutputPort[nPortIndex].length = nLength;
		for (unsigned i = 0 ; i < E131_DMX_LENGTH; i++) {
			*dst++ = *src++;
		}
		return true;
	}

	for (unsigned i = 0; i < E131_DMX_LENGTH; i++) {
		if (*dst != *src) {
			*dst = *src;
			isChanged = true;
		}
		dst++;
		src++;
	}

	return isChanged;
}

bool E131Bridge::IsMergedDmxDataChanged(uint8_t nPortIndex, const uint8_t *pData, uint16_t nLength) {
	assert(nPortIndex < E131_MAX_PORTS);
	assert(pData != 0);

	bool isChanged = false;

	if (!m_State.IsMergeMode) {
		m_State.IsMergeMode = true;
		m_State.IsChanged = true;
	}

	m_OutputPort[nPortIndex].IsMerging = true;

	if (m_OutputPort[nPortIndex].mergeMode == E131_MERGE_HTP) {

		if (nLength != m_OutputPort[nPortIndex].length) {
			m_OutputPort[nPortIndex].length = nLength;
			for (unsigned i = 0; i < nLength; i++) {
				uint8_t data = MAX(m_OutputPort[nPortIndex].sourceA.data[i], m_OutputPort[nPortIndex].sourceB.data[i]);
				m_OutputPort[nPortIndex].data[i] = data;
			}
			return true;
		}

		for (unsigned i = 0; i < nLength; i++) {
			uint8_t data = MAX(m_OutputPort[nPortIndex].sourceA.data[i], m_OutputPort[nPortIndex].sourceB.data[i]);
			if (data != m_OutputPort[nPortIndex].data[i]) {
				m_OutputPort[nPortIndex].data[i] = data;
				isChanged = true;
			}
		}

		return isChanged;
	} else {
		return IsDmxDataChanged(nPortIndex, pData, nLength);
	}
}

void E131Bridge::CheckMergeTimeouts(uint8_t nPortIndex) {
	assert(nPortIndex < E131_MAX_PORTS);

	const uint32_t timeOutA = m_nCurrentPacketMillis - m_OutputPort[nPortIndex].sourceA.time;
	const uint32_t timeOutB = m_nCurrentPacketMillis - m_OutputPort[nPortIndex].sourceB.time;

	if (timeOutA > (uint32_t) (E131_MERGE_TIMEOUT_SECONDS * 1000)) {
		m_OutputPort[nPortIndex].sourceA.ip = 0;
		memset(m_OutputPort[nPortIndex].sourceA.cid, 0, E131_CID_LENGTH);
		m_OutputPort[nPortIndex].IsMerging = false;
	}

	if (timeOutB > (uint32_t) (E131_MERGE_TIMEOUT_SECONDS * 1000)) {
		m_OutputPort[nPortIndex].sourceB.ip = 0;
		memset(m_OutputPort[nPortIndex].sourceB.cid, 0, E131_CID_LENGTH);
		m_OutputPort[nPortIndex].IsMerging = false;
	}

	bool bIsMerging = false;

	for (uint32_t i = 0; i < E131_MAX_PORTS; i++) {
		bIsMerging |= m_OutputPort[i].IsMerging;
	}

	if (!bIsMerging) {
		m_State.IsChanged = true;
		m_State.IsMergeMode = false;
	}

}

bool E131Bridge::IsPriorityTimeOut(uint8_t nPortIndex) {
	assert(nPortIndex < E131_MAX_PORTS);

	const uint32_t timeOutA = m_nCurrentPacketMillis - m_OutputPort[nPortIndex].sourceA.time;
	const uint32_t timeOutB = m_nCurrentPacketMillis - m_OutputPort[nPortIndex].sourceB.time;

	if ( (m_OutputPort[nPortIndex].sourceA.ip != 0) && (m_OutputPort[nPortIndex].sourceB.ip != 0) ) {
		if ( (timeOutA < (uint32_t)(E131_PRIORITY_TIMEOUT_SECONDS * 1000)) || (timeOutB < (uint32_t)(E131_PRIORITY_TIMEOUT_SECONDS * 1000)) ) {
			return false;
		} else {
			return true;
		}
	} else if ( (m_OutputPort[nPortIndex].sourceA.ip != 0) && (m_OutputPort[nPortIndex].sourceB.ip == 0) ) {
		if (timeOutA > (uint32_t)(E131_PRIORITY_TIMEOUT_SECONDS * 1000)) {
			return true;
		}
	} else if ( (m_OutputPort[nPortIndex].sourceA.ip == 0) && (m_OutputPort[nPortIndex].sourceB.ip != 0) ) {
		if (timeOutB > (uint32_t)(E131_PRIORITY_TIMEOUT_SECONDS * 1000)) {
			return true;
		}
	}

	return false;
}

bool E131Bridge::isIpCidMatch(const struct TSource *source) {
	if (source->ip != m_E131.IPAddressFrom) {
		return false;
	}

	if (memcmp(source->cid, m_E131.E131Packet.Raw.RootLayer.Cid, E131_CID_LENGTH) != 0) {
		return false;
	}

	return true;
}

void E131Bridge::HandleDmx(void) {
	const uint8_t *p = &m_E131.E131Packet.Data.DMPLayer.PropertyValues[1];
	const uint16_t slots = __builtin_bswap16(m_E131.E131Packet.Data.DMPLayer.PropertyValueCount) - (uint16_t) 1;

	for (uint32_t i = 0; i < E131_MAX_PORTS; i++) {
		if (!m_OutputPort[i].bIsEnabled) {
			continue;
		}

		// Frame layer
		// 8.2 Association of Multicast Addresses and Universe
		// Note: The identity of the universe shall be determined by the universe number in the
		// packet and not assumed from the multicast address.
		if (m_E131.E131Packet.Data.FrameLayer.Universe != __builtin_bswap16(m_OutputPort[i].nUniverse)) {
			continue;
		}

		struct TSource *pSourceA = &m_OutputPort[i].sourceA;
		struct TSource *pSourceB = &m_OutputPort[i].sourceB;

		const uint32_t ipA = pSourceA->ip;
		const uint32_t ipB = pSourceB->ip;

		const bool isSourceA = isIpCidMatch(pSourceA);
		const bool isSourceB = isIpCidMatch(pSourceB);

		bool sendNewData = false;

		// 6.9.2 Sequence Numbering
		// Having first received a packet with sequence number A, a second packet with sequence number B
		// arrives. If, using signed 8-bit binary arithmetic, B – A is less than or equal to 0, but greater than -20 then
		// the packet containing sequence number B shall be deemed out of sequence and discarded
		if (isSourceA) {
			const int8_t diff = (int8_t) (m_E131.E131Packet.Data.FrameLayer.SequenceNumber - pSourceA->sequenceNumberData);
			pSourceA->sequenceNumberData = m_E131.E131Packet.Data.FrameLayer.SequenceNumber;
			if ((diff <= (int8_t) 0) && (diff > (int8_t) -20)) {
				continue;
			}
		} else if (isSourceB) {
			const int8_t diff = (int8_t) (m_E131.E131Packet.Data.FrameLayer.SequenceNumber - pSourceB->sequenceNumberData);
			pSourceB->sequenceNumberData = m_E131.E131Packet.Data.FrameLayer.SequenceNumber;
			if ((diff <= (int8_t) 0) && (diff > (int8_t) -20)) {
				continue;
			}
		}

		// This bit, when set to 1, indicates that the data in this packet is intended for use in visualization or media
		// server preview applications and shall not be used to generate live output.
		if ((m_E131.E131Packet.Data.FrameLayer.Options & E131_OPTIONS_MASK_PREVIEW_DATA) != 0) {
			continue;
		}

		// Upon receipt of a packet containing this bit set to a value of 1, receiver shall enter network data loss condition.
		// Any property values in these packets shall be ignored.
		if ((m_E131.E131Packet.Data.FrameLayer.Options & E131_OPTIONS_MASK_STREAM_TERMINATED) != 0) {
			if (isSourceA || isSourceB) {
				SetNetworkDataLossCondition(isSourceA, isSourceB);
			}
			continue;
		}

		if (m_State.IsMergeMode) {
			if (__builtin_expect((!m_State.bDisableMergeTimeout), 1)) {
				CheckMergeTimeouts(i);
			}
		}

		if (m_E131.E131Packet.Data.FrameLayer.Priority < m_State.nPriority ){
			if (!IsPriorityTimeOut(i)) {
				continue;
			}
			m_State.nPriority = m_E131.E131Packet.Data.FrameLayer.Priority;
		} else if (m_E131.E131Packet.Data.FrameLayer.Priority > m_State.nPriority) {
			m_OutputPort[i].sourceA.ip = 0;
			m_OutputPort[i].sourceB.ip = 0;
			m_State.IsMergeMode = false;
			m_State.nPriority = m_E131.E131Packet.Data.FrameLayer.Priority;
		}

		if ((ipA == 0) && (ipB == 0)) {
			//printf("1. First package from Source\n");
			pSourceA->ip = m_E131.IPAddressFrom;
			pSourceA->sequenceNumberData = m_E131.E131Packet.Data.FrameLayer.SequenceNumber;
			memcpy(pSourceA->cid, m_E131.E131Packet.Data.RootLayer.Cid, 16);
			pSourceA->time = m_nCurrentPacketMillis;
			memcpy((void *)pSourceA->data, (const void *)p, slots);
			sendNewData = IsDmxDataChanged(i, p, slots);

		} else if (isSourceA && (ipB == 0)) {
			//printf("2. Continue package from SourceA\n");
			pSourceA->sequenceNumberData = m_E131.E131Packet.Data.FrameLayer.SequenceNumber;
			pSourceA->time = m_nCurrentPacketMillis;
			memcpy((void *)pSourceA->data, (const void *)p, slots);
			sendNewData = IsDmxDataChanged(i, p, slots);

		} else if ((ipA == 0) && isSourceB) {
			//printf("3. Continue package from SourceB\n");
			pSourceB->sequenceNumberData = m_E131.E131Packet.Data.FrameLayer.SequenceNumber;
			pSourceB->time = m_nCurrentPacketMillis;
			memcpy((void *)pSourceB->data, (const void *)p, slots);
			sendNewData = IsDmxDataChanged(i, p, slots);

		} else if (!isSourceA && (ipB == 0)) {
			//printf("4. New ip, start merging\n");
			pSourceB->ip = m_E131.IPAddressFrom;
			pSourceB->sequenceNumberData = m_E131.E131Packet.Data.FrameLayer.SequenceNumber;
			memcpy(pSourceB->cid, m_E131.E131Packet.Data.RootLayer.Cid, 16);
			pSourceB->time = m_nCurrentPacketMillis;
			memcpy((void *)pSourceB->data, (const void *)p, slots);
			sendNewData = IsMergedDmxDataChanged(i, pSourceB->data, slots);

		} else if ((ipA == 0) && !isSourceB) {
			//printf("5. New ip, start merging\n");
			pSourceA->ip = m_E131.IPAddressFrom;
			pSourceA->sequenceNumberData = m_E131.E131Packet.Data.FrameLayer.SequenceNumber;
			memcpy(pSourceA->cid, m_E131.E131Packet.Data.RootLayer.Cid, 16);
			pSourceA->time = m_nCurrentPacketMillis;
			memcpy((void *)pSourceA->data, (const void *)p, slots);
			sendNewData = IsMergedDmxDataChanged(i, pSourceA->data, slots);

		} else if (isSourceA && !isSourceB) {
			//printf("6. Continue merging\n");
			pSourceA->sequenceNumberData = m_E131.E131Packet.Data.FrameLayer.SequenceNumber;
			pSourceA->time = m_nCurrentPacketMillis;
			memcpy((void *)pSourceA->data, (const void *)p, slots);
			sendNewData = IsMergedDmxDataChanged(i, pSourceA->data, slots);

		} else if (!isSourceA && isSourceB) {
			//printf("7. Continue merging\n");
			pSourceB->sequenceNumberData = m_E131.E131Packet.Data.FrameLayer.SequenceNumber;
			pSourceB->time = m_nCurrentPacketMillis;
			memcpy((void *)pSourceB->data, (const void *)p, slots);
			sendNewData = IsMergedDmxDataChanged(i, pSourceB->data, slots);

		} else if (isSourceA && isSourceB) {
			printf("8. Source matches both buffers, this shouldn't be happening!\n");
			assert(0);
			return;

		} else if (!isSourceA && !isSourceB) {
			printf("9. More than two sources, discarding data\n");
			assert(0);
			return;

		} else {
			printf("0. No cases matched, this shouldn't happen!\n");
			assert(0);
			return;
		}

		// This bit indicates whether to lock or revert to an unsynchronized state when synchronization is lost
		// (See Section 11 on Universe Synchronization and 11.1 for discussion on synchronization states).
		// When set to 0, components that had been operating in a synchronized state shall not update with any
		// new packets until synchronization resumes. When set to 1, once synchronization has been lost,
		// components that had been operating in a synchronized state need not wait for a new
		// E1.31 Synchronization Packet in order to update to the next E1.31 Data Packet.
		if ((m_E131.E131Packet.Data.FrameLayer.Options & E131_OPTIONS_MASK_FORCE_SYNCHRONIZATION) == 0) {
			// 6.3.3.1 Synchronization Address Usage in an E1.31 Synchronization Packet
			// An E1.31 Synchronization Packet is sent to synchronize the E1.31 data on a specific universe number.
			// A Synchronization Address of 0 is thus meaningless, and shall not be transmitted.
			// Receivers shall ignore E1.31 Synchronization Packets containing a Synchronization Address of 0.
			if (m_E131.E131Packet.Data.FrameLayer.SynchronizationAddress != 0) {
				if (!m_State.IsForcedSynchronized) {
					if (!(isSourceA || isSourceB)) {
						SetSynchronizationAddress((pSourceA->ip != 0), (pSourceB->ip != 0), (uint16_t) __builtin_bswap16(m_E131.E131Packet.Data.FrameLayer.SynchronizationAddress));
					} else {
						SetSynchronizationAddress(isSourceA, isSourceB, (uint16_t) __builtin_bswap16(m_E131.E131Packet.Data.FrameLayer.SynchronizationAddress));
					}
					m_State.IsForcedSynchronized = true;
					m_State.IsSynchronized = true;
				}
			}
		} else {
			m_State.IsForcedSynchronized = false;
		}

		if (sendNewData || m_bDirectUpdate) {
			if (!m_State.IsSynchronized) {

				m_pLightSet->SetData(i, m_OutputPort[i].data, m_OutputPort[i].length);

				if (!m_OutputPort[i].IsTransmitting) {
					m_pLightSet->Start(i);
					m_State.IsChanged |= (!m_OutputPort[i].IsTransmitting);
					m_OutputPort[i].IsTransmitting = true;
				}
			} else {
				m_OutputPort[i].IsDataPending = sendNewData;
			}

		}

		m_State.bIsReceivingDmx = true;
	}
}

void E131Bridge::HandleSynchronization(void) {
	// 6.3.3.1 Synchronization Address Usage in an E1.31 Synchronization Packet
	// Receivers may ignore Synchronization Packets sent to multicast addresses
	// which do not correspond to their Synchronization Address.
	//
	// NOTE: There is no multicast addresses (To Ip) available
	// We just check if SynchronizationAddress is published by a Source

	const uint16_t nSynchronizationAddress = __builtin_bswap16(m_E131.E131Packet.Synchronization.FrameLayer.UniverseNumber);

	if ((nSynchronizationAddress != m_State.nSynchronizationAddressSourceA) && (nSynchronizationAddress != m_State.nSynchronizationAddressSourceB)) {
		DEBUG_PUTS("");
		return;
	}

	m_State.SynchronizationTime = m_nCurrentPacketMillis;

	for (uint32_t i = 0; i < E131_MAX_PORTS; i++) {
		if ((m_OutputPort[i].IsDataPending) || (m_OutputPort[i].bIsEnabled && m_bDirectUpdate)){

			m_pLightSet->SetData(i, m_OutputPort[i].data, m_OutputPort[i].length);

			if (!m_OutputPort[i].IsTransmitting) {
				m_pLightSet->Start(i);
				m_OutputPort[i].IsTransmitting = true;
			}

			m_OutputPort[i].IsDataPending = false;
		}
	}
}

void E131Bridge::SetNetworkDataLossCondition(bool bSourceA, bool bSourceB) {
	DEBUG_ENTRY
	DEBUG_PRINTF("%d %d", bSourceA, bSourceB);

	m_State.IsChanged = true;

	if (bSourceA && bSourceB) {
		m_State.IsNetworkDataLoss = true;
		m_State.IsMergeMode = false;
		m_State.IsSynchronized = false;
		m_State.IsForcedSynchronized = false;
		m_State.nPriority = E131_PRIORITY_LOWEST;

		for (uint32_t i = 0; i < E131_MAX_PORTS; i++) {
			if (m_OutputPort[i].IsTransmitting) {
				m_pLightSet->Stop(i);
				m_OutputPort[i].sourceA.ip = 0;
				memset(m_OutputPort[i].sourceA.cid, 0, E131_CID_LENGTH);
				m_OutputPort[i].sourceB.ip = 0;
				memset(m_OutputPort[i].sourceB.cid, 0, E131_CID_LENGTH);
				m_OutputPort[i].length = 0;
				m_OutputPort[i].IsDataPending = false;
				m_OutputPort[i].IsTransmitting = false;
				m_OutputPort[i].IsMerging = false;
			}
		}

	} else {
		for (uint32_t i = 0; i < E131_MAX_PORTS; i++) {
			if (m_OutputPort[i].IsTransmitting) {

				if ((bSourceA) && (m_OutputPort[i].sourceA.ip != 0)) {
					m_OutputPort[i].sourceA.ip = 0;
					memset(m_OutputPort[i].sourceA.cid, 0, E131_CID_LENGTH);
					m_OutputPort[i].IsMerging = false;
				}

				if ((bSourceB) && (m_OutputPort[i].sourceB.ip != 0)) {
					m_OutputPort[i].sourceB.ip = 0;
					memset(m_OutputPort[i].sourceB.cid, 0, E131_CID_LENGTH);
					m_OutputPort[i].IsMerging = false;
				}

				if (!m_State.IsMergeMode) {
					m_pLightSet->Stop(i);
					m_OutputPort[i].length = 0;
					m_OutputPort[i].IsDataPending = false;
					m_OutputPort[i].IsTransmitting = false;
				}
			}
		}
	}

	DEBUG_EXIT
}

bool E131Bridge::IsTransmitting(uint8_t nPortIndex) const {
	assert(nPortIndex < E131_MAX_PORTS);

	return m_OutputPort[nPortIndex].IsTransmitting;
}

bool E131Bridge::IsMerging(uint8_t nPortIndex) const {
	assert(nPortIndex < E131_MAX_PORTS);

	return m_OutputPort[nPortIndex].IsMerging;
}

bool E131Bridge::IsStatusChanged(void) {
	if (m_State.IsChanged) {
		m_State.IsChanged = false;
		return true;
	}

	return false;
}

void E131Bridge::Clear(uint8_t nPortIndex) {
	assert(nPortIndex < E131_MAX_PORTS);

	uint8_t *dst = (uint8_t *)m_OutputPort[nPortIndex].data;

	for (uint32_t i = 0; i < E131_DMX_LENGTH; i++) {
		*dst++ = 0;
	}

	m_OutputPort[nPortIndex].length = E131_DMX_LENGTH;

	m_pLightSet->SetData(nPortIndex, m_OutputPort[nPortIndex].data, m_OutputPort[nPortIndex].length);

	if (m_OutputPort[nPortIndex].bIsEnabled && !m_OutputPort[nPortIndex].IsTransmitting) {
		m_pLightSet->Start(nPortIndex);
		m_OutputPort[nPortIndex].IsTransmitting = true;
	}

	m_State.IsNetworkDataLoss = false; // Force timeout
}

bool E131Bridge::IsValidRoot(void) {
	// 5 E1.31 use of the ACN Root Layer Protocol
	// Receivers shall discard the packet if the ACN Packet Identifier is not valid.
	if (memcmp(m_E131.E131Packet.Raw.RootLayer.ACNPacketIdentifier, ACN_PACKET_IDENTIFIER, 12) != 0) {
		return false;
	}
	
	if (m_E131.E131Packet.Raw.RootLayer.Vector != __builtin_bswap32(E131_VECTOR_ROOT_DATA)
			 && (m_E131.E131Packet.Raw.RootLayer.Vector != __builtin_bswap32(E131_VECTOR_ROOT_EXTENDED)) ) {
		return false;
	}

	return true;
}

bool E131Bridge::IsValidDataPacket(void) {
	// DMP layer

	// The DMP Layer's Vector shall be set to 0x02, which indicates a DMP Set Property message by
	// transmitters. Receivers shall discard the packet if the received value is not 0x02.
	if (m_E131.E131Packet.Data.DMPLayer.Vector != (uint8_t)E131_VECTOR_DMP_SET_PROPERTY) {
		return false;
	}

	// Transmitters shall set the DMP Layer's Address Type and Data Type to 0xa1. Receivers shall discard the
	// packet if the received value is not 0xa1.
	if (m_E131.E131Packet.Data.DMPLayer.Type != (uint8_t)0xa1) {
		return false;
	}

	// Transmitters shall set the DMP Layer's First Property Address to 0x0000. Receivers shall discard the
	// packet if the received value is not 0x0000.
	if (m_E131.E131Packet.Data.DMPLayer.FirstAddressProperty != __builtin_bswap16((uint16_t)0x0000)) {
		return false;
	}

	// Transmitters shall set the DMP Layer's Address Increment to 0x0001. Receivers shall discard the packet if
	// the received value is not 0x0001.
	if (m_E131.E131Packet.Data.DMPLayer.AddressIncrement != __builtin_bswap16((uint16_t)0x0001)) {
		return false;
	}

	return true;
}

int E131Bridge::Run(void) {
	const char *packet = (char *) &(m_E131.E131Packet);
	uint16_t nForeignPort;

	const int nBytesReceived = Network::Get()->RecvFrom(m_nHandle, (uint8_t *)packet, (const uint16_t)sizeof(m_E131.E131Packet), &m_E131.IPAddressFrom, &nForeignPort) ;

	m_nCurrentPacketMillis = Hardware::Get()->Millis();

	if (nBytesReceived == 0) {
		if (m_State.nActivePorts != 0) {
			if (!m_State.bDisableNetworkDataLossTimeout && ((m_nCurrentPacketMillis - m_nPreviousPacketMillis) >= (uint32_t)(E131_NETWORK_DATA_LOSS_TIMEOUT_SECONDS * 1000))) {
				if (!m_State.IsNetworkDataLoss) {
					DEBUG_PUTS("");
					SetNetworkDataLossCondition();
				}
			}

			if (m_bEnableDataIndicator){
				if ((m_nCurrentPacketMillis - m_nPreviousPacketMillis) >= 1000) {
					LedBlink::Get()->SetMode(LEDBLINK_MODE_NORMAL);
				}
			}
		}

		return 0;
	}

	if (!IsValidRoot()) {
		return 0;
	}

	m_State.IsNetworkDataLoss = false;
	m_nPreviousPacketMillis = m_nCurrentPacketMillis;

	if (m_State.IsSynchronized && !m_State.IsForcedSynchronized) {
		if ((m_nCurrentPacketMillis - m_State.SynchronizationTime) >= (uint32_t) (E131_NETWORK_DATA_LOSS_TIMEOUT_SECONDS * 1000)) {
			m_State.IsSynchronized = false;
		}
	}

	const uint32_t nRootVector = __builtin_bswap32(m_E131.E131Packet.Raw.RootLayer.Vector);

	if (nRootVector == E131_VECTOR_ROOT_DATA) {
		if (!IsValidDataPacket()) {
			return 0;
		}
		HandleDmx();
	} else if (nRootVector == E131_VECTOR_ROOT_EXTENDED) {
		const uint32_t nFramingVector = __builtin_bswap32(m_E131.E131Packet.Raw.FrameLayer.Vector);

		if (nFramingVector == E131_VECTOR_EXTENDED_SYNCHRONIZATION) {
			HandleSynchronization();
		}

	}

	if (m_bEnableDataIndicator){
		if (m_State.bIsReceivingDmx) {
			LedBlink::Get()->SetMode(LEDBLINK_MODE_DATA);
			m_State.bIsReceivingDmx = false;
		} else {
			LedBlink::Get()->SetMode(LEDBLINK_MODE_NORMAL);
		}
	}

	return nBytesReceived;
}
