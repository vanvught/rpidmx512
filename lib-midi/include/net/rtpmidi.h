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
 #include "firmware/debug/debug_debug.h"

/**
 * @namespace rtpmidi
 * @brief Contains constants and structures for RTP-MIDI communication.
 */
namespace rtpmidi
{
/**
 * @brief Buffer size for RTP-MIDI messages.
 */
inline constexpr auto kBufferSize = 512U;

/**
 * @struct Header
 * @brief Structure representing the RTP-MIDI header.
 */
struct Header
{
    uint16_t fixed;
    uint16_t sequence_number;
    uint32_t timestamp;
    uint32_t nSenderSSRC;
} __attribute__((packed));

/**
 * @brief Offset for commands in the RTP-MIDI buffer.
 */
inline constexpr auto kCommandOffset = sizeof(struct Header);
} // namespace rtpmidi

/**
 * @class RtpMidi
 * @brief Class implementing RTP-MIDI communication.
 *
 * RtpMidi extends the AppleMidi class to handle real-time MIDI data transfer
 * over RTP. It supports sending raw MIDI messages, timecodes, and MIDI quarter frames.
 */
class RtpMidi final : public AppleMidi
{
   public:
    RtpMidi()
    {
        DEBUG_ENTRY();

        assert(s_this == nullptr);
        s_this = this;

        DEBUG_EXIT();
    }

    /**
     * @brief Starts the RTP-MIDI service.
     */
    void Start()
    {
        DEBUG_ENTRY();

        AppleMidi::Start();

        send_buffer_ = new uint8_t[rtpmidi::kBufferSize];
        assert(send_buffer_ != nullptr);

        auto* header = reinterpret_cast<rtpmidi::Header*>(send_buffer_);
        header->fixed = 0x6180;
        header->nSenderSSRC = AppleMidi::GetSSRC();

        DEBUG_EXIT();
    }

    /**
     * @brief Stops the RTP-MIDI service.
     */
    void Stop()
    {
        DEBUG_ENTRY();

        AppleMidi::Stop();

        DEBUG_EXIT();
    }

    /**
     * @brief Sends a single raw MIDI byte.
     * @param nByte The raw MIDI byte to send.
     */
    void SendRaw(uint8_t byte)
    {
        auto* data = &send_buffer_[rtpmidi::kCommandOffset + 1];
        data[0] = byte;
        Send(1);
    }

    /**
     * @brief Sends a raw MIDI type.
     * @param type The MIDI type to send.
     */
    void SendRaw(midi::Types type) { SendRaw(static_cast<uint8_t>(type)); }

    /**
     * @brief Sends a full MIDI timecode message.
     * @param tTimeCode Pointer to the MIDI timecode structure.
     */
    void SendTimeCode(const midi::Timecode* timecode)
    {
        auto* data = &send_buffer_[rtpmidi::kCommandOffset + 1];

        data[0] = 0xF0;
        data[1] = 0x7F;
        data[2] = 0x7F;
        data[3] = 0x01;
        data[4] = 0x01;
        data[5] = static_cast<uint8_t>(((timecode->type) & 0x03) << 5) | (timecode->hours & 0x1F);
        data[6] = timecode->minutes & 0x3F;
        data[7] = timecode->seconds & 0x3F;
        data[8] = timecode->frames & 0x1F;
        data[9] = 0xF7;

        Send(10);
    }

    /**
     * @brief Sends a MIDI quarter frame message.
     * @param nValue The quarter frame value to send.
     */
    void SendQf(uint8_t value)
    {
        auto* data = &send_buffer_[rtpmidi::kCommandOffset + 1];

        data[0] = 0xF1;
        data[1] = value;

        Send(2);
    }

    /**
     * @brief Sends a MIDI quarter frame message based on timecode and updates the piece index.
     * @param timeCode Pointer to the MIDI timecode structure.
     * @param nMidiQuarterFramePiece Reference to the quarter frame piece index.
     */
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

    /**
     * @brief Sets the RTP-MIDI handler.
     * @param pRtpMidiHandler Pointer to the RTP-MIDI handler instance.
     */
    void SetHandler(RtpMidiHandler* handler) { handler_ = handler; }

    /**
     * @brief Prints information about the RTP-MIDI instance.
     */
    void Print() { AppleMidi::Print(); }

    /**
     * @brief Gets the current instance of RtpMidi.
     * @return Pointer to the current RtpMidi instance.
     */
    static RtpMidi* Get() { return s_this; }

   private:
    void HandleRtpMidi(const uint8_t* buffer) override;

    /**
     * @brief Decode the delta time from the MIDI command buffer.
     *
     * Decodes the variable-length delta time from the RTP MIDI command buffer.
     *
     * @param nCommandLength The total length of the command.
     * @param nOffset The offset in the receive buffer to start decoding.
     * @return The size (in bytes) of the decoded delta time.
     */
    int32_t DecodeTime(uint32_t command_length, uint32_t offset);

    /**
     * @brief Decode a MIDI message from the command buffer.
     *
     * Decodes an RTP MIDI message from the receive buffer and populates the MIDI message structure.
     *
     * @param nCommandLength The length of the MIDI command to decode.
     * @param nOffset The offset in the receive buffer to start decoding.
     * @return The size (in bytes) of the decoded MIDI message.
     */
    int32_t DecodeMidi(uint32_t command_length, uint32_t offset);

    midi::Types GetTypeFromStatusByte(uint8_t status_byte)
    {
        if ((status_byte < 0x80) || (status_byte == 0xf4) || (status_byte == 0xf5) || (status_byte == 0xf9) || (status_byte == 0xfD))
        {
            return midi::Types::INVALIDE_TYPE;
        }

        if (status_byte < 0xF0)
        {
            return static_cast<midi::Types>(status_byte & 0xF0);
        }

        return static_cast<midi::Types>(status_byte);
    }

    uint8_t GetChannelFromStatusByte(uint8_t status_byte) { return static_cast<uint8_t>((status_byte & 0x0F) + 1); }

    void Send(uint32_t length)
    {
        auto* header = reinterpret_cast<rtpmidi::Header*>(send_buffer_);

        header->sequence_number = __builtin_bswap16(sequence_number_++);
        header->timestamp = __builtin_bswap32(AppleMidi::Now());

        send_buffer_[rtpmidi::kCommandOffset] = static_cast<uint8_t>(length); // FIXME BUG works now only

        AppleMidi::Send(send_buffer_, 1 + sizeof(struct rtpmidi::Header) + length);
    }

   private:
    midi::Message message_;
    RtpMidiHandler* handler_{nullptr}; ///< Pointer to the RTP-MIDI handler.
    uint8_t* receive_buffer_{nullptr};          ///< Receive buffer pointer.
    uint8_t* send_buffer_{nullptr};            ///< Send buffer pointer.
    uint16_t sequence_number_{0};              ///< Sequence number for outgoing messages.

    static inline RtpMidi* s_this; ///< Static pointer to the current instance.
};

#endif  // NET_RTPMIDI_H_
