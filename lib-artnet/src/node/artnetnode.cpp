/**
 * @file artnetnode.cpp
 *
 */
/**
 * Art-Net Designed by and Copyright Artistic Licence Holdings Ltd.
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

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cassert>

#include "artnetnode.h"
#include "artnetconst.h"
#include "artnet.h"

#include "lightset.h"
#include "lightsetdata.h"

#include "hardware.h"
#include "network.h"

#include "panel_led.h"

#include "artnetnode_internal.h"

#include "debug.h"

using namespace artnetnode;

static constexpr auto ARTNET_MIN_HEADER_SIZE = 12;

ArtNetNode *ArtNetNode::s_pThis;

ArtNetNode::ArtNetNode() {
	assert(s_pThis == nullptr);
	s_pThis = this;

	DEBUG_PRINTF("PAGE_SIZE=%u, PAGES=%u, MAX_PORTS=%u", artnetnode::PAGE_SIZE, artnetnode::PAGES, artnetnode::MAX_PORTS);

	memset(&m_Node, 0, sizeof(struct Node));
	m_Node.IPAddressBroadcast = Network::Get()->GetBroadcastIp();
	m_Node.IPAddressTimeCode = m_Node.IPAddressBroadcast;
	Network::Get()->MacAddressCopyTo(m_Node.MACAddressLocal);

	m_Node.Status1 = artnet::Status1::INDICATOR_NORMAL_MODE | artnet::Status1::PAP_FRONT_PANEL;
	m_Node.Status2 = artnet::Status2::PORT_ADDRESS_15BIT | (artnet::VERSION > 3 ? artnet::Status2::SACN_ABLE_TO_SWITCH : artnet::Status2::SACN_NO_SWITCH);
#if defined (RDM_CONTROLLER) || defined (RDM_RESPONDER)
	m_Node.Status2 |= artnet::Status2::RDM_SWITCH;
#endif
	m_Node.Status3 = artnet::Status3::NETWORKLOSS_LAST_STATE | artnet::Status3::FAILSAFE_CONTROL;
#if defined (ARTNET_HAVE_DMXIN)
	m_Node.Status3 |= artnet::Status3::OUTPUT_SWITCH;
#endif

	memset(&m_State, 0, sizeof(struct State));
	m_State.reportCode = ReportCode::RCPOWEROK;
	m_State.status = Status::STANDBY;

	for (uint32_t nPortIndex = 0; nPortIndex < artnetnode::MAX_PORTS; nPortIndex++) {
		memset(&m_OutputPort[nPortIndex], 0 , sizeof(struct OutputPort));
		m_OutputPort[nPortIndex].GoodOutputB = artnet::GoodOutputB::RDM_DISABLED | artnet::GoodOutputB::STYLE_CONSTANT;
		memset(&m_InputPort[nPortIndex], 0 , sizeof(struct InputPort));
		m_InputPort[nPortIndex].nDestinationIp = m_Node.IPAddressBroadcast;
	}

	SetShortName(nullptr);	// Default
	SetLongName(nullptr);	// Default

#if defined (ARTNET_HAVE_DMXIN)
	memcpy(m_ArtDmx.Id, artnet::NODE_ID, sizeof(m_PollReply.Id));
	m_ArtDmx.OpCode = OP_DMX;
	m_ArtDmx.ProtVerHi = 0;
	m_ArtDmx.ProtVerLo = artnet::PROTOCOL_REVISION;
#endif

#if defined (ARTNET_HAVE_TIMECODE)
	memcpy(m_ArtTimeCode.Id, artnet::NODE_ID, sizeof(m_PollReply.Id));
	m_ArtTimeCode.OpCode = OP_TIMECODE;
	m_ArtTimeCode.ProtVerHi = 0;
	m_ArtTimeCode.ProtVerLo = artnet::PROTOCOL_REVISION;
	m_ArtTimeCode.Filler1 = 0;
	m_ArtTimeCode.Filler2 = 0;
#endif
}

ArtNetNode::~ArtNetNode() {
	DEBUG_ENTRY

	DEBUG_EXIT
}

void ArtNetNode::Start() {
	if (artnet::VERSION > 3) {
		assert(m_pArtNet4Handler != nullptr);
	}

#if (LIGHTSET_PORTS > 0)
	assert(m_pLightSet != nullptr);
#endif	

#if defined (ARTNET_HAVE_TRIGGER)
	assert(m_pArtNetTrigger != nullptr);
#endif	

#if defined (ARTNET_HAVE_TIMECODE)
	assert(m_pArtNetTimeCode != nullptr);
#endif

	m_Node.Status2 = static_cast<uint8_t>((m_Node.Status2 & ~(artnet::Status2::IP_DHCP)) | (Network::Get()->IsDhcpUsed() ? artnet::Status2::IP_DHCP : artnet::Status2::IP_MANUALY));
	m_Node.Status2 = static_cast<uint8_t>((m_Node.Status2 & ~(artnet::Status2::DHCP_CAPABLE)) | (Network::Get()->IsDhcpCapable() ? artnet::Status2::DHCP_CAPABLE : 0));

	FillPollReply();
#if defined ( ARTNET_ENABLE_SENDDIAG )
	FillDiagData();
#endif

	m_nHandle = Network::Get()->Begin(artnet::UDP_PORT);
	assert(m_nHandle != -1);

	m_State.status = Status::ON;

#if defined (ARTNET_HAVE_DMXIN)
	for (uint32_t i = 0; i < artnetnode::MAX_PORTS; i++) {
		if (m_InputPort[i].genericPort.bIsEnabled) {
			artnet::dmx_start(i);
		}
	}
#endif

	SendPollRelply(false);	// send a reply on startup

#if defined (RDM_CONTROLLER) || defined (RDM_RESPONDER)
	if (m_pArtNetRdm != nullptr) {
		for (uint32_t nPortIndex = 0; nPortIndex < artnetnode::MAX_PORTS; nPortIndex++) {
			/* An Output Gateway will send the ArtTodData packet in the following circumstances:
			 * - Upon power on or decice reset. 
			 * - In response to an ArtTodRequest if the Port-Address matches.
			 * - In response to an ArtTodControl if the Port-Address matches.
			 * - When their ToD changes due to the addition or deletion of a UID.
			 * - At the end of full RDM discovery.
			 */
			const auto isRdmDisabled = ((m_OutputPort[nPortIndex].GoodOutputB & artnet::GoodOutputB::RDM_DISABLED) == artnet::GoodOutputB::RDM_DISABLED);
			if (!isRdmDisabled && m_OutputPort[nPortIndex].genericPort.bIsEnabled) {
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
#endif

	Hardware::Get()->SetMode(hardware::ledblink::Mode::NORMAL);
	hal::panel_led_on(hal::panelled::ARTNET);
}

void ArtNetNode::Stop() {
	DEBUG_ENTRY

	for (uint32_t nPortIndex = 0; nPortIndex < artnetnode::MAX_PORTS; nPortIndex++) {
		if (m_OutputPort[nPortIndex].protocol == artnet::PortProtocol::ARTNET) {
			if (m_pLightSet != nullptr) {
				m_pLightSet->Stop(nPortIndex);
			}
			lightset::Data::ClearLength(nPortIndex);
			m_OutputPort[nPortIndex].IsTransmitting = false;
		}
	}

#if defined (ARTNET_HAVE_DMXIN)
	for (uint32_t i = 0; i < artnetnode::MAX_PORTS; i++) {
		if (m_InputPort[i].genericPort.bIsEnabled) {
			artnet::dmx_stop(i);
		}
	}
#endif

	Hardware::Get()->SetMode(hardware::ledblink::Mode::OFF_OFF);
	hal::panel_led_off(hal::panelled::ARTNET);

	m_Node.Status1 = static_cast<uint8_t>((m_Node.Status1 & ~artnet::Status1::INDICATOR_MASK) | artnet::Status1::INDICATOR_MUTE_MODE);
	m_State.status = Status::STANDBY;

	DEBUG_EXIT
}

void ArtNetNode::GetShortNameDefault(char *pShortName) {
	snprintf(pShortName, artnet::SHORT_NAME_LENGTH - 1, IPSTR, IP2STR(Network::Get()->GetIp()));
}

void ArtNetNode::SetShortName(const char *pShortName) {
	DEBUG_ENTRY

	if (pShortName == nullptr) {
		GetShortNameDefault(m_Node.ShortName);
	} else {
		strncpy(m_Node.ShortName, pShortName, artnet::SHORT_NAME_LENGTH - 1);
	}

	m_Node.ShortName[artnet::SHORT_NAME_LENGTH - 1] = '\0';

	memcpy(m_PollReply.ShortName, m_Node.ShortName, artnet::SHORT_NAME_LENGTH);

	if (m_State.status == Status::ON) {
		if (m_pArtNetStore != nullptr) {
			m_pArtNetStore->SaveShortName(m_Node.ShortName);
		}

		artnet::display_shortname(m_Node.ShortName);
	}

	DEBUG_PUTS(m_Node.ShortName);
	DEBUG_EXIT
}

void ArtNetNode::GetLongNameDefault(char *pLongName) {
	uint8_t nBoardNameLength;
	const auto *const pBoardName = Hardware::Get()->GetBoardName(nBoardNameLength);
	const auto *const pWebsiteUrl = Hardware::Get()->GetWebsiteUrl();
	snprintf(pLongName, artnet::LONG_NAME_LENGTH - 1, "%s %s %d %s", pBoardName, artnet::NODE_ID, artnet::VERSION, pWebsiteUrl);
}

void ArtNetNode::SetLongName(const char *pLongName) {
	DEBUG_ENTRY

	if (pLongName == nullptr) {
		GetLongNameDefault(m_Node.LongName);
	} else {
		strncpy(m_Node.LongName, pLongName, artnet::LONG_NAME_LENGTH - 1);
	}

	m_Node.LongName[artnet::LONG_NAME_LENGTH - 1] = '\0';

	memcpy(m_PollReply.LongName, m_Node.LongName, artnet::LONG_NAME_LENGTH);

	if (m_State.status == Status::ON) {
		if (m_pArtNetStore != nullptr) {
			m_pArtNetStore->SaveLongName(m_Node.LongName);
		}

		artnet::display_longname(m_Node.LongName);
	}

	DEBUG_PUTS(m_Node.LongName);
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

void ArtNetNode::GetType(const uint32_t nBytesReceived, enum TOpCodes& OpCode) {
	const auto *const pPacket = reinterpret_cast<char *>(m_pReceiveBuffer);

	if (nBytesReceived < ARTNET_MIN_HEADER_SIZE) {
		OpCode = OP_NOT_DEFINED;
		return;
	}

	if ((pPacket[10] != 0) || (pPacket[11] != artnet::PROTOCOL_REVISION)) {
		OpCode = OP_NOT_DEFINED;
		return;
	}

	if (memcmp(pPacket, artnet::NODE_ID, 8) == 0) {
		OpCode = static_cast<TOpCodes>((static_cast<uint16_t>(pPacket[9] << 8)) + pPacket[8]);
	} else {
		OpCode = OP_NOT_DEFINED;
	}
}

void ArtNetNode::Run() {
	uint16_t nForeignPort;

	const auto nBytesReceived = Network::Get()->RecvFrom(m_nHandle, const_cast<const void**>(reinterpret_cast<void **>(&m_pReceiveBuffer)), &m_nIpAddressFrom, &nForeignPort);

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


#if defined (ARTNET_HAVE_DMXIN)
		HandleDmxIn();
#endif

#if defined (RDM_CONTROLLER)
		if (m_pArtNetRdm != nullptr) {
			HandleRdmIn();
		}
#endif

#if (LIGHTSET_PORTS > 0)
		if ((((m_Node.Status1 & artnet::Status1::INDICATOR_MASK) == artnet::Status1::INDICATOR_NORMAL_MODE)) && (Hardware::Get()->GetMode() != hardware::ledblink::Mode::FAST)) {
			if (artnet::VERSION > 3) {
				if (m_State.nReceivingDmx != 0) {
					m_pArtNet4Handler->SetLedBlinkMode(hardware::ledblink::Mode::DATA);
				} else {
					m_pArtNet4Handler->SetLedBlinkMode(hardware::ledblink::Mode::NORMAL);
				}
			} else {
				if (m_State.nReceivingDmx != 0) {
					Hardware::Get()->SetMode(hardware::ledblink::Mode::DATA);
				} else {
					Hardware::Get()->SetMode(hardware::ledblink::Mode::NORMAL);
				}
			}
		}
#endif		

		return;
	}

	m_nPreviousPacketMillis = m_nCurrentPacketMillis;

	if (m_State.IsSynchronousMode) {
		if (m_nCurrentPacketMillis - m_State.nArtSyncMillis >= (4 * 1000)) {
			m_State.IsSynchronousMode = false;
		}
	}

	enum TOpCodes OpCode;
	GetType(nBytesReceived, OpCode);

	switch (OpCode) {
#if (LIGHTSET_PORTS > 0)		
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
#endif		
	case OP_ADDRESS:
		HandleAddress();
		break;
#if defined (ARTNET_HAVE_TIMECODE)		
	case OP_TIMECODE:
		if (m_pArtNetTimeCode != nullptr) {
			HandleTimeCode();
		}
		break;
#endif		
	case OP_TIMESYNC:
		HandleTimeSync();
		break;
#if defined (RDM_CONTROLLER) || defined (RDM_RESPONDER)
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
#endif
	case OP_IPPROG:
		HandleIpProg();
		break;
#if defined (ARTNET_HAVE_TRIGGER)		
	case OP_TRIGGER:
		HandleTrigger();
		break;
#endif
	case OP_POLL:
		HandlePoll();
		break;
	default:
		// ArtNet but OpCode is not implemented
		// Just skip ... no error
		break;
	}

#if defined (ARTNET_HAVE_DMXIN)
	HandleDmxIn();
#endif

#if defined (RDM_CONTROLLER)
	if (m_pArtNetRdm != nullptr) {
		HandleRdmIn();
	}
#endif

#if (LIGHTSET_PORTS > 0)
	if ((((m_Node.Status1 & artnet::Status1::INDICATOR_MASK) == artnet::Status1::INDICATOR_NORMAL_MODE)) && (Hardware::Get()->GetMode() != hardware::ledblink::Mode::FAST)) {
		if (artnet::VERSION > 3) {
			if (m_State.nReceivingDmx != 0) {
				m_pArtNet4Handler->SetLedBlinkMode(hardware::ledblink::Mode::DATA);
			} else {
				m_pArtNet4Handler->SetLedBlinkMode(hardware::ledblink::Mode::NORMAL);
			}
		} else {
			if (m_State.nReceivingDmx != 0) {
				Hardware::Get()->SetMode(hardware::ledblink::Mode::DATA);
			} else {
				Hardware::Get()->SetMode(hardware::ledblink::Mode::NORMAL);
			}
		}
	}
#endif	

	hal::panel_led_on(hal::panelled::ARTNET);
}
