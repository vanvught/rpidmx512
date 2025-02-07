/**
 * @file artnetnodehandlepoll.cpp
 *
 */
/**
 * Art-Net Designed by and Copyright Artistic Licence Holdings Ltd.
 */
/* Copyright (C) 2021-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#if defined(DEBUG_ARTNET_POLL)
# undef NDEBUG
#endif

#if defined(__GNUC__) && !defined(__clang__)
# pragma GCC push_options
# pragma GCC optimize ("O2")
# pragma GCC optimize ("no-tree-loop-distribute-patterns")
# pragma GCC optimize ("-fprefetch-loop-arrays")
#endif

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

template<uint8_t N>
static inline void uitoa(uint32_t v, uint8_t *p) {
	static_assert(N >= 1);
	auto *o = p + (N - 1);
	do {
		*o-- = static_cast<uint8_t>('0' + (v % 10U));
		v /= 10U;
	} while ((o >= p) && (v > 0));

	// If there are remaining digits, fill with zeros
	while (o >= p) {
		*o-- = '0';
	}
}

using namespace artnet;

/*
 * Table 3 – NodeReport Codes
 */
static const char *get_report_code_string(const ReportCode code) {
	switch (code) {
	case ReportCode::RCDEBUG: return "Booted in debug mode (Only used in development)";
	case ReportCode::RCPOWEROK: return "Power On Tests successful";
	case ReportCode::RCPOWERFAIL: return "Hardware tests failed at Power On";
	case ReportCode::RCSOCKETWR1: return "Last UDP from Node failed due to truncated length";
	case ReportCode::RCPARSEFAIL: return "Unable to identify last UDP transmission.";
	case ReportCode::RCUDPFAIL: return "Unable to open Udp Socket in last transmission";
	case ReportCode::RCSHNAMEOK: return "Short Name programming [ArtAddress] was successful.";
	case ReportCode::RCLONAMEOK: return "Long Name programming [ArtAddress] was successful.";
	case ReportCode::RCDMXERROR: return "DMX512 receive errors detected.";
	case ReportCode::RCDMXUDPFULL: return "Ran out of internal DMX transmit buffers.";
	case ReportCode::RCDMXRXFULL: return "Ran out of internal DMX Rx buffers.";
	case ReportCode::RCSWITCHERR: return "Rx Universe switches conflict.";
	case ReportCode::RCCONFIGERR: return "Product configuration does not match firmware.";
	case ReportCode::RCDMXSHORT: return "DMX output short detected. See GoodOutput field.";
	case ReportCode::RCFIRMWAREFAIL: return "Last attempt to upload new firmware failed.";
	case ReportCode::RCUSERFAIL: return "User changed switch settings when address locked.";
	default: return "Unknown Report Code";
	}
}

/*
 * NodeReport [64]
 *
 * The array is a textual report of the Node’s operating status or operational errors. It is
 * primarily intended for ‘engineering’ data rather than ‘end user’ data. The field is formatted as:
 * “#xxxx [yyyy..] zzzzz…”
 * xxxx is a hex status code as defined in Table 3.
 * yyyy is a decimal counter that increments every time the Node sends an ArtPollResponse.
 */
static void create_node_report(uint8_t *pNodeReport, const ReportCode code, const uint32_t nCounter) {
	[[maybe_unused]] const auto *pBegin = pNodeReport;

	*pNodeReport++ = '#';
	uitoa<4>(static_cast<uint32_t>(code), pNodeReport);
	pNodeReport += 4;
	*pNodeReport++ = ' ';
	*pNodeReport++ = '[';
	uitoa<4>(nCounter, pNodeReport);
	pNodeReport += 4;
	*pNodeReport++ = ']';
	*pNodeReport++ = ' ';

	assert((REPORT_LENGTH - (pNodeReport - pBegin) - 1) == 50);
	constexpr auto nRemainingSize = 50; // REPORT_LENGTH - (pNodeReport - pBegin) - 1;
	const auto *preportStr = get_report_code_string(code);

	strncpy(reinterpret_cast<char *>(pNodeReport), preportStr, nRemainingSize);
}

union uip {
	uint32_t u32;
	uint8_t u8[4];
} static ip;

void ArtNetNode::ProcessPollReply(const uint32_t nPortIndex, [[maybe_unused]] uint32_t& NumPortsInput, uint32_t& NumPortsOutput) {
	if (m_Node.Port[nPortIndex].direction == lightset::PortDir::OUTPUT) {
#if (ARTNET_VERSION >= 4)
		if (m_Node.Port[nPortIndex].protocol == PortProtocol::SACN) {
			constexpr auto MASK = GoodOutput::OUTPUT_IS_MERGING | GoodOutput::DATA_IS_BEING_TRANSMITTED | GoodOutput::OUTPUT_IS_SACN;
			auto GoodOutput = m_OutputPort[nPortIndex].GoodOutput;
			GoodOutput &= static_cast<uint8_t>(~MASK);
			GoodOutput = static_cast<uint8_t>(GoodOutput | (GetGoodOutput4(nPortIndex) & MASK));
			m_OutputPort[nPortIndex].GoodOutput = GoodOutput;
		}
#endif
		m_ArtPollReply.PortTypes[0] |= PortType::OUTPUT_ARTNET;
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
		if (m_Node.Port[nPortIndex].protocol == PortProtocol::SACN) {

		}
#endif
		m_ArtPollReply.PortTypes[0] |= PortType::INPUT_ARTNET;
		m_ArtPollReply.GoodOutput[0] = 0;
		m_ArtPollReply.GoodOutputB[0] = 0;
		m_ArtPollReply.GoodInput[0] = m_InputPort[nPortIndex].GoodInput;
		m_ArtPollReply.SwOut[0] = 0;
		m_ArtPollReply.SwIn[0] = m_Node.Port[nPortIndex].DefaultAddress;
		NumPortsInput++;
		return;
	}
#endif

	m_ArtPollReply.PortTypes[0] = 0;
	m_ArtPollReply.GoodOutput[0] = 0;
	m_ArtPollReply.GoodOutputB[0] = 0;
	m_ArtPollReply.GoodInput[0] = 0;
	m_ArtPollReply.SwOut[0] = 0;
	m_ArtPollReply.SwIn[0] = 0;
}

void ArtNetNode::SendPollReply(const uint32_t nBindIndex, const uint32_t nDestinationIp, ArtPollQueue *pQueue) {
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

		m_ArtPollReply.NetSwitch = m_Node.Port[nPortIndex].NetSwitch;
		m_ArtPollReply.SubSwitch = m_Node.Port[nPortIndex].SubSwitch;
		m_ArtPollReply.BindIndex = static_cast<uint8_t>(nPortIndex + 1);

		memcpy(m_ArtPollReply.ShortName, m_Node.Port[nPortIndex].ShortName, SHORT_NAME_LENGTH);

		if (__builtin_expect((m_pLightSet != nullptr), 1)) {
			const auto nRefreshRate = m_pLightSet->GetRefreshRate();
			m_ArtPollReply.RefreshRateLo = static_cast<uint8_t>(nRefreshRate);
			m_ArtPollReply.RefreshRateHi = static_cast<uint8_t>(nRefreshRate >> 8);
			const auto nUserData = m_pLightSet->GetUserData();
			m_ArtPollReply.UserLo = static_cast<uint8_t>(nUserData);
			m_ArtPollReply.UserHi = static_cast<uint8_t>(nUserData >> 8);
		}

		m_State.ArtPollReplyCount++;
		create_node_report(m_ArtPollReply.NodeReport, m_State.reportCode, m_State.ArtPollReplyCount);

		uint32_t nPortsOutput = 0;
		uint32_t nPortsInput = 0;
		ProcessPollReply(nPortIndex, nPortsInput, nPortsOutput);

		m_ArtPollReply.NumPortsLo = static_cast<uint8_t>(std::max(nPortsInput, nPortsOutput));

		Network::Get()->SendTo(m_nHandle, &m_ArtPollReply, sizeof(ArtPollReply), nDestinationIp, UDP_PORT);
	}

	m_State.IsChanged = false;
}

void ArtNetNode::HandlePoll() {
	const auto *const pArtPoll = reinterpret_cast<ArtPoll *>(m_pReceiveBuffer);

	m_State.SendArtPollReplyOnChange = ((pArtPoll->Flags & Flags::SEND_ARTP_ON_CHANGE) == Flags::SEND_ARTP_ON_CHANGE);

	// If any controller requests diagnostics, the node will send diagnostics. (ArtPoll->Flags->2).
	if (pArtPoll->Flags & Flags::SEND_DIAG_MESSAGES) {
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
		if (!m_State.IsMultipleControllersReqDiag && (pArtPoll->Flags & Flags::SEND_DIAG_UNICAST)) {
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

	if (pArtPoll->Flags & Flags::USE_TARGET_PORT_ADDRESS) {
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
