/**
 * @file applemidi.h
 *
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

#ifndef NET_APPLEMIDI_H_
#define NET_APPLEMIDI_H_

#include <cstdint>
#include <cstdio>
#include <algorithm>
#include <cassert>
#include <cstring>

#include "midi.h"
#include "apps/mdns.h"
#include "hal.h"
#include "hal_millis.h"
#include "network.h"
 #include "firmware/debug/debug_debug.h"

namespace applemidi
{
static constexpr size_t kSessionNameLengthMax = 24;
static constexpr uint32_t kVersion = 2;

struct ExchangePacket
{
    uint16_t signature;
    uint16_t command;
    uint32_t protocol_version;
    uint32_t initiator_token;
    uint32_t ssrc;
    uint8_t name[kSessionNameLengthMax + 1];
} __attribute__((packed));

enum class SessionState
{
    kWaitingInControl,
    kWaitingInMidi,
    kInSync,
    kEstablished
};

struct SessionStatus
{
    SessionState session_state;
    uint32_t remote_ip;
    uint16_t remote_port_midi;
};

static constexpr auto kExchangePacketMinLength = sizeof(struct applemidi::ExchangePacket) - applemidi::kSessionNameLengthMax - 1;
} // namespace applemidi

class AppleMidi
{
    static constexpr uint16_t kUpdPortControlDefault = 5004;
    static constexpr uint16_t kUpdPortMidiDefault = kUpdPortControlDefault + 1;
    static constexpr uint16_t kSignature = 0xffff;

   public:
    AppleMidi();

    virtual ~AppleMidi() { Stop(); }

    void Start()
    {
        DEBUG_ENTRY();
        mdns::ServiceRecordAdd(nullptr, mdns::Services::MIDI, nullptr, port_);

        assert(handle_control_ == -1);
        handle_control_ = net::udp::Begin(port_, StaticCallbackFunctionControlMessage);
        assert(handle_control_ != -1);

        assert(handle_midi_ == -1);
        handle_midi_ = net::udp::Begin((port_ + 1U), StaticCallbackFunctionMidiMessage);
        assert(handle_midi_ != -1);

        DEBUG_PRINTF("Session name: [%s]", exchange_packet_reply_.name);

        start_time_ = hal::Millis();

        DEBUG_EXIT();
    }

    void Stop()
    {
        DEBUG_ENTRY();

        assert(handle_midi_ != -1);
        net::udp::End(static_cast<uint16_t>(port_ + 1U));
        handle_midi_ = -1;

        assert(handle_control_ != -1);
        net::udp::End(port_);
        handle_control_ = -1;

        DEBUG_EXIT();
    }

    /**
     * @brief Processes incoming Apple MIDI MIDI messages.
     *
     * Handles MIDI messages such as RTP-MIDI data and timestamp synchronization commands.
     *
     * @param buffer Pointer to the received data buffer.
     * @param nSize Size of the received data.
     * @param from_ip Source IP address.
     * @param from_port Source port.
     */
    void InputMidiMessage(const uint8_t* buffer, uint32_t size, uint32_t from_ip, uint16_t from_port);

    /**
     * @brief Processes incoming Apple MIDI control messages.
     *
     * Handles control messages such as session invitations, session end commands,
     * and synchronization requests.
     *
     * @param buffer Pointer to the received data buffer.
     * @param nSize Size of the received data.
     * @param from_ip Source IP address.
     * @param from_port Source port.
     */
    void InputControlMessage(const uint8_t* buffer, uint32_t size, uint32_t from_ip, uint16_t from_port);

    void SetPort(uint16_t port)
    {
        assert(port > 1024);
        port_ = port;
    }

    void SetSessionName(const char* session_name)
    {
        const auto kLength = std::min(strlen(session_name), static_cast<size_t>(applemidi::kSessionNameLengthMax));
        memcpy(reinterpret_cast<char*>(&exchange_packet_reply_.name), session_name, kLength);
        exchange_packet_reply_.name[kLength] = '\0';
        exchange_packet_reply_size_ = static_cast<uint16_t>(applemidi::kExchangePacketMinLength + 1 + kLength);
    }

    inline uint32_t GetSSRC() { return ssrc_; }

    void Print()
    {
        puts("AppleMIDI");
        printf(" Session : %s\n", exchange_packet_reply_.name);
    }

    static auto GetSessionState()
    {
        assert(s_this != nullptr);
        return s_this->session_status_.session_state;
    }

    static void ResetSession()
    {
        assert(s_this != nullptr);
        s_this->session_status_.session_state = applemidi::SessionState::kWaitingInControl;
        s_this->session_status_.remote_ip = 0;
    }

   protected:
    uint32_t Now()
    {
        const auto kElapsed = hal::Millis() - start_time_;
        return (kElapsed * 10U);
    }

    bool Send(const uint8_t* buffer, uint32_t length)
    {
        if (session_status_.session_state != applemidi::SessionState::kEstablished)
        {
            return false;
        }

        net::udp::Send(handle_midi_, buffer, length, session_status_.remote_ip, session_status_.remote_port_midi);
        return true;
    }

   private:
    /**
     * @brief Static callback function for receiving UDP packets.
     *
     * @param buffer Pointer to the packet buffer.
     * @param nSize Size of the packet buffer.
     * @param from_ip IP address of the sender.
     * @param from_port Port number of the sender.
     */
    void static StaticCallbackFunctionControlMessage(const uint8_t* buffer, uint32_t size, uint32_t from_ip, uint16_t from_port)
    {
        s_this->InputControlMessage(buffer, size, from_ip, from_port);
    }

    /**
     * @brief Static callback function for receiving UDP packets.
     *
     * @param buffer Pointer to the packet buffer.
     * @param nSize Size of the packet buffer.
     * @param from_ip IP address of the sender.
     * @param from_port Port number of the sender.
     */
    void static StaticCallbackFunctionMidiMessage(const uint8_t* buffer, uint32_t size, uint32_t from_ip, uint16_t from_port)
    {
        s_this->InputMidiMessage(buffer, size, from_ip, from_port);
    }

    virtual void HandleRtpMidi(const uint8_t* buffer) = 0;

   private:
    uint32_t start_time_{0};
    int32_t handle_control_{-1};
    int32_t handle_midi_{-1};
    uint32_t ssrc_{0};
    uint16_t exchange_packet_reply_size_{applemidi::kExchangePacketMinLength};
    uint16_t port_{kUpdPortControlDefault};
    applemidi::ExchangePacket exchange_packet_reply_;
    applemidi::SessionStatus session_status_;

    static inline AppleMidi* s_this; ///< Static instance pointer for the callback function.
};

#endif  // NET_APPLEMIDI_H_
