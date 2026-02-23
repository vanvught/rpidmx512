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

namespace midi
{
/**
 * NoteOn with 0 velocity should be handled as NoteOf.
 * Set to true  to get NoteOff events when receiving null-velocity NoteOn messages.
 * Set to false to get NoteOn  events when receiving null-velocity NoteOn messages.
 */
inline constexpr auto HANDLE_NULL_VELOCITY_NOTE_ON_AS_NOTE_OFF = true;

namespace defaults
{
inline constexpr auto BAUDRATE = 31250;
} // namespace defaults

namespace pitchbend
{
inline constexpr auto MIN = -8192;
inline constexpr auto MAX = 8191;
} // namespace pitchbend

namespace control
{
enum class Change : uint8_t
{
    ALL_SOUND_OFF = 0x78,
    RESET_ALL_CONTROLLERS = 0x79,
    LOCAL_CONTROL = 0x7A,
    ALL_NOTES_OFF = 0x7B,
    OMNI_MODE_OFF = 0x7C,
    OMNI_MODE_ON = 0x7D,
    MONO_MODE_ON = 0x7E,
    POLY_MODE_ON = 0x7F
};

enum class Function : uint8_t
{
    BANK_SELECT = 0x00,           ///< MSB
    MODULATION_WHEEL = 0x01,      ///< MSB
    BREATH_CONTROLLER = 0x02,     ///< MSB
    UNDEFINED_03 = 0x03,          ///< MSB
    FOOT_CONTROLLER = 0x04,       ///< MSB
    PORTAMENTO_TIME = 0x05,       ///< MSB
    DATA_ENTRY_MSB = 0x06,        ///< MSB
    CHANNEL_VOLUME = 0x07,        ///< MSB
    BALANCE = 0x08,               ///< MSB
    UNDEFINED_09 = 0x09,          ///< MSB
    PAN = 0x0A,                   ///< MSB
    EXPRESSION_CONTROLLER = 0x0B, ///< MSB
    EFFECT_CONTROL_1 = 0x0C,      ///< MSB
    EFFECT_CONTROL_2 = 0x0D,      ///< MSB
    UNDEFINED_0E = 0x0E,          ///< MSB
    UNDEFINED_0F = 0x0F,          ///< MSB
    GP_CONTROLLER_1 = 0x10,       ///< MSB
    GP_CONTROLLER_2 = 0x11,       ///< MSB
    GP_CONTROLLER_3 = 0x12,       ///< MSB
    GP_CONTROLLER_4 = 0x13,       ///< MSB
    DAMPER_PEDAL_ON_OFF = 0x40,   ///< 63 off, 64 on
    PORTAMENTO_ON_OFF = 0x41,     ///< 63 off, 64 on
    SOSTENUTO_ON_OFF = 0x42,      ///< 63 off, 64 on
    SOFT_PEDAL_ON_OFF = 0x43,     ///< 63 off, 64 on
    LEGATO_FOOTSWITCH = 0x44,     ///< 63 off, 64 on
    HOLD_2 = 0x45                 ///< 63 off, 64 on
};
} // namespace control

enum class Channel : uint8_t
{
    OMNI = 0,
    OFF = 17
};

enum class ActiveSenseState
{
    NOT_ENABLED,
    ENABLED,
    FAILED
};

enum class Types : uint8_t
{
    INVALIDE_TYPE = 0x00,           ///< For notifying errors
    NOTE_OFF = 0x80,                ///< Note Off
    NOTE_ON = 0x90,                 ///< Note On
    AFTER_TOUCH_POLY = 0xA0,        ///< Polyphonic AfterTouch
    CONTROL_CHANGE = 0xB0,          ///< Control Change / Channel Mode
    PROGRAM_CHANGE = 0xC0,          ///< Program Change
    AFTER_TOUCH_CHANNEL = 0xD0,     ///< Channel (monophonic) AfterTouch
    PITCH_BEND = 0xE0,              ///< Pitch Bend
    SYSTEM_EXCLUSIVE = 0xF0,        ///< System Exclusive
    TIME_CODE_QUARTER_FRAME = 0xF1, ///< System Common - MIDI Time Code Quarter Frame
    SONG_POSITION = 0xF2,           ///< System Common - Song Position Pointer
    SONG_SELECT = 0xF3,             ///< System Common - Song Select
    TUNE_REQUEST = 0xF6,            ///< System Common - Tune Request
    CLOCK = 0xF8,                   ///< System Real Time - Timing Clock
    START = 0xFA,                   ///< System Real Time - Start
    CONTINUE = 0xFB,                ///< System Real Time - Continue
    STOP = 0xFC,                    ///< System Real Time - Stop
    ACTIVE_SENSING = 0xFE,          ///< System Real Time - Active Sensing
    SYSTEM_RESET = 0xFF             ///< System Real Time - System Reset
};

#define MIDI_SYSTEM_EXCLUSIVE_INDEX_ENTRIES 128

struct Message
{
    uint32_t timestamp;
    midi::Types type;
    uint8_t channel;
    uint8_t data1;
    uint8_t data2;
    uint8_t system_exclusive[MIDI_SYSTEM_EXCLUSIVE_INDEX_ENTRIES];
    uint8_t bytes_count;
};

enum class Direction
{
    INPUT = (1U << 0),
    OUTPUT = (1U << 1)
};

struct Timecode
{
    uint8_t frames;
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
    uint8_t type;
} __attribute__((packed));

enum class TimecodeType
{
    FILM = 0,
    EBU,
    DF,
    SMPTE,
    UNKNOWN = 255
};

namespace bpm
{
inline constexpr auto MIN = 8;
inline constexpr auto MAX = 300;
} // namespace bpm

typedef void (*thunk_irq_timer1_t)();

} // namespace midi

class Midi
{
   public:
    Midi() { s_this = this; }

    void Init(midi::Direction direction);

    void Run();

    midi::Direction GetDirection() const { return direction_; }

    void SetBaudrate(uint32_t baudrate) { baudrate_ = baudrate; }
    uint32_t GetBaudrate() const { return baudrate_; }

    void SetActiveSense(bool active_sense = true) { active_sense_ = active_sense; }

    bool GetActiveSense() const { return active_sense_; }

    void SetChannel(uint8_t channel) { input_channel_ = channel; }

    uint8_t GetChannel() const { return input_channel_; }

    void SendTimeCode(const struct midi::Timecode* timecode)
    {
        uint8_t data[10] = {0xF0, 0x7F, 0x7F, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0xF7};

        data[5] = static_cast<uint8_t>((((timecode->type) & 0x03) << 5) | (timecode->hours & 0x1F));
        data[6] = timecode->minutes & 0x3F;
        data[7] = timecode->seconds & 0x3F;
        data[8] = timecode->frames & 0x1F;

        SendRaw(data, 10);
    }

    void SendQf(uint8_t value)
    {
        uint8_t data[2];

        data[0] = 0xF1;
        data[1] = value;

        SendRaw(data, 2);
    }

    void SendQf(const struct midi::Timecode* timecode, uint32_t& quarter_frame_piece)
    {
        auto data = static_cast<uint8_t>(quarter_frame_piece << 4);

        switch (quarter_frame_piece)
        {
            case 0:
                data = data | (timecode->frames & 0x0F);
                break;
            case 1:
                data = data | static_cast<uint8_t>((timecode->frames & 0x10) >> 4);
                break;
            case 2:
                data = data | (timecode->seconds & 0x0F);
                break;
            case 3:
                data = data | static_cast<uint8_t>((timecode->seconds & 0x30) >> 4);
                break;
            case 4:
                data = data | (timecode->minutes & 0x0F);
                break;
            case 5:
                data = data | static_cast<uint8_t>((timecode->minutes & 0x30) >> 4);
                break;
            case 6:
                data = data | (timecode->hours & 0x0F);
                break;
            case 7:
                data = static_cast<uint8_t>(data | (timecode->type << 1) | ((timecode->hours & 0x10) >> 4));
                break;
            default:
                break;
        }

        SendQf(data);

        quarter_frame_piece = (quarter_frame_piece + 1) & 0x07;
    }

    void SendRaw(const uint8_t* data, uint32_t length) { FUNC_PREFIX(UartTransmit(EXT_MIDI_UART_BASE, data, length)); }

    void SendRaw(uint8_t byte) { SendRaw(&byte, 1); }

    void SendRaw(midi::Types type) { SendRaw(static_cast<uint8_t>(type)); }

    bool Read(uint8_t channel)
    {
        if (channel >= static_cast<uint8_t>(midi::Channel::OFF))
        {
            return false; // MIDI Input disabled.
        }

        if (!Parse())
        {
            return false;
        }

        HandleNullVelocityNoteOnAsNoteOff();
        const auto kIsChannelMatch = InputFilter(channel);

        return kIsChannelMatch;
    }

    bool Read() { return Read(input_channel_); }

    const struct midi::Message* GetMessage() const { return &message_; }

    uint32_t GetMessageTimeStamp() const { return message_.timestamp; }

    midi::Types GetMessageType() const { return static_cast<midi::Types>(message_.type); }

    void GetMessageData(uint8_t& data1, uint8_t& data2) const
    {
        data1 = message_.data1;
        data2 = message_.data2;
    }

    const uint8_t* GetSystemExclusive(uint8_t& length) const
    {
        length = message_.bytes_count;
        return reinterpret_cast<const uint8_t*>(message_.system_exclusive);
    }

    void SetIrqTimer1(midi::thunk_irq_timer1_t func);

    uint32_t GetUpdatesPerSecond();

    midi::ActiveSenseState GetActiveSenseState();

    void Print()
    {
        printf("MIDI\n");
        printf(" Direction    : %s\n", direction_ == midi::Direction::INPUT ? "Input" : "Output");
        if (direction_ == midi::Direction::INPUT)
        {
            printf(" Channel      : %d %s\n", input_channel_, input_channel_ == 0 ? "(OMNI mode)" : "");
        }
        printf(" Active sense : %s\n", active_sense_ ? "Enabled" : "Disabled");
        printf(" Baudrate     : %d %s\n", static_cast<int>(baudrate_), baudrate_ == midi::defaults::BAUDRATE ? "(Default)" : "");
    }

    static Midi* Get() { return s_this; }

   private:
    bool InputFilter(uint8_t channel) const
    {
        if (message_.type == midi::Types::INVALIDE_TYPE) return false;

        // First, check if the received message is Channel
        if (message_.type >= midi::Types::NOTE_OFF && message_.type <= midi::Types::PITCH_BEND)
        {
            // Then we need to know if we listen to it
            if ((message_.channel == channel) || (channel == static_cast<uint8_t>(midi::Channel::OMNI)))
            {
                return true;
            }
            else
            {
                // We don't listen to this channel
                return false;
            }
        }
        else
        {
            // System messages are always received
            return true;
        }
    }

    midi::Types GetTypeFromStatusByte(uint8_t status_byte) const
    {
        if ((status_byte < 0x80) || (status_byte == 0xf4) || (status_byte == 0xf5) || (status_byte == 0xf9) || (status_byte == 0xfD))
        {
            // Data bytes and undefined.
            return midi::Types::INVALIDE_TYPE;
        }

        if (status_byte < 0xF0)
        {
            // Channel message, remove channel nibble.
            return static_cast<midi::Types>(status_byte & 0xF0);
        }

        return static_cast<midi::Types>(status_byte);
    }

    uint8_t GetChannelFromStatusByte(uint8_t status_byte) const { return static_cast<uint8_t>((status_byte & 0x0F) + 1); }

    bool IsChannelMessage(midi::Types type) const
    {
        return type == midi::Types::NOTE_OFF || type == midi::Types::NOTE_ON || type == midi::Types::CONTROL_CHANGE || type == midi::Types::AFTER_TOUCH_POLY ||
               type == midi::Types::AFTER_TOUCH_CHANNEL || type == midi::Types::PITCH_BEND || type == midi::Types::PROGRAM_CHANGE;
    }

    void HandleNullVelocityNoteOnAsNoteOff()
    {
        if (midi::HANDLE_NULL_VELOCITY_NOTE_ON_AS_NOTE_OFF && (message_.type == midi::Types::NOTE_ON) && (message_.data2 == 0))
        {
            message_.type = midi::Types::NOTE_OFF;
        }
    }

    void ResetInput()
    {
        pending_message_index_ = 0;
        pending_message_expected_lenght_ = 0;
        running_status_rx_ = static_cast<uint8_t>(midi::Types::INVALIDE_TYPE);
    }

    bool Parse();
    bool ReadRaw(uint8_t* byte, uint32_t* timestamp);

   private:
    uint32_t baudrate_{midi::defaults::BAUDRATE};
    midi::Direction direction_{midi::Direction::INPUT};
    bool active_sense_{false};
    struct midi::Message message_;
    uint8_t input_channel_{static_cast<uint8_t>(midi::Channel::OMNI)};
    uint8_t pending_message_index_{0};
    uint8_t pending_message_expected_lenght_{0};
    uint8_t running_status_rx_{static_cast<uint8_t>(midi::Types::INVALIDE_TYPE)};
    uint8_t pending_message_[8];

    inline static Midi* s_this;
};

#endif  // MIDI_H_
