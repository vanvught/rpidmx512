/**
 * @file artnetnode.cpp
 *
 */
/**
 * Art-Net Designed by and Copyright Artistic Licence Holdings Ltd.
 */
/* Copyright (C) 2016-2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "artnetnode.h"
#include "artnetconst.h"
#include "packets.h"

#include "lightset.h"

#include "hardware.h"
#include "network.h"
#include "ledblink.h"

#include "artnetnode_internal.h"

#include "debug.h"

using namespace artnet;
using namespace artnetnode;

static constexpr auto ARTNET_MIN_HEADER_SIZE = 12;

ArtNetNode *ArtNetNode::s_pThis = nullptr;

ArtNetNode::ArtNetNode(uint8_t nPages) : m_nPages(nPages <= ArtNet::PAGES ? nPages : ArtNet::PAGES) {
	assert(Hardware::Get() != nullptr);
	assert(Network::Get() != nullptr);
	assert(LedBlink::Get() != nullptr);

	s_pThis = this;

	memset(&m_Node, 0, sizeof(struct Node));
	m_Node.IPAddressLocal = Network::Get()->GetIp();
	m_Node.IPAddressBroadcast = m_Node.IPAddressLocal | ~(Network::Get()->GetNetmask());
	m_Node.IPAddressTimeCode = m_Node.IPAddressBroadcast;
	Network::Get()->MacAddressCopyTo(m_Node.MACAddressLocal);
	m_Node.Status1 = STATUS1_INDICATOR_NORMAL_MODE | STATUS1_PAP_FRONT_PANEL;
	m_Node.Status2 = ArtNetStatus2::PORT_ADDRESS_15BIT | (ArtNet::VERSION > 3 ? ArtNetStatus2::SACN_ABLE_TO_SWITCH : ArtNetStatus2::SACN_NO_SWITCH);
	m_Node.Status3 = ArtNetStatus3::NETWORKLOSS_OFF_STATE;

	memset(&m_State, 0, sizeof(struct State));
	m_State.reportCode = ReportCode::RCPOWEROK;
	m_State.status = Status::STANDBY;
	m_State.nNetworkDataLossTimeoutMillis = artnet::NETWORK_DATA_LOSS_TIMEOUT * 1000U;

	for (uint32_t i = 0; i < artnetnode::MAX_PORTS; i++) {
		// Output
		memset(&m_OutputPorts[i], 0 , sizeof(struct OutputPort));
		// Input
		memset(&m_InputPorts[i], 0 , sizeof(struct InputPort));
		m_InputPorts[i].nDestinationIp = m_Node.IPAddressBroadcast;
		m_InputPorts[i].genericPort.nStatus = GI_DISABLED;
	}

	SetShortName(defaults::SHORT_NAME);

	uint8_t nBoardNameLength;
	const auto *pBoardName = Hardware::Get()->GetBoardName(nBoardNameLength);
	const auto *pWebsiteUrl = Hardware::Get()->GetWebsiteUrl();
	snprintf(m_aDefaultNodeLongName, ArtNet::LONG_NAME_LENGTH, "%s %s %d %s", pBoardName, artnet::NODE_ID, ArtNet::VERSION, pWebsiteUrl);
	SetLongName(m_aDefaultNodeLongName);

	SetOemValue(ArtNetConst::OEM_ID);

	uint8_t nSysNameLenght;
	const auto *pSysName = Hardware::Get()->GetSysName(nSysNameLenght);
	strncpy(m_aSysName, pSysName, (sizeof(m_aSysName)) - 1);
	m_aSysName[(sizeof(m_aSysName)) - 1] = '\0';
}

void ArtNetNode::Start() {
	m_Node.Status2 = static_cast<uint8_t>((m_Node.Status2 & ~(ArtNetStatus2::IP_DHCP)) | (Network::Get()->IsDhcpUsed() ? ArtNetStatus2::IP_DHCP : ArtNetStatus2::IP_MANUALY));
	m_Node.Status2 = static_cast<uint8_t>((m_Node.Status2 & ~(ArtNetStatus2::DHCP_CAPABLE)) | (Network::Get()->IsDhcpCapable() ? ArtNetStatus2::DHCP_CAPABLE : 0));

	FillPollReply();
#if defined ( ENABLE_SENDDIAG )
	FillDiagData();
#endif

	m_nHandle = Network::Get()->Begin(ArtNet::UDP_PORT);
	assert(m_nHandle != -1);

	m_State.status = Status::ON;

	if (m_pArtNetDmx != nullptr) {
		for (uint32_t i = 0; i < ArtNet::PORTS; i++) {
			if (m_InputPorts[i].genericPort.bIsEnabled) {
				m_pArtNetDmx->Start(i);
			}
		}
	}

	LedBlink::Get()->SetMode(ledblink::Mode::NORMAL);

	SendPollRelply(false);	// send a reply on startup
}

void ArtNetNode::Stop() {
	DEBUG_ENTRY

	if (m_pLightSet != nullptr) {
		for (uint32_t i = 0; i < artnetnode::MAX_PORTS; i++) {
			if (m_OutputPorts[i].protocol == PortProtocol::ARTNET) {
				m_pLightSet->Stop(i);
				m_OutputPorts[i].nLength = 0;
				m_OutputPorts[i].IsTransmitting = false;
			}
		}
	}

	if (m_pArtNetDmx != nullptr) {
		for (uint32_t i = 0; i < ArtNet::PORTS; i++) {
			if (m_InputPorts[i].genericPort.bIsEnabled) {
				m_pArtNetDmx->Stop(i);
			}
		}
	}

	LedBlink::Get()->SetMode(ledblink::Mode::OFF_OFF);

	m_Node.Status1 = static_cast<uint8_t>((m_Node.Status1 & ~STATUS1_INDICATOR_MASK) | STATUS1_INDICATOR_MUTE_MODE);
	m_State.status = Status::OFF;

	DEBUG_EXIT
}

void ArtNetNode::SetShortName(const char *pShortName) {
	assert(pShortName != nullptr);

	strncpy(m_Node.ShortName, pShortName, ArtNet::SHORT_NAME_LENGTH - 1);
	m_Node.ShortName[ArtNet::SHORT_NAME_LENGTH - 1] = '\0';

	memcpy(m_PollReply.ShortName, m_Node.ShortName, ArtNet::SHORT_NAME_LENGTH);

	if (m_State.status == Status::ON) {
		if (m_pArtNetStore != nullptr) {
			m_pArtNetStore->SaveShortName(m_Node.ShortName);
		}
		if (m_pArtNetDisplay != nullptr) {
			m_pArtNetDisplay->ShowShortName(m_Node.ShortName);
		}
	}
}

void ArtNetNode::SetLongName(const char *pLongName) {
	assert(pLongName != nullptr);

	strncpy(m_Node.LongName, pLongName, ArtNet::LONG_NAME_LENGTH - 1);
	m_Node.LongName[ArtNet::LONG_NAME_LENGTH - 1] = '\0';

	memcpy(m_PollReply.LongName, m_Node.LongName, ArtNet::LONG_NAME_LENGTH);

	if (m_State.status == Status::ON) {
		if (m_pArtNetStore != nullptr) {
			m_pArtNetStore->SaveLongName(m_Node.LongName);
		}
		if (m_pArtNetDisplay != nullptr) {
			m_pArtNetDisplay->ShowLongName(m_Node.LongName);
		}
	}
}

void ArtNetNode::SetOemValue(const uint8_t *pOem) {
	assert(pOem != nullptr);

	m_Node.Oem[0] = pOem[0];
	m_Node.Oem[1] = pOem[1];
}

void ArtNetNode::SetNetworkDataLossCondition() {
	m_State.IsMergeMode = false;
	m_State.IsSynchronousMode = false;

	for (uint32_t i = 0; i < (ArtNet::PORTS * m_nPages); i++) {
		if ((m_OutputPorts[i].protocol == PortProtocol::ARTNET) && (m_OutputPorts[i].IsTransmitting)) {
			m_pLightSet->Stop(i);
			m_OutputPorts[i].IsTransmitting = false;
		}

		m_OutputPorts[i].genericPort.nStatus &= static_cast<uint8_t>(~GO_DATA_IS_BEING_TRANSMITTED);
		m_OutputPorts[i].nLength = 0;
		m_OutputPorts[i].sourceA.nIp = 0;
		m_OutputPorts[i].sourceB.nIp = 0;
	}
}

void ArtNetNode::GetType() {
	const auto *pPacket = reinterpret_cast<char*>(&(m_ArtNetPacket.ArtPacket));

	if (m_ArtNetPacket.nLength < ARTNET_MIN_HEADER_SIZE) {
		m_ArtNetPacket.OpCode = OP_NOT_DEFINED;
		return;
	}

	if ((pPacket[10] != 0) || (pPacket[11] != ArtNet::PROTOCOL_REVISION)) {
		m_ArtNetPacket.OpCode = OP_NOT_DEFINED;
		return;
	}

	if (memcmp(pPacket, "Art-Net\0", 8) == 0) {
		m_ArtNetPacket.OpCode = static_cast<TOpCodes>((static_cast<uint16_t>(pPacket[9] << 8)) + pPacket[8]);
	} else {
		m_ArtNetPacket.OpCode = OP_NOT_DEFINED;
	}
}

void ArtNetNode::Run() {
	uint16_t nForeignPort;

	const auto nBytesReceived = Network::Get()->RecvFrom(m_nHandle, &(m_ArtNetPacket.ArtPacket), sizeof(m_ArtNetPacket.ArtPacket), &m_ArtNetPacket.IPAddressFrom, &nForeignPort);

	m_nCurrentPacketMillis = Hardware::Get()->Millis();

	if (__builtin_expect((nBytesReceived == 0), 1)) {
		if ((m_State.nNetworkDataLossTimeoutMillis != 0) && ((m_nCurrentPacketMillis - m_nPreviousPacketMillis) >= m_State.nNetworkDataLossTimeoutMillis)) {
			SetNetworkDataLossCondition();
		}

		if (m_State.SendArtPollReplyOnChange) {
			auto doSend = m_State.IsChanged;
			if (m_pArtNet4Handler != nullptr) {
				doSend |= m_pArtNet4Handler->IsStatusChanged();
			}
			if (doSend) {
				SendPollRelply(false);
			}
		}

		if ((m_nCurrentPacketMillis - m_nPreviousPacketMillis) >= (1 * 1000)) {
			m_State.nReceivingDmx &= static_cast<uint8_t>(~(1U << static_cast<uint8_t>(lightset::PortDir::OUTPUT)));
		}

		if (m_pArtNetDmx != nullptr) {
			HandleDmxIn();
		}

		if ((((m_Node.Status1 & STATUS1_INDICATOR_MASK) == STATUS1_INDICATOR_NORMAL_MODE)) && (LedBlink::Get()->GetMode() != ledblink::Mode::FAST))  {
			if (m_State.nReceivingDmx != 0) {
				LedBlink::Get()->SetMode(ledblink::Mode::DATA);
			} else {
				LedBlink::Get()->SetMode(ledblink::Mode::NORMAL);
			}
		}

		return;
	}

	m_ArtNetPacket.nLength = nBytesReceived;
	m_nPreviousPacketMillis = m_nCurrentPacketMillis;

	GetType();

	if (m_State.IsSynchronousMode) {
		if (m_nCurrentPacketMillis - m_State.nArtSyncMillis >= (4 * 1000)) {
			m_State.IsSynchronousMode = false;
		}
	}

	switch (m_ArtNetPacket.OpCode) {
	case OP_POLL:
		HandlePoll();
		break;
	case OP_DMX:
		if (m_pLightSet != nullptr) {
			HandleDmx();
		}
		break;
	case OP_SYNC:
		if (m_pLightSet != nullptr) {
			HandleSync();
		}
		break;
	case OP_ADDRESS:
		HandleAddress();
		break;
	case OP_TIMECODE:
		if (m_pArtNetTimeCode != nullptr) {
			HandleTimeCode();
		}
		break;
	case OP_TIMESYNC:
		HandleTimeSync();
		break;
	case OP_TODREQUEST:
		if (m_pArtNetRdm != nullptr) {
			HandleTodRequest();
		}
		break;
	case OP_TODCONTROL:
		if (m_pArtNetRdm != nullptr) {
			HandleTodControl();
		}
		break;
	case OP_RDM:
		if (m_pArtNetRdm != nullptr) {
			HandleRdm();
		}
		break;
	case OP_IPPROG:
		HandleIpProg();
		break;
	case OP_TRIGGER:
		if (m_pArtNetTrigger != nullptr) {
			HandleTrigger();
		}
		break;
	default:
		// ArtNet but OpCode is not implemented
		// Just skip ... no error
		break;
	}

	if (m_pArtNetDmx != nullptr) {
		HandleDmxIn();
	}

	if ((((m_Node.Status1 & STATUS1_INDICATOR_MASK) == STATUS1_INDICATOR_NORMAL_MODE)) && (LedBlink::Get()->GetMode() != ledblink::Mode::FAST)) {
		if (m_State.nReceivingDmx != 0) {
			LedBlink::Get()->SetMode(ledblink::Mode::DATA);
		} else {
			LedBlink::Get()->SetMode(ledblink::Mode::NORMAL);
		}
	}
}
