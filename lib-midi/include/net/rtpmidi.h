/**
 * @file rtpmidi.h
 * @brief RTP-MIDI implementation for real-time MIDI data transfer.
 *
 * This file provides the definition and implementation of the RtpMidi class,
 * which extends AppleMidi to add functionality for RTP-MIDI communication.
 * It supports sending and receiving raw MIDI data, timecodes, and MIDI quarter frames.
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

#ifndef NET_RTPMIDI_H_
#define NET_RTPMIDI_H_

#include <cstdint>
#include <cassert>

#include "net/applemidi.h"
#include "net/rtpmidihandler.h"

#include "midi.h"

#include "debug.h"

/**
 * @namespace rtpmidi
 * @brief Contains constants and structures for RTP-MIDI communication.
 */
namespace rtpmidi {
/**
 * @brief Buffer size for RTP-MIDI messages.
 */
static constexpr auto BUFFER_SIZE = 512U;

/**
 * @struct Header
 * @brief Structure representing the RTP-MIDI header.
 */
struct Header {
	uint16_t nStatic;
	uint16_t nSequenceNumber;
	uint32_t nTimestamp;
	uint32_t nSenderSSRC;
}__attribute__((packed));

/**
 * @brief Offset for commands in the RTP-MIDI buffer.
 */
static constexpr auto COMMAND_OFFSET = sizeof(struct Header);
}  // namespace rtpmidi

/**
 * @class RtpMidi
 * @brief Class implementing RTP-MIDI communication.
 *
 * RtpMidi extends the AppleMidi class to handle real-time MIDI data transfer
 * over RTP. It supports sending raw MIDI messages, timecodes, and MIDI quarter frames.
 */
class RtpMidi final: public AppleMidi {
public:
	RtpMidi() {
		DEBUG_ENTRY

		assert(s_pThis == nullptr);
		s_pThis = this;

		DEBUG_EXIT
	}

	/**
	 * @brief Starts the RTP-MIDI service.
	 */
	void Start() {
		DEBUG_ENTRY

		AppleMidi::Start();

		m_pSendBuffer = new uint8_t[rtpmidi::BUFFER_SIZE];
		assert(m_pSendBuffer != nullptr);

		auto *pHeader = reinterpret_cast<rtpmidi::Header*>(m_pSendBuffer);
		pHeader->nStatic = 0x6180;
		pHeader->nSenderSSRC = AppleMidi::GetSSRC();

		DEBUG_EXIT
	}

	/**
	 * @brief Stops the RTP-MIDI service.
	 */
	void Stop() {
		DEBUG_ENTRY

		AppleMidi::Stop();

		DEBUG_EXIT
	}

	/**
	 * @brief Sends a single raw MIDI byte.
	 * @param nByte The raw MIDI byte to send.
	 */
	void SendRaw(const uint8_t nByte) {
		auto *data = &m_pSendBuffer[rtpmidi::COMMAND_OFFSET + 1];
		data[0] = nByte;
		Send(1);
	}

	/**
	 * @brief Sends a raw MIDI type.
	 * @param type The MIDI type to send.
	 */
	void SendRaw(const midi::Types type) {
		SendRaw(static_cast<uint8_t>(type));
	}

	/**
	 * @brief Sends a full MIDI timecode message.
	 * @param tTimeCode Pointer to the MIDI timecode structure.
	 */
	void SendTimeCode(const midi::Timecode *tTimeCode) {
		auto *data = &m_pSendBuffer[rtpmidi::COMMAND_OFFSET + 1];

		data[0] = 0xF0;
		data[1] = 0x7F;
		data[2] = 0x7F;
		data[3] = 0x01;
		data[4] = 0x01;
		data[5] = static_cast<uint8_t>(((tTimeCode->nType) & 0x03) << 5) | (tTimeCode->nHours & 0x1F);
		data[6] = tTimeCode->nMinutes & 0x3F;
		data[7] = tTimeCode->nSeconds & 0x3F;
		data[8] = tTimeCode->nFrames & 0x1F;
		data[9] = 0xF7;

		Send(10);
	}

	/**
	 * @brief Sends a MIDI quarter frame message.
	 * @param nValue The quarter frame value to send.
	 */
	void SendQf(const uint8_t nValue) {
		auto *data = &m_pSendBuffer[rtpmidi::COMMAND_OFFSET + 1];

		data[0] = 0xF1;
		data[1] = nValue;

		Send(2);
	}

	/**
	 * @brief Sends a MIDI quarter frame message based on timecode and updates the piece index.
	 * @param timeCode Pointer to the MIDI timecode structure.
	 * @param nMidiQuarterFramePiece Reference to the quarter frame piece index.
	 */
	void SendQf(const struct midi::Timecode *timeCode, uint32_t& nMidiQuarterFramePiece) {
		auto data = static_cast<uint8_t>(nMidiQuarterFramePiece << 4);

		switch (nMidiQuarterFramePiece) {
		case 0:
			data = data | (timeCode->nFrames & 0x0F);
			break;
		case 1:
			data = data | static_cast<uint8_t>((timeCode->nFrames & 0x10) >> 4);
			break;
		case 2:
			data = data | (timeCode->nSeconds & 0x0F);
			break;
		case 3:
			data = data | static_cast<uint8_t>((timeCode->nSeconds & 0x30) >> 4);
			break;
		case 4:
			data = data | (timeCode->nMinutes & 0x0F);
			break;
		case 5:
			data = data | static_cast<uint8_t>((timeCode->nMinutes & 0x30) >> 4);
			break;
		case 6:
			data = data | (timeCode->nHours & 0x0F);
			break;
		case 7:
			data = static_cast<uint8_t>(data | (timeCode->nType << 1) | ((timeCode->nHours & 0x10) >> 4));
			break;
		default:
			break;
		}

		SendQf(data);

		nMidiQuarterFramePiece = (nMidiQuarterFramePiece + 1) & 0x07;
	}

	/**
	 * @brief Sets the RTP-MIDI handler.
	 * @param pRtpMidiHandler Pointer to the RTP-MIDI handler instance.
	 */
	void SetHandler(RtpMidiHandler *pRtpMidiHandler) {
		m_pRtpMidiHandler = pRtpMidiHandler;
	}

	/**
	 * @brief Prints information about the RTP-MIDI instance.
	 */
	void Print() {
		AppleMidi::Print();
	}

	/**
	 * @brief Gets the current instance of RtpMidi.
	 * @return Pointer to the current RtpMidi instance.
	 */
	static RtpMidi *Get() {
		return s_pThis;
	}

private:
	void HandleRtpMidi(const uint8_t *pBuffer) override;

	/**
	 * @brief Decode the delta time from the MIDI command buffer.
	 *
	 * Decodes the variable-length delta time from the RTP MIDI command buffer.
	 *
	 * @param nCommandLength The total length of the command.
	 * @param nOffset The offset in the receive buffer to start decoding.
	 * @return The size (in bytes) of the decoded delta time.
	 */
	int32_t DecodeTime(uint32_t nCommandLength, uint32_t nOffset);

	/**
	 * @brief Decode a MIDI message from the command buffer.
	 *
	 * Decodes an RTP MIDI message from the receive buffer and populates the MIDI message structure.
	 *
	 * @param nCommandLength The length of the MIDI command to decode.
	 * @param nOffset The offset in the receive buffer to start decoding.
	 * @return The size (in bytes) of the decoded MIDI message.
	 */
	int32_t DecodeMidi(uint32_t nCommandLength, uint32_t nOffset);

	midi::Types GetTypeFromStatusByte(const uint8_t nStatusByte) {
		if ((nStatusByte < 0x80) || (nStatusByte == 0xf4) || (nStatusByte == 0xf5) || (nStatusByte == 0xf9) || (nStatusByte == 0xfD)) {
			return midi::Types::INVALIDE_TYPE;
		}

		if (nStatusByte < 0xF0) {
			return static_cast<midi::Types>(nStatusByte & 0xF0);
		}

		return static_cast<midi::Types>(nStatusByte);
	}

	uint8_t GetChannelFromStatusByte(const uint8_t nStatusByte) {
		return static_cast<uint8_t>((nStatusByte & 0x0F) + 1);
	}

	void Send(const uint32_t nLength) {
		auto *pHeader = reinterpret_cast<rtpmidi::Header*>(m_pSendBuffer);

		pHeader->nSequenceNumber = __builtin_bswap16(m_nSequenceNumber++);
		pHeader->nTimestamp = __builtin_bswap32(AppleMidi::Now());

		m_pSendBuffer[rtpmidi::COMMAND_OFFSET] = static_cast<uint8_t>(nLength); //FIXME BUG works now only

		AppleMidi::Send(m_pSendBuffer, 1 + sizeof(struct rtpmidi::Header) + nLength);
	}

private:
	midi::Message m_midiMessage;
	RtpMidiHandler *m_pRtpMidiHandler { nullptr };	///< Pointer to the RTP-MIDI handler.
	uint8_t *m_pReceiveBuffer { nullptr };			///< Receive buffer pointer.
	uint8_t *m_pSendBuffer { nullptr };				///< Send buffer pointer.
	uint16_t m_nSequenceNumber { 0 };				///< Sequence number for outgoing messages.

	static inline RtpMidi *s_pThis;	 				///< Static pointer to the current instance.
};

#endif /* NET_RTPMIDI_H_ */
