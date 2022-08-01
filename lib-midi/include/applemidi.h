/**
 * @file applemidi.h
 *
 */
/* Copyright (C) 2019-2022 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include "hardware.h"
#include "mdns.h"
#include "mdnsservices.h"

#include "debug.h"

namespace applemidi {
static constexpr auto UPD_PORT_CONTROL_DEFAULT = 5004U;
static constexpr auto UPD_PORT_MIDI_DEFAULT = UPD_PORT_CONTROL_DEFAULT + 1U;
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
	SessionState tSessionState;
	uint32_t nRemoteIp;
	uint16_t nRemotePortMidi;
	uint32_t nSynchronizationTimestamp;
};

static constexpr auto EXCHANGE_PACKET_MIN_LENGTH = sizeof(struct applemidi::ExchangePacket) - applemidi::SESSION_NAME_LENGTH_MAX - 1;
}  // namespace applemidi

class AppleMidi: public MDNS {
public:
	AppleMidi();

	virtual ~AppleMidi() {
		Stop();
	}

	void Start() {
		DEBUG_ENTRY

		MDNS::Start();
		MDNS::AddServiceRecord(nullptr, MDNS_SERVICE_MIDI, m_nPort);

		m_nHandleControl = Network::Get()->Begin(m_nPort);
		assert(m_nHandleControl != -1);

		m_nHandleMidi = Network::Get()->Begin(static_cast<uint16_t>(m_nPort + 1));
		assert(m_nHandleMidi != -1);

		DEBUG_PRINTF("Session name: [%s]", m_ExchangePacketReply.aName);

		m_nStartTime = Hardware::Get()->Millis();

		DEBUG_EXIT
	}

	void Stop() {
		DEBUG_ENTRY

		MDNS::Stop();
		Network::Get()->End(static_cast<uint16_t>(m_nPort + 1U));
		Network::Get()->End(m_nPort);

		DEBUG_EXIT
	}

	void Run();

	void SetPort(uint16_t nPort) {
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
		MDNS::Print();
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
		if (m_tSessionStatus.tSessionState != applemidi::SessionState::ESTABLISHED) {
			return false;
		}

		Network::Get()->SendTo(m_nHandleMidi, pBuffer, static_cast<uint16_t>(nLength), m_tSessionStatus.nRemoteIp, m_tSessionStatus.nRemotePortMidi);

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
	uint16_t m_nExchangePacketReplySize;
	uint16_t m_nPort { applemidi::UPD_PORT_CONTROL_DEFAULT };
	uint16_t m_nRemotePort { 0 };
	uint16_t m_nBytesReceived { 0 };
	applemidi::ExchangePacket m_ExchangePacketReply;
	applemidi::SessionStatus m_tSessionStatus;
	uint8_t *m_pBuffer { nullptr };
};

#endif /* APPLEMIDI_H_ */
