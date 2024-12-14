/**
 * @file applemidi.cpp
 *
 */
/* Copyright (C) 2019-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include <cstdint>
#include <cstring>
#include <cstdio>
#include <cassert>

#ifndef ALIGNED
 #define ALIGNED __attribute__ ((aligned (4)))
#endif

#include "applemidi.h"

#include "midi.h"

#include "hardware.h"
#include "network.h"

#include "debug.h"

namespace applemidi {

}  // namespace applemidi

enum TAppleMidiCommand {
	APPLEMIDI_COMMAND_INVITATION = __builtin_bswap16(0x494e),			///< Invitation 'IN'
	APPLEMIDI_COMMAND_INVITATION_ACCEPTED = __builtin_bswap16(0x4f4b),	///< Invitation accepted 'OK'
	APPLEMIDI_COMMAND_INVITATION_REJECTED = __builtin_bswap16(0x4e4f),	///< Invitation refused 'NO'
	APPLEMIDI_COMMAND_ENDSESSION = __builtin_bswap16(0x4259),			///< Closing session 'BY'
	APPLEMIDI_COMMAND_SYNCHRONIZATION = __builtin_bswap16(0x434b),		///< Clock synchronization 'CK'
	APPLEMIDI_COMMAND_RECEIVER_FEEDBACK = __builtin_bswap16(0x5253),	///< Journalling synchronization 'RS'
	APPLEMIDI_COMMAND_BITRATE_RECEIVE_LIMIT = __builtin_bswap16(0x524c)	///< Bitrate 'RL'
};

struct TTimestampSynchronization {
	uint16_t nSignature;
	uint16_t nCommand;
	uint32_t nSSRC;
	uint8_t nCount;
	uint8_t padding[3];
	uint64_t nTimestamps[3];
}__attribute__((packed));

AppleMidi::AppleMidi() : m_nSSRC(Network::Get()->GetIp()), m_nExchangePacketReplySize(applemidi::EXCHANGE_PACKET_MIN_LENGTH) {
	DEBUG_ENTRY

	m_ExchangePacketReply.nSignature = SIGNATURE;
	m_ExchangePacketReply.nProtocolVersion = __builtin_bswap32(applemidi::VERSION);
	m_ExchangePacketReply.nSSRC = m_nSSRC;

	SetSessionName(Network::Get()->GetHostName());

	memset(&m_SessionStatus, 0, sizeof (struct applemidi::SessionStatus));

	DEBUG_PRINTF("applemidi::EXCHANGE_PACKET_MIN_LENGTH = %u", static_cast<uint32_t>(applemidi::EXCHANGE_PACKET_MIN_LENGTH));
	DEBUG_EXIT
}

void AppleMidi::HandleControlMessage() {
	DEBUG_ENTRY
	assert(m_pBuffer != nullptr);

	auto *pPacket = reinterpret_cast<struct applemidi::ExchangePacket*>(m_pBuffer);

	debug_dump(m_pBuffer, m_nBytesReceived);
	DEBUG_PRINTF("Command: %.4x, m_nSessionState=%u", pPacket->nCommand, static_cast<uint32_t>(m_SessionStatus.sessionState));

	if (m_SessionStatus.sessionState == applemidi::SessionState::WAITING_IN_CONTROL) {
		DEBUG_PUTS("SESSION_STATE_WAITING_IN_CONTROL");

		if ((m_SessionStatus.nRemoteIp == 0) && (pPacket->nCommand == APPLEMIDI_COMMAND_INVITATION)) {
 			DEBUG_PUTS("Invitation");

			m_ExchangePacketReply.nCommand = APPLEMIDI_COMMAND_INVITATION_ACCEPTED;
			m_ExchangePacketReply.nInitiatorToken = pPacket->nInitiatorToken;

			Network::Get()->SendTo(m_nHandleControl, &m_ExchangePacketReply, m_nExchangePacketReplySize, m_nRemoteIp, m_nRemotePort);

			debug_dump(&m_ExchangePacketReply, m_nExchangePacketReplySize);

			m_SessionStatus.sessionState = applemidi::SessionState::WAITING_IN_MIDI;
			m_SessionStatus.nRemoteIp = m_nRemoteIp;

			return;
		} else {
			return;
		}
	}

	if (m_SessionStatus.sessionState == applemidi::SessionState::ESTABLISHED) {
		DEBUG_PUTS("SESSION_STATE_ESTABLISHED");

		if ((m_SessionStatus.nRemoteIp == m_nRemoteIp) && (pPacket->nCommand == APPLEMIDI_COMMAND_ENDSESSION)) {
			m_SessionStatus.sessionState = applemidi::SessionState::WAITING_IN_CONTROL;
			m_SessionStatus.nRemoteIp = 0;
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
	assert(m_pBuffer != nullptr);

	debug_dump(m_pBuffer, m_nBytesReceived);

	if (*reinterpret_cast<uint16_t *>(m_pBuffer) == 0x6180) {
		HandleRtpMidi(m_pBuffer);
		return;
	}

	if (m_nBytesReceived >= applemidi::EXCHANGE_PACKET_MIN_LENGTH) {

		if (*reinterpret_cast<uint16_t *>(m_pBuffer) == SIGNATURE) {

			if (m_SessionStatus.sessionState == applemidi::SessionState::WAITING_IN_MIDI) {
				DEBUG_PUTS("SESSION_STATE_WAITING_IN_MIDI");

				auto *pPacket = reinterpret_cast<struct applemidi::ExchangePacket*>(m_pBuffer);

				DEBUG_PRINTF("Command: %.4x", pPacket->nCommand);

				if (pPacket->nCommand == APPLEMIDI_COMMAND_INVITATION) {
					DEBUG_PUTS("Invitation");

					m_ExchangePacketReply.nCommand = APPLEMIDI_COMMAND_INVITATION_ACCEPTED;
					m_ExchangePacketReply.nInitiatorToken = pPacket->nInitiatorToken;

					Network::Get()->SendTo(m_nHandleMidi, &m_ExchangePacketReply, m_nExchangePacketReplySize, m_nRemoteIp, m_nRemotePort);

					m_SessionStatus.sessionState = applemidi::SessionState::ESTABLISHED;
					m_SessionStatus.nRemotePortMidi = m_nRemotePort;
					m_SessionStatus.nSynchronizationTimestamp = Hardware::Get()->Millis();
				}

				return;
			}

			if (m_SessionStatus.sessionState == applemidi::SessionState::ESTABLISHED) {
				DEBUG_PUTS("SESSION_STATE_ESTABLISHED");

				auto *pPacket = reinterpret_cast<struct applemidi::ExchangePacket*>(m_pBuffer);

				if (pPacket->nCommand == APPLEMIDI_COMMAND_SYNCHRONIZATION) {
					DEBUG_PUTS("Timestamp Synchronization");
					auto *t = reinterpret_cast<struct TTimestampSynchronization*>(m_pBuffer);

					m_SessionStatus.nSynchronizationTimestamp = Hardware::Get()->Millis();

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
