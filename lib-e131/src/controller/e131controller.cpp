/**
 * @file e131controller.cpp
 *
 */
/* Copyright (C) 2020-2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <cstdio>
#include <cstring>
#include <cassert>

#include "e131controller.h"

#include "e131.h"
#include "e131packets.h"

#include "e117const.h"

#include "hardware.h"
#include "network.h"

#include "debug.h"

using namespace e131;


static const uint8_t DEVICE_SOFTWARE_VERSION[] = { 1, 0 };

struct TSequenceNumbers {
	uint16_t nUniverse;
	uint8_t nSequenceNumber;
	uint32_t nIpAddress;
};

static struct TSequenceNumbers s_SequenceNumbers[512] __attribute__ ((aligned (8)));

E131Controller *E131Controller::s_pThis = nullptr;

E131Controller::E131Controller() {
	DEBUG_ENTRY

	assert(s_pThis == nullptr);
	s_pThis = this;

	memset(&m_State, 0, sizeof(struct TE131ControllerState));
	m_State.nPriority = 100;

	char aSourceName[e131::SOURCE_NAME_LENGTH];
	uint8_t nLength;
	snprintf(aSourceName, e131::SOURCE_NAME_LENGTH, "%.48s %s", Network::Get()->GetHostName(), Hardware::Get()->GetBoardName(nLength));
	SetSourceName(aSourceName);

	Hardware::Get()->GetUuid(m_Cid);

	for (uint32_t nIndex = 0; nIndex < sizeof(s_SequenceNumbers) / sizeof(s_SequenceNumbers[0]); nIndex++) {
		memset(&s_SequenceNumbers[nIndex], 0, sizeof(s_SequenceNumbers[0]));
	}

	SetSynchronizationAddress();

	const auto nIpMulticast = network::convert_to_uint(239, 255, 0, 0);
	m_DiscoveryIpAddress = nIpMulticast | ((universe::DISCOVERY & static_cast<uint32_t>(0xFF)) << 24) | ((universe::DISCOVERY & 0xFF00) << 8);

	// TE131DataPacket
	m_pE131DataPacket = new struct TE131DataPacket;
	assert(m_pE131DataPacket != nullptr);

	// TE131DiscoveryPacket
	m_pE131DiscoveryPacket = new struct TE131DiscoveryPacket;
	assert(m_pE131DiscoveryPacket != nullptr);

	// TE131SynchronizationPacket
	m_pE131SynchronizationPacket = new struct TE131SynchronizationPacket;
	assert(m_pE131SynchronizationPacket != nullptr);

	m_nHandle = Network::Get()->Begin(e131::UDP_PORT);
	assert(m_nHandle != -1);

	DEBUG_EXIT
}

E131Controller::~E131Controller() {
	DEBUG_ENTRY

	Network::Get()->End(e131::UDP_PORT);

	if (m_pE131SynchronizationPacket != nullptr) {
		delete m_pE131SynchronizationPacket;
		m_pE131SynchronizationPacket = nullptr;
	}

	if (m_pE131DiscoveryPacket != nullptr) {
		delete m_pE131DiscoveryPacket;
		m_pE131DiscoveryPacket = nullptr;
	}

	if (m_pE131DataPacket != nullptr) {
		delete m_pE131DataPacket;
		m_pE131DataPacket = nullptr;
	}

	DEBUG_EXIT
}

void E131Controller::Start() {
	DEBUG_ENTRY

	FillDataPacket();
	FillDiscoveryPacket();
	FillSynchronizationPacket();

	m_State.bIsRunning = true;

	DEBUG_EXIT
}

void E131Controller::Stop() {
	m_State.bIsRunning = false;
}

void E131Controller::Run() {
	if (__builtin_expect((m_State.bIsRunning), 1)) {
		m_nCurrentPacketMillis = Hardware::Get()->Millis();
		SendDiscoveryPacket();
	}
}

void E131Controller::FillDataPacket() {
	// Root Layer (See Section 5)
	m_pE131DataPacket->RootLayer.PreAmbleSize = __builtin_bswap16(0x0010);
	m_pE131DataPacket->RootLayer.PostAmbleSize = __builtin_bswap16(0x0000);
	memcpy(m_pE131DataPacket->RootLayer.ACNPacketIdentifier, E117Const::ACN_PACKET_IDENTIFIER, e117::PACKET_IDENTIFIER_LENGTH);
	m_pE131DataPacket->RootLayer.Vector = __builtin_bswap32(vector::root::DATA);
	memcpy(m_pE131DataPacket->RootLayer.Cid, m_Cid, e131::CID_LENGTH);

	// E1.31 Framing Layer (See Section 6)
	m_pE131DataPacket->FrameLayer.Vector = __builtin_bswap32(vector::data::PACKET);
	memcpy(m_pE131DataPacket->FrameLayer.SourceName, m_SourceName, e131::SOURCE_NAME_LENGTH);
	m_pE131DataPacket->FrameLayer.Priority = m_State.nPriority;
	m_pE131DataPacket->FrameLayer.SynchronizationAddress = __builtin_bswap16(m_State.SynchronizationPacket.nUniverseNumber);
	m_pE131DataPacket->FrameLayer.Options = 0;

	// Data Layer
	m_pE131DataPacket->DMPLayer.Vector = e131::vector::dmp::SET_PROPERTY;
	m_pE131DataPacket->DMPLayer.Type = 0xa1;
	m_pE131DataPacket->DMPLayer.FirstAddressProperty = __builtin_bswap16(0x0000);
	m_pE131DataPacket->DMPLayer.AddressIncrement = __builtin_bswap16(0x0001);
	m_pE131DataPacket->DMPLayer.PropertyValues[0] = 0;
}

void E131Controller::FillDiscoveryPacket() {
	memset(m_pE131DiscoveryPacket, 0, sizeof(struct TE131DiscoveryPacket));

	// Root Layer (See Section 5)
	m_pE131DiscoveryPacket->RootLayer.PreAmbleSize = __builtin_bswap16(0x10);
	memcpy(m_pE131DiscoveryPacket->RootLayer.ACNPacketIdentifier, E117Const::ACN_PACKET_IDENTIFIER, e117::PACKET_IDENTIFIER_LENGTH);
	m_pE131DiscoveryPacket->RootLayer.Vector = __builtin_bswap32(vector::root::EXTENDED);
	memcpy(m_pE131DiscoveryPacket->RootLayer.Cid, m_Cid, e131::CID_LENGTH);

	// E1.31 Framing Layer (See Section 6)
	m_pE131DiscoveryPacket->FrameLayer.Vector = __builtin_bswap32(vector::extended::DISCOVERY);
	memcpy(m_pE131DiscoveryPacket->FrameLayer.SourceName, m_SourceName, e131::SOURCE_NAME_LENGTH);

	// Universe Discovery Layer (See Section 8)
	m_pE131DiscoveryPacket->UniverseDiscoveryLayer.Vector = __builtin_bswap32(vector::universe::DISCOVERY_UNIVERSE_LIST);
}

void E131Controller::FillSynchronizationPacket() {
	memset(m_pE131SynchronizationPacket, 0, sizeof(struct TE131SynchronizationPacket));

	// Root Layer (See Section 4.2)
	m_pE131SynchronizationPacket->RootLayer.PreAmbleSize = __builtin_bswap16(0x10);
	memcpy(m_pE131SynchronizationPacket->RootLayer.ACNPacketIdentifier, E117Const::ACN_PACKET_IDENTIFIER, e117::PACKET_IDENTIFIER_LENGTH);
	m_pE131SynchronizationPacket->RootLayer.FlagsLength = __builtin_bswap16((0x07 << 12) | (SYNCHRONIZATION_ROOT_LAYER_LENGTH));
	m_pE131SynchronizationPacket->RootLayer.Vector = __builtin_bswap32(vector::root::EXTENDED);
	memcpy(m_pE131SynchronizationPacket->RootLayer.Cid, m_Cid, e131::CID_LENGTH);

	// E1.31 Framing Layer (See Section 6)
	m_pE131SynchronizationPacket->FrameLayer.FLagsLength = __builtin_bswap16((0x07 << 12) | (SYNCHRONIZATION_LAYER_LENGTH) );
	m_pE131SynchronizationPacket->FrameLayer.Vector = __builtin_bswap32(vector::extended::SYNCHRONIZATION);
	m_pE131SynchronizationPacket->FrameLayer.UniverseNumber = __builtin_bswap16(m_State.SynchronizationPacket.nUniverseNumber);
}

void E131Controller::HandleDmxOut(uint16_t nUniverse, const uint8_t *pDmxData, uint32_t nLength) {
	uint32_t nIp;

	// Root Layer (See Section 5)
	m_pE131DataPacket->RootLayer.FlagsLength = __builtin_bswap16(static_cast<uint16_t>((0x07 << 12) | (DATA_ROOT_LAYER_LENGTH(1U + nLength))));

	// E1.31 Framing Layer (See Section 6)
	m_pE131DataPacket->FrameLayer.FLagsLength = __builtin_bswap16(static_cast<uint16_t>((0x07 << 12) | (DATA_FRAME_LAYER_LENGTH(1U + nLength))));
	m_pE131DataPacket->FrameLayer.SequenceNumber = GetSequenceNumber(nUniverse, nIp);
	m_pE131DataPacket->FrameLayer.Universe = __builtin_bswap16(nUniverse);

	// Data Layer
	m_pE131DataPacket->DMPLayer.FlagsLength = __builtin_bswap16(static_cast<uint16_t>((0x07 << 12) | (DATA_LAYER_LENGTH(1U + nLength))));

	if (__builtin_expect((m_nMaster == DMX_MAX_VALUE), 1)) {
		memcpy(&m_pE131DataPacket->DMPLayer.PropertyValues[1], pDmxData, nLength);
	} else if (m_nMaster == 0) {
		memset(&m_pE131DataPacket->DMPLayer.PropertyValues[1], 0, nLength);
	} else {
		for (uint32_t i = 0; i < nLength; i++) {
			m_pE131DataPacket->DMPLayer.PropertyValues[1 + i] = static_cast<uint8_t>((m_nMaster * pDmxData[i]) / DMX_MAX_VALUE);
		}
	}

	m_pE131DataPacket->DMPLayer.PropertyValueCount = __builtin_bswap16(static_cast<uint16_t>(1 + nLength));

	Network::Get()->SendTo(m_nHandle, m_pE131DataPacket, static_cast<uint16_t>(DATA_PACKET_SIZE(1U + nLength)), nIp, e131::UDP_PORT);
}

void E131Controller::HandleSync() {
	if (m_State.SynchronizationPacket.nUniverseNumber != 0) {
		m_pE131SynchronizationPacket->FrameLayer.SequenceNumber = m_State.SynchronizationPacket.nSequenceNumber++;
		Network::Get()->SendTo(m_nHandle, m_pE131SynchronizationPacket, SYNCHRONIZATION_PACKET_SIZE, m_State.SynchronizationPacket.nIpAddress, e131::UDP_PORT);
	}
}

void E131Controller::HandleBlackout() {
	// Root Layer (See Section 5)
	m_pE131DataPacket->RootLayer.FlagsLength = __builtin_bswap16((0x07 << 12) | (DATA_ROOT_LAYER_LENGTH(513)));

	// E1.31 Framing Layer (See Section 6)
	m_pE131DataPacket->FrameLayer.FLagsLength = __builtin_bswap16((0x07 << 12) | (DATA_FRAME_LAYER_LENGTH(513)));

	// Data Layer
	m_pE131DataPacket->DMPLayer.FlagsLength = __builtin_bswap16((0x07 << 12) | (DATA_LAYER_LENGTH(513)));
	m_pE131DataPacket->DMPLayer.PropertyValueCount = __builtin_bswap16(513);
	memset(&m_pE131DataPacket->DMPLayer.PropertyValues[1], 0, 512);

	for (uint32_t nIndex = 0; nIndex < m_State.nActiveUniverses; nIndex++) {
		uint32_t nIp;
		uint16_t nUniverse = s_SequenceNumbers[nIndex].nUniverse;

		m_pE131DataPacket->FrameLayer.SequenceNumber = GetSequenceNumber(nUniverse, nIp);
		m_pE131DataPacket->FrameLayer.Universe = __builtin_bswap16(nUniverse);

		Network::Get()->SendTo(m_nHandle, m_pE131DataPacket, DATA_PACKET_SIZE(513), nIp, e131::UDP_PORT);
	}

	if (m_State.SynchronizationPacket.nUniverseNumber != 0) {
		HandleSync();
	}
}

const uint8_t *E131Controller::GetSoftwareVersion() {
	return DEVICE_SOFTWARE_VERSION;
}

void E131Controller::SetSourceName(const char *pSourceName) {
	assert(pSourceName != nullptr);

	//TODO https://gcc.gnu.org/bugzilla/show_bug.cgi?id=88780
#if (__GNUC__ > 8)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstringop-truncation"
#endif
	strncpy(m_SourceName, pSourceName, e131::SOURCE_NAME_LENGTH - 1);
	m_SourceName[e131::SOURCE_NAME_LENGTH - 1] = '\0';
#if (__GNUC__ > 8)
#pragma GCC diagnostic pop
#endif
}

void E131Controller::SetPriority(uint8_t nPriority) { //TODO SetPriority
	m_State.nPriority = nPriority;
}

void E131Controller::SendDiscoveryPacket() {
	assert(m_DiscoveryIpAddress != 0);

	if (m_nCurrentPacketMillis - m_State.DiscoveryTime >= (UNIVERSE_DISCOVERY_INTERVAL_SECONDS * 1000)) {
		m_State.DiscoveryTime = m_nCurrentPacketMillis;

		m_pE131DiscoveryPacket->RootLayer.FlagsLength = __builtin_bswap16(static_cast<uint16_t>((0x07 << 12) | (DISCOVERY_ROOT_LAYER_LENGTH(m_State.nActiveUniverses))));
		m_pE131DiscoveryPacket->FrameLayer.FLagsLength = __builtin_bswap16(static_cast<uint16_t>((0x07 << 12) | (DISCOVERY_FRAME_LAYER_LENGTH(m_State.nActiveUniverses))));
		m_pE131DiscoveryPacket->UniverseDiscoveryLayer.FlagsLength = __builtin_bswap16(static_cast<uint16_t>((0x07 << 12) | DISCOVERY_LAYER_LENGTH(m_State.nActiveUniverses)));

		for (uint32_t i = 0; i < m_State.nActiveUniverses; i++) {
			m_pE131DiscoveryPacket->UniverseDiscoveryLayer.ListOfUniverses[i] = __builtin_bswap16(s_SequenceNumbers[i].nUniverse);
		}

		Network::Get()->SendTo(m_nHandle, m_pE131DiscoveryPacket, static_cast<uint16_t>(DISCOVERY_PACKET_SIZE(m_State.nActiveUniverses)), m_DiscoveryIpAddress, e131::UDP_PORT);

		DEBUG_PUTS("Discovery sent");
	}
}

uint8_t E131Controller::GetSequenceNumber(uint16_t nUniverse, uint32_t &nMulticastIpAddress) {
	assert(sizeof(struct TSequenceNumbers) == sizeof(uint64_t));

	int32_t nLow = 0;
	int32_t nMid = 0;
	int32_t nHigh = m_State.nActiveUniverses;

	while (nLow <= nHigh) {

		nMid = nLow + ((nHigh - nLow) / 2);

		const uint32_t nMidValue = s_SequenceNumbers[nMid].nUniverse;

		if (nMidValue < nUniverse) {
			nLow = nMid + 1;
		} else if (nMidValue > nUniverse) {
			nHigh = nMid - 1;
		} else {
			DEBUG_PRINTF("Found nUniverse=%u", nUniverse);
			nMulticastIpAddress = s_SequenceNumbers[nMid].nIpAddress;
			s_SequenceNumbers[nMid].nSequenceNumber++;
			return s_SequenceNumbers[nMid].nSequenceNumber;
		}
	}

	DEBUG_PRINTF("nActiveUniverses=%u -> %u : nLow=%d, nMid=%d, nHigh=%d", m_State.nActiveUniverses, nUniverse, nLow, nMid, nHigh);

	if ((nHigh != -1) && (m_State.nActiveUniverses != static_cast<uint32_t>(nHigh))) {
		auto p64 = reinterpret_cast<uint64_t *>(s_SequenceNumbers);

		for (int32_t i = m_State.nActiveUniverses - 1; i >= nLow; i--) {
			p64[i + 1] = p64[i];
		}

		s_SequenceNumbers[nLow].nIpAddress = universe_to_multicast_ip(nUniverse);
		s_SequenceNumbers[nLow].nUniverse = nUniverse;
		s_SequenceNumbers[nLow].nSequenceNumber = 0;

		nMulticastIpAddress = s_SequenceNumbers[nLow].nIpAddress;

		DEBUG_PRINTF(">m< nUniverse=%u, nLow=%d", nUniverse, nLow);
	} else {
		s_SequenceNumbers[nMid].nIpAddress = universe_to_multicast_ip(nUniverse);
		s_SequenceNumbers[nMid].nUniverse = nUniverse;

		nMulticastIpAddress = s_SequenceNumbers[nMid].nIpAddress;

		DEBUG_PRINTF(">a< nUniverse=%u, nMid=%d", nUniverse, nMid);
	}

	m_State.nActiveUniverses++;

	return 0;
}

void E131Controller::Print() {
	puts("sACN E1.31 Controller");
	printf(" Max Universes : %d\n", static_cast<int>(sizeof(s_SequenceNumbers) / sizeof(s_SequenceNumbers[0])));
	if (m_State.SynchronizationPacket.nUniverseNumber != 0) {
		printf(" Synchronization Universe : %u\n", m_State.SynchronizationPacket.nUniverseNumber);
	} else {
		puts(" Synchronization is disabled");
	}
}
