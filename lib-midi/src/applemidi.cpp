/**
 * @file applemidi.cpp
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

/*
 * https://developer.apple.com/library/archive/documentation/Audio/Conceptual/MIDINetworkDriverProtocol/MIDI/MIDI.html
 */

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <cassert>

#ifndef ALIGNED
 #define ALIGNED __attribute__ ((aligned (4)))
#endif

#include "applemidi.h"

#include "midi.h"
#include "mdnsservices.h"

#include "hardware.h"
#include "network.h"

#include "debug.h"

enum TAppleMidiSignature {
	APPLEMIDI_SIGNATURE = 0xffff
};

enum TAppleMidiCommand {
	APPLEMIDI_COMMAND_INVITATION = __builtin_bswap16(0x494e),			///< Invitation 'IN'
	APPLEMIDI_COMMAND_INVITATION_ACCEPTED = __builtin_bswap16(0x4f4b),	///< Invitation accepted 'OK'
	APPLEMIDI_COMMAND_INVITATION_REJECTED = __builtin_bswap16(0x4e4f),	///< Invitation refused 'NO'
	APPLEMIDI_COMMAND_ENDSESSION = __builtin_bswap16(0x4259),			///< Closing session 'BY'
	APPLEMIDI_COMMAND_SYNCHRONIZATION = __builtin_bswap16(0x434b),		///< Clock synchronization 'CK'
	APPLEMIDI_COMMAND_RECEIVER_FEEDBACK = __builtin_bswap16(0x5253),	///< Journalling synchronization 'RS'
	APPLEMIDI_COMMAND_BITRATE_RECEIVE_LIMIT = __builtin_bswap16(0x524c)	///< Bitrate 'RL'
};

#define BUFFER_SIZE	512

struct TTimestampSynchronization {
	uint16_t nSignature;
	uint16_t nCommand;
	uint32_t nSSRC;
	uint8_t nCount;
	uint8_t padding[3];
	uint64_t nTimestamps[3];
}__attribute__((packed));

constexpr uint16_t APPLE_MIDI_EXCHANGE_PACKET_MIN_LENGTH = sizeof(struct TExchangePacket) - APPLE_MIDI_SESSION_NAME_LENGTH_MAX - 1;

AppleMidi::AppleMidi() :
	
	m_nSSRC(Network::Get()->GetIp()),
	
	m_nExchangePacketReplySize(APPLE_MIDI_EXCHANGE_PACKET_MIN_LENGTH)
{
	m_ExchangePacketReply.nSignature = APPLEMIDI_SIGNATURE;
	m_ExchangePacketReply.nProtocolVersion = __builtin_bswap32(APPLE_MIDI_VERSION);
	m_ExchangePacketReply.nSSRC = m_nSSRC;

	SetSessionName(Network::Get()->GetHostName());

	m_pBuffer = new uint8_t[BUFFER_SIZE];
	assert(m_pBuffer != nullptr);

	memset(&m_tSessionStatus, 0, sizeof (struct TSessionStatus));

	DEBUG_PRINTF("APPLE_MIDI_EXCHANGE_PACKET_MIN_LENGTH = %d", APPLE_MIDI_EXCHANGE_PACKET_MIN_LENGTH);
}

AppleMidi::~AppleMidi() {
	Stop();
}

void AppleMidi::Start() {
	DEBUG_ENTRY

	MDNS::Start();
	MDNS::AddServiceRecord(nullptr, MDNS_SERVICE_MIDI, m_nPort);

	m_nHandleControl = Network::Get()->Begin(m_nPort);
	assert(m_nHandleControl != -1);

	m_nHandleMidi = Network::Get()->Begin(m_nPort + 1);
	assert(m_nHandleMidi != -1);

	DEBUG_PRINTF("Session name: [%s]", m_ExchangePacketReply.aName);

	m_nStartTime = Hardware::Get()->Millis();

	DEBUG_EXIT
}

void AppleMidi::Stop() {
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

void AppleMidi::SetSessionName(const char *pSessionName) {
	strncpy(reinterpret_cast<char*>(&m_ExchangePacketReply.aName), pSessionName, APPLE_MIDI_SESSION_NAME_LENGTH_MAX);
	m_ExchangePacketReply.aName[APPLE_MIDI_SESSION_NAME_LENGTH_MAX] = '\0';
	m_nExchangePacketReplySize = APPLE_MIDI_EXCHANGE_PACKET_MIN_LENGTH + 1 + strlen(reinterpret_cast<const char*>(m_ExchangePacketReply.aName));
}

void AppleMidi::HandleControlMessage() {
	DEBUG_ENTRY

	auto *pPacket = reinterpret_cast<struct TExchangePacket*>(m_pBuffer);

	debug_dump(m_pBuffer, m_nBytesReceived);
	DEBUG_PRINTF("Command: %.4x, m_nSessionState=%d", pPacket->nCommand, m_tSessionStatus.tSessionState);

	if (m_tSessionStatus.tSessionState == SESSION_STATE_WAITING_IN_CONTROL) {
		DEBUG_PUTS("SESSION_STATE_WAITING_IN_CONTROL");

		if ((m_tSessionStatus.nRemoteIp == 0) && (pPacket->nCommand == APPLEMIDI_COMMAND_INVITATION)) {
 			DEBUG_PUTS("Invitation");

			m_ExchangePacketReply.nCommand = APPLEMIDI_COMMAND_INVITATION_ACCEPTED;
			m_ExchangePacketReply.nInitiatorToken = pPacket->nInitiatorToken;

			Network::Get()->SendTo(m_nHandleControl, &m_ExchangePacketReply, m_nExchangePacketReplySize, m_nRemoteIp, m_nRemotePort);

			debug_dump(&m_ExchangePacketReply, m_nExchangePacketReplySize);

			m_tSessionStatus.tSessionState = SESSION_STATE_WAITING_IN_MIDI;
			m_tSessionStatus.nRemoteIp = m_nRemoteIp;

			return;
		} else {
			return;
		}
	}

	if (m_tSessionStatus.tSessionState == SESSION_STATE_ESTABLISHED) {
		DEBUG_PUTS("SESSION_STATE_ESTABLISHED");

		if ((m_tSessionStatus.nRemoteIp == m_nRemoteIp) && (pPacket->nCommand == APPLEMIDI_COMMAND_ENDSESSION)) {
			m_tSessionStatus.tSessionState = SESSION_STATE_WAITING_IN_CONTROL;
			m_tSessionStatus.nRemoteIp = 0;
			DEBUG_PUTS("End Session");

			return;
		}

		if (pPacket->nCommand == APPLEMIDI_COMMAND_INVITATION) {
		 	DEBUG_PUTS("Invitation rejected");

			m_ExchangePacketReply.nCommand = APPLEMIDI_COMMAND_INVITATION_REJECTED;
			m_ExchangePacketReply.nInitiatorToken = pPacket->nInitiatorToken;

			Network::Get()->SendTo(m_nHandleControl, &m_ExchangePacketReply, m_nExchangePacketReplySize, m_nRemoteIp, m_nRemotePort);

			debug_dump(&m_ExchangePacketReply, m_nExchangePacketReplySize);

			return;
		}
	}

	DEBUG_EXIT
}

void AppleMidi::HandleMidiMessage() {
	DEBUG_ENTRY

	debug_dump(m_pBuffer, m_nBytesReceived);

	if (*reinterpret_cast<uint16_t*>(m_pBuffer) == 0x6180) {
		HandleRtpMidi(m_pBuffer);
		return;
	}

	if (m_nBytesReceived >= APPLE_MIDI_EXCHANGE_PACKET_MIN_LENGTH) {

		if (*reinterpret_cast<uint16_t*>(m_pBuffer) == APPLEMIDI_SIGNATURE) {

			if (m_tSessionStatus.tSessionState == SESSION_STATE_WAITING_IN_MIDI) {
				DEBUG_PUTS("SESSION_STATE_WAITING_IN_MIDI");

				auto *pPacket = reinterpret_cast<struct TExchangePacket*>(m_pBuffer);

				DEBUG_PRINTF("Command: %.4x", pPacket->nCommand);

				if (pPacket->nCommand == APPLEMIDI_COMMAND_INVITATION) {
					DEBUG_PUTS("Invitation");

					m_ExchangePacketReply.nCommand = APPLEMIDI_COMMAND_INVITATION_ACCEPTED;
					m_ExchangePacketReply.nInitiatorToken = pPacket->nInitiatorToken;

					Network::Get()->SendTo(m_nHandleMidi, &m_ExchangePacketReply, m_nExchangePacketReplySize, m_nRemoteIp, m_nRemotePort);

					m_tSessionStatus.tSessionState = SESSION_STATE_ESTABLISHED;
					m_tSessionStatus.nRemotePortMidi = m_nRemotePort;
					m_tSessionStatus.nSynchronizationTimestamp = Hardware::Get()->Millis();
				}

				return;
			}

			if (m_tSessionStatus.tSessionState == SESSION_STATE_ESTABLISHED) {
				DEBUG_PUTS("SESSION_STATE_ESTABLISHED");

				auto *pPacket = reinterpret_cast<struct TExchangePacket*>(m_pBuffer);

				if (pPacket->nCommand == APPLEMIDI_COMMAND_SYNCHRONIZATION) {
					DEBUG_PUTS("Timestamp Synchronization");
					auto *t = reinterpret_cast<struct TTimestampSynchronization*>(m_pBuffer);

					m_tSessionStatus.nSynchronizationTimestamp = Hardware::Get()->Millis();

					if (t->nCount == 0) {
						t->nSSRC = m_nSSRC;
						t->nCount = 1;
						t->nTimestamps[1] = __builtin_bswap64(Now());

						Network::Get()->SendTo(m_nHandleMidi, m_pBuffer, sizeof(struct TTimestampSynchronization), m_nRemoteIp, m_nRemotePort);
					} else if (t->nCount == 1) {
						t->nSSRC = m_nSSRC;
						t->nCount = 2;
						t->nTimestamps[2] = __builtin_bswap64(Now());

						Network::Get()->SendTo(m_nHandleMidi, m_pBuffer, sizeof(struct TTimestampSynchronization), m_nRemoteIp, m_nRemotePort);
					} else if (t->nCount == 2) {
						t->nSSRC = m_nSSRC;
						t->nCount = 0;
						t->nTimestamps[0] = __builtin_bswap64(Now());
						t->nTimestamps[1] = 0;
						t->nTimestamps[2] = 0;

						Network::Get()->SendTo(m_nHandleMidi, m_pBuffer, sizeof(struct TTimestampSynchronization), m_nRemoteIp, m_nRemotePort);
					}
				}
			}
		}
	}

	DEBUG_EXIT
}

void AppleMidi::Run() {

	m_nBytesReceived = Network::Get()->RecvFrom(m_nHandleMidi, m_pBuffer, BUFFER_SIZE, &m_nRemoteIp, &m_nRemotePort);

	if (__builtin_expect((m_nBytesReceived >= 12), 0)) {
		if (m_tSessionStatus.nRemoteIp == m_nRemoteIp) {
			HandleMidiMessage();
		}
	}

	m_nBytesReceived = Network::Get()->RecvFrom(m_nHandleControl, m_pBuffer, BUFFER_SIZE, &m_nRemoteIp, &m_nRemotePort);

	if (__builtin_expect((m_nBytesReceived >= APPLE_MIDI_EXCHANGE_PACKET_MIN_LENGTH), 0)) {
		if (*reinterpret_cast<uint16_t*>(m_pBuffer) == APPLEMIDI_SIGNATURE) {
			HandleControlMessage();
		}
	}

	if (m_tSessionStatus.tSessionState == SESSION_STATE_ESTABLISHED) {
		if (__builtin_expect((Hardware::Get()->Millis() - m_tSessionStatus.nSynchronizationTimestamp > (90 * 1000)), 0)) {
			m_tSessionStatus.tSessionState = SESSION_STATE_WAITING_IN_CONTROL;
			m_tSessionStatus.nRemoteIp = 0;
			DEBUG_PUTS("End Session {time-out}");
		}
	}

	MDNS::Run();
}

void AppleMidi::HandleRtpMidi(__attribute__((unused)) const uint8_t *pBuffer) {
	// override
}

uint32_t AppleMidi::Now() {
	const uint32_t nElapsed = Hardware::Get()->Millis() - m_nStartTime;

	return (nElapsed * 10);
}

bool AppleMidi::Send(const uint8_t *pBuffer, uint32_t nLength) {
	if (m_tSessionStatus.tSessionState != SESSION_STATE_ESTABLISHED) {
		return false;
	}

	Network::Get()->SendTo(m_nHandleMidi, pBuffer, nLength, m_tSessionStatus.nRemoteIp, m_tSessionStatus.nRemotePortMidi);

	debug_dump(&pBuffer, nLength);

	return true;
}


void AppleMidi::Print() {
	MDNS::Print();
	
	const uint32_t nSSRC = __builtin_bswap32(m_nSSRC);

	printf("AppleMIDI\n");
	printf(" SSRC    : %x (%u)\n", nSSRC, nSSRC);
	printf(" Session : %s\n", m_ExchangePacketReply.aName);
}
