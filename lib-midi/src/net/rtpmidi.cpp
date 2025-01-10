/**
 * @file rtpmidi.cpp
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

#if defined(__GNUC__) && !defined(__clang__)
# pragma GCC push_options
# pragma GCC optimize ("O2")
# pragma GCC optimize ("no-tree-loop-distribute-patterns")
#endif

#include <cstdio>
#include <cassert>

#include "net/rtpmidi.h"
#include "net/applemidi.h"
#include "net/rtpmidihandler.h"

#include "hardware.h"

#include "debug.h"

#define RTP_MIDI_COMMAND_STATUS_FLAG 	0x80

#define RTP_MIDI_DELTA_TIME_OCTET_MASK	0x7f
#define RTP_MIDI_DELTA_TIME_EXTENSION	0x80

#define RTP_MIDI_CS_FLAG_B 				0x80
#define RTP_MIDI_CS_FLAG_J 				0x40
#define RTP_MIDI_CS_FLAG_Z 				0x20
#define RTP_MIDI_CS_FLAG_P 				0x10
#define RTP_MIDI_CS_MASK_SHORTLEN 		0x0f
#define RTP_MIDI_CS_MASK_LONGLEN 		0x0fff

int32_t RtpMidi::DecodeTime([[maybe_unused]] uint32_t nCommandLength, uint32_t nOffset) {
	DEBUG_ENTRY

	int32_t nSize = 0;
	unsigned long deltatime = 0;

	for (uint32_t i = 0; i < 4; i++ ) {
		const auto nOctet = m_pReceiveBuffer[nOffset + static_cast<uint32_t>(nSize)];
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

int32_t RtpMidi::DecodeMidi(uint32_t nCommandLength, uint32_t nOffset) {
	DEBUG_ENTRY

	int32_t nSize = -1;

	const auto nStatusByte = m_pReceiveBuffer[nOffset];
	const auto nType = GetTypeFromStatusByte(nStatusByte);

	m_midiMessage.nTimestamp = __builtin_bswap32(*reinterpret_cast<uint32_t *>(&m_pReceiveBuffer[4]));
	m_midiMessage.tType = nType;
	m_midiMessage.nChannel = 0;
	m_midiMessage.nData1 = 0;
	m_midiMessage.nData2 = 0;

	switch (static_cast<midi::Types>(nType)) {
	case midi::Types::ACTIVE_SENSING:
	case midi::Types::START:
	case midi::Types::STOP:
	case midi::Types::CONTINUE:
	case midi::Types::CLOCK:
	case midi::Types::TUNE_REQUEST:
	case midi::Types::SYSTEM_RESET:
		m_midiMessage.nBytesCount = 1;
		nSize = 1;
		break;
	case midi::Types::PROGRAM_CHANGE:
	case midi::Types::AFTER_TOUCH_CHANNEL:
	case midi::Types::TIME_CODE_QUARTER_FRAME:
	case midi::Types::SONG_SELECT:
		m_midiMessage.nChannel = GetChannelFromStatusByte(nStatusByte);
		m_midiMessage.nData1 = m_pReceiveBuffer[++nOffset];
		m_midiMessage.nBytesCount = 2;
		nSize = 2;
		break;
	case midi::Types::NOTE_ON:
	case midi::Types::NOTE_OFF:
	case midi::Types::CONTROL_CHANGE:
	case midi::Types::PITCH_BEND:
	case midi::Types::AFTER_TOUCH_POLY:
	case midi::Types::SONG_POSITION:
		m_midiMessage.nChannel = GetChannelFromStatusByte(nStatusByte);
		m_midiMessage.nData1 = m_pReceiveBuffer[++nOffset];
		m_midiMessage.nData2 = m_pReceiveBuffer[++nOffset];
		m_midiMessage.nBytesCount = 3;
		nSize = 3;
		break;
	case midi::Types::SYSTEM_EXCLUSIVE: {
		for (nSize = 0; (static_cast<uint32_t>(nSize) < nCommandLength) && (static_cast<uint32_t>(nSize) < MIDI_SYSTEM_EXCLUSIVE_INDEX_ENTRIES); nSize++) {
			m_midiMessage.aSystemExclusive[nSize] = m_pReceiveBuffer[nOffset++];
			if (m_midiMessage.aSystemExclusive[nSize] == 0xF7) {
				break;
			}
		}
		nSize++;
		m_midiMessage.nData1 = static_cast<uint8_t>(nSize & 0xFF); // LSB
		m_midiMessage.nData2 = static_cast<uint8_t>(nSize >> 8);   // MSB
		m_midiMessage.nBytesCount = static_cast<uint8_t>(nSize);
	}
		break;
	default:
		break;
	}

	DEBUG_PRINTF("nSize=%d", nSize);

	if (m_pRtpMidiHandler != nullptr) {
		m_pRtpMidiHandler->MidiMessage(&m_midiMessage);
		DEBUG_PUTS("");
	}

	DEBUG_EXIT
	return nSize;
}

void RtpMidi::HandleRtpMidi(const uint8_t *pBuffer) {
	DEBUG_ENTRY

	m_pReceiveBuffer = const_cast<uint8_t *>(pBuffer);

	const auto nFlags = m_pReceiveBuffer[rtpmidi::COMMAND_OFFSET];

	int32_t nCommandLength = nFlags & RTP_MIDI_CS_MASK_SHORTLEN;
	int32_t nOffset;

	if (nFlags & RTP_MIDI_CS_FLAG_B) {
		const auto nOctet = m_pReceiveBuffer[rtpmidi::COMMAND_OFFSET + 1];
		nCommandLength = (nCommandLength << 8) | nOctet;
		nOffset = rtpmidi::COMMAND_OFFSET + 2;
	} else {
		nOffset = rtpmidi::COMMAND_OFFSET + 1;
	}

	DEBUG_PRINTF("nCommandLength=%d, nOffset=%d", nCommandLength, nOffset);

	debug_dump(&m_pReceiveBuffer[nOffset], static_cast<uint16_t>(nCommandLength));

	uint32_t nCommandCount = 0;

	while (nCommandLength != 0) {

		if ((nCommandCount != 0) || (nFlags & RTP_MIDI_CS_FLAG_Z)) {
			auto nSize = DecodeTime(static_cast<uint32_t>(nCommandLength), static_cast<uint32_t>(nOffset));

			if (nSize < 0) {
				return;
			}

			nOffset += nSize;
			nCommandLength -= nSize;
		}

		if (nCommandLength != 0) {
			auto nSize = DecodeMidi(static_cast<uint32_t>(nCommandLength), static_cast<uint32_t>(nOffset));

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
