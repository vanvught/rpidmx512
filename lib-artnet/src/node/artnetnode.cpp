/**
 * @file artnetnode.cpp
 *
 */
/**
 * Art-Net Designed by and Copyright Artistic Licence Holdings Ltd.
 */
/* Copyright (C) 2016-2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include "artnetstore.h"

#if defined (ARTNET_HAVE_DMXIN)
# include "dmx.h"
#endif

#if (ARTNET_VERSION >= 4)
# include "e131.h"
#endif

#if defined (ARTNET_SHOWFILE)
namespace showfile {
void record(const struct artnet::ArtDmx *pArtDmx, const uint32_t nMillis);
void record(const struct artnet::ArtSync *pArtSync, const uint32_t nMillis);
}  // namespace showfile
#endif

#include "lightset.h"
#include "lightsetdata.h"

#include "hardware.h"
#include "network.h"

#include "panel_led.h"

#include "artnetnode_internal.h"

#include "debug.h"

static constexpr auto ARTNET_MIN_HEADER_SIZE = 12;

ArtNetNode *ArtNetNode::s_pThis;

ArtNetNode::ArtNetNode() {
	DEBUG_ENTRY

	assert(s_pThis == nullptr);
	s_pThis = this;

	DEBUG_PRINTF("MAX_PORTS=%u", artnetnode::MAX_PORTS);

	memset(&m_ArtPollReply, 0, sizeof(struct artnet::ArtPollReply));
	memcpy(m_ArtPollReply.Id, artnet::NODE_ID, sizeof(m_ArtPollReply.Id));
	m_ArtPollReply.OpCode = static_cast<uint16_t>(artnet::OpCodes::OP_POLLREPLY);
	m_ArtPollReply.Port = artnet::UDP_PORT;
	m_ArtPollReply.VersInfoH = ArtNetConst::VERSION[0];
	m_ArtPollReply.VersInfoL = ArtNetConst::VERSION[1];
	m_ArtPollReply.OemHi = ArtNetConst::OEM_ID[0];
	m_ArtPollReply.Oem = ArtNetConst::OEM_ID[1];
	m_ArtPollReply.EstaMan[0] = ArtNetConst::ESTA_ID[1];
	m_ArtPollReply.EstaMan[1] = ArtNetConst::ESTA_ID[0];
	Network::Get()->MacAddressCopyTo(m_ArtPollReply.MAC);
#if (ARTNET_VERSION >= 4)
	m_ArtPollReply.AcnPriority = e131::priority::DEFAULT;
#endif

	memset(&m_State, 0, sizeof(struct artnetnode::State));
	m_State.reportCode = artnet::ReportCode::RCPOWEROK;
	m_State.status = artnet::Status::STANDBY;
	// The device should wait for a random delay of up to 1s before sending the reply.
	m_State.ArtPollReplyDelayMillis = (m_ArtPollReply.MAC[5] | (static_cast<uint32_t>(m_ArtPollReply.MAC[4]) << 8)) % 1000;

	SetLongName(nullptr);	// Set default long name

	memset(&m_Node, 0, sizeof(struct artnetnode::Node));
	m_Node.IPAddressTimeCode = Network::Get()->GetBroadcastIp();

	for (auto& port : m_Node.Port) {
		port.direction = lightset::PortDir::DISABLE;
	}

	for (uint32_t nPortIndex = 0; nPortIndex < artnetnode::MAX_PORTS; nPortIndex++) {
		SetShortName(nPortIndex, nullptr);	// Set default port label
	}

	for (uint32_t nPortIndex = 0; nPortIndex < artnetnode::MAX_PORTS; nPortIndex++) {
		memset(&m_OutputPort[nPortIndex], 0, sizeof(struct artnetnode::OutputPort));
		m_OutputPort[nPortIndex].SourceA.nPhysical = 0x100;
		m_OutputPort[nPortIndex].SourceB.nPhysical = 0x100;
		m_OutputPort[nPortIndex].GoodOutputB = artnet::GoodOutputB::RDM_DISABLED | artnet::GoodOutputB::DISCOVERY_NOT_RUNNING;
		memset(&m_InputPort[nPortIndex], 0, sizeof(struct artnetnode::InputPort));
		m_InputPort[nPortIndex].nDestinationIp = Network::Get()->GetBroadcastIp();
	}

#if defined (ARTNET_HAVE_DMXIN)
	memcpy(m_ArtDmx.Id, artnet::NODE_ID, sizeof(m_ArtPollReply.Id));
	m_ArtDmx.OpCode = static_cast<uint16_t>(artnet::OpCodes::OP_DMX);
	m_ArtDmx.ProtVerHi = 0;
	m_ArtDmx.ProtVerLo = artnet::PROTOCOL_REVISION;
#endif

#if defined (ARTNET_HAVE_TIMECODE)
	memcpy(m_ArtTimeCode.Id, artnet::NODE_ID, sizeof(m_ArtPollReply.Id));
	m_ArtTimeCode.OpCode = static_cast<uint16_t>(artnet::OpCodes::OP_TIMECODE);
	m_ArtTimeCode.ProtVerHi = 0;
	m_ArtTimeCode.ProtVerLo = artnet::PROTOCOL_REVISION;
	m_ArtTimeCode.Filler1 = 0;
	m_ArtTimeCode.Filler2 = 0;
#endif

#if defined (ARTNET_ENABLE_SENDDIAG)
	memset(&m_DiagData, 0, sizeof(struct artnet::ArtDiagData));
	memcpy(m_DiagData.Id, artnet::NODE_ID, sizeof(m_DiagData.Id));
	m_DiagData.OpCode = static_cast<uint16_t>(artnet::OpCodes::OP_DIAGDATA);
	m_DiagData.ProtVerLo = artnet::PROTOCOL_REVISION;
#endif

	DEBUG_EXIT
}

ArtNetNode::~ArtNetNode() {
	DEBUG_ENTRY

	DEBUG_EXIT
}

void ArtNetNode::Start() {
	DEBUG_ENTRY

#if (LIGHTSET_PORTS > 0)
	assert(m_pLightSet != nullptr);
#endif	

#if defined (ARTNET_HAVE_TRIGGER)
	assert(m_pArtNetTrigger != nullptr);
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
#if defined (OUTPUT_HAVE_STYLESWITCH)
	m_ArtPollReply.Status2 |= artnet::Status2::OUTPUT_STYLE_SWITCH;
#endif
#if defined (RDM_CONTROLLER) || defined (RDM_RESPONDER)
	m_ArtPollReply.Status2 |= artnet::Status2::RDM_SWITCH;
#endif
	/*
	 * Status 3
	 */
	m_ArtPollReply.Status3 = artnet::Status3::NETWORKLOSS_LAST_STATE | artnet::Status3::FAILSAFE_CONTROL | artnet::Status3::SUPPORTS_BACKGROUNDDISCOVERY;
#if defined (ARTNET_HAVE_DMXIN)
	m_ArtPollReply.Status3 |= artnet::Status3::OUTPUT_SWITCH;
#endif

	m_nHandle = Network::Get()->Begin(artnet::UDP_PORT);
	assert(m_nHandle != -1);

#if defined (ARTNET_HAVE_DMXIN)
	for (uint32_t nPortIndex = 0; nPortIndex < artnetnode::MAX_PORTS; nPortIndex++) {
		if ((m_Node.Port[nPortIndex].protocol == artnet::PortProtocol::ARTNET)
		 && (m_Node.Port[nPortIndex].direction == lightset::PortDir::INPUT)) {
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
		for (uint32_t nPortIndex = 0; nPortIndex < artnetnode::MAX_PORTS; nPortIndex++) {
			if (m_Node.Port[nPortIndex].direction == lightset::PortDir::OUTPUT) {
				SetOutputStyle(nPortIndex, GetOutputStyle(nPortIndex));
			}
		}
	}
#endif

#if defined (RDM_CONTROLLER)
	if (m_State.rdm.IsEnabled) {
		m_State.rdm.IsDiscoveryRunning = true;

		for (uint32_t nPortIndex = 0; nPortIndex < artnetnode::MAX_PORTS; nPortIndex++) {
			if (m_Node.Port[nPortIndex].direction == lightset::PortDir::INPUT) {
				SendTodRequest(nPortIndex);
			}
		}
	}
#endif

#if (ARTNET_VERSION >= 4)
	E131Bridge::Start();
#endif

	m_State.status = artnet::Status::ON;
	Hardware::Get()->SetMode(hardware::ledblink::Mode::NORMAL);

	DEBUG_EXIT
}

void ArtNetNode::Stop() {
	DEBUG_ENTRY

#if (ARTNET_VERSION >= 4)
	E131Bridge::Stop();
#endif

	for (uint32_t nPortIndex = 0; nPortIndex < artnetnode::MAX_PORTS; nPortIndex++) {
		if (m_Node.Port[nPortIndex].protocol == artnet::PortProtocol::ARTNET) {
			if (m_pLightSet != nullptr) {
				m_pLightSet->Stop(nPortIndex);
			}
			lightset::Data::ClearLength(nPortIndex);
			m_OutputPort[nPortIndex].IsTransmitting = false;
		}
	}

#if defined (ARTNET_HAVE_DMXIN)
	for (uint32_t nPortIndex = 0; nPortIndex < artnetnode::MAX_PORTS; nPortIndex++) {
		if (m_Node.Port[nPortIndex].direction == lightset::PortDir::INPUT) {
			Dmx::Get()->SetPortDirection(nPortIndex, dmx::PortDirection::INP, false);
		}
	}
#endif

	Hardware::Get()->SetMode(hardware::ledblink::Mode::OFF_OFF);
	hal::panel_led_off(hal::panelled::ARTNET);

	m_ArtPollReply.Status1 = static_cast<uint8_t>((m_ArtPollReply.Status1 & ~artnet::Status1::INDICATOR_MASK) | artnet::Status1::INDICATOR_MUTE_MODE);
	m_State.status = artnet::Status::STANDBY;

	DEBUG_EXIT
}

void ArtNetNode::SetShortName(const uint32_t nPortIndex, const char *pShortName) {
	DEBUG_ENTRY
	DEBUG_PRINTF("nPortIndex=%u, pShortName=%s", nPortIndex, pShortName == nullptr ? "nullptr" : pShortName);
	assert(nPortIndex < artnetnode::MAX_PORTS);

	if (pShortName == nullptr) {
		lightset::node::get_short_name_default(nPortIndex, m_Node.Port[nPortIndex].ShortName);
	} else {
		strncpy(m_Node.Port[nPortIndex].ShortName, pShortName, artnet::SHORT_NAME_LENGTH - 1);
	}

	m_Node.Port[nPortIndex].ShortName[artnet::SHORT_NAME_LENGTH - 1] = '\0';

	if (m_State.status == artnet::Status::ON) {
		ArtNetStore::SaveShortName(nPortIndex, m_Node.Port[nPortIndex].ShortName);
	}

	DEBUG_PUTS(m_Node.Port[nPortIndex].ShortName);
	DEBUG_EXIT
}

void ArtNetNode::GetLongNameDefault(char *pLongName) {
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

void ArtNetNode::SetLongName(const char *pLongName) {
	DEBUG_ENTRY

	if (pLongName == nullptr) {
		GetLongNameDefault(reinterpret_cast<char *>(m_ArtPollReply.LongName));
	} else {
		strncpy(reinterpret_cast<char *>(m_ArtPollReply.LongName), pLongName, artnet::LONG_NAME_LENGTH - 1);
	}

	m_ArtPollReply.LongName[artnet::LONG_NAME_LENGTH - 1] = '\0';

	if (m_State.status == artnet::Status::ON) {
		ArtNetStore::SaveLongName(reinterpret_cast<char *>(m_ArtPollReply.LongName));
		artnet::display_longname(reinterpret_cast<char *>(m_ArtPollReply.LongName));
	}

	DEBUG_PUTS(reinterpret_cast<char *>(m_ArtPollReply.LongName));
	DEBUG_EXIT
}

#if defined (OUTPUT_HAVE_STYLESWITCH)
void ArtNetNode::SetOutputStyle(const uint32_t nPortIndex, lightset::OutputStyle outputStyle) {
	assert(nPortIndex < artnetnode::MAX_PORTS);

	if ((outputStyle == GetOutputStyle(nPortIndex)) && (m_State.status == artnet::Status::ON)) {
		return;
	}

	if (m_pLightSet != nullptr) {
		m_pLightSet->SetOutputStyle(nPortIndex, outputStyle);
		outputStyle = m_pLightSet->GetOutputStyle(nPortIndex);
	}

	if (outputStyle == lightset::OutputStyle::CONSTANT) {
		m_OutputPort[nPortIndex].GoodOutputB |= artnet::GoodOutputB::STYLE_CONSTANT;
	} else {
		m_OutputPort[nPortIndex].GoodOutputB &= static_cast<uint8_t>(~artnet::GoodOutputB::STYLE_CONSTANT);
	}

	m_State.IsSynchronousMode = false;

	if (m_State.status == artnet::Status::ON) {
		ArtNetStore::SaveOutputStyle(nPortIndex, outputStyle);
		artnet::display_outputstyle(nPortIndex, outputStyle);
	}
}

lightset::OutputStyle ArtNetNode::GetOutputStyle(const uint32_t nPortIndex) const {
	assert(nPortIndex < artnetnode::MAX_PORTS);

	const auto isStyleConstant = (m_OutputPort[nPortIndex].GoodOutputB & artnet::GoodOutputB::STYLE_CONSTANT) == artnet::GoodOutputB::STYLE_CONSTANT;
	return isStyleConstant ? lightset::OutputStyle::CONSTANT : lightset::OutputStyle::DELTA;
}
#endif

void ArtNetNode::SetNetworkDataLossCondition() {
	m_State.IsMergeMode = false;
	m_State.IsSynchronousMode = false;

	uint32_t nIpCount = 0;

	for (uint32_t nPortIndex = 0; nPortIndex < artnetnode::MAX_PORTS; nPortIndex++) {
#if defined (ARTNET_HAVE_DMXIN)
		if (m_Node.Port[nPortIndex].bLocalMerge) {
			continue;
		}
#endif
		nIpCount += (m_OutputPort[nPortIndex].SourceA.nIp + m_OutputPort[nPortIndex].SourceB.nIp);
		if (nIpCount != 0) {
			break;
		}
	}

	if (nIpCount == 0) {
		return;
	}

	const auto networkloss = (m_ArtPollReply.Status3 & artnet::Status3::NETWORKLOSS_MASK);

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
		m_OutputPort[i].SourceA.nIp = 0;
		m_OutputPort[i].SourceB.nIp = 0;
		lightset::Data::ClearLength(i);
	}

#if defined (ARTNET_HAVE_DMXIN)
	SetLocalMerging();
#endif
}

static artnet::OpCodes get_op_code(const uint32_t nBytesReceived, const uint8_t *pBuffer) {
	if (nBytesReceived < ARTNET_MIN_HEADER_SIZE) {
		return artnet::OpCodes::OP_NOT_DEFINED;
	}

	if ((pBuffer[10] != 0) || (pBuffer[11] != artnet::PROTOCOL_REVISION)) {
		return artnet::OpCodes::OP_NOT_DEFINED;
	}

	if (memcmp(pBuffer, artnet::NODE_ID, 8) == 0) {
		return static_cast<artnet::OpCodes>((static_cast<uint16_t>(pBuffer[9] << 8)) + pBuffer[8]);
	}

	return artnet::OpCodes::OP_NOT_DEFINED;
}

void ArtNetNode::Process(const uint32_t nBytesReceived) {
	if (__builtin_expect((nBytesReceived == 0), 1)) {
		const auto nDeltaMillis = m_nCurrentPacketMillis - m_nPreviousPacketMillis;

		if (nDeltaMillis >= artnet::NETWORK_DATA_LOSS_TIMEOUT * 1000) {
			SetNetworkDataLossCondition();
			hal::panel_led_off(hal::panelled::ARTNET);
		}

		if (nDeltaMillis >= (1U * 1000U)) {
			m_State.nReceivingDmx &= static_cast<uint8_t>(~(1U << static_cast<uint8_t>(lightset::PortDir::OUTPUT)));
		}

#if defined (ARTNET_HAVE_DMXIN)
		HandleDmxIn();
#endif

#if defined (RDM_CONTROLLER)
		if (m_State.rdm.IsEnabled) {
			HandleRdmIn();
		}
#endif

#if (LIGHTSET_PORTS > 0)
		if ((((m_ArtPollReply.Status1 & artnet::Status1::INDICATOR_MASK) == artnet::Status1::INDICATOR_NORMAL_MODE)) && (Hardware::Get()->GetMode() != hardware::ledblink::Mode::FAST)) {
#if (ARTNET_VERSION >= 4)
			if (m_State.nReceivingDmx != 0) {
				SetLedBlinkMode4(hardware::ledblink::Mode::DATA);
			} else {
				SetLedBlinkMode4(hardware::ledblink::Mode::NORMAL);
			}
#else
			if (m_State.nReceivingDmx != 0) {
				Hardware::Get()->SetMode(hardware::ledblink::Mode::DATA);
			} else {
				Hardware::Get()->SetMode(hardware::ledblink::Mode::NORMAL);
			}
#endif
		}
#endif

		for (auto& entry : m_State.ArtPollReplyQueue) {
			if (entry.ArtPollMillis != 0) {
				if ((m_nCurrentPacketMillis - entry.ArtPollMillis) > m_State.ArtPollReplyDelayMillis) {
					entry.ArtPollMillis = 0;
					SendPollRelply(0, entry.ArtPollReplyIpAddress, &entry);
				}
			}
		}

		return;
	}

	m_nPreviousPacketMillis = m_nCurrentPacketMillis;

	if (m_State.IsSynchronousMode) {
		if (m_nCurrentPacketMillis - m_State.ArtSyncMillis >= (4 * 1000)) {
			m_State.IsSynchronousMode = false;
		}
	}

	switch (get_op_code(nBytesReceived, m_pReceiveBuffer)) {
#if (LIGHTSET_PORTS > 0)		
	case artnet::OpCodes::OP_DMX:
		if (m_pLightSet != nullptr) {
			HandleDmx();
			m_State.ArtDmxIpAddress = m_nIpAddressFrom;
#if defined (ARTNET_SHOWFILE)
			if (m_State.DoRecord) {
				showfile::record(reinterpret_cast<const artnet::ArtDmx *>(m_pReceiveBuffer), m_nCurrentPacketMillis);
			}
#endif
		}
		break;
	case artnet::OpCodes::OP_SYNC:
		if (m_pLightSet != nullptr) {
			/*
			 * In order to allow for multiple controllers on a network,
			 * a node shall compare the source IP of the ArtSync to the source IP
			 * of the most recent ArtDmx packet.
			 * The ArtSync shall be ignored if the IP addresses do not match.
			 */
			/*
			 * When a port is merging multiple streams of ArtDmx from different IP addresses,
			 * ArtSync packets shall be ignored.
			 */
			if ((m_State.ArtDmxIpAddress == m_nIpAddressFrom) && (!m_State.IsMergeMode)) {
				m_State.ArtSyncMillis = Hardware::Get()->Millis();
				HandleSync();
			}
#if defined (ARTNET_SHOWFILE)
			if (m_State.DoRecord) {
				showfile::record(reinterpret_cast<const artnet::ArtSync *>(m_pReceiveBuffer), m_nCurrentPacketMillis);
			}
#endif
		}
		break;
#endif		
	case artnet::OpCodes::OP_ADDRESS:
		HandleAddress();
		break;
#if defined (ARTNET_HAVE_TIMECODE)		
	case artnet::OpCodes::OP_TIMECODE:
		if (m_pArtNetTimeCode != nullptr) {
			const auto *const pArtTimeCode = reinterpret_cast<artnet::ArtTimeCode *>(m_pReceiveBuffer);
			m_pArtNetTimeCode->Handler(reinterpret_cast<const struct artnet::TimeCode *>(&pArtTimeCode->Frames));
		}
		break;
#endif		
	case artnet::OpCodes::OP_TIMESYNC:
		HandleTimeSync();
		break;
#if defined (RDM_CONTROLLER) || defined (RDM_RESPONDER)
	case artnet::OpCodes::OP_TODREQUEST:
		if (m_State.rdm.IsEnabled) {
			HandleTodRequest();
		}
		break;
	case artnet::OpCodes::OP_TODDATA:
		if (m_State.rdm.IsEnabled) {
			HandleTodData();
		}
		break;
	case artnet::OpCodes::OP_TODCONTROL:
		if (m_State.rdm.IsEnabled) {
			HandleTodControl();
		}
		break;
	case artnet::OpCodes::OP_RDM:
		if (m_State.rdm.IsEnabled) {
			HandleRdm();
		}
		break;
	case artnet::OpCodes::OP_RDMSUB:
		if (m_State.rdm.IsEnabled) {
			HandleRdmSub();
		}
		break;
#endif
	case artnet::OpCodes::OP_IPPROG:
		HandleIpProg();
		break;
#if defined (ARTNET_HAVE_TRIGGER)		
	case artnet::OpCodes::OP_TRIGGER:
		HandleTrigger();
		break;
#endif
#if defined (ARTNET_HAVE_DMXIN)
	case artnet::OpCodes::OP_INPUT:
		HandleInput();
		break;
#endif
	case artnet::OpCodes::OP_POLL:
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
	if (m_State.rdm.IsEnabled) {
		HandleRdmIn();
	}
#endif

#if (LIGHTSET_PORTS > 0)
	if ((((m_ArtPollReply.Status1 & artnet::Status1::INDICATOR_MASK) == artnet::Status1::INDICATOR_NORMAL_MODE)) && (Hardware::Get()->GetMode() != hardware::ledblink::Mode::FAST)) {
#if (ARTNET_VERSION >= 4)
		if (m_State.nReceivingDmx != 0) {
			SetLedBlinkMode4(hardware::ledblink::Mode::DATA);
		} else {
			SetLedBlinkMode4(hardware::ledblink::Mode::NORMAL);
		}
#else
		if (m_State.nReceivingDmx != 0) {
			Hardware::Get()->SetMode(hardware::ledblink::Mode::DATA);
		} else {
			Hardware::Get()->SetMode(hardware::ledblink::Mode::NORMAL);
		}
#endif
	}
#endif	

	hal::panel_led_on(hal::panelled::ARTNET);

	for (auto& entry : m_State.ArtPollReplyQueue) {
		if (entry.ArtPollMillis != 0) {
			if ((m_nCurrentPacketMillis - entry.ArtPollMillis) > m_State.ArtPollReplyDelayMillis) {
				entry.ArtPollMillis = 0;
				SendPollRelply(0, entry.ArtPollReplyIpAddress, &entry);
			}
		}
	}
}
