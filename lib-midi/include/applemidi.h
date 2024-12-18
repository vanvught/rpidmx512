/**
 * @file applemidi.h
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

#ifndef APPLEMIDI_H_
#define APPLEMIDI_H_

#include <cstdint>
#include <cstdio>
#include <algorithm>
#include <cassert>

#include "midi.h"
#include "net/apps/mdns.h"

#include "hardware.h"
#include "network.h"

#include "debug.h"

namespace applemidi {
static constexpr auto SESSION_NAME_LENGTH_MAX = 24;
static constexpr auto VERSION = 2;

struct ExchangePacket {
	uint16_t nSignature;
	uint16_t nCommand;
	uint32_t nProtocolVersion;
	uint32_t nInitiatorToken;
	uint32_t nSSRC;
	uint8_t aName[SESSION_NAME_LENGTH_MAX + 1];
}__attribute__((packed));

enum class SessionState {
	WAITING_IN_CONTROL, WAITING_IN_MIDI, IN_SYNC, ESTABLISHED
};

struct SessionStatus {
	SessionState sessionState;
	uint32_t nRemoteIp;
	uint16_t nRemotePortMidi;
	uint32_t nSynchronizationTimestamp;
};

static constexpr auto EXCHANGE_PACKET_MIN_LENGTH = sizeof(struct applemidi::ExchangePacket) - applemidi::SESSION_NAME_LENGTH_MAX - 1;
}  // namespace applemidi

class AppleMidi {
	static constexpr uint16_t UPD_PORT_CONTROL_DEFAULT = 5004;
	static constexpr uint16_t UPD_PORT_MIDI_DEFAULT = UPD_PORT_CONTROL_DEFAULT + 1;
	static constexpr uint16_t SIGNATURE = 0xffff;
public:
	AppleMidi();

	virtual ~AppleMidi() {
		Stop();
	}

	void Start() {
		DEBUG_ENTRY
		mdns_service_record_add(nullptr, mdns::Services::MIDI, nullptr, m_nPort);

		assert(m_nHandleControl == -1);
		m_nHandleControl = Network::Get()->Begin(m_nPort);
		assert(m_nHandleControl != -1);

		assert(m_nHandleMidi == -1);
		m_nHandleMidi = Network::Get()->Begin(static_cast<uint16_t>(m_nPort + 1));
		assert(m_nHandleMidi != -1);

		DEBUG_PRINTF("Session name: [%s]", m_ExchangePacketReply.aName);

		m_nStartTime = Hardware::Get()->Millis();

		DEBUG_EXIT
	}

	void Stop() {
		DEBUG_ENTRY

		Network::Get()->End(static_cast<uint16_t>(m_nPort + 1U));
		Network::Get()->End(m_nPort);

		m_nHandleControl = -1;
		m_nHandleMidi = -1;

		DEBUG_EXIT
	}

	void Run() {
		m_nBytesReceived = Network::Get()->RecvFrom(m_nHandleMidi, const_cast<const void **>(reinterpret_cast<void **>(&m_pBuffer)), &m_nRemoteIp, &m_nRemotePort);

		if (__builtin_expect((m_nBytesReceived >= 12), 0)) {
			if (m_SessionStatus.nRemoteIp == m_nRemoteIp) {
				HandleMidiMessage();
			}
		}

		m_nBytesReceived = Network::Get()->RecvFrom(m_nHandleControl, const_cast<const void **>(reinterpret_cast<void **>(&m_pBuffer)), &m_nRemoteIp, &m_nRemotePort);

		if (__builtin_expect((m_nBytesReceived >= applemidi::EXCHANGE_PACKET_MIN_LENGTH), 0)) {
			if (*reinterpret_cast<uint16_t *>(m_pBuffer) == SIGNATURE) {
				HandleControlMessage();
			}
		}

		if (m_SessionStatus.sessionState == applemidi::SessionState::ESTABLISHED) {
			if (__builtin_expect((Hardware::Get()->Millis() - m_SessionStatus.nSynchronizationTimestamp > (90 * 1000)), 0)) {
				m_SessionStatus.sessionState = applemidi::SessionState::WAITING_IN_CONTROL;
				m_SessionStatus.nRemoteIp = 0;
				DEBUG_PUTS("End Session {time-out}");
			}
		}
	}

	void SetPort(const uint16_t nPort) {
		assert(nPort > 1024);
		m_nPort = nPort;
	}

	void SetSessionName(const char *pSessionName) {
		const auto nLength = std::min(strlen(pSessionName), static_cast<size_t>(applemidi::SESSION_NAME_LENGTH_MAX));
		memcpy(reinterpret_cast<char *>(&m_ExchangePacketReply.aName), pSessionName, nLength);
		m_ExchangePacketReply.aName[nLength] = '\0';
		m_nExchangePacketReplySize = static_cast<uint16_t>(applemidi::EXCHANGE_PACKET_MIN_LENGTH + 1 + nLength);
	}

	uint32_t GetSSRC() const {
		return m_nSSRC;
	}

	void Print() {
		const auto nSSRC = __builtin_bswap32(m_nSSRC);
		printf("AppleMIDI\n");
		printf(" SSRC    : %x (%u)\n", nSSRC, nSSRC);
		printf(" Session : %s\n", m_ExchangePacketReply.aName);
	}

protected:
	uint32_t Now() {
		const auto nElapsed = Hardware::Get()->Millis() - m_nStartTime;
		return (nElapsed * 10U);
	}

	bool Send(const uint8_t *pBuffer, uint32_t nLength) {
		if (m_SessionStatus.sessionState != applemidi::SessionState::ESTABLISHED) {
			return false;
		}

		Network::Get()->SendTo(m_nHandleMidi, pBuffer, static_cast<uint16_t>(nLength), m_SessionStatus.nRemoteIp, m_SessionStatus.nRemotePortMidi);

		debug_dump(&pBuffer, static_cast<uint16_t>(nLength));

		return true;
	}

private:
	void HandleControlMessage();
	void HandleMidiMessage();

	virtual void HandleRtpMidi(const uint8_t *pBuffer)=0;

private:
	uint32_t m_nStartTime { 0 };
	uint32_t m_nSSRC;
	int32_t m_nHandleControl { -1 };
	int32_t m_nHandleMidi { -1 };
	uint32_t m_nRemoteIp { 0 };
	uint32_t m_nBytesReceived { 0 };
	uint16_t m_nExchangePacketReplySize;
	uint16_t m_nPort { UPD_PORT_CONTROL_DEFAULT };
	uint16_t m_nRemotePort { 0 };
	applemidi::ExchangePacket m_ExchangePacketReply;
	applemidi::SessionStatus m_SessionStatus;
	uint8_t *m_pBuffer { nullptr };
};

#endif /* APPLEMIDI_H_ */
