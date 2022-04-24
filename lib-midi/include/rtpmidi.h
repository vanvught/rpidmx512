/**
 * @file rtpmidi.h
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

#ifndef RTPMIDI_H_
#define RTPMIDI_H_

#include <cstdint>

#include "applemidi.h"
#include "midi.h"

#include "rtpmidihandler.h"

#include "debug.h"

namespace rtpmidi {
static constexpr auto BUFFER_SIZE = 512U;

struct Header {
	uint16_t nStatic;
	uint16_t nSequenceNumber;
	uint32_t nTimestamp;
	uint32_t nSenderSSRC;
}__attribute__((packed));

static constexpr auto COMMAND_OFFSET = sizeof(struct Header);
}  // namespace rtpmidi

class RtpMidi: public AppleMidi {
public:
	RtpMidi() {
		DEBUG_ENTRY

		assert(s_pThis == nullptr);
		s_pThis = this;

		DEBUG_EXIT
	}

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

	void Stop() {
		DEBUG_ENTRY

		AppleMidi::Stop();

		DEBUG_EXIT
	}

	void Run() {
		AppleMidi::Run();
	}

	void SendRaw(uint8_t nByte) {
		auto *data = &m_pSendBuffer[rtpmidi::COMMAND_OFFSET + 1];
		data[0] = nByte;
		Send(1);
	}

	void SendRaw(midi::Types tType) {
		SendRaw(static_cast<uint8_t>(tType));
	}

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

	void SetHandler(RtpMidiHandler *pRtpMidiHandler) {
		m_pRtpMidiHandler = pRtpMidiHandler;
	}

	void Print() {
		AppleMidi::Print();
	}

	static RtpMidi* Get() {
		return s_pThis;
	}

private:
	void HandleRtpMidi(const uint8_t *pBuffer) override;

	int32_t DecodeTime(uint32_t nCommandLength, uint32_t nOffset);
	int32_t DecodeMidi(uint32_t nCommandLength, uint32_t nOffset);

	midi::Types GetTypeFromStatusByte(uint8_t nStatusByte) {
		if ((nStatusByte < 0x80) || (nStatusByte == 0xf4) || (nStatusByte == 0xf5) || (nStatusByte == 0xf9) || (nStatusByte == 0xfD)) {
			return midi::Types::INVALIDE_TYPE;
		}

		if (nStatusByte < 0xF0) {
			return static_cast<midi::Types>(nStatusByte & 0xF0);
		}

		return static_cast<midi::Types>(nStatusByte);
	}

	uint8_t GetChannelFromStatusByte(uint8_t nStatusByte) {
		return static_cast<uint8_t>((nStatusByte & 0x0F) + 1);
	}

	void  Send(uint32_t nLength) {
		auto *pHeader = reinterpret_cast<rtpmidi::Header*>(m_pSendBuffer);

		pHeader->nSequenceNumber = __builtin_bswap16(m_nSequenceNumber++);
		pHeader->nTimestamp = __builtin_bswap32(Now());

		m_pSendBuffer[rtpmidi::COMMAND_OFFSET] = static_cast<uint8_t>(nLength); //FIXME BUG works now only

		AppleMidi::Send(m_pSendBuffer, 1 + sizeof(struct rtpmidi::Header) + nLength);
	}

private:
	struct midi::Message m_tMidiMessage;
	RtpMidiHandler *m_pRtpMidiHandler { nullptr };
	uint8_t *m_pReceiveBuffer { nullptr };
	uint8_t *m_pSendBuffer { nullptr };
	uint16_t m_nSequenceNumber { 0 };

	static RtpMidi *s_pThis;
};

#endif /* RTPMIDI_H_ */
