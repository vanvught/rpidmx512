/**
 * @file artnetnodehandlepoll.cpp
 *
 */
/**
 * Art-Net Designed by and Copyright Artistic Licence Holdings Ltd.
 */
/* Copyright (C) 2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <algorithm>
#include <cassert>

#include "artnetnode.h"
#include "artnet.h"
#include "artnetconst.h"

#include "artnetnode_internal.h"

#include "hardware.h"
#include "network.h"

using namespace artnet;

union uip {
	uint32_t u32;
	uint8_t u8[4];
} static ip;

void ArtNetNode::FillPollReply() {
	memset(&m_PollReply, 0, sizeof(struct TArtPollReply));

	memcpy(m_PollReply.Id, artnet::NODE_ID, sizeof m_PollReply.Id);

	m_PollReply.OpCode = OP_POLLREPLY;

	ip.u32 = m_Node.IPAddressLocal;
	memcpy(m_PollReply.IPAddress, ip.u8, sizeof m_PollReply.IPAddress);

	m_PollReply.Port = ArtNet::UDP_PORT;

	m_PollReply.VersInfoH = ArtNetConst::VERSION[0];
	m_PollReply.VersInfoL = ArtNetConst::VERSION[1];

	m_PollReply.OemHi = m_Node.Oem[0];
	m_PollReply.Oem = m_Node.Oem[1];

	m_PollReply.Status1 = m_Node.Status1;

	m_PollReply.EstaMan[0] = ArtNetConst::ESTA_ID[1];
	m_PollReply.EstaMan[1] = ArtNetConst::ESTA_ID[0];

	memcpy(m_PollReply.ShortName, m_Node.ShortName, sizeof m_PollReply.ShortName);
	memcpy(m_PollReply.LongName, m_Node.LongName, sizeof m_PollReply.LongName);

	m_PollReply.Style = ARTNET_ST_NODE;

	memcpy(m_PollReply.MAC, m_Node.MACAddressLocal, sizeof m_PollReply.MAC);

	if (m_nVersion > 3) {
		memcpy(m_PollReply.BindIp, ip.u8, sizeof m_PollReply.BindIp);
	}

	m_PollReply.Status2 = m_Node.Status2;
	m_PollReply.Status3 = m_Node.Status3;

	m_PollReply.NumPortsLo = 4; // Default

	memcpy(m_PollReply.DefaultUidResponder, m_Node.DefaultUidResponder, sizeof m_PollReply.DefaultUidResponder);
}

void ArtNetNode::SendPollRelply(bool bResponse) {
	if (!bResponse && m_State.status == ARTNET_ON) {
		m_State.ArtPollReplyCount++;
	}

	m_PollReply.Status1 = m_Node.Status1;

	for (uint8_t nPage = 0; nPage < m_nPages; nPage++) {

		m_PollReply.NetSwitch = m_Node.NetSwitch[nPage];
		m_PollReply.SubSwitch = m_Node.SubSwitch[nPage];
		m_PollReply.BindIndex = static_cast<uint8_t>(nPage + 1);

		const auto nPortIndexStart = static_cast<uint8_t>(nPage * ArtNet::PORTS);

		uint32_t NumPortsLo = 0;

		for (uint8_t nPortIndex = nPortIndexStart; nPortIndex < (nPortIndexStart + ArtNet::PORTS); nPortIndex++) {
			uint8_t nStatus = m_OutputPorts[nPortIndex].port.nStatus;

			if (m_OutputPorts[nPortIndex].tPortProtocol == PortProtocol::ARTNET) {
				nStatus &= static_cast<uint8_t>(~GO_DATA_IS_BEING_TRANSMITTED);

				if (m_OutputPorts[nPortIndex].ipA != 0) {
					if ((m_nCurrentPacketMillis - m_OutputPorts[nPortIndex].nMillisA) < 1000) {
						nStatus |= GO_DATA_IS_BEING_TRANSMITTED;
					}
				}

				if (m_OutputPorts[nPortIndex].ipB != 0) {
					if ((m_nCurrentPacketMillis - m_OutputPorts[nPortIndex].nMillisB) < 1000) {
						nStatus |= GO_DATA_IS_BEING_TRANSMITTED;
					}
				}
			} else {
				if (m_pArtNet4Handler != nullptr) {
					const auto nMask = GO_OUTPUT_IS_MERGING | GO_DATA_IS_BEING_TRANSMITTED | GO_OUTPUT_IS_SACN;

					nStatus &= static_cast<uint8_t>(~nMask);
					nStatus = static_cast<uint8_t>(nStatus | (m_pArtNet4Handler->GetStatus(nPortIndex) & nMask));

					if ((nStatus & GO_OUTPUT_IS_SACN) == 0) {
						m_OutputPorts[nPortIndex].tPortProtocol = PortProtocol::ARTNET;
					}
				}
			}

			m_OutputPorts[nPortIndex].port.nStatus = nStatus;

			if (m_OutputPorts[nPortIndex].bIsEnabled) {
				m_PollReply.PortTypes[nPortIndex - nPortIndexStart] = ARTNET_ENABLE_OUTPUT | ARTNET_PORT_DMX;
				NumPortsLo++;
			} else {
				m_PollReply.PortTypes[nPortIndex - nPortIndexStart] = 0;
			}

			m_PollReply.GoodOutput[nPortIndex - nPortIndexStart] = m_OutputPorts[nPortIndex].port.nStatus;
			m_PollReply.SwOut[nPortIndex - nPortIndexStart] = m_OutputPorts[nPortIndex].port.nDefaultAddress;

			if (nPortIndex < ArtNet::PORTS) {
				if (m_InputPorts[nPortIndex].bIsEnabled) {
					m_PollReply.PortTypes[nPortIndex - nPortIndexStart] |= ARTNET_ENABLE_INPUT | ARTNET_PORT_DMX;
					NumPortsLo++;
				}

				m_PollReply.GoodInput[nPortIndex - nPortIndexStart] = m_InputPorts[nPortIndex].port.nStatus;
				m_PollReply.SwIn[nPortIndex - nPortIndexStart] = m_InputPorts[nPortIndex].port.nDefaultAddress;
			}

		}

		m_PollReply.NumPortsLo = static_cast<uint8_t>(NumPortsLo);
		assert(NumPortsLo <= 4);

		snprintf(reinterpret_cast<char*>(m_PollReply.NodeReport), ArtNet::REPORT_LENGTH, "%04x [%04d] %s AvV", static_cast<int>(m_State.reportCode), static_cast<int>(m_State.ArtPollReplyCount), m_aSysName);

		Network::Get()->SendTo(m_nHandle, &m_PollReply, sizeof(struct TArtPollReply), m_Node.IPAddressBroadcast, ArtNet::UDP_PORT);
	}

	m_State.IsChanged = false;
}

void ArtNetNode::HandlePoll() {
	const auto *pArtPoll = &(m_ArtNetPacket.ArtPacket.ArtPoll);

	if (pArtPoll->TalkToMe & ArtNetTalkToMe::SEND_ARTP_ON_CHANGE) {
		m_State.SendArtPollReplyOnChange = true;
	} else {
		m_State.SendArtPollReplyOnChange = false;
	}

	// If any controller requests diagnostics, the node will send diagnostics. (ArtPoll->TalkToMe->2).
	if (pArtPoll->TalkToMe & ArtNetTalkToMe::SEND_DIAG_MESSAGES) {
		m_State.SendArtDiagData = true;

		if (m_State.IPAddressArtPoll == 0) {
			m_State.IPAddressArtPoll = m_ArtNetPacket.IPAddressFrom;
		} else if (!m_State.IsMultipleControllersReqDiag && (m_State.IPAddressArtPoll != m_ArtNetPacket.IPAddressFrom)) {
			// If there are multiple controllers requesting diagnostics, diagnostics shall be broadcast.
			m_State.IPAddressDiagSend = m_Node.IPAddressBroadcast;
			m_State.IsMultipleControllersReqDiag = true;
		}

		if (m_State.IsMultipleControllersReqDiag ) {
			// The lowest minimum value of Priority shall be used. (Ignore ArtPoll->Priority).
			m_State.Priority = std::min(m_State.Priority , pArtPoll->Priority);
		} else {
			m_State.Priority = pArtPoll->Priority;
		}

		// If there are multiple controllers requesting diagnostics, diagnostics shall be broadcast. (Ignore ArtPoll->TalkToMe->3).
		if (!m_State.IsMultipleControllersReqDiag && (pArtPoll->TalkToMe & ArtNetTalkToMe::SEND_DIAG_UNICAST)) {
			m_State.IPAddressDiagSend = m_ArtNetPacket.IPAddressFrom;
		} else {
			m_State.IPAddressDiagSend = m_Node.IPAddressBroadcast;
		}
	} else {
		m_State.SendArtDiagData = false;
		m_State.IPAddressDiagSend = 0;
	}

	SendPollRelply(true);
}
