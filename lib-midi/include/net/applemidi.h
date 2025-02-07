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

#ifndef NET_APPLEMIDI_H_
#define NET_APPLEMIDI_H_

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
static constexpr size_t SESSION_NAME_LENGTH_MAX = 24;
static constexpr uint32_t VERSION = 2;

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
		m_nHandleControl = Network::Get()->Begin(m_nPort, StaticCallbackFunctionControlMessage);
		assert(m_nHandleControl != -1);

		assert(m_nHandleMidi == -1);
		m_nHandleMidi = Network::Get()->Begin((m_nPort + 1U), StaticCallbackFunctionMidiMessage);
		assert(m_nHandleMidi != -1);

		DEBUG_PRINTF("Session name: [%s]", m_ExchangePacketReply.aName);

		m_nStartTime = Hardware::Get()->Millis();

		DEBUG_EXIT
	}

	void Stop() {
		DEBUG_ENTRY

		assert(m_nHandleMidi != -1);
		Network::Get()->End(static_cast<uint16_t>(m_nPort + 1U));
		m_nHandleMidi = -1;

		assert(m_nHandleControl != -1);
		Network::Get()->End(m_nPort);
		m_nHandleControl = -1;

		DEBUG_EXIT
	}

	/**
	 * @brief Processes incoming Apple MIDI MIDI messages.
	 *
	 * Handles MIDI messages such as RTP-MIDI data and timestamp synchronization commands.
	 *
	 * @param pBuffer Pointer to the received data buffer.
	 * @param nSize Size of the received data.
	 * @param nFromIp Source IP address.
	 * @param nFromPort Source port.
	 */
	void InputMidiMessage(const uint8_t *pBuffer, uint32_t nSize, uint32_t nFromIp, uint16_t nFromPort);

	/**
	 * @brief Processes incoming Apple MIDI control messages.
	 *
	 * Handles control messages such as session invitations, session end commands,
	 * and synchronization requests.
	 *
	 * @param pBuffer Pointer to the received data buffer.
	 * @param nSize Size of the received data.
	 * @param nFromIp Source IP address.
	 * @param nFromPort Source port.
	 */
	void InputControlMessage(const uint8_t *pBuffer, uint32_t nSize, uint32_t nFromIp, uint16_t nFromPort);

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

	inline uint32_t GetSSRC() {
		return m_nSSRC;
	}

	void Print() {
		puts("AppleMIDI");
		printf(" Session : %s\n", m_ExchangePacketReply.aName);
	}

	static auto GetSessionState() {
		assert(s_pThis != nullptr);
		return s_pThis->m_SessionStatus.sessionState;
	}

	static void ResetSession() {
		assert(s_pThis != nullptr);
		s_pThis->m_SessionStatus.sessionState = applemidi::SessionState::WAITING_IN_CONTROL;
		s_pThis->m_SessionStatus.nRemoteIp = 0;
	}

protected:
	uint32_t Now() {
		const auto nElapsed = Hardware::Get()->Millis() - m_nStartTime;
		return (nElapsed * 10U);
	}

	bool Send(const uint8_t *pBuffer, const uint32_t nLength) {
		if (m_SessionStatus.sessionState != applemidi::SessionState::ESTABLISHED) {
			return false;
		}

		Network::Get()->SendTo(m_nHandleMidi, pBuffer, nLength, m_SessionStatus.nRemoteIp, m_SessionStatus.nRemotePortMidi);
		return true;
	}

private:
	/**
	 * @brief Static callback function for receiving UDP packets.
	 *
	 * @param pBuffer Pointer to the packet buffer.
	 * @param nSize Size of the packet buffer.
	 * @param nFromIp IP address of the sender.
	 * @param nFromPort Port number of the sender.
	 */
	void static StaticCallbackFunctionControlMessage(const uint8_t *pBuffer, uint32_t nSize, uint32_t nFromIp, uint16_t nFromPort) {
		s_pThis->InputControlMessage(pBuffer, nSize, nFromIp, nFromPort);
	}

	/**
	 * @brief Static callback function for receiving UDP packets.
	 *
	 * @param pBuffer Pointer to the packet buffer.
	 * @param nSize Size of the packet buffer.
	 * @param nFromIp IP address of the sender.
	 * @param nFromPort Port number of the sender.
	 */
	void static StaticCallbackFunctionMidiMessage(const uint8_t *pBuffer, uint32_t nSize, uint32_t nFromIp, uint16_t nFromPort) {
		s_pThis->InputMidiMessage(pBuffer, nSize, nFromIp, nFromPort);
	}

	virtual void HandleRtpMidi(const uint8_t *pBuffer)=0;

private:
	uint32_t m_nStartTime { 0 };
	int32_t m_nHandleControl { -1 };
	int32_t m_nHandleMidi { -1 };
	uint32_t m_nSSRC { 0 };
	uint16_t m_nExchangePacketReplySize { applemidi::EXCHANGE_PACKET_MIN_LENGTH };
	uint16_t m_nPort { UPD_PORT_CONTROL_DEFAULT };
	applemidi::ExchangePacket m_ExchangePacketReply;
	applemidi::SessionStatus m_SessionStatus;

	static inline AppleMidi *s_pThis;	///< Static instance pointer for the callback function.
};

#endif /* NET_APPLEMIDI_H_ */
