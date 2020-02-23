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

#ifdef NDEBUG
//#undef NDEBUG
#endif

#include <stdint.h>
#include <string.h>
#include <assert.h>

#include "tcnet.h"
#include "tcnettimecode.h"

#include "tcnetpackets.h"

#include "hardware.h"
#include "network.h"

#include "debug.h"

const char *NODE_NAME_DEFAULT = "AvV-OPi";

TCNet *TCNet::s_pThis = 0;

TCNet::TCNet(TTCNetNodeType tNodeType) :
	m_bUseTimeCode(false),
	m_pTCNetTimeCode(0),
	m_nSeqTimeMessage(0)
{
	s_pThis = this;

	memset((void *)&m_tOptIn, 0, sizeof(struct TTCNetPacketOptIn));

	// Fill the static fields for Opt-IN
	m_tOptIn.ManagementHeader.ProtocolVersionMajor = 3;
	m_tOptIn.ManagementHeader.ProtocolVersionMinor = 3;
	memcpy(m_tOptIn.ManagementHeader.Header, "TCN", 3);
	m_tOptIn.ManagementHeader.MessageType = (uint8_t) TCNET_MESSAGE_TYPE_OPTIN;
	m_tOptIn.ManagementHeader.SEQ = 0;
	m_tOptIn.ManagementHeader.NodeType = (uint8_t) tNodeType;
	m_tOptIn.ManagementHeader.NodeOptions = 0;
	m_tOptIn.NodeCount = 1;
	m_tOptIn.NodeListenerPort = TCNET_UNICAST_PORT;
	memcpy((char *)&m_tOptIn.VendorName, "orangepi-dmx.org", TCNET_VENDOR_NAME_LENGTH);
	memcpy((char *)&m_tOptIn.DeviceName, "LTC SMPTE Node  ", TCNET_DEVICE_NAME_LENGTH);
	m_tOptIn.DeviceMajorVersion = _TIME_STAMP_YEAR_ - 2000;
	m_tOptIn.DeviceMinorVersion = _TIME_STAMP_MONTH_;
	m_tOptIn.DeviceBugVersion = _TIME_STAMP_DAY_;

	SetNodeName(NODE_NAME_DEFAULT);
	SetLayer(TCNET_LAYER_M);
	SetTimeCodeType(TCNET_TIMECODE_TYPE_SMPTE_30FPS);
}

TCNet::~TCNet(void) {
}

void TCNet::Start(void) {
	m_tNode.nIPAddressLocal = Network::Get()->GetIp();
	m_tNode.nIPAddressBroadcast = m_tNode.nIPAddressLocal | ~(Network::Get()->GetNetmask());

	m_aHandles[0] = Network::Get()->Begin(TCNET_BROADCAST_PORT_0);
	assert(m_aHandles[0] >= 0);

	m_aHandles[1] = Network::Get()->Begin(TCNET_BROADCAST_PORT_1);
	assert(m_aHandles[1] >= 0);

#if defined(USE_PORT_60002)
	m_aHandles[2] = Network::Get()->Begin(NODE_BROADCAST_PORT_2);
	assert(m_aHandles[2] >= 0);
#endif

#if defined(USE_PORT_UNICAST)
	m_aHandles[3] = Network::Get()->Begin(TCNET_UNICAST_PORT);
	assert(m_aHandles[3] >= 0);
#endif
}

void TCNet::Stop(void) {
	DEBUG_ENTRY

	struct TTCNetPacketOptOut OptOut;

	memcpy(&OptOut, (const void *)&m_tOptIn, sizeof(struct TTCNetPacketOptOut));

	Network::Get()->SendTo(m_aHandles[0],  (const uint8_t *) &OptOut, (uint16_t) sizeof(struct TTCNetPacketOptOut), m_tNode.nIPAddressBroadcast, TCNET_BROADCAST_PORT_0);

	DEBUG_EXIT
}

void TCNet::HandlePort60000Incoming(void) {
	DEBUG_ENTRY

	const struct TTCNetPacket *packet = &(m_TTCNet.TCNetPacket);
	const TTCNetMessageType type  = (TTCNetMessageType) packet->ManagementHeader.MessageType;

	DEBUG_PRINTF("MessageType = %d", (int) type);

	if (type == TCNET_MESSAGE_TYPE_OPTIN) {
#ifndef NDEBUG
		DumpOptIn();
#endif
		DEBUG_EXIT
		return;
	}

	DEBUG_EXIT
}

void TCNet::HandlePort60001Incoming(void) {
	if (__builtin_expect((m_pTCNetTimeCode != 0), 1)) {
		if ((TTCNetMessageType) m_TTCNet.TCNetPacket.ManagementHeader.MessageType == TCNET_MESSAGE_TYPE_TIME) {
			struct TTCNetTimeCode TimeCode;

			if (!m_bUseTimeCode) {
				uint32_t nTime = *m_pLTime;

				const uint32_t nHours = nTime / (uint32_t) 3600000;
				nTime -= nHours * (uint32_t) 3600000;
				const uint32_t nMinutes = nTime / (uint32_t) 60000;
				nTime -= nMinutes * (uint32_t) 60000;
				const uint32_t nSeconds = nTime / (uint32_t) 1000;
				const uint32_t nMillis = nTime - nSeconds * (uint32_t) 1000;
				const uint32_t nFrames = (float) nMillis / m_fTypeDivider;

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

void TCNet::HandlePort60002Incoming(void) {
	DEBUG_ENTRY

	DEBUG_EXIT
}

void TCNet::HandlePortUnicastIncoming(void) {
	DEBUG_ENTRY

	DEBUG_EXIT
}

void TCNet::HandleOptInOutgoing(void) {
	m_tOptIn.ManagementHeader.SEQ += 1;
	m_tOptIn.ManagementHeader.TimeStamp = Hardware::Get()->Micros();
	m_tOptIn.Uptime = Hardware::Get()->GetUpTime();

	Network::Get()->SendTo(m_aHandles[0],  (const uint8_t *) &m_tOptIn, (uint16_t) sizeof(struct TTCNetPacketOptIn), m_tNode.nIPAddressBroadcast, TCNET_BROADCAST_PORT_0);
}

void TCNet::Run(void) {
	const char *packet = (char *) &(m_TTCNet.TCNetPacket);
	uint16_t nForeignPort;

#if defined(USE_PORT_UNICAST)
	m_TTCNet.BytesReceived = Network::Get()->RecvFrom(m_aHandles[3], (uint8_t *)packet, (const uint16_t)sizeof(m_TTCNet.TCNetPacket), &m_TTCNet.IPAddressFrom, &nForeignPort) ;

	if (m_TTCNet.BytesReceived != 0) {
		HandlePortUnicastIncoming();
	}
#endif

	m_TTCNet.BytesReceived = Network::Get()->RecvFrom(m_aHandles[1], (uint8_t *)packet, (const uint16_t)sizeof(m_TTCNet.TCNetPacket), &m_TTCNet.IPAddressFrom, &nForeignPort) ;

	if (m_TTCNet.BytesReceived != 0) {
		HandlePort60001Incoming();
	}

	m_TTCNet.BytesReceived = Network::Get()->RecvFrom(m_aHandles[0], (uint8_t *)packet, (const uint16_t)sizeof(m_TTCNet.TCNetPacket), &m_TTCNet.IPAddressFrom, &nForeignPort) ;

	if (m_TTCNet.BytesReceived != 0) {
		HandlePort60000Incoming();
	}

#if defined(USE_PORT_60002)
	m_TTCNet.BytesReceived = Network::Get()->RecvFrom(m_aHandles[2], (uint8_t *)packet, (const uint16_t)sizeof(m_TTCNet.TCNetPacket), &m_TTCNet.IPAddressFrom, &nForeignPort) ;

	if (m_TTCNet.BytesReceived != 0) {
		HandlePort60002Incoming();
	}
#endif

	m_nCurrentMillis = Hardware::Get()->Millis();

	if (__builtin_expect(((m_nCurrentMillis - m_nPreviousMillis) >= 1000), 0)) {
		m_nPreviousMillis = m_nCurrentMillis;
		HandleOptInOutgoing();
	}
}

void TCNet::SetLayer(TTCNetLayers tLayer) {
	if(tLayer >= TCNET_LAYER_UNDEFINED) { // TODO Backward compatibility, subject for removal
		tLayer = TCNET_LAYER_M;
		m_bUseTimeCode = true;
	}

	m_tLayer = tLayer;
	m_pLTime = (uint32_t *)(&m_TTCNet.TCNetPacket.Time.L1Time + (uint32_t) tLayer);
	m_pLTimeCode = (struct TTCNetPacketTimeTimeCode *)((uintptr_t) &m_TTCNet.TCNetPacket.Time.L1TimeCode + (uintptr_t) tLayer * sizeof(struct TTCNetPacketTimeTimeCode));
}

void TCNet::SetNodeName(const char *pNodeName) {
	strncpy((char *)m_tOptIn.ManagementHeader.NodeName, pNodeName, TCNET_NODE_NAME_LENGTH - 1);
}

char TCNet::GetLayerName(TTCNetLayers tLayer) {
	switch (tLayer) {
	case TCNET_LAYER_1:
	case TCNET_LAYER_2:
	case TCNET_LAYER_3:
	case TCNET_LAYER_4:
		return (char) tLayer +  '1';
		break;
	case TCNET_LAYER_A:
		return 'A';
		break;
	case TCNET_LAYER_B:
		return 'B';
		break;
	case TCNET_LAYER_M:
		return 'M';
		break;
	case TCNET_LAYER_C:
		return 'C';
		break;
	default:
		break;
	}

	return ' ';
}

TTCNetLayers TCNet::GetLayer(uint8_t nChar) {
	switch (nChar | 0x20) {
	case '1':
	case '2':
	case '3':
	case '4':
		return (TTCNetLayers) (nChar - '1');
		break;
	case 'A':
		return TCNET_LAYER_A;
		break;
	case 'B':
		return TCNET_LAYER_B;
		break;
	case 'M':
		return TCNET_LAYER_M;
		break;
	case 'C':
		return TCNET_LAYER_C;
		break;
	default:
		break;
	}

	return TCNET_LAYER_UNDEFINED;
}

void TCNet::SetTimeCodeType(TTCNetTimeCodeType tType) {

	switch (tType) {
	case TCNET_TIMECODE_TYPE_FILM:
		m_fTypeDivider = (float) 1000 / 24;
		break;
	case TCNET_TIMECODE_TYPE_EBU_25FPS:
		m_fTypeDivider = 1000 / 25;
		break;
	case TCNET_TIMECODE_TYPE_DF:
		m_fTypeDivider = (float) 1000 / (float) 29.97;
		break;
	case TCNET_TIMECODE_TYPE_SMPTE_30FPS:
		m_fTypeDivider = (float) 1000 / 30;
		break;
	default:
		return;
		break;
	}

	m_tTimeCodeType = tType;
}
