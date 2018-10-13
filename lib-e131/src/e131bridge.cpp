/**
 * @file e131bridge.cpp
 *
 */
/* Copyright (C) 2016-2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#include <netinet/in.h>
#ifndef NDEBUG
 #include <stdio.h>
#endif
#include <assert.h>

#if defined(BARE_METAL)
 #include "util.h"
#else
 #include <string.h>
 #include <sys/socket.h>
 #include <netinet/in.h>
 #include <arpa/inet.h>
#endif

#ifndef MAX
 #define MAX(a,b)	(((a) > (b)) ? (a) : (b))
 #define MIN(a,b)	(((a) < (b)) ? (a) : (b))
#endif

#include "e131.h"
#include "e131packets.h"
#include "e131bridge.h"

#include "lightset.h"

#include "hardware.h"
#include "network.h"

static const uint8_t DEVICE_SOFTWARE_VERSION[] = { 1, 5 };
static const uint8_t ACN_PACKET_IDENTIFIER[E131_PACKET_IDENTIFIER_LENGTH] = { 0x41, 0x53, 0x43, 0x2d, 0x45, 0x31, 0x2e, 0x31, 0x37, 0x00, 0x00, 0x00 }; ///< 5.3 ACN Packet Identifier

#define DEFAULT_SOURCE_NAME_SUFFIX  " sACN E1.31 www.raspberrypi-dmx.org"

E131Bridge::E131Bridge(void) :
	m_pLightSet(0),
	m_nUniverse(E131_UNIVERSE_DEFAULT),
	m_nMulticastIp(0),
	m_nCurrentPacketMillis(0),
	m_nPreviousPacketMillis(0)
{
	assert(Hardware::Get() != 0);
	assert(Network::Get() != 0);

	memset(&m_OutputPort, 0, sizeof(struct TOutputPort));
	m_OutputPort.mergeMode = E131_MERGE_HTP;
	m_OutputPort.IsDataPending = false;

	memset(&m_State, 0, sizeof(struct TE131BridgeState));
	m_State.IsNetworkDataLoss = true;
	m_State.IsMergeMode = false;
	m_State.IsTransmitting = false;
	m_State.IsSynchronized = false;
	m_State.IsForcedSynchronized = false;
	m_State.nPriority = E131_PRIORITY_LOWEST;
	m_State.DiscoveryTime = 0;

	m_DiscoveryIpAddress = 0;

	struct in_addr addr;
	(void)inet_aton("239.255.0.0", &addr);
	m_DiscoveryIpAddress = addr.s_addr | ((uint32_t)(((uint32_t)E131_UNIVERSE_DISCOVERY & (uint32_t)0xFF) << 24)) | ((uint32_t)(((uint32_t)E131_UNIVERSE_DISCOVERY & (uint32_t)0xFF00) << 8));

	char aDefaultSourceName[E131_SOURCE_NAME_LENGTH];
	uint8_t nBoardNameLength;
	const char *pBoardName = Hardware::Get()->GetBoardName(nBoardNameLength);
	strncpy(aDefaultSourceName, pBoardName, E131_SOURCE_NAME_LENGTH);
	strncpy(aDefaultSourceName + nBoardNameLength, DEFAULT_SOURCE_NAME_SUFFIX, E131_SOURCE_NAME_LENGTH - nBoardNameLength);
	SetSourceName(aDefaultSourceName);

	SetUniverse(E131_UNIVERSE_DEFAULT);
}

E131Bridge::~E131Bridge(void) {
	Stop();
}

void E131Bridge::Start(void) {
	assert(m_pLightSet != 0);

	FillDiscoveryPacket();

	Network::Get()->Begin(E131_DEFAULT_PORT);
	Network::Get()->JoinGroup(m_nMulticastIp);
}

void E131Bridge::Stop(void) {
	if (m_pLightSet != 0) {
		m_pLightSet->Stop(0);
	}
	//
	m_State.IsNetworkDataLoss = true;
	m_State.IsTransmitting = false;
	//
	m_OutputPort.length = 0;
	m_OutputPort.IsDataPending = false;
}

const uint8_t *E131Bridge::GetSoftwareVersion(void) {
	return DEVICE_SOFTWARE_VERSION;
}

void E131Bridge::SetOutput(LightSet *pLightSet) {
	assert(pLightSet != 0);

	m_pLightSet = pLightSet;
}

uint16_t E131Bridge::GetUniverse() const{
	return m_nUniverse;
}

void E131Bridge::SetUniverse(const uint16_t nUniverse) {
	assert((nUniverse >= E131_UNIVERSE_DEFAULT) && (nUniverse <= E131_UNIVERSE_MAX));

	struct in_addr group_ip;
	(void) inet_aton("239.255.0.0", &group_ip);

	m_nMulticastIp = group_ip.s_addr
			| ((uint32_t) (((uint32_t) nUniverse & (uint32_t) 0xFF) << 24))
			| ((uint32_t) (((uint32_t) nUniverse & (uint32_t) 0xFF00) << 8));

	m_nUniverse = nUniverse;
}

const uint8_t* E131Bridge::GetCid(void) {
	return m_Cid;
}

void E131Bridge::SetCid(const uint8_t aCid[E131_CID_LENGTH]) {
	assert(aCid != 0);

	memcpy(m_Cid, aCid, E131_CID_LENGTH);
	memcpy(m_E131DiscoveryPacket.RootLayer.Cid, aCid, E131_CID_LENGTH);
}

const char* E131Bridge::GetSourceName(void) {
	return m_SourceName;
}

void E131Bridge::SetSourceName(const char *aSourceName) {
	memset(m_SourceName, 0, E131_SOURCE_NAME_LENGTH);
	strncpy(m_SourceName, aSourceName, E131_SOURCE_NAME_LENGTH);

	memset((char *)m_E131DiscoveryPacket.FrameLayer.SourceName, 0, E131_SOURCE_NAME_LENGTH);
	strncpy((char *)m_E131DiscoveryPacket.FrameLayer.SourceName, aSourceName, E131_SOURCE_NAME_LENGTH);
}

TMerge E131Bridge::GetMergeMode(void) const {
	return m_OutputPort.mergeMode;
}

void E131Bridge::SetMergeMode(TMerge mergeMode) {
	m_OutputPort.mergeMode = mergeMode;
}

void E131Bridge::FillDiscoveryPacket(void) {
	uint16_t root_layer_length = sizeof(struct TRootLayer);
	uint16_t framing_layer_size = sizeof(struct TDiscoveryFrameLayer);
	uint16_t discovery_layer_size = sizeof(struct TUniverseDiscoveryLayer) - (511 * 2);

	m_State.DiscoveryPacketLength = root_layer_length + framing_layer_size + discovery_layer_size;

	memset(&m_E131DiscoveryPacket, 0, sizeof(struct TE131DiscoveryPacket));

	// Root Layer (See Section 5)
	m_E131DiscoveryPacket.RootLayer.PreAmbleSize = __builtin_bswap16(0x10);
	memcpy(m_E131DiscoveryPacket.RootLayer.ACNPacketIdentifier, ACN_PACKET_IDENTIFIER, E131_PACKET_IDENTIFIER_LENGTH);
	m_E131DiscoveryPacket.RootLayer.FlagsLength = __builtin_bswap16((0x07 << 12) | (m_State.DiscoveryPacketLength));
	m_E131DiscoveryPacket.RootLayer.Vector = __builtin_bswap32(E131_VECTOR_ROOT_EXTENDED);
	memcpy(m_E131DiscoveryPacket.RootLayer.Cid, m_Cid, E131_CID_LENGTH);


	// E1.31 Framing Layer (See Section 6)
	m_E131DiscoveryPacket.FrameLayer.FLagsLength = __builtin_bswap16((0x07 << 12) | (framing_layer_size + discovery_layer_size) );
	m_E131DiscoveryPacket.FrameLayer.Vector = __builtin_bswap32(E131_VECTOR_EXTENDED_DISCOVERY);
	memcpy(m_E131DiscoveryPacket.FrameLayer.SourceName, m_SourceName, E131_SOURCE_NAME_LENGTH);

	// Universe Discovery Layer (See Section 8)
	m_E131DiscoveryPacket.UniverseDiscoveryLayer.FlagsLength = __builtin_bswap16((0x07 << 12) | discovery_layer_size);
	m_E131DiscoveryPacket.UniverseDiscoveryLayer.Vector = __builtin_bswap32(VECTOR_UNIVERSE_DISCOVERY_UNIVERSE_LIST);
	m_E131DiscoveryPacket.UniverseDiscoveryLayer.ListOfUniverses[0] = __builtin_bswap16(m_nUniverse);
}

bool E131Bridge::IsDmxDataChanged(const uint8_t *pData, uint16_t nLength) {
	bool isChanged = false;

	uint8_t *src = (uint8_t *)pData;
	uint8_t *dst = (uint8_t *)m_OutputPort.data;

	if (nLength != m_OutputPort.length) {
		m_OutputPort.length = nLength;
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

bool E131Bridge::IsMergedDmxDataChanged(const uint8_t *pData, uint16_t nLength) {
	bool isChanged = false;

	if (m_OutputPort.mergeMode == E131_MERGE_HTP) {

		if (nLength != m_OutputPort.length) {
			m_OutputPort.length = nLength;
			for (unsigned i = 0; i < nLength; i++) {
				uint8_t data = MAX(m_OutputPort.sourceA.data[i], m_OutputPort.sourceB.data[i]);
				m_OutputPort.data[i] = data;
			}
			return true;
		}

		for (unsigned i = 0; i < nLength; i++) {
			uint8_t data = MAX(m_OutputPort.sourceA.data[i], m_OutputPort.sourceB.data[i]);
			if (data != m_OutputPort.data[i]) {
				m_OutputPort.data[i] = data;
				isChanged = true;
			}
		}

		return isChanged;
	} else {
		return IsDmxDataChanged(pData, nLength);
	}
}

void E131Bridge::CheckMergeTimeouts(void) {
	const uint32_t timeOutA = m_nCurrentPacketMillis - m_OutputPort.sourceA.time;
	const uint32_t timeOutB = m_nCurrentPacketMillis - m_OutputPort.sourceB.time;

	if (timeOutA > (uint32_t)(E131_MERGE_TIMEOUT_SECONDS * 1000)) {
		m_OutputPort.sourceA.ip = 0;
		m_State.IsMergeMode = false;
	}

	if (timeOutB > (uint32_t)(E131_MERGE_TIMEOUT_SECONDS * 1000)) {
		m_OutputPort.sourceB.ip = 0;
		m_State.IsMergeMode = false;
	}
}

bool E131Bridge::IsPriorityTimeOut(void) {
	const uint32_t timeOutA = m_nCurrentPacketMillis - m_OutputPort.sourceA.time;
	const uint32_t timeOutB = m_nCurrentPacketMillis - m_OutputPort.sourceB.time;

	if ( (m_OutputPort.sourceA.ip != 0) && (m_OutputPort.sourceB.ip != 0) ) {
		if ( (timeOutA < (uint32_t)(E131_PRIORITY_TIMEOUT_SECONDS * 1000)) || (timeOutB < (uint32_t)(E131_PRIORITY_TIMEOUT_SECONDS * 1000)) ) {
			return false;
		} else {
			return true;
		}
	} else if ( (m_OutputPort.sourceA.ip != 0) && (m_OutputPort.sourceB.ip == 0) ) {
		if (timeOutA > (uint32_t)(E131_PRIORITY_TIMEOUT_SECONDS * 1000)) {
			return true;
		}
	} else if ( (m_OutputPort.sourceA.ip == 0) && (m_OutputPort.sourceB.ip != 0) ) {
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
	const uint16_t slots = __builtin_bswap16(m_E131.E131Packet.Data.DMPLayer.PropertyValueCount) - (uint16_t)1;
	const uint32_t ipA = m_OutputPort.sourceA.ip;
	const uint32_t ipB = m_OutputPort.sourceB.ip;
	struct TSource *pSourceA = &m_OutputPort.sourceA;
	struct TSource *pSourceB = &m_OutputPort.sourceB;
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
			return;
		}
	} else if (isSourceB) {
		const int8_t diff = (int8_t) (m_E131.E131Packet.Data.FrameLayer.SequenceNumber - pSourceB->sequenceNumberData);
		pSourceB->sequenceNumberData = m_E131.E131Packet.Data.FrameLayer.SequenceNumber;
		if ((diff <= (int8_t) 0) && (diff > (int8_t) -20)) {
			return;
		}
	}

	// This bit, when set to 1, indicates that the data in this packet is intended for use in visualization or media
	// server preview applications and shall not be used to generate live output.
	if ((m_E131.E131Packet.Data.FrameLayer.Options & E131_OPTIONS_MASK_PREVIEW_DATA) != 0) {
		return;
	}

	// Upon receipt of a packet containing this bit set to a value of 1, receiver shall enter network data loss condition.
	// Any property values in these packets shall be ignored.
	if ((m_E131.E131Packet.Data.FrameLayer.Options & E131_OPTIONS_MASK_STREAM_TERMINATED) != 0) {
		if (isSourceA || isSourceB) {
			if (!m_State.IsMergeMode) {
				SetNetworkDataLossCondition();
			}
		}
		return;
	}

	// This bit indicates whether to lock or revert to an unsynchronized state when synchronization is lost
	// (See Section 11 on Universe Synchronization and 11.1 for discussion on synchronization states).
	// When set to 0, components that had been operating in a synchronized state shall not update with any new packets
	// until synchronization resumes.
	// When set to 1, once synchronization has been lost, components that had been operating in a synchronized state
	// need not wait for a new E1.31 Synchronization Packet in order to update to the next E1.31 Data Packet.
	if ((m_E131.E131Packet.Data.FrameLayer.Options & E131_OPTIONS_MASK_FORCE_SYNCHRONIZATION) == 0) {
		m_State.IsForcedSynchronized = true;
		if (m_State.IsSynchronized) {
			return;
		}
	} else {
		m_State.IsForcedSynchronized = false;
	}

	if (m_State.IsMergeMode) {
		CheckMergeTimeouts();
	}

	if (m_E131.E131Packet.Data.FrameLayer.Priority < m_State.nPriority ){
		if (!IsPriorityTimeOut()) {
			return;
		}
		m_State.nPriority = m_E131.E131Packet.Data.FrameLayer.Priority;
	} else if (m_E131.E131Packet.Data.FrameLayer.Priority > m_State.nPriority) {
		m_OutputPort.sourceA.ip = 0;
		m_OutputPort.sourceB.ip = 0;
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
		sendNewData = IsDmxDataChanged(p, slots);

	} else if (isSourceA && (ipB == 0)) {
		//printf("2. Continue package from SourceA\n");
		pSourceA->sequenceNumberData = m_E131.E131Packet.Data.FrameLayer.SequenceNumber;
		pSourceA->time = m_nCurrentPacketMillis;
		memcpy((void *)pSourceA->data, (const void *)p, slots);
		sendNewData = IsDmxDataChanged(p, slots);

	} else if ((ipA == 0) && isSourceB) {
		//printf("3. Continue package from SourceB\n");
		pSourceB->sequenceNumberData = m_E131.E131Packet.Data.FrameLayer.SequenceNumber;
		pSourceB->time = m_nCurrentPacketMillis;
		memcpy((void *)pSourceB->data, (const void *)p, slots);
		sendNewData = IsDmxDataChanged(p, slots);

	} else if (!isSourceA && (ipB == 0)) {
		//printf("4. New ip, start merging\n");
		pSourceB->ip = m_E131.IPAddressFrom;
		pSourceB->sequenceNumberData = m_E131.E131Packet.Data.FrameLayer.SequenceNumber;
		memcpy(m_OutputPort.sourceB.cid, m_E131.E131Packet.Data.RootLayer.Cid, 16);
		pSourceB->time = m_nCurrentPacketMillis;
		m_State.IsMergeMode = true;
		memcpy((void *)pSourceB->data, (const void *)p, slots);
		sendNewData = IsMergedDmxDataChanged(pSourceB->data, slots);

	} else if ((ipA == 0) && !isSourceB) {
		//printf("5. New ip, start merging\n");
		pSourceA->ip = m_E131.IPAddressFrom;
		pSourceA->sequenceNumberData = m_E131.E131Packet.Data.FrameLayer.SequenceNumber;
		memcpy(m_OutputPort.sourceA.cid, m_E131.E131Packet.Data.RootLayer.Cid, 16);
		pSourceA->time = m_nCurrentPacketMillis;
		m_State.IsMergeMode = true;
		memcpy((void *)pSourceA->data, (const void *)p, slots);
		sendNewData = IsMergedDmxDataChanged(pSourceA->data, slots);

	} else if (isSourceA && !isSourceB) {
		//printf("6. Continue merging\n");
		pSourceA->sequenceNumberData = m_E131.E131Packet.Data.FrameLayer.SequenceNumber;
		pSourceA->time = m_nCurrentPacketMillis;
		memcpy((void *)pSourceA->data, (const void *)p, slots);
		sendNewData = IsMergedDmxDataChanged(pSourceA->data, slots);

	} else if (!isSourceA && isSourceB) {
		//printf("7. Continue merging\n");
		pSourceB->sequenceNumberData = m_E131.E131Packet.Data.FrameLayer.SequenceNumber;
		pSourceB->time = m_nCurrentPacketMillis;
		memcpy((void *)pSourceB->data, (const void *)p, slots);
		sendNewData = IsMergedDmxDataChanged(pSourceB->data, slots);

	} else if (isSourceA && isSourceB) {
		//printf("8. Source matches both buffers, this shouldn't be happening!\n");
		return;

	} else if (!isSourceA && !isSourceB) {
		//printf("9. More than two sources, discarding data\n");
		return;

	} else {
		//printf("0. No cases matched, this shouldn't happen!\n");
		return;
	}

	if (sendNewData) {
		if (!m_State.IsSynchronized) {
			m_pLightSet->SetData(0, m_OutputPort.data, m_OutputPort.length);
			if (!m_State.IsTransmitting) {
				m_pLightSet->Start(0);
				m_State.IsTransmitting = true;
			}
		} else {
			m_OutputPort.IsDataPending = true;
		}


	}
}

void E131Bridge::HandleSynchronization(void) {
	if (m_E131.E131Packet.Synchronization.FrameLayer.UniverseNumber != __builtin_bswap16(m_nUniverse)) {
		return;
	}

	m_State.IsSynchronized = true;
	m_State.SynchronizationTime = m_nCurrentPacketMillis;

	if (m_OutputPort.IsDataPending) {
		m_pLightSet->SetData(0, m_OutputPort.data, m_OutputPort.length);
		if (m_State.IsTransmitting) {
			m_pLightSet->Start(0);
			m_State.IsTransmitting = true;
		}
		m_OutputPort.IsDataPending = false;
	}
}

void E131Bridge::SetNetworkDataLossCondition(void) {
	m_pLightSet->Stop(0);
	//
	m_State.IsNetworkDataLoss = true;
	m_State.IsMergeMode = false;
	m_State.IsTransmitting = false;
	m_State.IsSynchronized = false;
	m_State.IsForcedSynchronized = false;
	m_State.nPriority = E131_PRIORITY_LOWEST;
	//
	m_OutputPort.length = 0;
	m_OutputPort.IsDataPending = false;
	m_OutputPort.sourceA.ip = (uint32_t) 0;
	m_OutputPort.sourceB.ip = (uint32_t) 0;
}

void E131Bridge::SendDiscoveryPacket(void) {
	Network::Get()->SendTo((const uint8_t *)&(m_E131DiscoveryPacket), m_State.DiscoveryPacketLength, m_DiscoveryIpAddress, (uint16_t)E131_DEFAULT_PORT);
	m_State.DiscoveryTime = m_nCurrentPacketMillis;
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
	// Frame layer

	// 8.2 Association of Multicast Addresses and Universe
	// Note: The identity of the universe shall be determined by the universe number in the
	// packet and not assumed from the multicast address.
	if (m_E131.E131Packet.Data.FrameLayer.Universe != __builtin_bswap16(m_nUniverse)) {
		return false;
	}

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
	uint32_t IPAddressFrom;

	const int nBytesReceived = Network::Get()->RecvFrom((uint8_t *)packet, (const uint16_t)sizeof(m_E131.E131Packet), &IPAddressFrom, &nForeignPort) ;

	m_nCurrentPacketMillis = Hardware::Get()->Millis();

	if (m_nCurrentPacketMillis - m_State.DiscoveryTime >= (E131_UNIVERSE_DISCOVERY_INTERVAL_SECONDS * 1000)) {
		SendDiscoveryPacket();
	}

	if (nBytesReceived == 0) {
		if ((m_nCurrentPacketMillis - m_nPreviousPacketMillis) >= (uint32_t)(E131_NETWORK_DATA_LOSS_TIMEOUT_SECONDS * 1000)) {
			SetNetworkDataLossCondition();
		}
		return 0;
	}

	if (!IsValidRoot()) {
		return 0;
	}

	m_nPreviousPacketMillis = m_nCurrentPacketMillis;

	if (m_State.IsSynchronized && !m_State.IsForcedSynchronized) {
		if ((m_nCurrentPacketMillis - m_State.SynchronizationTime) >= (E131_NETWORK_DATA_LOSS_TIMEOUT_SECONDS * 1000)) {
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

	return nBytesReceived;
}
