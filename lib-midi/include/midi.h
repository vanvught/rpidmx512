/**
 * @file midi.h
 *
 */
/* Copyright (C) 2016-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <stdint.h>

namespace midi {

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

}  // namespace midi

class Midi {
public:
	Midi();

	void Init(midi::Direction tMidiDirection);

	void Run();

	midi::Direction GetDirection() const {
		return m_tDirection;
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

	uint32_t GetUpdatesPerSeconde();

	void SetChannel(uint8_t nChannel) {
		m_nInputChannel = nChannel;
	}

	uint8_t GetChannel() const {
		return m_nInputChannel;
	}

	midi::ActiveSenseState GetActiveSenseState();

	const char* GetInterfaceDescription() const {
		return "UART2";
	}

	void SendTimeCode(const struct midi::Timecode *tTimeCode);

	void SendQf(uint8_t nData);
	void SendQf(const struct midi::Timecode *tMidiTimeCode, uint32_t& nMidiQuarterFramePiece);

	void SendRaw(const uint8_t *pData, uint16_t nLength) {
		SendUart2(pData, nLength);
	}
	void SendRaw(uint8_t nByte) {
		SendRaw(&nByte, 1);
	}
	void SendRaw(midi::Types tType) {
		SendRaw(static_cast<uint8_t>(tType));
	}

	bool Read(uint8_t nChannel);
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

	static Midi* Get() {
		return s_pThis;
	}

	void Print();

private:
	void InitUart2();
	void SendUart2(const uint8_t *data, uint16_t length);
	bool InputFilter(uint8_t nChannel) const;
	midi::Types GetTypeFromStatusByte(uint8_t nStatusByte) const;
	uint8_t GetChannelFromStatusByte(uint8_t nStatusByte) const;
	bool isChannelMessage(midi::Types tType) const;
	void HandleNullVelocityNoteOnAsNoteOff();
	bool Parse();
	void ResetInput();
	bool ReadRaw(uint8_t *byte, uint32_t *timestamp);

private:
	uint32_t m_nBaudrate { midi::defaults::BAUDRATE };
	midi::Direction m_tDirection { midi::Direction::INPUT };
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
