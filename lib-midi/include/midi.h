/**
 * @file midi.h
 *
 */
/* Copyright (C) 2016-2023 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef MIDI_H_
#define MIDI_H_

#include <cstdint>
#include <cstdio>

#include "hal_uart.h"

namespace midi {
/**
 * NoteOn with 0 velocity should be handled as NoteOf.
 * Set to true  to get NoteOff events when receiving null-velocity NoteOn messages.
 * Set to false to get NoteOn  events when receiving null-velocity NoteOn messages.
 */
static constexpr auto HANDLE_NULL_VELOCITY_NOTE_ON_AS_NOTE_OFF = true;

namespace defaults {
static constexpr auto BAUDRATE = 31250;
}  // namespace defaults

namespace pitchbend {
static constexpr auto MIN = -8192;
static constexpr auto MAX = 8191;
}  // namespace pitchband

namespace control {
enum class Change : uint8_t {
	ALL_SOUND_OFF 			= 0x78,
	RESET_ALL_CONTROLLERS 	= 0x79,
	LOCAL_CONTROL 			= 0x7A,
	ALL_NOTES_OFF 			= 0x7B,
	OMNI_MODE_OFF 			= 0x7C,
	OMNI_MODE_ON 			= 0x7D,
	MONO_MODE_ON 			= 0x7E,
	POLY_MODE_ON 			= 0x7F
};

enum class Function : uint8_t {
	BANK_SELECT				= 0x00,	///< MSB
	MODULATION_WHEEL		= 0x01,	///< MSB
	BREATH_CONTROLLER		= 0x02,	///< MSB
	UNDEFINED_03			= 0x03,	///< MSB
	FOOT_CONTROLLER			= 0x04,	///< MSB
	PORTAMENTO_TIME			= 0x05,	///< MSB
	DATA_ENTRY_MSB			= 0x06,	///< MSB
	CHANNEL_VOLUME			= 0x07,	///< MSB
	BALANCE					= 0x08,	///< MSB
	UNDEFINED_09			= 0x09,	///< MSB
	PAN						= 0x0A,	///< MSB
	EXPRESSION_CONTROLLER	= 0x0B,	///< MSB
	EFFECT_CONTROL_1		= 0x0C,	///< MSB
	EFFECT_CONTROL_2		= 0x0D,	///< MSB
	UNDEFINED_0E			= 0x0E,	///< MSB
	UNDEFINED_0F			= 0x0F,	///< MSB
	GP_CONTROLLER_1			= 0x10, ///< MSB
	GP_CONTROLLER_2			= 0x11, ///< MSB
	GP_CONTROLLER_3			= 0x12, ///< MSB
	GP_CONTROLLER_4			= 0x13, ///< MSB
	DAMPER_PEDAL_ON_OFF		= 0x40,	///< 63 off, 64 on
	PORTAMENTO_ON_OFF		= 0x41,	///< 63 off, 64 on
	SOSTENUTO_ON_OFF		= 0x42,	///< 63 off, 64 on
	SOFT_PEDAL_ON_OFF		= 0x43,	///< 63 off, 64 on
	LEGATO_FOOTSWITCH		= 0x44,	///< 63 off, 64 on
	HOLD_2					= 0x45	///< 63 off, 64 on
} ;
}  // namespace control

enum class Channel : uint8_t {
	OMNI = 0, OFF = 17
};

enum class ActiveSenseState {
	NOT_ENABLED, ENABLED, FAILED
};

enum class Types: uint8_t {
	INVALIDE_TYPE 			= 0x00,	///< For notifying errors
	NOTE_OFF				= 0x80,	///< Note Off
	NOTE_ON					= 0x90,	///< Note On
	AFTER_TOUCH_POLY		= 0xA0,	///< Polyphonic AfterTouch
	CONTROL_CHANGE			= 0xB0,	///< Control Change / Channel Mode
	PROGRAM_CHANGE			= 0xC0,	///< Program Change
	AFTER_TOUCH_CHANNEL		= 0xD0,	///< Channel (monophonic) AfterTouch
	PITCH_BEND				= 0xE0,	///< Pitch Bend
	SYSTEM_EXCLUSIVE		= 0xF0,	///< System Exclusive
	TIME_CODE_QUARTER_FRAME	= 0xF1,	///< System Common - MIDI Time Code Quarter Frame
	SONG_POSITION			= 0xF2,	///< System Common - Song Position Pointer
	SONG_SELECT				= 0xF3,	///< System Common - Song Select
	TUNE_REQUEST			= 0xF6,	///< System Common - Tune Request
	CLOCK					= 0xF8,	///< System Real Time - Timing Clock
	START					= 0xFA,	///< System Real Time - Start
	CONTINUE				= 0xFB,	///< System Real Time - Continue
	STOP					= 0xFC,	///< System Real Time - Stop
	ACTIVE_SENSING			= 0xFE,	///< System Real Time - Active Sensing
	SYSTEM_RESET			= 0xFF	///< System Real Time - System Reset
};

#define MIDI_SYSTEM_EXCLUSIVE_INDEX_ENTRIES			128

struct Message {
	uint32_t nTimestamp;
	midi::Types tType;
	uint8_t nChannel;
	uint8_t nData1;
	uint8_t nData2;
	uint8_t aSystemExclusive[MIDI_SYSTEM_EXCLUSIVE_INDEX_ENTRIES];
	uint8_t nBytesCount;
};

enum class Direction {
	INPUT = (1U << 0), OUTPUT = (1U << 1)
};

struct Timecode {
	uint8_t nFrames;
	uint8_t nSeconds;
	uint8_t nMinutes;
	uint8_t nHours;
	uint8_t nType;
} __attribute__((packed));

enum class TimecodeType {
	FILM = 0, EBU, DF, SMPTE, UNKNOWN = 255
};

namespace bpm {
static constexpr auto MIN = 8;
static constexpr auto MAX = 300;
}  // namespace bpm

typedef void (*thunk_irq_timer1_t)();

}  // namespace midi

class Midi {
public:
	Midi();

	void Init(midi::Direction tMidiDirection);

	void Run();

	midi::Direction GetDirection() const {
		return m_Direction;
	}

	void SetBaudrate(uint32_t nBaudrate) {
		m_nBaudrate = nBaudrate;
	}
	uint32_t GetBaudrate() const {
		return m_nBaudrate;
	}

	void SetActiveSense(bool bActiveSense = true) {
		m_bActiveSense = bActiveSense;
	}

	bool GetActiveSense() const {
		return m_bActiveSense;
	}

	void SetChannel(uint8_t nChannel) {
		m_nInputChannel = nChannel;
	}

	uint8_t GetChannel() const {
		return m_nInputChannel;
	}

	void SendTimeCode(const struct midi::Timecode *pTimeCode) {
		uint8_t data[10] = {0xF0, 0x7F, 0x7F, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0xF7};

		data[5] = static_cast<uint8_t>((((pTimeCode->nType) & 0x03) << 5) | (pTimeCode->nHours & 0x1F));
		data[6] = pTimeCode->nMinutes & 0x3F;
		data[7] = pTimeCode->nSeconds & 0x3F;
		data[8] = pTimeCode->nFrames & 0x1F;

		SendRaw(data, 10);
	}

	void SendQf(const uint8_t nValue) {
		uint8_t data[2];

		data[0] = 0xF1;
		data[1] = nValue;

		SendRaw(data, 2);
	}

	void SendQf(const struct midi::Timecode *tMidiTimeCode, uint32_t& nMidiQuarterFramePiece) {
		auto data = static_cast<uint8_t>(nMidiQuarterFramePiece << 4);

		switch (nMidiQuarterFramePiece) {
		case 0:
			data = data | (tMidiTimeCode->nFrames & 0x0F);
			break;
		case 1:
			data = data | static_cast<uint8_t>((tMidiTimeCode->nFrames & 0x10) >> 4);
			break;
		case 2:
			data = data | (tMidiTimeCode->nSeconds & 0x0F);
			break;
		case 3:
			data = data | static_cast<uint8_t>((tMidiTimeCode->nSeconds & 0x30) >> 4);
			break;
		case 4:
			data = data | (tMidiTimeCode->nMinutes & 0x0F);
			break;
		case 5:
			data = data | static_cast<uint8_t>((tMidiTimeCode->nMinutes & 0x30) >> 4);
			break;
		case 6:
			data = data | (tMidiTimeCode->nHours & 0x0F);
			break;
		case 7:
			data = static_cast<uint8_t>(data | (tMidiTimeCode->nType << 1) | ((tMidiTimeCode->nHours & 0x10) >> 4));
			break;
		default:
			break;
		}

		SendQf(data);

		nMidiQuarterFramePiece = (nMidiQuarterFramePiece + 1) & 0x07;
	}

	void SendRaw(const uint8_t *pData, uint32_t nLength) {
		FUNC_PREFIX (uart_transmit(EXT_MIDI_UART_BASE, pData, nLength));
	}

	void SendRaw(uint8_t nByte) {
		SendRaw(&nByte, 1);
	}

	void SendRaw(midi::Types tType) {
		SendRaw(static_cast<uint8_t>(tType));
	}

	bool Read(uint8_t nChannel) {
		if (nChannel >= static_cast<uint8_t>(midi::Channel::OFF)) {
			return false; // MIDI Input disabled.
		}

		if (!Parse()) {
			return false;
		}

		HandleNullVelocityNoteOnAsNoteOff();
		const auto isChannelMatch = InputFilter(nChannel);

		return isChannelMatch;
	}

	bool Read() {
		return Read(m_nInputChannel);
	}

	const struct midi::Message *GetMessage() const {
		return &m_Message;
	}

	uint32_t GetMessageTimeStamp() const {
		return m_Message.nTimestamp;
	}

	midi::Types GetMessageType() const {
		return static_cast<midi::Types>(m_Message.tType);
	}

	void GetMessageData(uint8_t &nData1, uint8_t &nData2) const {
		nData1 = m_Message.nData1;
		nData2 = m_Message.nData2;
	}

	const uint8_t* GetSystemExclusive(uint8_t &nLength) const {
		nLength = m_Message.nBytesCount;
		return reinterpret_cast<const uint8_t*>(m_Message.aSystemExclusive);
	}

	void SetIrqTimer1(midi::thunk_irq_timer1_t pFunc);

	uint32_t GetUpdatesPerSecond();

	midi::ActiveSenseState GetActiveSenseState();

	void Print() {
		printf("MIDI\n");
		printf(" Direction    : %s\n", m_Direction == midi::Direction::INPUT ? "Input" : "Output");
		if (m_Direction == midi::Direction::INPUT) {
			printf(" Channel      : %d %s\n", m_nInputChannel, m_nInputChannel == 0 ? "(OMNI mode)" : "");
		}
		printf(" Active sense : %s\n", m_bActiveSense ? "Enabled" : "Disabled");
		printf(" Baudrate     : %d %s\n", static_cast<int>(m_nBaudrate), m_nBaudrate == midi::defaults::BAUDRATE ? "(Default)" : "");
	}

	static Midi* Get() {
		return s_pThis;
	}

private:
	bool InputFilter(uint8_t nChannel) const {
		if (m_Message.tType == midi::Types::INVALIDE_TYPE)
			return false;

		// First, check if the received message is Channel
		if (m_Message.tType >= midi::Types::NOTE_OFF && m_Message.tType <= midi::Types::PITCH_BEND) {
			// Then we need to know if we listen to it
			if ((m_Message.nChannel == nChannel) || (nChannel == static_cast<uint8_t>(midi::Channel::OMNI))) {
				return true;
			} else {
				// We don't listen to this channel
				return false;
			}
		} else {
			// System messages are always received
			return true;
		}
	}


	midi::Types GetTypeFromStatusByte(uint8_t nStatusByte) const {
		if ((nStatusByte < 0x80) || (nStatusByte == 0xf4) || (nStatusByte == 0xf5) || (nStatusByte == 0xf9) || (nStatusByte == 0xfD)) {
			// Data bytes and undefined.
			return midi::Types::INVALIDE_TYPE;
		}

		if (nStatusByte < 0xF0) {
			// Channel message, remove channel nibble.
			return static_cast<midi::Types>(nStatusByte & 0xF0);
		}

		return static_cast<midi::Types>(nStatusByte);
	}

	uint8_t GetChannelFromStatusByte(uint8_t nStatusByte) const {
		return static_cast<uint8_t>((nStatusByte & 0x0F) + 1);
	}

	bool isChannelMessage(const midi::Types nType) const {
		return nType == midi::Types::NOTE_OFF || nType == midi::Types::NOTE_ON
				|| nType == midi::Types::CONTROL_CHANGE
				|| nType == midi::Types::AFTER_TOUCH_POLY
				|| nType == midi::Types::AFTER_TOUCH_CHANNEL || nType == midi::Types::PITCH_BEND
				|| nType == midi::Types::PROGRAM_CHANGE;
	}

	void HandleNullVelocityNoteOnAsNoteOff() {
		if (midi::HANDLE_NULL_VELOCITY_NOTE_ON_AS_NOTE_OFF && (m_Message.tType == midi::Types::NOTE_ON) && (m_Message.nData2 == 0)) {
			m_Message.tType = midi::Types::NOTE_OFF;
		}
	}

	void ResetInput() {
		m_nPendingMessageIndex = 0;
		m_nPendingMessageExpectedLenght = 0;
		m_nRunningStatusRx = static_cast<uint8_t>(midi::Types::INVALIDE_TYPE);
	}

	bool Parse();
	bool ReadRaw(uint8_t *byte, uint32_t *timestamp);

private:
	uint32_t m_nBaudrate { midi::defaults::BAUDRATE };
	midi::Direction m_Direction { midi::Direction::INPUT };
	bool m_bActiveSense { false };
	struct midi::Message m_Message;
	uint8_t m_nInputChannel { static_cast<uint8_t>(midi::Channel::OMNI) };
	uint8_t m_nPendingMessageIndex { 0 };
	uint8_t m_nPendingMessageExpectedLenght { 0 };
	uint8_t m_nRunningStatusRx { static_cast<uint8_t>(midi::Types::INVALIDE_TYPE) };
	uint8_t m_aPendingMessage[8];

	static Midi *s_pThis;
};

#endif /* MIDI_H_ */
