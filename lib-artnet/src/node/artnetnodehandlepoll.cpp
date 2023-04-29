/**
 * @file artnetnodehandlepoll.cpp
 *
 */
/**
 * Art-Net Designed by and Copyright Artistic Licence Holdings Ltd.
 */
/* Copyright (C) 2021-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "debug.h"

union uip {
	uint32_t u32;
	uint8_t u8[4];
} static ip;

void ArtNetNode::FillPollReply() {
	memset(&m_PollReply, 0, sizeof(struct TArtPollReply));

	memcpy(m_PollReply.Id, artnet::NODE_ID, sizeof(m_PollReply.Id));

	m_PollReply.OpCode = OP_POLLREPLY;

	ip.u32 = Network::Get()->GetIp();
	memcpy(m_PollReply.IPAddress, ip.u8, sizeof(m_PollReply.IPAddress));

	m_PollReply.Port = artnet::UDP_PORT;

	m_PollReply.VersInfoH = ArtNetConst::VERSION[0];
	m_PollReply.VersInfoL = ArtNetConst::VERSION[1];

	m_PollReply.OemHi = ArtNetConst::OEM_ID[0];
	m_PollReply.Oem = ArtNetConst::OEM_ID[1];

	m_PollReply.EstaMan[0] = ArtNetConst::ESTA_ID[1];
	m_PollReply.EstaMan[1] = ArtNetConst::ESTA_ID[0];

	memcpy(m_PollReply.ShortName, m_Node.ShortName, sizeof(m_PollReply.ShortName));
	memcpy(m_PollReply.LongName, m_Node.LongName, sizeof(m_PollReply.LongName));

	m_PollReply.Style = artnet::StyleCode::ST_NODE;

	memcpy(m_PollReply.MAC, m_Node.MACAddressLocal, sizeof(m_PollReply.MAC));

	if (artnet::VERSION > 3) {
		memcpy(m_PollReply.BindIp, ip.u8, sizeof(m_PollReply.BindIp));
	}

	memcpy(m_PollReply.DefaultUidResponder, m_Node.DefaultUidResponder, sizeof(m_PollReply.DefaultUidResponder));
}

void ArtNetNode::ProcessPollRelply(uint32_t nPortIndex, __attribute__((unused)) uint32_t& NumPortsInput, uint32_t& NumPortsOutput) {
	if (artnet::VERSION > 3) {
		if (m_OutputPort[nPortIndex].protocol == artnet::PortProtocol::SACN) {
			constexpr auto MASK = artnet::GoodOutput::OUTPUT_IS_MERGING | artnet::GoodOutput::DATA_IS_BEING_TRANSMITTED | artnet::GoodOutput::OUTPUT_IS_SACN;
			auto GoodOutput = m_OutputPort[nPortIndex].GoodOutput;
			GoodOutput &= static_cast<uint8_t>(~MASK);
			GoodOutput = static_cast<uint8_t>(GoodOutput | (m_pArtNet4Handler->GetStatus(nPortIndex) & MASK));

			if ((GoodOutput & artnet::GoodOutput::OUTPUT_IS_SACN) == 0) {
				m_OutputPort[nPortIndex].protocol = artnet::PortProtocol::ARTNET;

			}

			m_OutputPort[nPortIndex].GoodOutput = GoodOutput;
		}
	}

	if (m_OutputPort[nPortIndex].genericPort.bIsEnabled) {
		const auto nIndex = m_OutputPort[nPortIndex].genericPort.nPollReplyIndex;
		m_PollReply.PortTypes[nIndex] |= artnet::PortSettings::ENABLE_OUTPUT;
		m_PollReply.GoodOutput[nIndex] = m_OutputPort[nPortIndex].GoodOutput;
		m_PollReply.GoodOutputB[nIndex] = m_OutputPort[nPortIndex].GoodOutputB;
		m_PollReply.SwOut[nIndex] = m_OutputPort[nPortIndex].genericPort.nDefaultAddress;
		NumPortsOutput++;
		return;
	}

#if defined (ARTNET_HAVE_DMXIN)
	if (m_InputPort[nPortIndex].genericPort.bIsEnabled) {
		const auto nIndex = m_InputPort[nPortIndex].genericPort.nPollReplyIndex;
		m_PollReply.PortTypes[nIndex] |= artnet::PortSettings::ENABLE_INPUT;
		m_PollReply.GoodInput[nIndex] = m_InputPort[nPortIndex].GoodInput;
		m_PollReply.SwIn[nIndex] = m_InputPort[nPortIndex].genericPort.nDefaultAddress;
		NumPortsInput++;
	}
#endif
}

void ArtNetNode::SendPollRelply(bool bResponse) {
	if (!bResponse && m_State.status == artnetnode::Status::ON) {
		m_State.ArtPollReplyCount++;
	}

	m_PollReply.Status1 = m_Node.Status1;
	m_PollReply.Status2 = m_Node.Status2;
	m_PollReply.Status3 = m_Node.Status3;

	for (uint32_t nPage = 0; nPage < artnetnode::PAGES; nPage++) {
		for (uint32_t nIndex = 0; nIndex < artnet::PORTS; nIndex++) {
			m_PollReply.PortTypes[nIndex] = 0;
			m_PollReply.SwIn[nIndex] = 0;
			m_PollReply.SwOut[nIndex] = 0;
		}

		m_PollReply.NetSwitch = m_Node.NetSwitch[nPage];
		m_PollReply.SubSwitch = m_Node.SubSwitch[nPage];
		m_PollReply.BindIndex = static_cast<uint8_t>(nPage + 1);

		const auto nPortIndexStart = nPage * artnetnode::PAGE_SIZE;

		DEBUG_PRINTF("nPortIndexStart=%u artnetnode::PAGE_SIZE=%u", nPortIndexStart, artnetnode::PAGE_SIZE);

		uint32_t nPortsOutput = 0;
		uint32_t nPortsInput = 0;

		for (auto nPortIndex = nPortIndexStart; nPortIndex < (nPortIndexStart + artnetnode::PAGE_SIZE); nPortIndex++) {
			if (nPortIndex >= artnetnode::MAX_PORTS) {
				const auto nIndex = nPortIndex - nPortIndexStart;
				assert(nIndex < artnet::PORTS);

				m_PollReply.GoodOutput[nIndex] = 0;
				m_PollReply.GoodOutputB[nIndex] = 0;
				m_PollReply.GoodInput[nIndex] = 0;
				continue;
			}
			ProcessPollRelply(nPortIndex, nPortsInput, nPortsOutput);
		}

		m_PollReply.NumPortsLo = static_cast<uint8_t>(std::max(nPortsInput, nPortsOutput));

		uint8_t nSysNameLenght;
		const auto *pSysName = Hardware::Get()->GetSysName(nSysNameLenght);
		snprintf(reinterpret_cast<char*>(m_PollReply.NodeReport), artnet::REPORT_LENGTH, "#%04x [%04d] %.*s AvV", static_cast<int>(m_State.reportCode), static_cast<int>(m_State.ArtPollReplyCount), nSysNameLenght, pSysName);

		Network::Get()->SendTo(m_nHandle, &m_PollReply, sizeof(TArtPollReply), m_Node.IPAddressBroadcast, artnet::UDP_PORT);
	}

	m_State.IsChanged = false;
}

void ArtNetNode::HandlePoll() {
	const auto *const pArtPoll = reinterpret_cast<TArtPoll *>(m_pReceiveBuffer);

	if (pArtPoll->TalkToMe & artnet::TalkToMe::SEND_ARTP_ON_CHANGE) {
		m_State.SendArtPollReplyOnChange = true;
	} else {
		m_State.SendArtPollReplyOnChange = false;
	}

	// If any controller requests diagnostics, the node will send diagnostics. (ArtPoll->TalkToMe->2).
	if (pArtPoll->TalkToMe & artnet::TalkToMe::SEND_DIAG_MESSAGES) {
		m_State.SendArtDiagData = true;

		if (m_State.IPAddressArtPoll == 0) {
			m_State.IPAddressArtPoll = m_nIpAddressFrom;
		} else if (!m_State.IsMultipleControllersReqDiag && (m_State.IPAddressArtPoll != m_nIpAddressFrom)) {
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
		if (!m_State.IsMultipleControllersReqDiag && (pArtPoll->TalkToMe & artnet::TalkToMe::SEND_DIAG_UNICAST)) {
			m_State.IPAddressDiagSend = m_nIpAddressFrom;
		} else {
			m_State.IPAddressDiagSend = m_Node.IPAddressBroadcast;
		}
	} else {
		m_State.SendArtDiagData = false;
		m_State.IPAddressDiagSend = 0;
	}

	SendPollRelply(true);
}
