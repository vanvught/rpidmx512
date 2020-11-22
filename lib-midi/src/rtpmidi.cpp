/**
 * @file rtpmidi.cpp
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

#include <stdio.h>
#include <cassert>

#include "rtpmidi.h"
#include "applemidi.h"

#include "rtpmidihandler.h"

#include "hardware.h"

#include "debug.h"

struct TRtpHeader {
	uint16_t nStatic;
	uint16_t nSequenceNumber;
	uint32_t nTimestamp;
	uint32_t nSenderSSRC;
}__attribute__((packed));

#define RTP_MIDI_COMMAND_OFFSET			sizeof(struct TRtpHeader)

#define RTP_MIDI_COMMAND_STATUS_FLAG 	0x80

#define RTP_MIDI_DELTA_TIME_OCTET_MASK	0x7f
#define RTP_MIDI_DELTA_TIME_EXTENSION	0x80

#define RTP_MIDI_CS_FLAG_B 				0x80
#define RTP_MIDI_CS_FLAG_J 				0x40
#define RTP_MIDI_CS_FLAG_Z 				0x20
#define RTP_MIDI_CS_FLAG_P 				0x10
#define RTP_MIDI_CS_MASK_SHORTLEN 		0x0f
#define RTP_MIDI_CS_MASK_LONGLEN 		0x0fff

#define BUFFER_SIZE	512

RtpMidi *RtpMidi::s_pThis = nullptr;

RtpMidi::RtpMidi()
	
{
	DEBUG_ENTRY

	s_pThis = this;

	DEBUG_EXIT
}

void RtpMidi::Start() {
	DEBUG_ENTRY

	AppleMidi::Start();

	m_pSendBuffer = new uint8_t[BUFFER_SIZE];
	assert(m_pSendBuffer != nullptr);

	auto *pHeader = reinterpret_cast<TRtpHeader*>(m_pSendBuffer);
	pHeader->nStatic = 0x6180;
	pHeader->nSenderSSRC = AppleMidi::GetSSRC();

	DEBUG_EXIT
}

void RtpMidi::Stop() {
	DEBUG_ENTRY

	AppleMidi::Stop();

	DEBUG_EXIT
}

void RtpMidi::Run() {
	AppleMidi::Run();
}

int32_t RtpMidi::DecodeTime(__attribute__((unused)) uint32_t nCommandLength, uint32_t nOffset) {
	DEBUG_ENTRY

	int32_t nSize = 0;
	unsigned long deltatime = 0;

	for (uint32_t i = 0; i < 4; i++ ) {
		const uint8_t nOctet = m_pReceiveBuffer[nOffset + static_cast<uint32_t>(nSize)];
		deltatime = ( deltatime << 7 ) | ( nOctet & RTP_MIDI_DELTA_TIME_OCTET_MASK );
		nSize++;

		if ((nOctet & RTP_MIDI_DELTA_TIME_EXTENSION) == 0) {
			break;
		}
	}

	DEBUG_PRINTF("nSize=%d, deltatime=%x", nSize, static_cast<unsigned>(deltatime));

	DEBUG_EXIT
	return nSize;
}

uint8_t RtpMidi::GetTypeFromStatusByte(uint8_t nStatusByte) {
	if ((nStatusByte < 0x80) || (nStatusByte == 0xf4) || (nStatusByte == 0xf5) || (nStatusByte == 0xf9) || (nStatusByte == 0xfD)) {
		return MIDI_TYPES_INVALIDE_TYPE;
	}

	if (nStatusByte < 0xF0) {
		return nStatusByte & 0xF0;
	}

	return nStatusByte;
}

uint8_t RtpMidi::GetChannelFromStatusByte(uint8_t nStatusByte) {
	return (nStatusByte & 0x0F) + 1;
}

int32_t RtpMidi::DecodeMidi(uint32_t nCommandLength, uint32_t nOffset) {
	DEBUG_ENTRY

	int32_t nSize = -1;

	const uint8_t nStatusByte = m_pReceiveBuffer[nOffset];
	const uint8_t nType = GetTypeFromStatusByte(nStatusByte);

	m_tMidiMessage.type = nType;
	m_tMidiMessage.channel = 0;
	m_tMidiMessage.data1 = 0;
	m_tMidiMessage.data2 = 0;

	if ((nType == MIDI_TYPES_ACTIVE_SENSING) || (nType == MIDI_TYPES_START)
			|| (nType == MIDI_TYPES_CONTINUE) || (nType == MIDI_TYPES_STOP)
			|| (nType == MIDI_TYPES_CLOCK) || (nType == MIDI_TYPES_SYSTEM_RESET)
			|| (nType == MIDI_TYPES_TUNE_REQUEST)) {
		m_tMidiMessage.bytes_count = 1;
		nSize = 1;
	} else if ((nType == MIDI_TYPES_PROGRAM_CHANGE)
			|| (nType == MIDI_TYPES_AFTER_TOUCH_CHANNEL)
			|| (nType == MIDI_TYPES_TIME_CODE_QUARTER_FRAME)
			|| (nType == MIDI_TYPES_SONG_SELECT)) {
		m_tMidiMessage.channel = GetChannelFromStatusByte(nStatusByte);
		m_tMidiMessage.data1 = m_pReceiveBuffer[++nOffset];
		m_tMidiMessage.bytes_count = 2;
		nSize = 2;
	} else if ((nType == MIDI_TYPES_NOTE_ON) || (nType == MIDI_TYPES_NOTE_OFF)
			|| (nType == MIDI_TYPES_CONTROL_CHANGE)
			|| (nType == MIDI_TYPES_PITCH_BEND)
			|| (nType == MIDI_TYPES_AFTER_TOUCH_POLY)
			|| (nType == MIDI_TYPES_SONG_POSITION)) {
		m_tMidiMessage.channel = GetChannelFromStatusByte(nStatusByte);
		m_tMidiMessage.data1 = m_pReceiveBuffer[++nOffset];
		m_tMidiMessage.data2 = m_pReceiveBuffer[++nOffset];
		m_tMidiMessage.bytes_count = 3;
		nSize = 3;
	} else if (nType == MIDI_TYPES_SYSTEM_EXCLUSIVE) {
		for (nSize = 0; (static_cast<uint32_t>(nSize) < nCommandLength) && (static_cast<uint32_t>(nSize) < MIDI_SYSTEM_EXCLUSIVE_INDEX_ENTRIES); nSize++) {
			m_tMidiMessage.system_exclusive[nSize] = m_pReceiveBuffer[nOffset++];
			if (m_tMidiMessage.system_exclusive[nSize] == 0xF7) {
				break;
			}
		}
		nSize++;
		m_tMidiMessage.data1 = nSize & 0xFF; // LSB
		m_tMidiMessage.data2 = nSize >> 8;   // MSB
		m_tMidiMessage.bytes_count = nSize;
	}

	DEBUG_PRINTF("nSize=%d", nSize);

	if (m_pRtpMidiHandler != nullptr) m_pRtpMidiHandler->MidiMessage(&m_tMidiMessage);

	DEBUG_EXIT
	return nSize;
}

void RtpMidi::HandleRtpMidi(const uint8_t *pBuffer) {
	DEBUG_ENTRY

	m_pReceiveBuffer = const_cast<uint8_t *>(pBuffer);

	const uint8_t nFlags = m_pReceiveBuffer[RTP_MIDI_COMMAND_OFFSET];

	int32_t nCommandLength = nFlags & RTP_MIDI_CS_MASK_SHORTLEN;
	int32_t nOffset;

	if (nFlags & RTP_MIDI_CS_FLAG_B) {
		const uint8_t nOctet = m_pReceiveBuffer[RTP_MIDI_COMMAND_OFFSET + 1];
		nCommandLength = (nCommandLength << 8) | nOctet;
		nOffset = RTP_MIDI_COMMAND_OFFSET + 2;
	} else {
		nOffset = RTP_MIDI_COMMAND_OFFSET + 1;
	}

	DEBUG_PRINTF("nCommandLength=%d, nOffset=%d", nCommandLength, nOffset);

	debug_dump(&m_pReceiveBuffer[nOffset], nCommandLength);

	uint32_t nCommandCount = 0;

	while (nCommandLength != 0) {

		if ((nCommandCount != 0) || (nFlags & RTP_MIDI_CS_FLAG_Z)) {
			int32_t nSize = DecodeTime(static_cast<uint32_t>(nCommandLength), static_cast<uint32_t>(nOffset));

			if (nSize < 0) {
				return;
			}

			nOffset += nSize;
			nCommandLength -= nSize;
		}

		if (nCommandLength != 0) {
			int32_t nSize = DecodeMidi(static_cast<uint32_t>(nCommandLength), static_cast<uint32_t>(nOffset));

			if (nSize < 0) {
				return;
			}

			nOffset += nSize;
			nCommandLength -= nSize;
			nCommandCount++;
		}
	}

	DEBUG_EXIT
}

void RtpMidi::SendTimeCode(const struct _midi_send_tc *tTimeCode) {
	uint8_t *data = &m_pSendBuffer[RTP_MIDI_COMMAND_OFFSET + 1];

	data[0] = 0xF0;
	data[1] = 0x7F;
	data[2] = 0x7F;
	data[3] = 0x01;
	data[4] = 0x01;
	data[5] = (((tTimeCode->nType) & 0x03) << 5) | (tTimeCode->nHours & 0x1F);
	data[6] = tTimeCode->nMinutes & 0x3F;
	data[7] = tTimeCode->nSeconds & 0x3F;
	data[8] = tTimeCode->nFrames & 0x1F;
	data[9] = 0xF7;

	Send(10);
}

void  RtpMidi::Send(uint32_t nLength) {
	auto *pHeader = reinterpret_cast<TRtpHeader*>(m_pSendBuffer);

	pHeader->nSequenceNumber = __builtin_bswap16(m_nSequenceNumber++);
	pHeader->nTimestamp = __builtin_bswap32(Now());

	m_pSendBuffer[RTP_MIDI_COMMAND_OFFSET] = nLength; //FIXME BUG works now only

	AppleMidi::Send(m_pSendBuffer, 1 + sizeof(struct TRtpHeader) + nLength);
}

void RtpMidi::Print() {
	DEBUG_ENTRY

	AppleMidi::Print();

	DEBUG_EXIT
}
