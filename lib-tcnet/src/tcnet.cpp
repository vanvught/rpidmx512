/**
 * @file tcnet.cpp
 *
 */
/* Copyright (C) 2019-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <stdint.h>
#include <string.h>
#include <cassert>

#include "tcnet.h"
#include "tcnettimecode.h"

#include "tcnetpackets.h"

#include "hardware.h"
#include "network.h"

#include "debug.h"

static constexpr char NODE_NAME_DEFAULT[] = "AvV-OPi";

TCNet *TCNet::s_pThis = nullptr;

TCNet::TCNet(TTCNetNodeType tNodeType) {
	assert(s_pThis == nullptr);
	s_pThis = this;

	memset(&m_tOptIn, 0, sizeof(struct TTCNetPacketOptIn));

	// Fill the static fields for Opt-IN
	m_tOptIn.ManagementHeader.ProtocolVersionMajor = 3;
	m_tOptIn.ManagementHeader.ProtocolVersionMinor = 3;
	memcpy(m_tOptIn.ManagementHeader.Header, "TCN", 3);
	m_tOptIn.ManagementHeader.MessageType = TCNET_MESSAGE_TYPE_OPTIN;
	m_tOptIn.ManagementHeader.SEQ = 0;
	m_tOptIn.ManagementHeader.NodeType = tNodeType;
	m_tOptIn.ManagementHeader.NodeOptions = 0;
	m_tOptIn.NodeCount = 1;
	m_tOptIn.NodeListenerPort = TCNETUnicast::PORT;
	memcpy(&m_tOptIn.VendorName, "orangepi-dmx.org", TCNET_VENDOR_NAME_LENGTH);
	memcpy(&m_tOptIn.DeviceName, "LTC SMPTE Node  ", TCNET_DEVICE_NAME_LENGTH);
	m_tOptIn.DeviceMajorVersion = static_cast<uint8_t>(_TIME_STAMP_YEAR_ - 2000);
	m_tOptIn.DeviceMinorVersion = _TIME_STAMP_MONTH_;
	m_tOptIn.DeviceBugVersion = _TIME_STAMP_DAY_;

	SetNodeName(NODE_NAME_DEFAULT);
	SetLayer(TCNetLayer::LAYER_M);
	SetTimeCodeType(TCNET_TIMECODE_TYPE_SMPTE_30FPS);
}

TCNet::~TCNet() {
	Stop();
}

void TCNet::Start() {
	m_tNode.nIPAddressLocal = Network::Get()->GetIp();
	m_tNode.nIPAddressBroadcast = m_tNode.nIPAddressLocal | ~(Network::Get()->GetNetmask());

	m_aHandles[0] = Network::Get()->Begin(TCNetBroadcast::PORT_0);
	assert(m_aHandles[0] >= 0);

	m_aHandles[1] = Network::Get()->Begin(TCNetBroadcast::PORT_1);
	assert(m_aHandles[1] >= 0);

#if defined(USE_PORT_60002)
	m_aHandles[2] = Network::Get()->Begin(TCNetBroadcast::PORT_2);
	assert(m_aHandles[2] >= 0);
#endif

#if defined(USE_PORT_UNICAST)
	m_aHandles[3] = Network::Get()->Begin(TCNETUnicast::PORT);
	assert(m_aHandles[3] >= 0);
#endif
}

void TCNet::Stop() {
	DEBUG_ENTRY

	struct TTCNetPacketOptOut OptOut;

	memcpy(&OptOut, &m_tOptIn, sizeof(struct TTCNetPacketOptOut));

	Network::Get()->SendTo(m_aHandles[0], &OptOut, sizeof(struct TTCNetPacketOptOut), m_tNode.nIPAddressBroadcast, TCNetBroadcast::PORT_0);

	DEBUG_EXIT
}

void TCNet::HandlePort60000Incoming() {
	const auto packet = &(m_TTCNet.TCNetPacket);
	const auto type  = static_cast<TTCNetMessageType>(packet->ManagementHeader.MessageType);

	DEBUG_PRINTF("MessageType = %d", static_cast<int>(type));

	if (type == TCNET_MESSAGE_TYPE_OPTIN) {
#ifndef NDEBUG
		DumpOptIn();
#endif
		return;
	}
}

void TCNet::HandlePort60001Incoming() {
	if (__builtin_expect((m_pTCNetTimeCode != nullptr), 1)) {
		if (static_cast<TTCNetMessageType>(m_TTCNet.TCNetPacket.ManagementHeader.MessageType) == TCNET_MESSAGE_TYPE_TIME) {
			struct TTCNetTimeCode TimeCode;

			if (!m_bUseTimeCode) {
				uint32_t nTime = *m_pLTime;

				const uint32_t nHours = nTime / 3600000;
				nTime -= nHours * 3600000;
				const uint32_t nMinutes = nTime / 60000;
				nTime -= nMinutes * 60000;
				const uint32_t nSeconds = nTime / 1000;
				const uint32_t nMillis = nTime - nSeconds * 1000;
				const uint32_t nFrames = static_cast<float>(nMillis) / m_fTypeDivider;

				TimeCode.nFrames = nFrames;
				TimeCode.nSeconds = nSeconds;
				TimeCode.nMinutes = nMinutes;
				TimeCode.nHours = nHours;
				TimeCode.nType = m_tTimeCodeType;
			} else {
				TimeCode.nFrames = m_pLTimeCode->Frames;
				TimeCode.nSeconds = m_pLTimeCode->Seconds;
				TimeCode.nMinutes = m_pLTimeCode->Minutes;
				TimeCode.nHours = m_pLTimeCode->Hours;

				uint8_t nSMPTEMode = m_pLTimeCode->SMPTEMode;

				if (nSMPTEMode < 24) {
					nSMPTEMode = m_TTCNet.TCNetPacket.Time.SMPTEMode;
				}

				if (nSMPTEMode == 24) {
					TimeCode.nType = TCNET_TIMECODE_TYPE_FILM;
				} else if (nSMPTEMode == 25) {
					TimeCode.nType = TCNET_TIMECODE_TYPE_EBU_25FPS;
				} else if (nSMPTEMode == 29) {
					TimeCode.nType = TCNET_TIMECODE_TYPE_DF;
				} else if (nSMPTEMode == 30) {
					TimeCode.nType = TCNET_TIMECODE_TYPE_SMPTE_30FPS;
				} else {
					TimeCode.nType = TCNET_TIMECODE_TYPE_EBU_25FPS;
				}
			}

			m_pTCNetTimeCode->Handler(&TimeCode);
		}
	}
}

void TCNet::HandlePort60002Incoming() {
	DEBUG_ENTRY

	DEBUG_EXIT
}

void TCNet::HandlePortUnicastIncoming() {
	DEBUG_ENTRY
#ifndef NDEBUG
	const auto packet = &(m_TTCNet.TCNetPacket);
	const auto type  = static_cast<TTCNetMessageType>(packet->ManagementHeader.MessageType);
#endif
	DEBUG_PRINTF("MessageType = %d", static_cast<int>(type));

	DEBUG_EXIT
}

void TCNet::HandleOptInOutgoing() {
	m_tOptIn.ManagementHeader.SEQ += 1;
	m_tOptIn.ManagementHeader.TimeStamp = Hardware::Get()->Micros();
	m_tOptIn.Uptime = Hardware::Get()->GetUpTime();

	Network::Get()->SendTo(m_aHandles[0], &m_tOptIn, sizeof(struct TTCNetPacketOptIn), m_tNode.nIPAddressBroadcast, TCNetBroadcast::PORT_0);
}

void TCNet::Run() {
	auto packet = reinterpret_cast<uint8_t*>(&m_TTCNet.TCNetPacket);
	uint16_t nForeignPort;

	m_TTCNet.BytesReceived = Network::Get()->RecvFrom(m_aHandles[1], packet, sizeof(m_TTCNet.TCNetPacket), &m_TTCNet.IPAddressFrom, &nForeignPort) ;

	if (m_TTCNet.BytesReceived != 0) {
		HandlePort60001Incoming();
	}

	m_TTCNet.BytesReceived = Network::Get()->RecvFrom(m_aHandles[0], packet, sizeof(m_TTCNet.TCNetPacket), &m_TTCNet.IPAddressFrom, &nForeignPort) ;

	if (m_TTCNet.BytesReceived != 0) {
		HandlePort60000Incoming();
	}

#if defined(USE_PORT_60002)
	m_TTCNet.BytesReceived = Network::Get()->RecvFrom(m_aHandles[2], packet, sizeof(m_TTCNet.TCNetPacket), &m_TTCNet.IPAddressFrom, &nForeignPort) ;

	if (m_TTCNet.BytesReceived != 0) {
		HandlePort60002Incoming();
	}
#endif

#if defined(USE_PORT_UNICAST)
	m_TTCNet.BytesReceived = Network::Get()->RecvFrom(m_aHandles[3], packet, sizeof(m_TTCNet.TCNetPacket), &m_TTCNet.IPAddressFrom, &nForeignPort) ;

	if (m_TTCNet.BytesReceived != 0) {
		HandlePortUnicastIncoming();
	}
#endif

	m_nCurrentMillis = Hardware::Get()->Millis();

	if (__builtin_expect(((m_nCurrentMillis - m_nPreviousMillis) >= 1000), 0)) {
		m_nPreviousMillis = m_nCurrentMillis;
		HandleOptInOutgoing();
	}
}

void TCNet::SetLayer(TCNetLayer tLayer) {
	if(tLayer >= TCNetLayer::LAYER_UNDEFINED) { // TODO Backward compatibility, subject for removal
		tLayer = TCNetLayer::LAYER_M;
		m_bUseTimeCode = true;
	}

	m_tLayer = tLayer;
	m_pLTime = &m_TTCNet.TCNetPacket.Time.L1Time + static_cast<uint32_t>(tLayer);
	m_pLTimeCode =
			reinterpret_cast<struct TTCNetPacketTimeTimeCode*>((reinterpret_cast<uintptr_t>((&m_TTCNet.TCNetPacket.Time.L1TimeCode))
					+ static_cast<uintptr_t>(tLayer) * sizeof(struct TTCNetPacketTimeTimeCode)));
}

void TCNet::SetNodeName(const char *pNodeName) {
#if (__GNUC__ > 8)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstringop-truncation"
#endif
	strncpy(reinterpret_cast<char*>(m_tOptIn.ManagementHeader.NodeName), pNodeName, sizeof m_tOptIn.ManagementHeader.NodeName - 1);
	m_tOptIn.ManagementHeader.NodeName[sizeof m_tOptIn.ManagementHeader.NodeName - 1] = '\0';
#if (__GNUC__ > 8)
#pragma GCC diagnostic pop
#endif
}

char TCNet::GetLayerName(TCNetLayer tLayer) {
	switch (tLayer) {
	case TCNetLayer::LAYER_1:
	case TCNetLayer::LAYER_2:
	case TCNetLayer::LAYER_3:
	case TCNetLayer::LAYER_4:
		return static_cast<char>(tLayer) + '1';
		break;
	case TCNetLayer::LAYER_A:
		return 'A';
		break;
	case TCNetLayer::LAYER_B:
		return 'B';
		break;
	case TCNetLayer::LAYER_M:
		return 'M';
		break;
	case TCNetLayer::LAYER_C:
		return 'C';
		break;
	default:
		break;
	}

	return ' ';
}

TCNetLayer TCNet::GetLayer(char nChar) {
	switch (nChar | 0x20) {	// to lower case
	case '1':
	case '2':
	case '3':
	case '4':
		return static_cast<TCNetLayer>(nChar - '1');
		break;
	case 'a':
		return TCNetLayer::LAYER_A;
		break;
	case 'b':
		return TCNetLayer::LAYER_B;
		break;
	case 'm':
		return TCNetLayer::LAYER_M;
		break;
	case 'c':
		return TCNetLayer::LAYER_C;
		break;
	default:
		break;
	}

	return TCNetLayer::LAYER_UNDEFINED;
}

void TCNet::SetTimeCodeType(TTCNetTimeCodeType tType) {
	switch (tType) {
	case TCNET_TIMECODE_TYPE_FILM:
		m_fTypeDivider = 1000.0F / 24;
		break;
	case TCNET_TIMECODE_TYPE_EBU_25FPS:
		m_fTypeDivider = 1000.0F / 25;
		break;
	case TCNET_TIMECODE_TYPE_DF:
		m_fTypeDivider = 1000.0F / 29.97;
		break;
	case TCNET_TIMECODE_TYPE_SMPTE_30FPS:
		m_fTypeDivider = 1000.0F / 30;
		break;
	default:
		return;
		break;
	}

	m_tTimeCodeType = tType;
}
