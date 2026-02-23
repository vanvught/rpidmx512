/**
 * @file tcnet.h
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

#ifndef TCNET_H_
#define TCNET_H_

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC push_options
#pragma GCC optimize("O3")
#pragma GCC optimize("no-tree-loop-distribute-patterns")
#pragma GCC optimize("-fprefetch-loop-arrays")
#endif

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cstddef>
#include <cassert>

#include "tcnetpackets.h"
#include "tcnettimecode.h"
#include "hal.h"
#include "hal_micros.h"
#include "network.h"
#include "hal_millis.h"
#include "firmware/debug/debug_debug.h"

namespace tcnet
{
inline constexpr char kNodeNameDefault[] = "AvV";

enum class Layer : uint8_t
{
    kLayer1,
    kLayer2,
    kLayer3,
    kLayer4,
    kLayerA,
    kLayerB,
    kLayerM,
    kLayerC,
    kLayerUndefined
};

[[nodiscard]] inline constexpr char GetLayer(Layer layer)
{
    switch (layer)
    {
        case Layer::kLayer1:
        case Layer::kLayer2:
        case Layer::kLayer3:
        case Layer::kLayer4:
            return static_cast<char>(static_cast<char>(layer) + '1');
            break;
        case Layer::kLayerA:
            return 'A';
            break;
        case Layer::kLayerB:
            return 'B';
            break;
        case Layer::kLayerM:
            return 'M';
            break;
        case Layer::kLayerC:
            return 'C';
            break;
        default:
            break;
    }

    return ' ';
}

[[nodiscard]] inline constexpr Layer GetLayer(char c)
{
    switch (c | 0x20)
    { // to lower case
        case '1':
        case '2':
        case '3':
        case '4':
            return static_cast<tcnet::Layer>(c - '1');
            break;
        case 'a':
            return Layer::kLayerA;
            break;
        case 'b':
            return Layer::kLayerB;
            break;
        case 'm':
            return Layer::kLayerM;
            break;
        case 'c':
            return Layer::kLayerC;
            break;
        default:
            break;
    }

    return Layer::kLayerUndefined;
}

enum class TimeCodeType : uint8_t
{
    kTimecodeTypeFilm,
    kTimecodeTypeEbu25Fps,
    kTimecodeTypeDf,
    kTimecodeTypeSmpte30Fps,
    kTimecodeTypeInvalid = 0xFF
};

inline constexpr const uint8_t kFps[5] = {24, 25, 29, 30, 25};

} // namespace tcnet

class TCNet
{
   public:
    TCNet()
    {
        DEBUG_ENTRY();

        assert(s_this == nullptr);
        s_this = this;

        memset(&packet_opt_in_, 0, sizeof(struct TTCNetPacketOptIn));

        // Fill the static fields for Opt-IN
        packet_opt_in_.ManagementHeader.ProtocolVersionMajor = 3;
        packet_opt_in_.ManagementHeader.ProtocolVersionMinor = 3;
        memcpy(packet_opt_in_.ManagementHeader.Header, "TCN", 3);
        packet_opt_in_.ManagementHeader.MessageType = TCNET_MESSAGE_TYPE_OPTIN;
        packet_opt_in_.ManagementHeader.SEQ = 0;
        packet_opt_in_.ManagementHeader.NodeType = TCNET_TYPE_SLAVE;
        packet_opt_in_.ManagementHeader.NodeOptions = 0;
        packet_opt_in_.NodeCount = 1;
        packet_opt_in_.NodeListenerPort = Unicast::kPort;
        memcpy(&packet_opt_in_.VendorName, "gd32-dmx.org", TCNET_VENDOR_NAME_LENGTH);
        memcpy(&packet_opt_in_.DeviceName, "LTC SMPTE Node  ", TCNET_DEVICE_NAME_LENGTH);
        packet_opt_in_.DeviceMajorVersion = static_cast<uint8_t>(_TIME_STAMP_YEAR_ - 2000);
        packet_opt_in_.DeviceMinorVersion = _TIME_STAMP_MONTH_;
        packet_opt_in_.DeviceBugVersion = _TIME_STAMP_DAY_;

        SetNodeName(tcnet::kNodeNameDefault);
        SetLayer(tcnet::Layer::kLayerM);
        SetTimeCodeType(tcnet::TimeCodeType::kTimecodeTypeSmpte30Fps);

        DEBUG_EXIT();
    }

    void Start()
    {
        DEBUG_ENTRY();

        handles_[0] = network::udp::Begin(Broadcast::kPort0, StaticCallbackFunctionPort60000);
        assert(handles_[0] >= 0);

        handles_[1] = network::udp::Begin(Broadcast::kPort1, StaticCallbackFunctionPort60001);
        assert(handles_[1] >= 0);

        DEBUG_EXIT();
    }

    void Stop()
    {
        DEBUG_ENTRY();

        TTCNetPacketOptOut opt_out;

        memcpy(&opt_out, &packet_opt_in_, sizeof(struct TTCNetPacketOptOut));

        network::udp::Send(handles_[0], reinterpret_cast<const uint8_t*>(&opt_out), sizeof(struct TTCNetPacketOptOut), network::GetBroadcastIp(), Broadcast::kPort0);

        DEBUG_EXIT();
    }

    void Run()
    {
        current_millis_ = hal::Millis();

        if (__builtin_expect(((current_millis_ - previous_millis_) >= 1000), 0))
        {
            previous_millis_ = current_millis_;
            HandleOptInOutgoing();
        }
    }

    void Print()
    {
        puts("TCNet");
        printf(" Node : %.8s\n", packet_opt_in_.ManagementHeader.NodeName);
        printf(" L%c", tcnet::GetLayer(layer_));
        if (use_time_code_)
        {
            puts(" TC");
        }
        else
        {
            printf(" T%u\n", tcnet::kFps[static_cast<uint32_t>(timecode_type_)]);
        }

        printf("%u:%u:%u\n", static_cast<unsigned>(layer_), lx_time_offset_, lx_time_code_offset_);
    }

    TTCNetNodeType GetNodeType() const { return static_cast<TTCNetNodeType>(packet_opt_in_.ManagementHeader.NodeType); }

    void SetNodeName(const char* node_name)
    {
        strncpy(reinterpret_cast<char*>(packet_opt_in_.ManagementHeader.NodeName), node_name, sizeof packet_opt_in_.ManagementHeader.NodeName - 1);
        packet_opt_in_.ManagementHeader.NodeName[sizeof packet_opt_in_.ManagementHeader.NodeName - 1] = '\0';
    }

    const char* GetNodeName() { return reinterpret_cast<char*>(packet_opt_in_.ManagementHeader.NodeName); }

    void SetLayer(tcnet::Layer layer)
    {
        layer_ = layer;
        lx_time_offset_ = offsetof(struct TTCNetPacketTime, L1Time) + (4 * static_cast<uint32_t>(layer));
        lx_time_code_offset_ = offsetof(struct TTCNetPacketTime, L1TimeCode) + static_cast<uint32_t>(layer) * sizeof(struct TTCNetPacketTimeTimeCode);
    }

    tcnet::Layer GetLayer() const { return layer_; }

    void SetUseTimeCode(bool use_time_code) { use_time_code_ = use_time_code; }
    bool GetUseTimeCode() const { return use_time_code_; }

    void SetTimeCodeType(tcnet::TimeCodeType type)
    {
        switch (type)
        {
            case tcnet::TimeCodeType::kTimecodeTypeFilm:
                type_divider_ = 1000.0f / 24;
                break;
            case tcnet::TimeCodeType::kTimecodeTypeEbu25Fps:
                type_divider_ = 1000.0f / 25;
                break;
            case tcnet::TimeCodeType::kTimecodeTypeDf:
                type_divider_ = 1000.0f / 29.97f;
                break;
            case tcnet::TimeCodeType::kTimecodeTypeSmpte30Fps:
                type_divider_ = 1000.0f / 30;
                break;
            default:
                return;
                break;
        }

        timecode_type_ = type;
    }

    tcnet::TimeCodeType GetTimeCodeType() const { return timecode_type_; }

#if defined(TCNET_HAVE_TIMECODE)
    void SetArtTimeCodeCallbackFunction(TCNetTimeCodeCallbackFunctionPtr function_ptr) { function_ptr_ = function_ptr; }
#endif

    static TCNet* Get() { return s_this; }

   private:
    void static StaticCallbackFunctionPort60000(const uint8_t* buffer, uint32_t size, uint32_t from_ip, uint16_t from_port)
    {
        s_this->InputPort60000(buffer, size, from_ip, from_port);
    }

    void InputPort60000(const uint8_t* buffer, [[maybe_unused]] uint32_t size, [[maybe_unused]] uint32_t from_ip, [[maybe_unused]] uint16_t from_port)
    {
        const auto* packet = reinterpret_cast<const struct TTCNetPacketManagementHeader*>(buffer);
        const auto kMessageType = static_cast<TTCNetMessageType>(packet->MessageType);

        DEBUG_PRINTF("kMessageType=%u", static_cast<uint32_t>(kMessageType));

        if (kMessageType == TCNET_MESSAGE_TYPE_OPTIN)
        {
#ifndef NDEBUG
            DumpOptIn(buffer);
#endif
            return;
        }
    }

    void static StaticCallbackFunctionPort60001(const uint8_t* buffer, uint32_t size, uint32_t from_ip, uint16_t from_port)
    {
        s_this->InputPort60001(buffer, size, from_ip, from_port);
    }

    void InputPort60001(const uint8_t* buffer, [[maybe_unused]] uint32_t size, [[maybe_unused]] uint32_t from_ip, [[maybe_unused]] uint16_t from_port)
    {
#if defined(TCNET_HAVE_TIMECODE)
        const auto* packet_time = reinterpret_cast<const struct TTCNetPacketTime*>(buffer);

        if (static_cast<TTCNetMessageType>(packet_time->ManagementHeader.MessageType) == TCNET_MESSAGE_TYPE_TIME)
        {
            tcnet::TimeCode timecode __attribute__((aligned(4)));

            if (use_time_code_)
            {
                const auto* tc = reinterpret_cast<const TTCNetPacketTimeTimeCode*>(buffer + lx_time_code_offset_);
                timecode.frames = tc->Frames;
                timecode.seconds = tc->Seconds;
                timecode.minutes = tc->Minutes;
                timecode.hours = tc->Hours;

                auto smpte_mode = tc->SMPTEMode;

                if (smpte_mode < 24)
                {
                    smpte_mode = packet_time->SMPTEMode;
                }

                switch (smpte_mode)
                {
                    case 24:
                        timecode.type = static_cast<uint8_t>(tcnet::TimeCodeType::kTimecodeTypeFilm);
                        break;
                    case 25:
                        timecode.type = static_cast<uint8_t>(tcnet::TimeCodeType::kTimecodeTypeEbu25Fps);
                        break;
                    case 29:
                        timecode.type = static_cast<uint8_t>(tcnet::TimeCodeType::kTimecodeTypeDf);
                        break;
                    case 30:
                        __attribute__((fallthrough));
                        /* no break */
                    default:
                        timecode.type = static_cast<uint8_t>(tcnet::TimeCodeType::kTimecodeTypeSmpte30Fps);
                        break;
                }
            }
            else
            {
                auto lx_time = *reinterpret_cast<const uint32_t*>(buffer + lx_time_offset_);

                const auto kHours = lx_time / 3600000U;
                lx_time -= kHours * 3600000U;
                const auto kMinutes = lx_time / 60000U;
                lx_time -= kMinutes * 60000U;
                const auto kSeconds = lx_time / 1000U;
                const auto kMillis = lx_time - kSeconds * 1000U;
                const auto kFrames = (kMillis * tcnet::kFps[static_cast<uint32_t>(timecode_type_)]) / 1000U;

                timecode.frames = static_cast<uint8_t>(kFrames);
                timecode.seconds = static_cast<uint8_t>(kSeconds);
                timecode.minutes = static_cast<uint8_t>(kMinutes);
                timecode.hours = static_cast<uint8_t>(kHours);
                timecode.type = static_cast<uint8_t>(timecode_type_);
            }

            const auto* src = reinterpret_cast<uint8_t*>(&timecode);
            auto* dst = reinterpret_cast<uint8_t*>(&timecode_);
            auto do_send = false;

            for (uint32_t index = 0; index < sizeof(struct tcnet::TimeCode); index++)
            {
                do_send |= (*src != *dst);
                *dst++ = *src++;
            }

            if (do_send)
            {
                assert(function_ptr_ != nullptr);
                function_ptr_(&timecode);
            }
        }
#endif
    }

    void HandleOptInOutgoing()
    {
        packet_opt_in_.ManagementHeader.SEQ++;
        packet_opt_in_.ManagementHeader.TimeStamp = hal::Micros();
        packet_opt_in_.Uptime = static_cast<uint16_t>(hal::Uptime());

        network::udp::Send(handles_[0], reinterpret_cast<const uint8_t*>(&packet_opt_in_), sizeof(struct TTCNetPacketOptIn), network::GetBroadcastIp(),
                       Broadcast::kPort0);
    }

    void DumpManagementHeader(const uint8_t* buffer);
    void DumpOptIn(const uint8_t* buffer);

   private:
    struct Broadcast
    {
        static constexpr uint16_t kPort0 = 60000;
        static constexpr uint16_t kPort1 = 60001;
        static constexpr uint16_t kPort2 = 60002;
    };

    struct Unicast
    {
        static constexpr uint16_t kPort = 65023;
    };

    int32_t handles_[2];
    uint32_t ip_address_from_;
    uint32_t current_millis_{0};
    uint32_t previous_millis_{0};
    uint32_t lx_time_offset_{0};
    uint32_t lx_time_code_offset_{0};

    tcnet::TimeCode timecode_ = {0, 0, 0, 0, UINT8_MAX};

    TCNetTimeCodeCallbackFunctionPtr function_ptr_{nullptr};

    float type_divider_{1000.0F / 30};
    tcnet::Layer layer_{tcnet::Layer::kLayerM};
    tcnet::TimeCodeType timecode_type_{tcnet::TimeCodeType::kTimecodeTypeInvalid};
    bool use_time_code_{false};
    uint8_t seq_time_message_{0};

    TTCNetPacketOptIn packet_opt_in_;

    static inline TCNet* s_this;
};

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC pop_options
#endif
#if defined(_NDEBUG)
#undef _NDEBUG
#define NDEBUG
#endif

#endif // TCNET_H_
