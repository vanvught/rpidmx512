/**
 * @file applemidi.cpp
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

/*
 * https://developer.apple.com/library/archive/documentation/Audio/Conceptual/MIDINetworkDriverProtocol/MIDI/MIDI.html
 */

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#ifndef ALIGNED
 #define ALIGNED __attribute__ ((aligned (4)))
#endif

#include "applemidi.h"

#include "midi.h"
#include "mdnsservices.h"

#include "hardware.h"
#include "network.h"

#include "debug.h"

static const uint8_t aSignature[] ALIGNED           = { 0xff, 0xff };
static const uint8_t aInvitation[] ALIGNED          = { 'I', 'N' };
static const uint8_t aInvitationAccepted[] ALIGNED  = { 'O', 'K' };
static const uint8_t aInvitationRejected[] ALIGNED  = { 'N', 'O' };
static const uint8_t aEndSession[] ALIGNED          = { 'B', 'Y' };
static const uint8_t aSyncronization[] ALIGNED      = { 'C', 'K' };
//static const uint8_t aReceiverFeedback[] ALIGNED    = { 'R', 'S' };
//static const uint8_t aBitrateReceiveLimit[] ALIGNED = { 'R', 'L' };

#define BUFFER_SIZE	512

enum TSessionState {
	SESSION_STATE_WAITING_IN_CONTROL,
	SESSION_STATE_WAITING_IN_MIDI,
	SESSION_STATE_ESTABLISHED
};

struct TTimestampSynchronization {
	uint8_t aSignature[2];
	uint8_t aCommand[2];
	uint32_t nSSRC;
	uint8_t nCount;
	uint8_t padding[3];
	uint64_t nTimestamps[3];
}__attribute__((packed));

#define APPLE_MIDI_EXCHANGE_PACKET_MIN_LENGTH	(sizeof(struct TExchangePacket) - APPLE_MIDI_SESSION_NAME_LENGTH_MAX - 1)

AppleMidi::AppleMidi(void) :
	m_nPort(APPLE_MIDI_UPD_PORT_CONTROL_DEFAULT),
	m_nHandleControl(-1),
	m_nHandleMidi(-1),
	m_pBuffer(0),
	m_nRemoteIp(0),
	m_nRemotePort(0),
	m_nBytesReceived(0),
	m_nExchangePacketReplySize(APPLE_MIDI_EXCHANGE_PACKET_MIN_LENGTH)
{

	memset(&m_ExchangePacketReply, 0, sizeof(struct TExchangePacket));

	memcpy(&m_ExchangePacketReply.aSignature, aSignature, sizeof(m_ExchangePacketReply.aSignature));
	m_ExchangePacketReply.nProtocolVersion = __builtin_bswap32(APPLE_MIDI_VERSION);

	m_pBuffer = new uint8_t[BUFFER_SIZE];
	assert(m_pBuffer != 0);

	memset(&m_tSessionStatus, 0, sizeof (struct TSessionStatus));

	DEBUG_PRINTF("APPLE_MIDI_EXCHANGE_PACKET_MIN_LENGTH = %d", (int) APPLE_MIDI_EXCHANGE_PACKET_MIN_LENGTH);
}

AppleMidi::~AppleMidi(void) {
}

void AppleMidi::Start(void) {
	DEBUG_ENTRY

	MDNS::Start();
	MDNS::AddServiceRecord(0, MDNS_SERVICE_MIDI, m_nPort);

	m_nHandleControl = Network::Get()->Begin(m_nPort);
	assert(m_nHandleControl != -1);

	m_nHandleMidi = Network::Get()->Begin(m_nPort + 1);
	assert(m_nHandleMidi != -1);

	if (m_ExchangePacketReply.aName[0] == 0) {
		SetSessionName((const uint8_t *) Network::Get()->GetHostName());
	}

	DEBUG_PRINTF("Session name: [%s]", m_ExchangePacketReply.aName);

	DEBUG_EXIT
}

void AppleMidi::Stop(void) {
	DEBUG_ENTRY

	MDNS::Stop();
	Network::Get()->End(m_nPort + 1);
	Network::Get()->End(m_nPort);

	DEBUG_EXIT
}

void AppleMidi::SetPort(uint16_t nPort) {
	assert(nPort > 1024);

	m_nPort = nPort;
}

void AppleMidi::SetSessionName(const uint8_t *pSessionName) {
	strncpy((char *)&m_ExchangePacketReply.aName, (const char *)pSessionName, APPLE_MIDI_SESSION_NAME_LENGTH_MAX);
	m_ExchangePacketReply.aName[APPLE_MIDI_SESSION_NAME_LENGTH_MAX] = '\0';
	m_nExchangePacketReplySize = APPLE_MIDI_EXCHANGE_PACKET_MIN_LENGTH + 1 + strlen((const char *)m_ExchangePacketReply.aName);
}

void AppleMidi::HandleControlMessage(void) {
	DEBUG_ENTRY

	struct TExchangePacket *pPacket = (struct TExchangePacket *) m_pBuffer;

	debug_dump((void *)m_pBuffer, m_nBytesReceived);
	DEBUG_PRINTF("Command: %c%c, m_nSessionState=%d", pPacket->aCommand[0], pPacket->aCommand[1], (int) m_tSessionStatus.nSessionState);

	if (m_tSessionStatus.nSessionState == SESSION_STATE_WAITING_IN_CONTROL) {
		DEBUG_PUTS("SESSION_STATE_WAITING_IN_CONTROL");

		if ((m_tSessionStatus.nRemoteIp == 0) && (memcmp(pPacket->aCommand, aInvitation, 2) == 0)) {
 			DEBUG_PUTS("Invitation");

			memcpy((void *)&m_ExchangePacketReply.aCommand, (const void *)aInvitationAccepted, sizeof(aInvitationAccepted));
			m_ExchangePacketReply.nInitiatorToken = pPacket->nInitiatorToken;

			debug_dump((void *)&m_ExchangePacketReply, m_nExchangePacketReplySize);
			Network::Get()->SendTo(m_nHandleControl, (const uint8_t *) &m_ExchangePacketReply, m_nExchangePacketReplySize, m_nRemoteIp, m_nRemotePort);

			m_tSessionStatus.nSessionState = SESSION_STATE_WAITING_IN_MIDI;
			m_tSessionStatus.nRemoteIp = m_nRemoteIp;

			return;
		} else {
			return;
		}
	}

	if (m_tSessionStatus.nSessionState == SESSION_STATE_ESTABLISHED) {
		DEBUG_PUTS("SESSION_STATE_ESTABLISHED");

		if ((m_tSessionStatus.nRemoteIp == m_nRemoteIp) && (memcmp(pPacket->aCommand, aEndSession, 2) == 0)) {
			m_tSessionStatus.nSessionState = SESSION_STATE_WAITING_IN_CONTROL;
			m_tSessionStatus.nRemoteIp = 0;
			DEBUG_PUTS("End Session");

			return;
		}

		if (memcmp(pPacket->aCommand, aInvitation, 2) == 0) {
		 	DEBUG_PUTS("Invitation rejected");

			memcpy((void *)&m_ExchangePacketReply.aCommand, (const void *)aInvitationRejected, sizeof(aInvitationRejected));
			m_ExchangePacketReply.nInitiatorToken = pPacket->nInitiatorToken;

			debug_dump((void *)&m_ExchangePacketReply, m_nExchangePacketReplySize);
			Network::Get()->SendTo(m_nHandleControl, (const uint8_t *) &m_ExchangePacketReply, m_nExchangePacketReplySize, m_nRemoteIp, m_nRemotePort);

			return;
		}
	}

	DEBUG_EXIT
}

void AppleMidi::HandleMidiMessage(void) {
	DEBUG_ENTRY

	debug_dump((void *)m_pBuffer, m_nBytesReceived);

	if (*(uint16_t*) m_pBuffer == 0x6180) {
		HandleRtpMidi((const uint8_t *)m_pBuffer);
		return;
	}

	if (m_nBytesReceived >= APPLE_MIDI_EXCHANGE_PACKET_MIN_LENGTH) {

		if (memcmp(m_pBuffer, aSignature, sizeof(aSignature)) == 0) {

			if (m_tSessionStatus.nSessionState == SESSION_STATE_WAITING_IN_MIDI) {
				DEBUG_PUTS("SESSION_STATE_WAITING_IN_MIDI");

				struct TExchangePacket *pPacket = (struct TExchangePacket *) m_pBuffer;

				DEBUG_PRINTF("Command: %c%c", pPacket->aCommand[0], pPacket->aCommand[1]);

				if (memcmp(pPacket->aCommand, aInvitation, 2) == 0) {
					DEBUG_PUTS("Invitation");

					memcpy((void *)&m_ExchangePacketReply.aCommand, (const void *)aInvitationAccepted, sizeof(aInvitationAccepted));
					m_ExchangePacketReply.nInitiatorToken = pPacket->nInitiatorToken;

					Network::Get()->SendTo(m_nHandleMidi, (const uint8_t *) &m_ExchangePacketReply, m_nExchangePacketReplySize, m_nRemoteIp, m_nRemotePort);

					m_tSessionStatus.nSessionState = SESSION_STATE_ESTABLISHED;
					m_tSessionStatus.nRemotePortMidi = m_nRemotePort;
				}

				return;
			}

			if (m_tSessionStatus.nSessionState == SESSION_STATE_ESTABLISHED) {
				DEBUG_PUTS("SESSION_STATE_ESTABLISHED");

				struct TExchangePacket *pPacket = (struct TExchangePacket *) m_pBuffer;

				if (memcmp(pPacket->aCommand, aSyncronization, 2) == 0) {
					DEBUG_PUTS("Timestamp Synchronization");
					struct TTimestampSynchronization *t = (struct TTimestampSynchronization*) m_pBuffer;

					if (t->nCount == 0) {
						t->nSSRC = 0;
						t->nCount = 1;
						t->nTimestamps[1] = __builtin_bswap64(Hardware::Get()->Micros() * 100);
						t->nTimestamps[2] = 0;

						Network::Get()->SendTo(m_nHandleMidi, (const uint8_t*) m_pBuffer, sizeof(struct TTimestampSynchronization), m_nRemoteIp, m_nRemotePort);
					}
				}
			}
		}
	}

	DEBUG_EXIT
}

void AppleMidi::Run(void) {

	m_nBytesReceived = Network::Get()->RecvFrom(m_nHandleMidi, m_pBuffer, BUFFER_SIZE, &m_nRemoteIp, &m_nRemotePort);

	if ((m_tSessionStatus.nRemoteIp == m_nRemoteIp) && (m_nBytesReceived >= 12)) {
		HandleMidiMessage();
	}

	m_nBytesReceived = Network::Get()->RecvFrom(m_nHandleControl, m_pBuffer, BUFFER_SIZE, &m_nRemoteIp, &m_nRemotePort);

	if (__builtin_expect((m_nBytesReceived >= APPLE_MIDI_EXCHANGE_PACKET_MIN_LENGTH), 0)) {
		if (memcmp(m_pBuffer, aSignature, sizeof(aSignature)) == 0) {
			HandleControlMessage();
		}
	}

	MDNS::Run();
}

void AppleMidi::HandleRtpMidi(const uint8_t *pBuffer) {
	// override
}

void AppleMidi::Print(void) {
	MDNS::Print();

	printf("AppleMIDI configuration\n");
	printf(" Session name : %s\n", m_ExchangePacketReply.aName);
}
