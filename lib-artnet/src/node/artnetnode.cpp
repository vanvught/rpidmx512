/**
 * @file artnetnode.cpp
 *
 */
/**
 * Art-Net Designed by and Copyright Artistic Licence Holdings Ltd.
 */
/* Copyright (C) 2016-2022 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include "lightsetdata.h"

#include "hardware.h"
#include "network.h"

#include "ledblink.h"
#include "panel_led.h"

#include "artnetnode_internal.h"

#include "debug.h"

using namespace artnet;
using namespace artnetnode;

static constexpr auto ARTNET_MIN_HEADER_SIZE = 12;

ArtNetNode *ArtNetNode::s_pThis;

ArtNetNode::ArtNetNode() {
	assert(s_pThis == nullptr);
	s_pThis = this;

	DEBUG_PRINTF("PAGE_SIZE=%u, PAGES=%u, MAX_PORTS=%u", artnetnode::PAGE_SIZE, artnetnode::PAGES, artnetnode::MAX_PORTS);

	memset(&m_Node, 0, sizeof(struct Node));
	m_Node.IPAddressLocal = Network::Get()->GetIp();
	m_Node.IPAddressBroadcast = m_Node.IPAddressLocal | ~(Network::Get()->GetNetmask());
	m_Node.IPAddressTimeCode = m_Node.IPAddressBroadcast;
	Network::Get()->MacAddressCopyTo(m_Node.MACAddressLocal);

	m_Node.Status1 = Status1::INDICATOR_NORMAL_MODE | Status1::PAP_FRONT_PANEL;
	m_Node.Status2 = Status2::PORT_ADDRESS_15BIT | (artnet::VERSION > 3 ? Status2::SACN_ABLE_TO_SWITCH : Status2::SACN_NO_SWITCH) | Status2::RDM_SWITCH;
	m_Node.Status3 = Status3::NETWORKLOSS_LAST_STATE | Status3::FAILSAFE_CONTROL | Status3::OUTPUT_SWITCH;

	memset(&m_State, 0, sizeof(struct State));
	m_State.reportCode = ReportCode::RCPOWEROK;
	m_State.status = Status::STANDBY;

	for (uint32_t i = 0; i < artnetnode::MAX_PORTS; i++) {
		memset(&m_OutputPort[i], 0 , sizeof(struct OutputPort));
		memset(&m_InputPort[i], 0 , sizeof(struct InputPort));
		m_InputPort[i].nDestinationIp = m_Node.IPAddressBroadcast;
		m_InputPort[i].genericPort.nStatus = GoodInput::DISABLED;
	}

	SetShortName(defaults::SHORT_NAME);

	uint8_t nBoardNameLength;
	const auto *pBoardName = Hardware::Get()->GetBoardName(nBoardNameLength);
	const auto *pWebsiteUrl = Hardware::Get()->GetWebsiteUrl();
	snprintf(m_aDefaultNodeLongName, artnet::LONG_NAME_LENGTH, "%s %s %d %s", pBoardName, artnet::NODE_ID, artnet::VERSION, pWebsiteUrl);
	SetLongName(m_aDefaultNodeLongName);

	uint8_t nSysNameLenght;
	const auto *pSysName = Hardware::Get()->GetSysName(nSysNameLenght);
	strncpy(m_aSysName, pSysName, (sizeof(m_aSysName)) - 1);
	m_aSysName[(sizeof(m_aSysName)) - 1] = '\0';

	memcpy(m_ArtDmx.Id, artnet::NODE_ID, sizeof(m_PollReply.Id));
	m_ArtDmx.OpCode = OP_DMX;
	m_ArtDmx.ProtVerHi = 0;
	m_ArtDmx.ProtVerLo = artnet::PROTOCOL_REVISION;

	memcpy(m_ArtRdm.Id, artnet::NODE_ID, sizeof(m_PollReply.Id));
	m_ArtRdm.OpCode = OP_RDM;
	m_ArtRdm.ProtVerHi = 0;
	m_ArtRdm.ProtVerLo = artnet::PROTOCOL_REVISION;
	m_ArtRdm.RdmVer = 0x01; // Devices that support RDM STANDARD V1.0 set field to 0x01.
	m_ArtRdm.Spare1 = 0;
	m_ArtRdm.Spare2 = 0;
	m_ArtRdm.Spare3 = 0;
	m_ArtRdm.Spare4 = 0;
	m_ArtRdm.Spare5 = 0;
	m_ArtRdm.Spare6 = 0;
	m_ArtRdm.Spare7 = 0;
}

ArtNetNode::~ArtNetNode() {
	DEBUG_ENTRY

	DEBUG_EXIT
}

void ArtNetNode::Start() {
	if (artnet::VERSION > 3) {
		assert(m_pArtNet4Handler != nullptr);
	}

#if defined	(RDM_CONTROLLER) || defined	(RDM_RESPONDER)
//	assert(m_pArtNetRdm != nullptr);
#endif

	m_Node.Status2 = static_cast<uint8_t>((m_Node.Status2 & ~(Status2::IP_DHCP)) | (Network::Get()->IsDhcpUsed() ? Status2::IP_DHCP : Status2::IP_MANUALY));
	m_Node.Status2 = static_cast<uint8_t>((m_Node.Status2 & ~(Status2::DHCP_CAPABLE)) | (Network::Get()->IsDhcpCapable() ? Status2::DHCP_CAPABLE : 0));

	FillPollReply();
#if defined ( ENABLE_SENDDIAG )
	FillDiagData();
#endif

	m_nHandle = Network::Get()->Begin(artnet::UDP_PORT);
	assert(m_nHandle != -1);

	m_State.status = Status::ON;

	if (m_pArtNetDmx != nullptr) {
		for (uint32_t i = 0; i < artnetnode::MAX_PORTS; i++) {
			if (m_InputPort[i].genericPort.bIsEnabled) {
				m_pArtNetDmx->Start(i);
			}
		}
	}

	SendPollRelply(false);	// send a reply on startup

	if (m_pArtNetRdm != nullptr) {
		for (uint32_t nPortIndex = 0; nPortIndex < artnetnode::MAX_PORTS; nPortIndex++) {
			/* An Output Gateway will send the ArtTodData packet in the following circumstances:
			 * - Upon power on or decice reset. 
			 * - In response to an ArtTodRequest if the Port-Address matches.
			 * - In response to an ArtTodControl if the Port-Address matches.
			 * - When their ToD changes due to the addition or deletion of a UID.
			 * - At the end of full RDM discovery.
			 */
			if ((m_OutputPort[nPortIndex].isRdmEnabled) && m_OutputPort[nPortIndex].genericPort.bIsEnabled) {
				SendTod(nPortIndex);
			}

			/* A controller should periodically broadcast an ArtTodRequest
			 * to the network in order to ensure its TODs are up to date. 
			 * 
			 * When to send:
			 * - Upon power on or device reset.
			 * - At regular intervals [10 minutes]. In theory not needed
			 *   since Output Gateways will broadcast an ArtTodData if their
			 *   ToD changes, however it is safe programming.
			 */
			if (m_InputPort[nPortIndex].genericPort.bIsEnabled) {
				SendTodRequest(nPortIndex);
			}
		}
	}

	LedBlink::Get()->SetMode(ledblink::Mode::NORMAL);
	hal::panel_led_on(hal::panelled::ARTNET);
}

void ArtNetNode::Stop() {
	DEBUG_ENTRY

	for (uint32_t nPortIndex = 0; nPortIndex < artnetnode::MAX_PORTS; nPortIndex++) {
		if (m_OutputPort[nPortIndex].protocol == PortProtocol::ARTNET) {
			if (m_pLightSet != nullptr) {
				m_pLightSet->Stop(nPortIndex);
			}
			lightset::Data::ClearLength(nPortIndex);
			m_OutputPort[nPortIndex].IsTransmitting = false;
		}
	}

	if (m_pArtNetDmx != nullptr) {
		for (uint32_t i = 0; i < artnet::PORTS; i++) {
			if (m_InputPort[i].genericPort.bIsEnabled) {
				m_pArtNetDmx->Stop(i);
			}
		}
	}

	LedBlink::Get()->SetMode(ledblink::Mode::OFF_OFF);
	hal::panel_led_off(hal::panelled::ARTNET);

	m_Node.Status1 = static_cast<uint8_t>((m_Node.Status1 & ~Status1::INDICATOR_MASK) | Status1::INDICATOR_MUTE_MODE);
	m_State.status = Status::STANDBY;

	DEBUG_EXIT
}

void ArtNetNode::SetShortName(const char *pShortName) {
	DEBUG_ENTRY

	assert(pShortName != nullptr);

	strncpy(m_Node.ShortName, pShortName, artnet::SHORT_NAME_LENGTH - 1);
	m_Node.ShortName[artnet::SHORT_NAME_LENGTH - 1] = '\0';

	memcpy(m_PollReply.ShortName, m_Node.ShortName, artnet::SHORT_NAME_LENGTH);

	if (m_State.status == Status::ON) {
		if (m_pArtNetStore != nullptr) {
			m_pArtNetStore->SaveShortName(m_Node.ShortName);
		}

		artnet::display_shortname(m_Node.ShortName);
	}

	DEBUG_EXIT
}

void ArtNetNode::SetLongName(const char *pLongName) {
	DEBUG_ENTRY
	assert(pLongName != nullptr);

	strncpy(m_Node.LongName, pLongName, artnet::LONG_NAME_LENGTH - 1);
	m_Node.LongName[artnet::LONG_NAME_LENGTH - 1] = '\0';

	memcpy(m_PollReply.LongName, m_Node.LongName, artnet::LONG_NAME_LENGTH);

	if (m_State.status == Status::ON) {
		if (m_pArtNetStore != nullptr) {
			m_pArtNetStore->SaveLongName(m_Node.LongName);
		}

		artnet::display_longname(m_Node.LongName);
	}

	DEBUG_EXIT
}

void ArtNetNode::SetNetworkDataLossCondition() {
	m_State.IsMergeMode = false;
	m_State.IsSynchronousMode = false;

	uint32_t nIpCount = 0;

	for (uint32_t i = 0; i < artnetnode::MAX_PORTS; i++) {
		nIpCount += (m_OutputPort[i].sourceA.nIp + m_OutputPort[i].sourceB.nIp);
		if (nIpCount != 0) {
			break;
		}
	}

	if (nIpCount == 0) {
		return;
	}

	const auto networkloss = (m_Node.Status3 & artnet::Status3::NETWORKLOSS_MASK);

	DEBUG_PRINTF("networkloss=%x", networkloss);

	switch (networkloss) {
	case artnet::Status3::NETWORKLOSS_LAST_STATE:
		break;
	case artnet::Status3::NETWORKLOSS_OFF_STATE:
		m_pLightSet->Blackout(true);
		break;
	case artnet::Status3::NETWORKLOSS_ON_STATE:
		m_pLightSet->FullOn();
		break;
	case artnet::Status3::NETWORKLOSS_PLAYBACK:
#if defined(ARTNET_HAVE_FAILSAFE_RECORD)
		FailSafePlayback();
#endif
		break;
	default:
		assert(0);
		__builtin_unreachable();
		break;
	}

	for (uint32_t i = 0; i < artnetnode::MAX_PORTS; i++) {
		m_OutputPort[i].sourceA.nIp = 0;
		m_OutputPort[i].sourceB.nIp = 0;
		lightset::Data::ClearLength(i);
	}
}

void ArtNetNode::GetType() {
	const auto *pPacket = reinterpret_cast<char*>(&(m_ArtNetPacket.ArtPacket));

	if (m_ArtNetPacket.nLength < ARTNET_MIN_HEADER_SIZE) {
		m_ArtNetPacket.OpCode = OP_NOT_DEFINED;
		return;
	}

	if ((pPacket[10] != 0) || (pPacket[11] != artnet::PROTOCOL_REVISION)) {
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
		if ((m_nCurrentPacketMillis - m_nPreviousPacketMillis) >= artnet::NETWORK_DATA_LOSS_TIMEOUT * 1000) {
			SetNetworkDataLossCondition();
			hal::panel_led_off(hal::panelled::ARTNET);
		}

		if (m_State.SendArtPollReplyOnChange) {
			auto doSend = m_State.IsChanged;
			if (artnet::VERSION > 3) {
				doSend |= m_pArtNet4Handler->IsStatusChanged();
			}
			if (doSend) {
				SendPollRelply(false);
			}
		}

		if ((m_nCurrentPacketMillis - m_nPreviousPacketMillis) >= (1U * 1000U)) {
			m_State.nReceivingDmx &= static_cast<uint8_t>(~(1U << static_cast<uint8_t>(lightset::PortDir::OUTPUT)));
		}

		if (m_pArtNetDmx != nullptr) {
			HandleDmxIn();
		}

		if (m_pArtNetRdm != nullptr) {
			HandleRdmIn();
		}

		if ((((m_Node.Status1 & Status1::INDICATOR_MASK) == Status1::INDICATOR_NORMAL_MODE)) && (LedBlink::Get()->GetMode() != ledblink::Mode::FAST))  {
			if (artnet::VERSION > 3) {
				if (m_State.nReceivingDmx != 0) {
					m_pArtNet4Handler->SetLedBlinkMode(ledblink::Mode::DATA);
				} else {
					m_pArtNet4Handler->SetLedBlinkMode(ledblink::Mode::NORMAL);
				}
			} else {
				if (m_State.nReceivingDmx != 0) {
					LedBlink::Get()->SetMode(ledblink::Mode::DATA);
				} else {
					LedBlink::Get()->SetMode(ledblink::Mode::NORMAL);
				}
			}
		}

		return;
	}

	m_ArtNetPacket.nLength = nBytesReceived;
	m_nPreviousPacketMillis = m_nCurrentPacketMillis;

	if (m_State.IsSynchronousMode) {
		if (m_nCurrentPacketMillis - m_State.nArtSyncMillis >= (4 * 1000)) {
			m_State.IsSynchronousMode = false;
		}
	}

	GetType();

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
	case OP_TODDATA:
		if (m_pArtNetRdm != nullptr) {
			HandleTodData();
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

	if (m_pArtNetRdm != nullptr) {
		HandleRdmIn();
	}

	if ((((m_Node.Status1 & Status1::INDICATOR_MASK) == Status1::INDICATOR_NORMAL_MODE)) && (LedBlink::Get()->GetMode() != ledblink::Mode::FAST)) {
		if (artnet::VERSION > 3) {
			if (m_State.nReceivingDmx != 0) {
				m_pArtNet4Handler->SetLedBlinkMode(ledblink::Mode::DATA);
			} else {
				m_pArtNet4Handler->SetLedBlinkMode(ledblink::Mode::NORMAL);
			}
		} else {
			if (m_State.nReceivingDmx != 0) {
				LedBlink::Get()->SetMode(ledblink::Mode::DATA);
			} else {
				LedBlink::Get()->SetMode(ledblink::Mode::NORMAL);
			}
		}
	}

	hal::panel_led_on(hal::panelled::ARTNET);
}
