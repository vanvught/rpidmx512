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

#if defined (DEBUG_NET_APPLEMIDI)
# undef NDEBUG
#endif

#if defined(__GNUC__) && !defined(__clang__)
# pragma GCC push_options
# pragma GCC optimize ("O2")
# pragma GCC optimize ("no-tree-loop-distribute-patterns")
#endif

#include <cstdint>
#include <cstring>
#include <cstdio>
#include <cassert>

#include "net/applemidi.h"

#include "midi.h"

#include "hardware.h"
#include "network.h"

#include "softwaretimers.h"

#include "debug.h"

namespace applemidi {

}  // namespace applemidi

/**
 * @enum TAppleMidiCommand
 * @brief Defines the Apple MIDI command identifiers with network byte order.
 */
enum TAppleMidiCommand {
	APPLEMIDI_COMMAND_INVITATION = __builtin_bswap16(0x494e),			///< Invitation 'IN'
	APPLEMIDI_COMMAND_INVITATION_ACCEPTED = __builtin_bswap16(0x4f4b),	///< Invitation accepted 'OK'
	APPLEMIDI_COMMAND_INVITATION_REJECTED = __builtin_bswap16(0x4e4f),	///< Invitation refused 'NO'
	APPLEMIDI_COMMAND_ENDSESSION = __builtin_bswap16(0x4259),			///< Closing session 'BY'
	APPLEMIDI_COMMAND_SYNCHRONIZATION = __builtin_bswap16(0x434b),		///< Clock synchronization 'CK'
	APPLEMIDI_COMMAND_RECEIVER_FEEDBACK = __builtin_bswap16(0x5253),	///< Journalling synchronization 'RS'
	APPLEMIDI_COMMAND_BITRATE_RECEIVE_LIMIT = __builtin_bswap16(0x524c)	///< Bitrate 'RL'
};

/**
 * @struct TTimestampSynchronization
 * @brief Represents the structure for timestamp synchronization in Apple MIDI.
 *
 * This structure is used to handle the synchronization of timestamps between devices
 * in an Apple MIDI session.
 */
struct TTimestampSynchronization {
	uint16_t nSignature;	///< Packet signature for Apple MIDI.
	uint16_t nCommand;		///< Command identifier.
	uint32_t nSSRC;			///< Synchronization source identifier.
	uint8_t nCount;			///< Count of synchronization steps.
	uint8_t padding[3];		///< Padding to align the structure.
	uint64_t nTimestamps[3];///< Array of timestamps for synchronization.
} __attribute__((packed));

/**
 * @brief Timeout value in milliseconds for an Apple MIDI session.
 */
static constexpr uint32_t TIMEOUT = 90 * 1000;

/**
 * @brief Timer handle for session timeout management.
 */
static TimerHandle_t s_nTimerId = TIMER_ID_NONE;

/**
 * @brief Timer callback function to handle session timeout.
 *
 * This function is called when the session times out, and it resets the session
 * and deletes the timer.
 *
 * @param nHandle Handle of the expired timer (unused).
 */
static void timeout_timer([[maybe_unused]] TimerHandle_t nHandle) {
	if (AppleMidi::GetSessionState() == applemidi::SessionState::ESTABLISHED) {
		AppleMidi::ResetSession();
		SoftwareTimerDelete(s_nTimerId);
		DEBUG_PUTS("End Session {time-out}");
	}
}

typedef union pcast32 {
	uint32_t u32;
	uint8_t u8[4];
} _pcast32;

AppleMidi::AppleMidi() {
	DEBUG_ENTRY
	assert(s_pThis == nullptr);
	s_pThis = this;

	m_ExchangePacketReply.nSignature = SIGNATURE;
	m_ExchangePacketReply.nProtocolVersion = __builtin_bswap32(applemidi::VERSION);

	uint8_t macAddress[net::MAC_SIZE];
	Network::Get()->MacAddressCopyTo(macAddress);
	_pcast32 cast32;
	memcpy(cast32.u8, &macAddress[2], 4);
	m_nSSRC = cast32.u32;

	SetSessionName(Network::Get()->GetHostName());

	memset(&m_SessionStatus, 0, sizeof (struct applemidi::SessionStatus));

	DEBUG_PRINTF("applemidi::EXCHANGE_PACKET_MIN_LENGTH = %u", static_cast<uint32_t>(applemidi::EXCHANGE_PACKET_MIN_LENGTH));
	DEBUG_EXIT
}

void AppleMidi::InputControlMessage(const uint8_t *pBuffer, uint32_t nSize, uint32_t nFromIp, uint16_t nFromPort) {
	DEBUG_ENTRY

	if (__builtin_expect((nSize >= applemidi::EXCHANGE_PACKET_MIN_LENGTH), 1)) {
		if (*reinterpret_cast<const uint16_t *>(pBuffer) != SIGNATURE) {

			DEBUG_EXIT
			return;
		}
	}

	auto *pPacket = reinterpret_cast<const applemidi::ExchangePacket *>(pBuffer);

	DEBUG_PRINTF("Command: %.4x, m_SessionStatus.sessionState=%u", pPacket->nCommand, static_cast<uint32_t>(m_SessionStatus.sessionState));

	if (m_SessionStatus.sessionState == applemidi::SessionState::WAITING_IN_CONTROL) {
		DEBUG_PUTS("SESSION_STATE_WAITING_IN_CONTROL");

		if ((m_SessionStatus.nRemoteIp == 0) && (pPacket->nCommand == APPLEMIDI_COMMAND_INVITATION)) {
 			DEBUG_PUTS("Invitation");

 			m_ExchangePacketReply.nSSRC = GetSSRC();
			m_ExchangePacketReply.nCommand = APPLEMIDI_COMMAND_INVITATION_ACCEPTED;
			m_ExchangePacketReply.nInitiatorToken = pPacket->nInitiatorToken;

			Network::Get()->SendTo(m_nHandleControl, &m_ExchangePacketReply, m_nExchangePacketReplySize, nFromIp, nFromPort);

			debug_dump(&m_ExchangePacketReply, m_nExchangePacketReplySize);

			m_SessionStatus.sessionState = applemidi::SessionState::WAITING_IN_MIDI;
			m_SessionStatus.nRemoteIp = nFromIp;

			DEBUG_EXIT
			return;
		} else {
			DEBUG_EXIT
			return;
		}
	}

	if (m_SessionStatus.sessionState == applemidi::SessionState::ESTABLISHED) {
		DEBUG_PUTS("SESSION_STATE_ESTABLISHED");

		if ((m_SessionStatus.nRemoteIp == nFromIp) && (pPacket->nCommand == APPLEMIDI_COMMAND_ENDSESSION)) {
			m_SessionStatus.sessionState = applemidi::SessionState::WAITING_IN_CONTROL;
			m_SessionStatus.nRemoteIp = 0;

			DEBUG_PUTS("End Session");
			DEBUG_EXIT
			return;
		}

		if (pPacket->nCommand == APPLEMIDI_COMMAND_INVITATION) {
		 	DEBUG_PUTS("Invitation rejected");

		 	m_ExchangePacketReply.nSSRC = GetSSRC();
			m_ExchangePacketReply.nCommand = APPLEMIDI_COMMAND_INVITATION_REJECTED;
			m_ExchangePacketReply.nInitiatorToken = pPacket->nInitiatorToken;

			Network::Get()->SendTo(m_nHandleControl, &m_ExchangePacketReply, m_nExchangePacketReplySize, nFromIp, nFromPort);

			debug_dump(&m_ExchangePacketReply, m_nExchangePacketReplySize);
			DEBUG_EXIT
			return;
		}
	}

	DEBUG_EXIT
}

void AppleMidi::InputMidiMessage(const uint8_t *pBuffer, uint32_t nSize, uint32_t nFromIp, uint16_t nFromPort) {
	DEBUG_ENTRY

	if (__builtin_expect((nSize >= 12), 0)) {
		if (m_SessionStatus.nRemoteIp != nFromIp) {

			DEBUG_EXIT
			return;
		}
	}

	if (*reinterpret_cast<const uint16_t *>(pBuffer) == 0x6180) {
		HandleRtpMidi(pBuffer);

		DEBUG_EXIT
		return;
	}

	if (nSize >= applemidi::EXCHANGE_PACKET_MIN_LENGTH) {

		if (*reinterpret_cast<const uint16_t *>(pBuffer) == SIGNATURE) {

			if (m_SessionStatus.sessionState == applemidi::SessionState::WAITING_IN_MIDI) {
				DEBUG_PUTS("SESSION_STATE_WAITING_IN_MIDI");

				auto *pPacket = reinterpret_cast<const applemidi::ExchangePacket*>(pBuffer);

				DEBUG_PRINTF("Command: %.4x", pPacket->nCommand);

				if (pPacket->nCommand == APPLEMIDI_COMMAND_INVITATION) {
					DEBUG_PUTS("Invitation");

					m_ExchangePacketReply.nSSRC = GetSSRC();
					m_ExchangePacketReply.nCommand = APPLEMIDI_COMMAND_INVITATION_ACCEPTED;
					m_ExchangePacketReply.nInitiatorToken = pPacket->nInitiatorToken;

					Network::Get()->SendTo(m_nHandleMidi, &m_ExchangePacketReply, m_nExchangePacketReplySize, nFromIp, nFromPort);

					m_SessionStatus.sessionState = applemidi::SessionState::ESTABLISHED;
					m_SessionStatus.nRemotePortMidi = nFromPort;

					s_nTimerId = SoftwareTimerAdd(TIMEOUT, timeout_timer);
				}

				DEBUG_EXIT
				return;
			}

			if (m_SessionStatus.sessionState == applemidi::SessionState::ESTABLISHED) {
				DEBUG_PUTS("SESSION_STATE_ESTABLISHED");

				auto *pPacket = reinterpret_cast<const applemidi::ExchangePacket *>(pBuffer);

				if (pPacket->nCommand == APPLEMIDI_COMMAND_SYNCHRONIZATION) {
					DEBUG_PUTS("Timestamp Synchronization");
					auto *t = reinterpret_cast<TTimestampSynchronization *>(const_cast<uint8_t *>(pBuffer));

					if (t->nCount == 0) {
						t->nSSRC = GetSSRC();
						t->nCount = 1;
						t->nTimestamps[1] = __builtin_bswap64(Now());

						Network::Get()->SendTo(m_nHandleMidi, pBuffer, sizeof(struct TTimestampSynchronization), nFromIp, nFromPort);
						SoftwareTimerChange(s_nTimerId, TIMEOUT);
					} else if (t->nCount == 1) {
						t->nSSRC = GetSSRC();
						t->nCount = 2;
						t->nTimestamps[2] = __builtin_bswap64(Now());

						Network::Get()->SendTo(m_nHandleMidi, pBuffer, sizeof(struct TTimestampSynchronization), nFromIp, nFromPort);
						SoftwareTimerChange(s_nTimerId, TIMEOUT);
					} else if (t->nCount == 2) {
						t->nSSRC = GetSSRC();
						t->nCount = 0;
						t->nTimestamps[0] = __builtin_bswap64(Now());
						t->nTimestamps[1] = 0;
						t->nTimestamps[2] = 0;

						Network::Get()->SendTo(m_nHandleMidi, pBuffer, sizeof(struct TTimestampSynchronization), nFromIp, nFromPort);
						SoftwareTimerChange(s_nTimerId, TIMEOUT);
					}
				}
			}
		}
	}

	DEBUG_EXIT
}
