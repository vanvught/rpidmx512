/**
 * @file applemidi.cpp
 */
/* Copyright (C) 2019-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#if defined(DEBUG_NET_APPLEMIDI)
#undef NDEBUG
#endif

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC push_options
#pragma GCC optimize("O2")
#pragma GCC optimize("no-tree-loop-distribute-patterns")
#endif

#include <cstdint>
#include <cstring>
#include <cassert>

#include "net/applemidi.h"
#include "network.h"
#include "softwaretimers.h"
#include "firmware/debug/debug_dump.h"
 #include "firmware/debug/debug_debug.h"

namespace applemidi
{

} // namespace applemidi

/**
 * @enum TAppleMidiCommand
 * @brief Defines the Apple MIDI command identifiers with network byte order.
 */
enum class AppleMidiCommand : uint16_t
{
    kInvitation = __builtin_bswap16(0x494e),         ///< Invitation 'IN'
    kInvitationAccepted = __builtin_bswap16(0x4f4b), ///< Invitation accepted 'OK'
    kInvitationRejected = __builtin_bswap16(0x4e4f), ///< Invitation refused 'NO'
    kEndsession = __builtin_bswap16(0x4259),         ///< Closing session 'BY'
    kSynchronization = __builtin_bswap16(0x434b),    ///< Clock synchronization 'CK'
    kReceiverFeedback = __builtin_bswap16(0x5253),   ///< Journalling synchronization 'RS'
    kBitrateReceiveLimit = __builtin_bswap16(0x524c) ///< Bitrate 'RL'
};

/**
 * @struct TTimestampSynchronization
 * @brief Represents the structure for timestamp synchronization in Apple MIDI.
 *
 * This structure is used to handle the synchronization of timestamps between devices
 * in an Apple MIDI session.
 */
struct TimestampSynchronization
{
    uint16_t signature;     ///< Packet signature for Apple MIDI.
    uint16_t command;       ///< Command identifier.
    uint32_t ssrc;          ///< Synchronization source identifier.
    uint8_t count;          ///< Count of synchronization steps.
    uint8_t padding[3];     ///< Padding to align the structure.
    uint64_t timestamps[3]; ///< Array of timestamps for synchronization.
} __attribute__((packed));

/**
 * @brief Timeout value in milliseconds for an Apple MIDI session.
 */
static constexpr uint32_t kTimeout = 90 * 1000;

/**
 * @brief Timer handle for session timeout management.
 */
static TimerHandle_t s_timer_id = kTimerIdNone;

/**
 * @brief Timer callback function to handle session timeout.
 *
 * This function is called when the session times out, and it resets the session
 * and deletes the timer.
 *
 * @param nHandle Handle of the expired timer (unused).
 */
static void TimeoutTimer([[maybe_unused]] TimerHandle_t handle)
{
    if (AppleMidi::GetSessionState() == applemidi::SessionState::kEstablished)
    {
        AppleMidi::ResetSession();
        SoftwareTimerDelete(s_timer_id);
        DEBUG_PUTS("End Session {time-out}");
    }
}

typedef union pcast32
{
    uint32_t u32;
    uint8_t u8[4];
} _pcast32;

AppleMidi::AppleMidi()
{
    DEBUG_ENTRY();
    assert(s_this == nullptr);
    s_this = this;

    exchange_packet_reply_.signature = kSignature;
    exchange_packet_reply_.protocol_version = __builtin_bswap32(applemidi::kVersion);

    uint8_t mac_address[network::MAC_SIZE];
     network::iface::CopyMacAddressTo(mac_address);
    _pcast32 cast32;
    memcpy(cast32.u8, &mac_address[2], 4);
    ssrc_ = cast32.u32;

    SetSessionName( network::iface::GetHostName());

    memset(&session_status_, 0, sizeof(struct applemidi::SessionStatus));

    DEBUG_PRINTF("applemidi::EXCHANGE_PACKET_MIN_LENGTH = %u", static_cast<uint32_t>(applemidi::kExchangePacketMinLength));
    DEBUG_EXIT();
}

void AppleMidi::InputControlMessage(const uint8_t* buffer, uint32_t size, uint32_t from_ip, uint16_t from_port)
{
    DEBUG_ENTRY();

    if (__builtin_expect((size >= applemidi::kExchangePacketMinLength), 1))
    {
        if (*reinterpret_cast<const uint16_t*>(buffer) != kSignature)
        {
            DEBUG_EXIT();
            return;
        }
    }

    auto* packet = reinterpret_cast<const applemidi::ExchangePacket*>(buffer);

    DEBUG_PRINTF("Command: %.4x, session_state=%u", packet->command, static_cast<uint32_t>(session_status_.session_state));

    if (session_status_.session_state == applemidi::SessionState::kWaitingInControl)
    {
        DEBUG_PUTS("SESSION_STATE_WAITING_IN_CONTROL");

        if ((session_status_.remote_ip == 0) && (static_cast<AppleMidiCommand>(packet->command) == AppleMidiCommand::kInvitation))
        {
            DEBUG_PUTS("Invitation");

            exchange_packet_reply_.ssrc = GetSSRC();
            exchange_packet_reply_.command = static_cast<uint16_t>(AppleMidiCommand::kInvitationAccepted);
            exchange_packet_reply_.initiator_token = packet->initiator_token;

            network::udp::Send(handle_control_, reinterpret_cast<const uint8_t*>(&exchange_packet_reply_), exchange_packet_reply_size_, from_ip, from_port);

            debug::Dump(&exchange_packet_reply_, exchange_packet_reply_size_);

            session_status_.session_state = applemidi::SessionState::kWaitingInMidi;
            session_status_.remote_ip = from_ip;

            DEBUG_EXIT();
            return;
        }
        else
        {
            DEBUG_EXIT();
            return;
        }
    }

    if (session_status_.session_state == applemidi::SessionState::kEstablished)
    {
        DEBUG_PUTS("SESSION_STATE_ESTABLISHED");

        if ((session_status_.remote_ip == from_ip) && (static_cast<AppleMidiCommand>(packet->command) == AppleMidiCommand::kEndsession))
        {
            session_status_.session_state = applemidi::SessionState::kWaitingInControl;
            session_status_.remote_ip = 0;

            DEBUG_PUTS("End Session");
            DEBUG_EXIT();
            return;
        }

        if (static_cast<AppleMidiCommand>(packet->command) == AppleMidiCommand::kInvitation)
        {
            DEBUG_PUTS("Invitation rejected");

            exchange_packet_reply_.ssrc = GetSSRC();
            exchange_packet_reply_.command = static_cast<uint16_t>(AppleMidiCommand::kInvitationRejected);
            exchange_packet_reply_.initiator_token = packet->initiator_token;

            network::udp::Send(handle_control_, reinterpret_cast<const uint8_t*>(&exchange_packet_reply_), exchange_packet_reply_size_, from_ip, from_port);

            DEBUG_EXIT();
            return;
        }
    }

    DEBUG_EXIT();
}

void AppleMidi::InputMidiMessage(const uint8_t* buffer, uint32_t size, uint32_t from_ip, uint16_t from_port)
{
    DEBUG_ENTRY();

    if (__builtin_expect((size >= 12), 0))
    {
        if (session_status_.remote_ip != from_ip)
        {
            DEBUG_EXIT();
            return;
        }
    }

    if (*reinterpret_cast<const uint16_t*>(buffer) == 0x6180)
    {
        HandleRtpMidi(buffer);

        DEBUG_EXIT();
        return;
    }

    if (size >= applemidi::kExchangePacketMinLength)
    {
        if (*reinterpret_cast<const uint16_t*>(buffer) == kSignature)
        {
            if (session_status_.session_state == applemidi::SessionState::kWaitingInMidi)
            {
                DEBUG_PUTS("SESSION_STATE_WAITING_IN_MIDI");

                auto* packet = reinterpret_cast<const applemidi::ExchangePacket*>(buffer);

                DEBUG_PRINTF("Command: %.4x", packet->command);

                if (static_cast<AppleMidiCommand>(packet->command) == AppleMidiCommand::kInvitation)
                {
                    DEBUG_PUTS("Invitation");

                    exchange_packet_reply_.ssrc = GetSSRC();
                    exchange_packet_reply_.command = static_cast<uint16_t>(AppleMidiCommand::kInvitationAccepted);
                    exchange_packet_reply_.initiator_token = packet->initiator_token;

                    network::udp::Send(handle_midi_, reinterpret_cast<const uint8_t*>(&exchange_packet_reply_), exchange_packet_reply_size_, from_ip, from_port);

                    session_status_.session_state = applemidi::SessionState::kEstablished;
                    session_status_.remote_port_midi = from_port;

                    s_timer_id = SoftwareTimerAdd(kTimeout, TimeoutTimer);
                }

                DEBUG_EXIT();
                return;
            }

            if (session_status_.session_state == applemidi::SessionState::kEstablished)
            {
                DEBUG_PUTS("SESSION_STATE_ESTABLISHED");

                auto* packet = reinterpret_cast<const applemidi::ExchangePacket*>(buffer);

                if (static_cast<AppleMidiCommand>(packet->command) == AppleMidiCommand::kSynchronization)
                {
                    DEBUG_PUTS("Timestamp Synchronization");
                    auto* t = reinterpret_cast<TimestampSynchronization*>(const_cast<uint8_t*>(buffer));

                    if (t->count == 0)
                    {
                        t->ssrc = GetSSRC();
                        t->count = 1;
                        t->timestamps[1] = __builtin_bswap64(Now());

                        network::udp::Send(handle_midi_, buffer, sizeof(struct TimestampSynchronization), from_ip, from_port);
                        SoftwareTimerChange(s_timer_id, kTimeout);
                    }
                    else if (t->count == 1)
                    {
                        t->ssrc = GetSSRC();
                        t->count = 2;
                        t->timestamps[2] = __builtin_bswap64(Now());

                        network::udp::Send(handle_midi_, buffer, sizeof(struct TimestampSynchronization), from_ip, from_port);
                        SoftwareTimerChange(s_timer_id, kTimeout);
                    }
                    else if (t->count == 2)
                    {
                        t->ssrc = GetSSRC();
                        t->count = 0;
                        t->timestamps[0] = __builtin_bswap64(Now());
                        t->timestamps[1] = 0;
                        t->timestamps[2] = 0;

                        network::udp::Send(handle_midi_, buffer, sizeof(struct TimestampSynchronization), from_ip, from_port);
                        SoftwareTimerChange(s_timer_id, kTimeout);
                    }
                }
            }
        }
    }

    DEBUG_EXIT();
}
