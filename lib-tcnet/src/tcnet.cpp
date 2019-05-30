/**
 * @file tcnet.cpp
 *
 */
/* Copyright (C) 2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#include <assert.h>

#include "tcnet.h"
#include "tcnettimecode.h"

#include "tcnetpackets.h"

#include "hardware.h"
#include "network.h"

#include "debug.h"

#define NODE_NAME_DEFAULT	"AvV-OPi"

enum TNodeBroadcastPorts {
	NODE_BROADCAST_PORT_0 =	60000,
	NODE_BROADCAST_PORT_1 =	60001,
	NODE_BROADCAST_PORT_2 =	60002
};

enum TNodeUnicastPort {
	NODE_UNICAST_PORT = 65023
};

TCNet::TCNet(TTCNetNodeType tNodeType) :
	m_tLayer(TCNET_LAYER_UNDEFINED),
	m_pLTime((uint32_t *)&m_TTCNet.TCNetPacket.Time.LMTime),
	m_pTCNetTimeCode(0),
	m_tTimeCodeType(TCNET_TIMECODE_TYPE_SMPTE_30FPS),
	m_fTypeDivider((float) 1000 / 30)
{
	assert(Hardware::Get() != 0);
	assert(Network::Get() != 0);

	// Fill the static fields for Opt-IN
	m_tOptIn.ManagementHeader.ProtocolVersionMajor = 3;
	m_tOptIn.ManagementHeader.ProtocolVersionMinor = 3;
	memcpy(m_tOptIn.ManagementHeader.Header, "TCN", 3);
	m_tOptIn.ManagementHeader.MessageType = (uint8_t) TCNET_MESSAGE_TYPE_OPTIN;
	SetNodeName((uint8_t *) NODE_NAME_DEFAULT);
	m_tOptIn.ManagementHeader.SEQ = 0;
	m_tOptIn.ManagementHeader.NodeType = (uint8_t) tNodeType;
	m_tOptIn.ManagementHeader.NodeOptions = 0;
	m_tOptIn.NodeCount = 1;
	m_tOptIn.NodeListenerPort = NODE_UNICAST_PORT;
	memcpy((char *)&m_tOptIn.VendorName, Hardware::Get()->GetWebsiteUrl(), TCNET_VENDOR_NAME_LENGTH);
}

TCNet::~TCNet(void) {
}

void TCNet::Start(void) {
	m_tNode.IPAddressLocal = Network::Get()->GetIp();
	m_tNode.IPAddressBroadcast = m_tNode.IPAddressLocal | ~(Network::Get()->GetNetmask());

	m_aHandles[0] = Network::Get()->Begin(NODE_BROADCAST_PORT_0);
	assert(m_aHandles[0] >= 0);

	m_aHandles[1] = Network::Get()->Begin(NODE_BROADCAST_PORT_1);
	assert(m_aHandles[1] >= 0);

#if defined(USE_PORT_60002)
	m_aHandles[2] = Network::Get()->Begin(NODE_BROADCAST_PORT_2);
	assert(m_aHandles[2] >= 0);
#endif

#if defined(USE_PORT_UNICAST)
	m_aHandles[3] = Network::Get()->Begin(NODE_UNICAST_PORT);
	assert(m_aHandles[3] >= 0);
#endif
}

void TCNet::Stop(void) {
#if defined(USE_PORT_UNICAST)
	Network::Get()->End(NODE_UNICAST_PORT);
#endif
#if defined(USE_PORT_60002)
	Network::Get()->End(NODE_BROADCAST_PORT_2);
#endif
	Network::Get()->End(NODE_BROADCAST_PORT_1);
	Network::Get()->End(NODE_BROADCAST_PORT_0);

}

void TCNet::HandlePort60000Incoming(void) {
	DEBUG_ENTRY

	const struct TTCNetPacket *packet = &(m_TTCNet.TCNetPacket);
	const TTCNetMessageType type  = (TTCNetMessageType) packet->OptIn.ManagementHeader.MessageType;

	DEBUG_PRINTF("MessageType = %d", (int) type);

	if (type == TCNET_MESSAGE_TYPE_STATUS) {
		DEBUG_PRINTF("m_tTimeCodeType=%d", (int ) m_tTimeCodeType);

		if (m_tTimeCodeType != (TTCNetTimeCodeType) packet->Status.SMPTEMode) {
			m_tTimeCodeType = (TTCNetTimeCodeType) packet->Status.SMPTEMode;

			switch (m_tTimeCodeType) {
			case TCNET_TIMECODE_TYPE_FILM:
				m_fTypeDivider = (float) 1000 / 24;
				break;
			case TCNET_TIMECODE_TYPE_EBU_25FPS:
				m_fTypeDivider = (float) 1000 / 25;
				break;
			case TCNET_TIMECODE_TYPE_DF:
				m_fTypeDivider = (float) 1000 / (float) 29.97;
				break;
			case TCNET_TIMECODE_TYPE_SMPTE_30FPS:
				m_fTypeDivider = (float) 1000 / 30;
				break;
			default:
				break;
			}
		}

		return;
	}

	if (type != TCNET_MESSAGE_TYPE_OPTIN) {
		DEBUG_EXIT
		return;
	}

	DEBUG_EXIT
}

void TCNet::HandlePort60001Incoming(void) {
	if (__builtin_expect((m_pTCNetTimeCode != 0), 1)) {
		const struct TTCNetPacket *packet = &(m_TTCNet.TCNetPacket);

		if ((TTCNetMessageType) packet->ManagementHeader.MessageType == TCNET_MESSAGE_TYPE_TIME) {
			struct TTCNetTimeCode TimeCode;

			if (m_tLayer != TCNET_LAYER_UNDEFINED) {
				uint32_t nTime = *m_pLTime;

				const uint32_t hours = nTime / (uint32_t) 3600000;
				nTime -= hours * (uint32_t) 3600000;
				const uint32_t minutes = nTime / (uint32_t) 60000;
				nTime -= minutes * (uint32_t) 60000;
				const uint32_t seconds = nTime / (uint32_t) 1000;
				const uint32_t millis = nTime - seconds * (uint32_t) 1000;
				const uint32_t frames = (float) millis / m_fTypeDivider;

				TimeCode.nHours = hours;
				TimeCode.nMinutes = minutes;
				TimeCode.nSeconds = seconds;
				TimeCode.nFrames = frames;
			} else {
				TimeCode.nHours = packet->Time.TimeCodeHours;
				TimeCode.nMinutes = packet->Time.TimeCodeMinutes;
				TimeCode.nSeconds = packet->Time.TimeCodeSeconds;
				TimeCode.nFrames = packet->Time.TimeCodeFrames;
			}

			TimeCode.nType = m_tTimeCodeType;

			m_pTCNetTimeCode->Handler(&TimeCode);
		}
	}
}

void TCNet::HandlePort60002Incoming(void) {
	DEBUG_ENTRY

	DEBUG_EXIT
}

void TCNet::HandlePortUnicastIncoming(void) {

}

void TCNet::HandleOptInOutgoing(void) {
	//DEBUG_ENTRY

	m_tOptIn.ManagementHeader.SEQ += 1;
	m_tOptIn.ManagementHeader.TimeStamp = Hardware::Get()->Micros();
	m_tOptIn.Uptime = Hardware::Get()->GetUpTime();

	Network::Get()->SendTo(m_aHandles[0],  (const uint8_t *) &m_tOptIn, (uint16_t) sizeof(struct TOptIn), m_tNode.IPAddressBroadcast, (uint16_t) NODE_BROADCAST_PORT_0);

	//DEBUG_EXIT
}

int TCNet::Run(void) {
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
		HandleOptInOutgoing();
		m_nPreviousMillis = m_nCurrentMillis;
	}

	return 0;
}

void TCNet::SetLayer(TTCNetLayers tLayer) {
	assert((int) tLayer <= (int )TCNET_LAYER_UNDEFINED);

	m_tLayer = tLayer;
	m_pLTime = (uint32_t *)(&m_TTCNet.TCNetPacket.Time.L1Time + (uint32_t) tLayer);
}

void TCNet::SetNodeName(uint8_t *pNodeName) {
	strncpy((char *)m_tOptIn.ManagementHeader.NodeName, (const char *)pNodeName, TCNET_NODE_NAME_LENGTH);
}

char TCNet::GetLayerName(TTCNetLayers tLayer) {
	switch (tLayer) {
	case TCNET_LAYER_1:
	case TCNET_LAYER_2:
	case TCNET_LAYER_3:
	case TCNET_LAYER_4:
		return (char) TCNET_LAYER_1 +  '1';
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

	__builtin_unreachable ();
	return ' ';
}

void TCNet::SetTimeCodeType(TTCNetTimeCodeType tType) {
	m_tTimeCodeType = tType;
	m_bIsSetTimeCodeType = true;

	switch (m_tTimeCodeType) {
	case TCNET_TIMECODE_TYPE_FILM:
		m_fTypeDivider = (float) 1000 / 24;
		break;
	case TCNET_TIMECODE_TYPE_EBU_25FPS:
		m_fTypeDivider = (float) 1000 / 25;
		break;
	case TCNET_TIMECODE_TYPE_DF:
		m_fTypeDivider = (float) 1000 / (float) 29.97;
		break;
	case TCNET_TIMECODE_TYPE_SMPTE_30FPS:
		m_fTypeDivider = (float) 1000 / 30;
		break;
	default:
		m_fTypeDivider = (float) 1000 / 30;
		m_tTimeCodeType = TCNET_TIMECODE_TYPE_SMPTE_30FPS;
		m_bIsSetTimeCodeType = false;
		break;
	}
}
