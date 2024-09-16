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

void ArtNetNode::ProcessPollRelply(const uint32_t nPortIndex, [[maybe_unused]] uint32_t& NumPortsInput, uint32_t& NumPortsOutput) {
	if (m_Node.Port[nPortIndex].direction == lightset::PortDir::OUTPUT) {
#if (ARTNET_VERSION >= 4)
		if (m_Node.Port[nPortIndex].protocol == artnet::PortProtocol::SACN) {
			constexpr auto MASK = artnet::GoodOutput::OUTPUT_IS_MERGING | artnet::GoodOutput::DATA_IS_BEING_TRANSMITTED | artnet::GoodOutput::OUTPUT_IS_SACN;
			auto GoodOutput = m_OutputPort[nPortIndex].GoodOutput;
			GoodOutput &= static_cast<uint8_t>(~MASK);
			GoodOutput = static_cast<uint8_t>(GoodOutput | (GetGoodOutput4(nPortIndex) & MASK));
			m_OutputPort[nPortIndex].GoodOutput = GoodOutput;
		}
#endif
		m_ArtPollReply.PortTypes[0] |= artnet::PortType::OUTPUT_ARTNET;
		m_ArtPollReply.GoodOutput[0] = m_OutputPort[nPortIndex].GoodOutput;
		m_ArtPollReply.GoodOutputB[0] = m_OutputPort[nPortIndex].GoodOutputB;
		m_ArtPollReply.GoodInput[0] = 0;
		m_ArtPollReply.SwOut[0] = m_Node.Port[nPortIndex].DefaultAddress;
		m_ArtPollReply.SwIn[0] = 0;
		NumPortsOutput++;
		return;
	}

#if defined (ARTNET_HAVE_DMXIN)
	if (m_Node.Port[nPortIndex].direction == lightset::PortDir::INPUT) {
#if (ARTNET_VERSION >= 4)
		if (m_Node.Port[nPortIndex].protocol == artnet::PortProtocol::SACN) {

		}
#endif
		m_ArtPollReply.PortTypes[0] |= artnet::PortType::INPUT_ARTNET;
		m_ArtPollReply.GoodOutput[0] = 0;
		m_ArtPollReply.GoodOutputB[0] = 0;
		m_ArtPollReply.GoodInput[0] = m_InputPort[nPortIndex].GoodInput;
		m_ArtPollReply.SwOut[0] = 0;
		m_ArtPollReply.SwIn[0] = m_Node.Port[nPortIndex].DefaultAddress;
		NumPortsInput++;
	}
#endif
}

void ArtNetNode::SendPollRelply(const uint32_t nBindIndex, const uint32_t nDestinationIp, artnet::ArtPollQueue *pQueue) {
	DEBUG_PRINTF("nBindIndex=%u", nBindIndex);

	ip.u32 = Network::Get()->GetIp();
	memcpy(m_ArtPollReply.IPAddress, ip.u8, sizeof(m_ArtPollReply.IPAddress));
#if (ARTNET_VERSION >= 4)
	memcpy(m_ArtPollReply.BindIp, ip.u8, sizeof(m_ArtPollReply.BindIp));
#endif

	for (uint32_t nPortIndex = 0; nPortIndex < artnetnode::MAX_PORTS; nPortIndex++) {
		if ((nBindIndex != 0) && (nBindIndex != (nPortIndex + 1))) {
			continue;
		}

		for (uint32_t nArtNetPortIndex = 0; nArtNetPortIndex < artnet::PORTS; nArtNetPortIndex++) {
			m_ArtPollReply.PortTypes[nArtNetPortIndex] = 0;
			m_ArtPollReply.SwIn[nArtNetPortIndex] = 0;
			m_ArtPollReply.SwOut[nArtNetPortIndex] = 0;
		}

		m_ArtPollReply.NetSwitch = m_Node.Port[nPortIndex].NetSwitch;
		m_ArtPollReply.SubSwitch = m_Node.Port[nPortIndex].SubSwitch;
		m_ArtPollReply.BindIndex = static_cast<uint8_t>(nPortIndex + 1);

		uint32_t nPortsOutput = 0;
		uint32_t nPortsInput = 0;

		if ((nBindIndex == 0) && (pQueue != nullptr)) {
			if (!((m_Node.Port[nPortIndex].PortAddress >= pQueue->ArtPollReply.TargetPortAddressBottom)
			   && (m_Node.Port[nPortIndex].PortAddress <= pQueue->ArtPollReply.TargetPortAddressTop))) {
				DEBUG_PRINTF("NOT: 	%u >= %u && %u <= %u",
						m_Node.Port[nPortIndex].PortAddress,
						pQueue->ArtPollReply.TargetPortAddressBottom,
						m_Node.Port[nPortIndex].PortAddress,
						pQueue->ArtPollReply.TargetPortAddressTop);
				continue;
			}
		}

		memcpy(m_ArtPollReply.ShortName, m_Node.Port[nPortIndex].ShortName, artnet::SHORT_NAME_LENGTH);

		ProcessPollRelply(nPortIndex, nPortsInput, nPortsOutput);

		if (__builtin_expect((m_pLightSet != nullptr), 1)) {
			const auto nRefreshRate = m_pLightSet->GetRefreshRate();
			m_ArtPollReply.RefreshRateLo = static_cast<uint8_t>(nRefreshRate);
			m_ArtPollReply.RefreshRateHi = static_cast<uint8_t>(nRefreshRate >> 8);
		}

		m_ArtPollReply.NumPortsLo = static_cast<uint8_t>(std::max(nPortsInput, nPortsOutput));

		m_State.ArtPollReplyCount++;

		uint8_t nSysNameLenght;
		const auto *pSysName = Hardware::Get()->GetSysName(nSysNameLenght);
		snprintf(reinterpret_cast<char*>(m_ArtPollReply.NodeReport), artnet::REPORT_LENGTH, "#%04x [%04d] %.*s AvV", static_cast<int>(m_State.reportCode), static_cast<int>(m_State.ArtPollReplyCount), nSysNameLenght, pSysName);

		Network::Get()->SendTo(m_nHandle, &m_ArtPollReply, sizeof(artnet::ArtPollReply), nDestinationIp, artnet::UDP_PORT);
	}

	m_State.IsChanged = false;
}

void ArtNetNode::HandlePoll() {
	const auto *const pArtPoll = reinterpret_cast<artnet::ArtPoll *>(m_pReceiveBuffer);

	if (pArtPoll->Flags & artnet::Flags::SEND_ARTP_ON_CHANGE) {
		m_State.SendArtPollReplyOnChange = true;
	} else {
		m_State.SendArtPollReplyOnChange = false;
	}

	// If any controller requests diagnostics, the node will send diagnostics. (ArtPoll->Flags->2).
	if (pArtPoll->Flags & artnet::Flags::SEND_DIAG_MESSAGES) {
		m_State.SendArtDiagData = true;

		if (m_State.ArtPollIpAddress == 0) {
			m_State.ArtPollIpAddress = m_nIpAddressFrom;
		} else if (!m_State.IsMultipleControllersReqDiag && (m_State.ArtPollIpAddress != m_nIpAddressFrom)) {
			// If there are multiple controllers requesting diagnostics, diagnostics shall be broadcast.
			m_State.ArtDiagIpAddress = Network::Get()->GetBroadcastIp();
			m_State.IsMultipleControllersReqDiag = true;
		}

		if (m_State.IsMultipleControllersReqDiag ) {
			// The lowest minimum value of Priority shall be used. (Ignore ArtPoll->DiagPriority).
			m_State.DiagPriority = std::min(m_State.DiagPriority , pArtPoll->DiagPriority);
		} else {
			m_State.DiagPriority = pArtPoll->DiagPriority;
		}

		// If there are multiple controllers requesting diagnostics, diagnostics shall be broadcast. (Ignore ArtPoll->Flags->3).
		if (!m_State.IsMultipleControllersReqDiag && (pArtPoll->Flags & artnet::Flags::SEND_DIAG_UNICAST)) {
			m_State.ArtDiagIpAddress = m_nIpAddressFrom;
		} else {
			m_State.ArtDiagIpAddress = Network::Get()->GetBroadcastIp();
		}
	} else {
		m_State.SendArtDiagData = false;
		m_State.ArtDiagIpAddress = 0;
	}

	uint16_t TargetPortAddressTop = 32767; //TODO
	uint16_t TargetPortAddressBottom = 0;

	if (pArtPoll->Flags & artnet::Flags::USE_TARGET_PORT_ADDRESS) {
		TargetPortAddressTop = static_cast<uint16_t>((static_cast<uint16_t>(pArtPoll->TargetPortAddressTopHi) >> 8) | pArtPoll->TargetPortAddressTopLo);
		TargetPortAddressBottom = static_cast<uint16_t>((static_cast<uint16_t>(pArtPoll->TargetPortAddressBottomHi) >> 8) | pArtPoll->TargetPortAddressBottomLo);
	}

	for (auto& entry : m_State.ArtPollReplyQueue) {
		if (entry.ArtPollMillis == 0) {
			entry.ArtPollMillis = Hardware::Get()->Millis();
			entry.ArtPollReplyIpAddress = m_nIpAddressFrom;
			entry.ArtPollReply.TargetPortAddressTop = TargetPortAddressTop;
			entry.ArtPollReply.TargetPortAddressBottom = TargetPortAddressBottom;
			DEBUG_PRINTF("[ArtPollReply queued for " IPSTR, IP2STR(entry.ArtPollReplyIpAddress));
			break;
		}
	}
}
