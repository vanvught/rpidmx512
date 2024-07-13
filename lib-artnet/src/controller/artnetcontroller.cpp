/**
 * @file artnetcontroller.cpp
 *
 */
/**
 * Art-Net Designed by and Copyright Artistic Licence Holdings Ltd.
 */
/* Copyright (C) 2017-2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <cstdio>
#include <cassert>

#include "artnetcontroller.h"

#include "artnet.h"
#include "artnetconst.h"

#if (ARTNET_VERSION >= 4)
# include "e131.h"
#endif

#include "hardware.h"
#include "network.h"

#include "debug.h"

ArtNetController *ArtNetController::s_pThis;

using namespace artnet;

static constexpr uint32_t ARTNET_MIN_HEADER_SIZE = 12;
static uint16_t s_ActiveUniverses[POLL_TABLE_SIZE_UNIVERSES] __attribute__ ((aligned (4)));

ArtNetController::ArtNetController() {
	DEBUG_ENTRY

	union uip {
		uint32_t u32;
		uint8_t u8[4];
	}  ip;

	assert(s_pThis == nullptr);
	s_pThis = this;

	m_pArtNetPacket = new struct TArtNetPacket;
	assert(m_pArtNetPacket != nullptr);

	memset(&m_State, 0, sizeof(struct State));
	m_State.reportCode = artnet::ReportCode::RCPOWEROK;
	m_State.status = artnet::Status::STANDBY;

	memset(&m_ArtNetPoll, 0, sizeof(struct ArtPoll));
	memcpy(&m_ArtNetPoll, artnet::NODE_ID, 8);
	m_ArtNetPoll.OpCode = static_cast<uint16_t>(artnet::OpCodes::OP_POLL);
	m_ArtNetPoll.ProtVerLo = artnet::PROTOCOL_REVISION;
	m_ArtNetPoll.Flags = Flags::SEND_ARTP_ON_CHANGE;

	memset(&m_ArtPollReply, 0, sizeof(struct ArtPollReply));
	memcpy(&m_ArtPollReply, artnet::NODE_ID, 8);
	m_ArtPollReply.OpCode = static_cast<uint16_t>(artnet::OpCodes::OP_POLLREPLY);
	m_ArtPollReply.Port = artnet::UDP_PORT;
	m_ArtPollReply.VersInfoH = ArtNetConst::VERSION[0];
	m_ArtPollReply.VersInfoL = ArtNetConst::VERSION[1];
	m_ArtPollReply.OemHi = ArtNetConst::OEM_ID[0];
	m_ArtPollReply.Oem = ArtNetConst::OEM_ID[1];
	m_ArtPollReply.EstaMan[0] = ArtNetConst::ESTA_ID[1];
	m_ArtPollReply.EstaMan[1] = ArtNetConst::ESTA_ID[0];
	m_ArtPollReply.Style = static_cast<uint8_t>(StyleCode::SERVER);
	Network::Get()->MacAddressCopyTo(m_ArtPollReply.MAC);
	m_ArtPollReply.BindIndex = 1;
	ip.u32 = Network::Get()->GetIp();
	memcpy(m_ArtPollReply.IPAddress, ip.u8, sizeof(m_ArtPollReply.IPAddress));
#if (ARTNET_VERSION >= 4)
	memcpy(m_ArtPollReply.BindIp, ip.u8, sizeof(m_ArtPollReply.BindIp));
	m_ArtPollReply.AcnPriority = e131::priority::DEFAULT;
#endif
	/*
	 * Status 1
	 */
	m_ArtPollReply.Status1 |= artnet::Status1::INDICATOR_NORMAL_MODE | artnet::Status1::PAP_NETWORK;
	/*
	 * Status 2
	 */
	m_ArtPollReply.Status2 &= static_cast<uint8_t>(~artnet::Status2::SACN_ABLE_TO_SWITCH);
	m_ArtPollReply.Status2 |= artnet::Status2::PORT_ADDRESS_15BIT | (artnet::VERSION >= 4 ? artnet::Status2::SACN_ABLE_TO_SWITCH : artnet::Status2::SACN_NO_SWITCH);
	m_ArtPollReply.Status2 &= static_cast<uint8_t>(~artnet::Status2::IP_DHCP);
	m_ArtPollReply.Status2 |= Network::Get()->IsDhcpUsed() ? artnet::Status2::IP_DHCP : artnet::Status2::IP_MANUALY;
	m_ArtPollReply.Status2 &= static_cast<uint8_t>(~artnet::Status2::DHCP_CAPABLE);
	m_ArtPollReply.Status2 |= Network::Get()->IsDhcpCapable() ? artnet::Status2::DHCP_CAPABLE : static_cast<uint8_t>(0);

#if defined (ENABLE_HTTPD) && defined (ENABLE_CONTENT)
	m_ArtPollReply.Status2 |= artnet::Status2::WEB_BROWSER_SUPPORT;
#endif

	m_ArtPollReply.PortTypes[0] = artnet::PortType::OUTPUT_ARTNET;
	m_ArtPollReply.PortTypes[1] = artnet::PortType::INPUT_ARTNET;
	m_ArtPollReply.GoodOutput[0] = artnet::GoodOutput::DATA_IS_BEING_TRANSMITTED;
	m_ArtPollReply.GoodInput[0] = artnet::GoodInput::DATA_RECIEVED;
	m_ArtPollReply.NumPortsLo = 2;

	m_pArtDmx = new struct ArtDmx;
	assert(m_pArtDmx != nullptr);

	memset(m_pArtDmx, 0, sizeof(struct ArtDmx));
	memcpy(m_pArtDmx, artnet::NODE_ID, 8);
	m_pArtDmx->OpCode = static_cast<uint16_t>(artnet::OpCodes::OP_DMX);
	m_pArtDmx->ProtVerLo = artnet::PROTOCOL_REVISION;

	m_pArtSync = new struct ArtSync;
	assert(m_pArtSync != nullptr);

	memset(m_pArtSync, 0, sizeof(struct ArtSync));
	memcpy(m_pArtSync, artnet::NODE_ID, 8);
	m_pArtSync->OpCode = static_cast<uint16_t>(artnet::OpCodes::OP_SYNC);
	m_pArtSync->ProtVerLo = artnet::PROTOCOL_REVISION;

	m_ArtNetController.Oem[0] = ArtNetConst::OEM_ID[0];
	m_ArtNetController.Oem[1] = ArtNetConst::OEM_ID[1];

	ActiveUniversesClear();

	SetShortName(nullptr);
	SetLongName(nullptr);

	DEBUG_EXIT
}

ArtNetController::~ArtNetController() {
	DEBUG_ENTRY

	delete m_pArtNetPacket;
	m_pArtNetPacket = nullptr;

	DEBUG_EXIT
}

void ArtNetController::GetShortNameDefault(char *ShortName) {
#if !defined (ARTNET_SHORT_NAME)
	uint8_t nBoardNameLength;
	const auto *const pBoardName = Hardware::Get()->GetBoardName(nBoardNameLength);
	const auto *const pWebsiteUrl = Hardware::Get()->GetWebsiteUrl();
	snprintf(ShortName, artnet::LONG_NAME_LENGTH - 1, "%s %s %u %s", pBoardName, artnet::NODE_ID, static_cast<unsigned int>(artnet::VERSION), pWebsiteUrl);
#else
	uint32_t i;

	for (i = 0; i < (sizeof(ARTNET_SHORT_NAME) - 1) && i < (artnet::SHORT_NAME_LENGTH - 1) ; i++ ) {
		if (ARTNET_SHORT_NAME[i] == '_') {
			ShortName[i] = ' ';
		} else {
			ShortName[i] = ARTNET_SHORT_NAME[i];
		}
	}

	ShortName[i] = '\0';
#endif
}

void ArtNetController::SetShortName(const char *ShortName) {
	DEBUG_ENTRY

	if (ShortName == nullptr) {
		GetShortNameDefault(reinterpret_cast<char *>(m_ArtPollReply.ShortName));
	} else {
		strncpy(reinterpret_cast<char *>(m_ArtPollReply.ShortName), ShortName, artnet::SHORT_NAME_LENGTH - 1);
	}

	m_ArtPollReply.LongName[artnet::SHORT_NAME_LENGTH - 1] = '\0';

	DEBUG_PUTS(reinterpret_cast<char *>(m_ArtPollReply.ShortName));
	DEBUG_EXIT
}

void ArtNetController::GetLongNameDefault(char *pLongName) {
#if !defined (ARTNET_LONG_NAME)
	uint8_t nBoardNameLength;
	const auto *const pBoardName = Hardware::Get()->GetBoardName(nBoardNameLength);
	const auto *const pWebsiteUrl = Hardware::Get()->GetWebsiteUrl();
	snprintf(pLongName, artnet::LONG_NAME_LENGTH - 1, "%s %s %u %s", pBoardName, artnet::NODE_ID, static_cast<unsigned int>(artnet::VERSION), pWebsiteUrl);
#else
	uint32_t i;

	for (i = 0; i < (sizeof(ARTNET_LONG_NAME) - 1) && i < (artnet::LONG_NAME_LENGTH - 1) ; i++ ) {
		if (ARTNET_LONG_NAME[i] == '_') {
			pLongName[i] = ' ';
		} else {
			pLongName[i] = ARTNET_LONG_NAME[i];
		}
	}

	pLongName[i] = '\0';
#endif
}

void ArtNetController::SetLongName(const char *pLongName) {
	DEBUG_ENTRY

	if (pLongName == nullptr) {
		GetLongNameDefault(reinterpret_cast<char *>(m_ArtPollReply.LongName));
	} else {
		strncpy(reinterpret_cast<char *>(m_ArtPollReply.LongName), pLongName, artnet::LONG_NAME_LENGTH - 1);
	}

	m_ArtPollReply.LongName[artnet::LONG_NAME_LENGTH - 1] = '\0';

	DEBUG_PUTS(reinterpret_cast<char *>(m_ArtPollReply.LongName));
	DEBUG_EXIT
}

void ArtNetController::Start() {
	DEBUG_ENTRY

	m_ArtNetController.nIPAddressLocal = Network::Get()->GetIp();
	m_ArtNetController.nIPAddressBroadcast = Network::Get()->GetBroadcastIp();

	assert(m_nHandle == -1);
	m_nHandle = Network::Get()->Begin(artnet::UDP_PORT);
	assert(m_nHandle != -1);

	Network::Get()->SendTo(m_nHandle, &m_ArtNetPoll, sizeof(struct ArtPoll), m_ArtNetController.nIPAddressBroadcast, artnet::UDP_PORT);

	m_State.status = artnet::Status::ON;

	DEBUG_EXIT
}

void ArtNetController::Stop() {
	DEBUG_ENTRY

//  FIXME ArtNetController::Stop
//
//	Network::Get()->End(artnet::UDP_PORT);
//	m_nHandle = -1;
//
//	m_State.status = artnet::Status::OFF;

	DEBUG_EXIT
}

void ArtNetController::HandleDmxOut(uint16_t nUniverse, const uint8_t *pDmxData, uint32_t nLength, uint8_t nPortIndex) {
	DEBUG_ENTRY

	ActiveUniversesAdd(nUniverse);

	m_pArtDmx->Physical = nPortIndex & 0xFF;
	m_pArtDmx->PortAddress = nUniverse;
	m_pArtDmx->LengthHi = static_cast<uint8_t>((nLength & 0xFF00) >> 8);
	m_pArtDmx->Length = static_cast<uint8_t>(nLength & 0xFF);

	// The sequence number is used to ensure that ArtDmx packets are used in the correct order.
	// This field is incremented in the range 0x01 to 0xff to allow the receiving node to resequence packets.
	m_pArtDmx->Sequence++;

	if (m_pArtDmx->Sequence == 0) {
		m_pArtDmx->Sequence = 1;
	}

#if defined(CONFIG_ARTNET_CONTROLLER_ENABLE_MASTER)
	if (__builtin_expect((m_nMaster == DMX_MAX_VALUE), 1)) {
#endif
		memcpy(m_pArtDmx->Data, pDmxData, nLength);
#if defined(CONFIG_ARTNET_CONTROLLER_ENABLE_MASTER)
	} else if (m_nMaster == 0) {
		memset(m_pArtDmx->Data, 0, nLength);
	} else {
		for (uint32_t i = 0; i < nLength; i++) {
			m_pArtDmx->Data[i] = ((m_nMaster * static_cast<uint32_t>(pDmxData[i])) / DMX_MAX_VALUE) & 0xFF;
		}
	}
#endif

	uint32_t nCount = 0;
	auto IpAddresses = const_cast<struct artnet::PollTableUniverses *>(GetIpAddress(nUniverse));

	if (m_bUnicast && !m_bForceBroadcast) {
		if (IpAddresses != nullptr) {
			nCount = IpAddresses->nCount;
		} else {
			DEBUG_EXIT
			return;
		}
	}

	// If the number of universe subscribers exceeds 40 for a given universe, the transmitting device may broadcast.

	if (m_bUnicast && (nCount <= 40) && !m_bForceBroadcast) {
		for (uint32_t nIndex = 0; nIndex < nCount; nIndex++) {
			Network::Get()->SendTo(m_nHandle, m_pArtDmx, sizeof(struct ArtDmx), IpAddresses->pIpAddresses[nIndex], artnet::UDP_PORT);
		}

		m_bDmxHandled = true;

		DEBUG_EXIT
		return;
	}

	if (!m_bUnicast || (nCount > 40) || !m_bForceBroadcast) {
		Network::Get()->SendTo(m_nHandle, m_pArtDmx, sizeof(struct ArtDmx), m_ArtNetController.nIPAddressBroadcast, artnet::UDP_PORT);

		m_bDmxHandled = true;
	}

	DEBUG_EXIT
}

void ArtNetController::HandleSync() {
	if (m_bSynchronization && m_bDmxHandled) {
		m_bDmxHandled = false;
		Network::Get()->SendTo(m_nHandle, m_pArtSync, sizeof(struct ArtSync), m_ArtNetController.nIPAddressBroadcast, artnet::UDP_PORT);
	}
}

void ArtNetController::HandleBlackout() {
	m_pArtDmx->LengthHi = (512 & 0xFF00) >> 8;
	m_pArtDmx->Length = (512 & 0xFF);

	memset(m_pArtDmx->Data, 0, 512);

	for (uint32_t nIndex = 0; nIndex < m_nActiveUniverses; nIndex++) {
		m_pArtDmx->PortAddress = s_ActiveUniverses[nIndex];

		uint32_t nCount = 0;
		const auto *IpAddresses = GetIpAddress(s_ActiveUniverses[nIndex]);

		if (m_bUnicast && !m_bForceBroadcast) {
			if (IpAddresses != nullptr) {
				nCount = IpAddresses->nCount;
			} else {
				continue;
			}
		}

		if (m_bUnicast && (nCount <= 40) && !m_bForceBroadcast) {
			// The sequence number is used to ensure that ArtDmx packets are used in the correct order.
			// This field is incremented in the range 0x01 to 0xff to allow the receiving node to resequence packets.
			m_pArtDmx->Sequence++;

			if (m_pArtDmx->Sequence == 0) {
				m_pArtDmx->Sequence = 1;
			}

			for (uint32_t nIndex = 0; nIndex < nCount; nIndex++) {
				Network::Get()->SendTo(m_nHandle, m_pArtDmx, sizeof(struct ArtDmx), IpAddresses->pIpAddresses[nIndex], artnet::UDP_PORT);
			}

			continue;
		}

		if (!m_bUnicast || (nCount > 40) || !m_bForceBroadcast) {
			// The sequence number is used to ensure that ArtDmx packets are used in the correct order.
			// This field is incremented in the range 0x01 to 0xff to allow the receiving node to resequence packets.
			m_pArtDmx->Sequence++;

			if (m_pArtDmx->Sequence == 0) {
				m_pArtDmx->Sequence = 1;
			}

			Network::Get()->SendTo(m_nHandle, m_pArtDmx, sizeof(struct ArtDmx), m_ArtNetController.nIPAddressBroadcast, artnet::UDP_PORT);
		}

	}

	m_bDmxHandled = true;
	HandleSync();
}

void ArtNetController::HandleTrigger() {
	DEBUG_ENTRY
	const ArtTrigger *pArtTrigger = &m_pArtNetPacket->ArtPacket.ArtTrigger;

	if ((pArtTrigger->OemCodeHi == 0xFF && pArtTrigger->OemCodeLo == 0xFF) || (pArtTrigger->OemCodeHi == m_ArtNetController.Oem[0] && pArtTrigger->OemCodeLo == m_ArtNetController.Oem[1])) {
		DEBUG_PRINTF("Key=%d, SubKey=%d, Data[0]=%d", pArtTrigger->Key, pArtTrigger->SubKey, pArtTrigger->Data[0]);

		m_pArtNetTrigger->Handler(reinterpret_cast<const struct TArtNetTrigger*>(&pArtTrigger->Key));
	}

	DEBUG_EXIT
}

void ArtNetController::ProcessPoll() {
	const auto nCurrentMillis = Hardware::Get()->Millis();

	if (__builtin_expect((nCurrentMillis - m_nLastPollMillis > POLL_INTERVAL_MILLIS), 0)) {
		Network::Get()->SendTo(m_nHandle, &m_ArtNetPoll, sizeof(struct ArtPoll), m_ArtNetController.nIPAddressBroadcast, artnet::UDP_PORT);
		m_nLastPollMillis= nCurrentMillis;

#ifndef NDEBUG
		Dump();
		DumpTableUniverses();
#endif
	}

	if (m_bDoTableCleanup && (__builtin_expect((nCurrentMillis - m_nLastPollMillis > POLL_INTERVAL_MILLIS/4), 0))) {
		Clean();
	}
}

void ArtNetController::HandlePoll() {
	DEBUG_ENTRY

	snprintf(reinterpret_cast<char*>(m_ArtPollReply.NodeReport), artnet::REPORT_LENGTH, "#%04x [%u]", static_cast<int>(m_State.reportCode), static_cast<unsigned>(m_State.ArtPollReplyCount++));

	Network::Get()->SendTo(m_nHandle, &m_ArtPollReply, sizeof(artnet::ArtPollReply), m_pArtNetPacket->IPAddressFrom, artnet::UDP_PORT);

	DEBUG_PRINTF(IPSTR, IP2STR(m_pArtNetPacket->IPAddressFrom));
	DEBUG_EXIT
}

void ArtNetController::HandlePollReply() {
	DEBUG_ENTRY
	DEBUG_PRINTF(IPSTR, IP2STR(m_pArtNetPacket->IPAddressFrom));

	if (m_pArtNetPacket->IPAddressFrom != Network::Get()->GetIp()) {
		Add(&m_pArtNetPacket->ArtPacket.ArtPollReply);

		DEBUG_EXIT
		return;
	}

	DEBUG_EXIT
}

void ArtNetController::Run() {
	auto *pArtPacket = reinterpret_cast<char *>(&m_pArtNetPacket->ArtPacket);
	uint16_t nForeignPort;

	if (m_bUnicast) {
		ProcessPoll();
	}

	const auto nBytesReceived = Network::Get()->RecvFrom(m_nHandle, pArtPacket, sizeof(struct TArtNetPacket), &m_pArtNetPacket->IPAddressFrom, &nForeignPort) ;

	if (__builtin_expect((nBytesReceived < ARTNET_MIN_HEADER_SIZE), 1)) {
		return;
	}

	if (memcmp(pArtPacket, "Art-Net\0", 8) != 0) {
		return;
	}

	const auto OpCode = static_cast<artnet::OpCodes>(((pArtPacket[9] << 8)) + pArtPacket[8]);

	switch (OpCode) {
	case artnet::OpCodes::OP_POLLREPLY:
		HandlePollReply();
		break;
	case artnet::OpCodes::OP_POLL:
		HandlePoll();
		break;
	case artnet::OpCodes::OP_TRIGGER:
		if (m_pArtNetTrigger != nullptr) {
			HandleTrigger();
		}
		break;
	default:
		break;
	}
}

void ArtNetController::ActiveUniversesClear() {
	memset(s_ActiveUniverses, 0, sizeof(s_ActiveUniverses));
	m_nActiveUniverses = 0;
}

void ArtNetController::ActiveUniversesAdd(uint16_t nUniverse) {
	DEBUG_ENTRY
	DEBUG_PRINTF("nUniverse=%d", static_cast<int>(nUniverse));

	int32_t nLow = 0;
	int32_t nMid = 0;
	auto nHigh = static_cast<int32_t>(m_nActiveUniverses);

	if (m_nActiveUniverses == (sizeof(s_ActiveUniverses) / sizeof(s_ActiveUniverses[0]))) {
		assert(0);
		return;
	}

	while (nLow <= nHigh) {

		nMid = nLow + ((nHigh - nLow) / 2);
		const uint32_t nMidValue = s_ActiveUniverses[nMid];

		if (nMidValue < nUniverse) {
			nLow = nMid + 1;
		} else if (nMidValue > nUniverse) {
			nHigh = nMid - 1;
		} else {
			DEBUG_EXIT
			return;
		}
	}

	DEBUG_PRINTF("nLow=%d, nMid=%d, nHigh=%d", nLow, nMid, nHigh);

	if ((nHigh != -1) && (m_nActiveUniverses != static_cast<uint32_t>(nHigh))) {

		auto p16 = reinterpret_cast<uint16_t*>(s_ActiveUniverses);

		assert(nLow >= 0);

		//TODO Can we make i unsigned ?
		for (uint32_t i = m_nActiveUniverses; i >= static_cast<uint32_t>(nLow); i--) {
			p16[i + 1] = p16[i];
		}

		s_ActiveUniverses[nLow] = nUniverse;

		DEBUG_PRINTF(">m< nUniverse=%u, nLow=%d", nUniverse, nLow);
	} else {
		s_ActiveUniverses[m_nActiveUniverses] = nUniverse;

		DEBUG_PRINTF(">a< nUniverse=%u, nMid=%d", nUniverse, nMid);
	}

	m_nActiveUniverses++;

	DEBUG_EXIT
}

void ArtNetController::Print() {
	puts("Art-Net Controller");
	printf(" Max Node's    : %u\n", POLL_TABLE_SIZE_ENRIES);
	printf(" Max Universes : %u\n", POLL_TABLE_SIZE_UNIVERSES);
	if (!m_bUnicast) {
		puts(" Unicast is disabled");
	}
	if (!m_bForceBroadcast) {
		puts(" Force broadcast is enabled");
	}
	if (!m_bSynchronization) {
		puts(" Synchronization is disabled");
	}
}
