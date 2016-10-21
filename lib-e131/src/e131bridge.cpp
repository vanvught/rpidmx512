/**
 * @file e131bridge.cpp
 *
 */
/* Copyright (C) 2016 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#include <stdbool.h>
#include <assert.h>

#include "e131.h"
#include "e131bridge.h"
#include "packets.h"


#include "lightset.h"
#include "udp.h"

#include "util.h"
#include "sys_time.h"

static const uint8_t DEVICE_SOFTWARE_VERSION[] = {0x01, 0x00 };	///<
static const uint8_t ACN_PACKET_IDENTIFIER[12] = { 0x41, 0x53, 0x43, 0x2d, 0x45, 0x31, 0x2e, 0x31, 0x37, 0x00, 0x00, 0x00 }; ///< 5.3 ACN Packet Identifier

#ifndef SWAP_UINT16
#  define SWAP_UINT16(x) (((x) >> 8) | ((x) << 8))
#endif
#ifndef SWAP_UINT32
#  define SWAP_UINT32(x) (((x) >> 24) | (((x) & 0x00FF0000) >> 8) | (((x) & 0x0000FF00) << 8) | ((x) << 24))
#endif

/**
 *
 */
E131Bridge::E131Bridge(void) :
		m_pLightSet(0),
		m_nUniverse(E131_UNIVERSE_DEFAULT),
		m_nCurrentPacketMillis(0),
		m_nPreviousPacketMillis(0) {

	memset(&m_OutputPort, 0, sizeof(struct TOutputPort));
	m_OutputPort.mergeMode = E131_MERGE_HTP;

	memset(&m_State, 0, sizeof(struct TE131BridgeState));
	m_State.IsMergeMode = false;
	m_State.IsNetworkDataLoss = true;
	m_State.nPriority = E131_PRIORITY_LOWEST;
	m_State.IsTransmitting = false;

	//FillDiscoveryPacket();
}

/**
 *
 */
E131Bridge::~E131Bridge(void) {
	if (m_pLightSet != 0) {
		m_pLightSet->Stop();
		m_pLightSet = 0;
	}
}


/**
 *
 */
void E131Bridge::Start(void) {
	assert(m_pLightSet != 0);

	m_pLightSet->Start();
	m_State.IsTransmitting = true;
}

/**
 *
 */
void E131Bridge::Stop(void) {
	m_pLightSet->Stop();
	m_State.IsTransmitting = false;
}

/**
 *
 */
void E131Bridge::FillDiscoveryPacket(void) {
	memset(&m_E131DiscoveryPacket, 0, sizeof(struct TE131DiscoveryPacket));
	// Root Layer (See Section 5)
	m_E131DiscoveryPacket.RootLayer.PreAmbleSize = SWAP_UINT16(0x10);
	memcpy(m_E131DiscoveryPacket.RootLayer.ACNPacketIdentifier, ACN_PACKET_IDENTIFIER, 12);
	m_E131DiscoveryPacket.RootLayer.FlagsLength = SWAP_UINT16(0 | 0x07); //TODO
	m_E131DiscoveryPacket.RootLayer.Vector = SWAP_UINT32(E131_VECTOR_ROOT_EXTENDED);
	uint8_t *s = m_Cid;
	uint8_t *d = &m_E131DiscoveryPacket.RootLayer.Cid[E131_CID_LENGTH -1];
	for (uint8_t i = 0 ; i < E131_CID_LENGTH; i++) {
		*d = *s;
		d++;
		s++;
	}

	// E1.31 Framing Layer (See Section 6)
	m_E131DiscoveryPacket.FrameLayer.FLagsLength = SWAP_UINT16(0 | 0x07); //TODO
	m_E131DiscoveryPacket.FrameLayer.Vector = SWAP_UINT32(E131_VECTOR_EXTENDED_DISCOVERY);
	memcpy(m_E131DiscoveryPacket.FrameLayer.SourceName, m_SourceName, E131_SOURCE_NAME_LENGTH);

	// Universe Discovery Layer (See Section 8)
	m_E131DiscoveryPacket.UniverseDiscoveryLayer.FlagsLength = SWAP_UINT16(0 | 0x07); //TODO
	m_E131DiscoveryPacket.UniverseDiscoveryLayer.Vector = SWAP_UINT32(VECTOR_UNIVERSE_DISCOVERY_UNIVERSE_LIST);
	//m_E131DiscoveryPacket.UniverseDiscoveryLayer.ListOfUniverses[0] = SWAP_UINT32(m_nUniverse);
}

/**
 *
 * @return
 */
const uint8_t *E131Bridge::GetSoftwareVersion(void) {
	return DEVICE_SOFTWARE_VERSION;
}

/**
 *
 */
void E131Bridge::SetOutput(LightSet *pLightSet) {
	assert(pLightSet != 0);
	m_pLightSet = pLightSet;
}

/**
 *
 * @return
 */
const uint16_t E131Bridge::getUniverse() {
	return m_nUniverse;
}

/**
 *
 * @param nUniverse
 */
void E131Bridge::setUniverse(const uint16_t nUniverse) {
	m_nUniverse = nUniverse;
}

/**
 *
 * @return
 */
const uint8_t* E131Bridge::GetCid(void) {
	return m_Cid;
}

/**
 *
 * @param
 */
void E131Bridge::setCid(const uint8_t aCid[E131_CID_LENGTH]) {
	memcpy(m_Cid, aCid, E131_CID_LENGTH);
}

/**
 *
 * @return
 */
const char* E131Bridge::GetSourceName(void) {
	return m_SourceName;
}

/**
 *
 * @param
 */
void E131Bridge::setSourceName(const char aSourceName[E131_SOURCE_NAME_LENGTH]) {
	memcpy(m_SourceName, aSourceName, E131_SOURCE_NAME_LENGTH);
}

/**
 *
 */
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

/**
 *
 * @param pData
 * @param nLength
 * @return
 */
const bool E131Bridge::IsDmxDataChanged(const uint8_t *pData, const uint16_t nLength) {
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

/**
 *
 * @param pData
 * @param nLength
 * @return
 */
const bool E131Bridge::IsMergedDmxDataChanged(const uint8_t *pData, const uint16_t nLength) {
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

/**
 *
 * @return
 */
const bool E131Bridge::IsPriorityTimeOut(void) {
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

/**
 *
 * @param ip
 * @param cid
 * @return
 */
const bool E131Bridge::isIpCidMatch(const struct TSource *source) {
	if (source->ip != m_E131Packet.IPAddressFrom) {
		return false;
	}

	if (memcmp(source->cid, m_E131Packet.E131.RootLayer.Cid, 16) != 0) {
		return false;
	}

	return true;
}

/**
 *
 */
void E131Bridge::HandleDmx(void) {
	const uint8_t *p = &m_E131Packet.E131.DataPacket.DMPLayer.PropertyValues[1];
	const uint16_t slots = SWAP_UINT16(m_E131Packet.E131.DataPacket.DMPLayer.PropertyValueCount) - (uint16_t)1;
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
		const int8_t diff = (int8_t) (m_E131Packet.E131.DataPacket.FrameLayer.SequenceNumber - pSourceA->sequenceNumber);
		pSourceA->sequenceNumber = m_E131Packet.E131.DataPacket.FrameLayer.SequenceNumber;
		if ((diff <= (int8_t) 0) && (diff > (int8_t) -20)) {
			return;
		}
	} else if (isSourceB) {
		const int8_t diff = (int8_t) (m_E131Packet.E131.DataPacket.FrameLayer.SequenceNumber - pSourceB->sequenceNumber);
		pSourceB->sequenceNumber = m_E131Packet.E131.DataPacket.FrameLayer.SequenceNumber;
		if ((diff <= (int8_t) 0) && (diff > (int8_t) -20)) {
			return;
		}
	}

	// This bit, when set to 1, indicates that the data in this packet is intended for use in visualization or media
	// server preview applications and shall not be used to generate live output.
	if ((m_E131Packet.E131.DataPacket.FrameLayer.Options & E131_OPTIONS_MASK_PREVIEW_DATA) != 0) {
		return;
	}

	// Upon receipt of a packet containing this bit set to a value of 1, receiver shall enter network data loss condition.
	// Any property values in these packets shall be ignored.
	if ((m_E131Packet.E131.DataPacket.FrameLayer.Options & E131_OPTIONS_MASK_STREAM_TERMINATED) != 0) {
		if (isSourceA || isSourceB) {
			SetNetworkDataLossCondition();
		}
		return;
	}

	if (m_State.IsMergeMode) {
		CheckMergeTimeouts();
	}

	if (m_E131Packet.E131.DataPacket.FrameLayer.Priority < m_State.nPriority ){
		if (!IsPriorityTimeOut()) {
			return;
		}
		m_State.nPriority = m_E131Packet.E131.DataPacket.FrameLayer.Priority;
	} else if (m_E131Packet.E131.DataPacket.FrameLayer.Priority > m_State.nPriority) {
		m_OutputPort.sourceA.ip = 0;
		m_OutputPort.sourceB.ip = 0;
		m_State.IsMergeMode = false;
		m_State.nPriority = m_E131Packet.E131.DataPacket.FrameLayer.Priority;
	}

	if ((ipA == 0) && (ipB == 0)) {
		//printf("1. First package from Source\n");
		pSourceA->ip = m_E131Packet.IPAddressFrom;
		pSourceA->sequenceNumber = m_E131Packet.E131.DataPacket.FrameLayer.SequenceNumber;
		memcpy(pSourceA->cid, m_E131Packet.E131.RootLayer.Cid, 16);
		pSourceA->time = m_nCurrentPacketMillis;
		memcpy((void *)pSourceA->data, (const void *)p, slots);
		sendNewData = IsDmxDataChanged(p, slots);

	} else if (isSourceA && (ipB == 0)) {
		//printf("2. Continue package from SourceA\n");
		pSourceA->sequenceNumber = m_E131Packet.E131.DataPacket.FrameLayer.SequenceNumber;
		pSourceA->time = m_nCurrentPacketMillis;
		memcpy((void *)pSourceA->data, (const void *)p, slots);
		sendNewData = IsDmxDataChanged(p, slots);

	} else if ((ipA == 0) && isSourceB) {
		//printf("3. Continue package from SourceB\n");
		pSourceB->sequenceNumber = m_E131Packet.E131.DataPacket.FrameLayer.SequenceNumber;
		pSourceB->time = m_nCurrentPacketMillis;
		memcpy((void *)pSourceB->data, (const void *)p, slots);
		sendNewData = IsDmxDataChanged(p, slots);

	} else if (!isSourceA && (ipB == 0)) {
		//printf("4. New ip, start merging\n");
		pSourceB->ip = m_E131Packet.IPAddressFrom;
		pSourceB->sequenceNumber = m_E131Packet.E131.DataPacket.FrameLayer.SequenceNumber;
		memcpy(m_OutputPort.sourceB.cid, m_E131Packet.E131.RootLayer.Cid, 16);
		pSourceB->time = m_nCurrentPacketMillis;
		m_State.IsMergeMode = true;
		memcpy((void *)pSourceB->data, (const void *)p, slots);
		sendNewData = IsMergedDmxDataChanged(pSourceB->data, slots);

	} else if ((ipA == 0) && !isSourceB) {
		//printf("5. New ip, start merging\n");
		pSourceA->ip = m_E131Packet.IPAddressFrom;
		pSourceA->sequenceNumber = m_E131Packet.E131.DataPacket.FrameLayer.SequenceNumber;
		memcpy(m_OutputPort.sourceA.cid, m_E131Packet.E131.RootLayer.Cid, 16);
		pSourceA->time = m_nCurrentPacketMillis;
		m_State.IsMergeMode = true;
		memcpy((void *)pSourceA->data, (const void *)p, slots);
		sendNewData = IsMergedDmxDataChanged(pSourceA->data, slots);

	} else if (isSourceA && !isSourceB) {
		//printf("6. Continue merging\n");
		pSourceA->sequenceNumber = m_E131Packet.E131.DataPacket.FrameLayer.SequenceNumber;
		pSourceA->time = m_nCurrentPacketMillis;
		memcpy((void *)pSourceA->data, (const void *)p, slots);
		sendNewData = IsMergedDmxDataChanged(pSourceA->data, slots);

	} else if (!isSourceA && isSourceB) {
		//printf("7. Continue merging\n");
		pSourceB->sequenceNumber = m_E131Packet.E131.DataPacket.FrameLayer.SequenceNumber;
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
		m_pLightSet->SetData(0, m_OutputPort.data, m_OutputPort.length);

	}
}

/**
 *
 * @return
 */
const bool E131Bridge::IsValidRoot(void) {
	// 5 E1.31 use of the ACN Root Layer Protocol
	// Receivers shall discard the packet if the ACN Packet Identifier is not valid.
	if (memcmp(m_E131Packet.E131.RootLayer.ACNPacketIdentifier, ACN_PACKET_IDENTIFIER, 12) != 0) {
		return false;
	}
	
	if (m_E131Packet.E131.RootLayer.Vector != SWAP_UINT32(E131_VECTOR_ROOT_DATA)) {
			// && (m_E131Packet.E131.RootLayer.Vector != SWAP_UINT32(E131_VECTOR_ROOT_EXTENDED)) ) {
		return false;
	}

	return true;
}

/**
 *
 * @return
 */
const bool E131Bridge::IsValidDataPacket(void) {
	// Frame layer

	// 8.2 Association of Multicast Addresses and Universe
	// Note: The identity of the universe shall be determined by the universe number in the
	// packet and not assumed from the multicast address.
	if (m_E131Packet.E131.DataPacket.FrameLayer.Universe != SWAP_UINT16(m_nUniverse)) {
		return false;
	}

	// DMP layer

	// The DMP Layer's Vector shall be set to 0x02, which indicates a DMP Set Property message by
	// transmitters. Receivers shall discard the packet if the received value is not 0x02.
	if (m_E131Packet.E131.DataPacket.DMPLayer.Vector != (uint8_t)E131_VECTOR_DMP_SET_PROPERTY) {
		return false;
	}

	// Transmitters shall set the DMP Layer's Address Type and Data Type to 0xa1. Receivers shall discard the
	// packet if the received value is not 0xa1.
	if (m_E131Packet.E131.DataPacket.DMPLayer.Type != (uint8_t)0xa1) {
		return false;
	}

	// Transmitters shall set the DMP Layer's First Property Address to 0x0000. Receivers shall discard the
	// packet if the received value is not 0x0000.
	if (m_E131Packet.E131.DataPacket.DMPLayer.FirstAddressProperty != SWAP_UINT16((uint16_t)0x0000)) {
		return false;
	}

	// Transmitters shall set the DMP Layer's Address Increment to 0x0001. Receivers shall discard the packet if
	// the received value is not 0x0001.
	if (m_E131Packet.E131.DataPacket.DMPLayer.AddressIncrement != SWAP_UINT16((uint16_t)0x0001)) {
		return false;
	}

	return true;
}

/**
 *
 */
int E131Bridge::HandlePacket(void) {
	const char *packet = (char *) &(m_E131Packet.E131);
	uint16_t nForeignPort;
	uint32_t IPAddressFrom;

	const int nBytesReceived = udp_recvfrom((const uint8_t *)packet, (const uint16_t)sizeof(m_E131Packet.E131), &IPAddressFrom, &nForeignPort) ;

	m_nCurrentPacketMillis = millis();

	if ((m_nCurrentPacketMillis - m_nPreviousPacketMillis) > 2500) {
		SetNetworkDataLossCondition();
	} else {
		if (m_State.IsNetworkDataLoss) {
			Start();
		}
		m_State.IsNetworkDataLoss = false;
	}

	if (nBytesReceived == 0) {
		return 0;
	}

	if (!IsValidRoot()) {
		return 0;
	}

	m_nPreviousPacketMillis = m_nCurrentPacketMillis;

	//const uint32_t nVector = SWAP_UINT32(m_E131Packet.E131.RootLayer.Vector);

	//if (nVector == E131_VECTOR_ROOT_DATA) {
	if (!IsValidDataPacket()) {
		return 0;
	}
	HandleDmx();
	//} else if (nVector == E131_VECTOR_ROOT_EXTENDED) {
	//
	//}

	return nBytesReceived;
}

/**
 *
 */
void E131Bridge::SetNetworkDataLossCondition(void) {
	Stop();
	m_State.IsNetworkDataLoss = true;
	m_State.nPriority = E131_PRIORITY_LOWEST;
	m_State.IsMergeMode = false;
	m_OutputPort.sourceA.ip = (uint32_t) 0;
	m_OutputPort.sourceB.ip = (uint32_t) 0;
}

/**
 *
 * @return
 */
const TMerge E131Bridge::getMergeMode(void) {
	return m_OutputPort.mergeMode;
}

/**
 *
 * @param mergeMode
 */
void E131Bridge::setMergeMode(TMerge mergeMode) {
	m_OutputPort.mergeMode = mergeMode;
}
