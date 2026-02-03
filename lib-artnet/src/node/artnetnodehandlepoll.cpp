/**
 * @file artnetnodehandlepoll.cpp
 */
/* Copyright (C) 2021-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#if defined(DEBUG_ARTNET_POLL)
#undef NDEBUG
#endif

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC push_options
#pragma GCC optimize("O2")
#pragma GCC optimize("no-tree-loop-distribute-patterns")
#pragma GCC optimize("-fprefetch-loop-arrays")
#endif

#include <cstdint>
#include <cstring>
#include <algorithm>
#include <cassert>

#include "artnetnode.h"
#include "artnet.h"
#include "hal_millis.h"
#include "network.h"
#include "firmware/debug/debug_debug.h"

template <uint8_t N> static inline void Uitoa(uint32_t v, uint8_t* p)
{
    static_assert(N >= 1);
    auto* o = p + (N - 1);
    do
    {
        *o-- = static_cast<uint8_t>('0' + (v % 10U));
        v /= 10U;
    } while ((o >= p) && (v > 0));

    // If there are remaining digits, fill with zeros
    while (o >= p)
    {
        *o-- = '0';
    }
}

/*
 * Table 3 – NodeReport Codes
 */
static const char* GetReportCodeString(artnet::ReportCode code)
{
    switch (code)
    {
        case artnet::ReportCode::kRcdebug:
            return "Booted in debug mode (Only used in development)";
        case artnet::ReportCode::kRcpowerok:
            return "Power On Tests successful";
        case artnet::ReportCode::kRcpowerfail:
            return "Hardware tests failed at Power On";
        case artnet::ReportCode::kRcsocketwR1:
            return "Last UDP from Node failed due to truncated length";
        case artnet::ReportCode::kRcparsefail:
            return "Unable to identify last UDP transmission.";
        case artnet::ReportCode::kRcudpfail:
            return "Unable to open Udp Socket in last transmission";
        case artnet::ReportCode::kRcshnameok:
            return "Short Name programming [ArtAddress] was successful.";
        case artnet::ReportCode::kRclonameok:
            return "Long Name programming [ArtAddress] was successful.";
        case artnet::ReportCode::kRcdmxerror:
            return "DMX512 receive errors detected.";
        case artnet::ReportCode::kRcdmxudpfull:
            return "Ran out of internal DMX transmit buffers.";
        case artnet::ReportCode::kRcdmxrxfull:
            return "Ran out of internal DMX Rx buffers.";
        case artnet::ReportCode::kRcswitcherr:
            return "Rx Universe switches conflict.";
        case artnet::ReportCode::kRcconfigerr:
            return "Product configuration does not match firmware.";
        case artnet::ReportCode::kRcdmxshort:
            return "DMX output short detected. See GoodOutput field.";
        case artnet::ReportCode::kRcfirmwarefail:
            return "Last attempt to upload new firmware failed.";
        case artnet::ReportCode::kRcuserfail:
            return "User changed switch settings when address locked.";
        default:
            return "Unknown Report Code";
    }
}

/*
 * NodeReport [64]
 *
 * The array is a textual report of the Node’s operating status or operational errors. It is
 * primarily intended for ‘engineering’ data rather than ‘end user’ data. The field is formatted as:
 * “#xxxx [yyyy..] zzzzz…”
 * xxxx is a hex status code as defined in Table 3.
 * yyyy is a decimal counter that increments every time the Node sends an ArtPollResponse.
 */
static void CreateNodeReport(uint8_t* node_report, artnet::ReportCode code, uint32_t counter)
{
    [[maybe_unused]] const auto* begin = node_report;

    *node_report++ = '#';
    Uitoa<4>(static_cast<uint32_t>(code), node_report);
    node_report += 4;
    *node_report++ = ' ';
    *node_report++ = '[';
    Uitoa<4>(counter, node_report);
    node_report += 4;
    *node_report++ = ']';
    *node_report++ = ' ';

    assert((artnet::kReportLength - (node_report - begin) - 1) == 50);
    constexpr auto kRemainingSize = 50; // REPORT_LENGTH - (pNodeReport - pBegin) - 1;
    const auto* preport_string = GetReportCodeString(code);

    strncpy(reinterpret_cast<char*>(node_report), preport_string, kRemainingSize);
}

union uip
{
    uint32_t u32;
    uint8_t u8[4];
} static ip;

void ArtNetNode::ProcessPollReply(uint32_t port_index)
{
    if (node_.port[port_index].direction == dmxnode::PortDirection::kOutput)
    {
#if (ARTNET_VERSION >= 4)
        if (node_.port[port_index].protocol == artnet::PortProtocol::kSacn)
        {
            constexpr auto kMask = artnet::GoodOutput::kOutputIsMerging | artnet::GoodOutput::kDataIsBeingTransmitted | artnet::GoodOutput::kOutputIsSacn;
            auto good_output = output_port_[port_index].good_output;
            good_output &= static_cast<uint8_t>(~kMask);
            good_output = static_cast<uint8_t>(good_output | (GetGoodOutput4(port_index) & kMask));
            output_port_[port_index].good_output = good_output;
        }
#endif
        art_poll_reply_.GoodOutput[0] = output_port_[port_index].good_output;
        art_poll_reply_.GoodOutputB[0] = output_port_[port_index].good_output_b;
        art_poll_reply_.GoodInput[0] = 0;
        art_poll_reply_.SwOut[0] = node_.port[port_index].sw;
        art_poll_reply_.SwIn[0] = 0;
        DEBUG_EXIT();
        return;
    }

#if defined(ARTNET_HAVE_DMXIN)
    if (node_.port[port_index].direction == dmxnode::PortDirection::kInput)
    {
#if (ARTNET_VERSION >= 4)
        if (node_.port[port_index].protocol == artnet::PortProtocol::kSacn)
        {
            input_port_[port_index].good_input |= artnet::GoodInput::kInputIsSacn;
        }
#endif
        art_poll_reply_.GoodOutput[0] = 0;
        art_poll_reply_.GoodOutputB[0] = 0;
        art_poll_reply_.GoodInput[0] = input_port_[port_index].good_input;
        art_poll_reply_.SwOut[0] = 0;
        art_poll_reply_.SwIn[0] = node_.port[port_index].sw;
        DEBUG_EXIT();
        return;
    }
#endif
}

void ArtNetNode::SendPollReply(uint32_t port_index, uint32_t destination_ip, artnet::ArtPollQueue* queue)
{
    if (node_.port[port_index].direction == dmxnode::PortDirection::kDisable)
    {
        return;
    }

    ip.u32 = network::GetPrimaryIp();
    memcpy(art_poll_reply_.IPAddress, ip.u8, sizeof(art_poll_reply_.IPAddress));
#if (ARTNET_VERSION >= 4)
    memcpy(art_poll_reply_.BindIp, ip.u8, sizeof(art_poll_reply_.BindIp));
#endif

    if (queue != nullptr)
    {
        if (!((node_.port[port_index].port_address >= queue->art_poll_reply.target_port_address_bottom) &&
              (node_.port[port_index].port_address <= queue->art_poll_reply.target_port_address_top)))
        {
            DEBUG_PRINTF("NOT: 	%u >= %u && %u <= %u", node_.port[port_index].port_address, queue->art_poll_reply.target_port_address_bottom,
                         node_.port[port_index].port_address, queue->art_poll_reply.target_port_address_top);
            return;
        }
    }

    art_poll_reply_.NetSwitch = node_.port[port_index].net_switch;
    art_poll_reply_.SubSwitch = node_.port[port_index].sub_switch;
    art_poll_reply_.bind_index = static_cast<uint8_t>(port_index + 1);
    art_poll_reply_.NumPortsLo = 1;
#if defined(ARTNET_HAVE_DMXIN)
    art_poll_reply_.PortTypes[0] = artnet::PortType::kOutputArtnet | artnet::PortType::kInputArtnet;
#else
    art_poll_reply_.PortTypes[0] = artnet::PortType::kOutputArtnet;
#endif

    const auto* shortname = DmxNode::Instance().GetShortName(port_index);
    memcpy(art_poll_reply_.ShortName, shortname, artnet::kShortNameLength);

    if (__builtin_expect((dmxnode_output_type_ != nullptr), 1))
    {
        const auto kRefreshRate = dmxnode_output_type_->GetRefreshRate();
        art_poll_reply_.RefreshRateLo = static_cast<uint8_t>(kRefreshRate);
        art_poll_reply_.RefreshRateHi = static_cast<uint8_t>(kRefreshRate >> 8);
        const auto kUserData = dmxnode_output_type_->GetUserData();
        art_poll_reply_.UserLo = static_cast<uint8_t>(kUserData);
        art_poll_reply_.UserHi = static_cast<uint8_t>(kUserData >> 8);
    }

    ProcessPollReply(port_index);

    state_.art.poll_reply_count++;
    if (state_.art.poll_reply_count == 10000)
    {
        state_.art.poll_reply_count = 0;
    }

    CreateNodeReport(art_poll_reply_.NodeReport, state_.report_code, state_.art.poll_reply_count);
    network::udp::Send(handle_, reinterpret_cast<const uint8_t*>(&art_poll_reply_), sizeof(artnet::ArtPollReply), destination_ip, artnet::kUdpPort);

    state_.is_changed = false;
}

void ArtNetNode::HandlePoll()
{
    const auto* const kArtPoll = reinterpret_cast<artnet::ArtPoll*>(receive_buffer_);

    state_.send_art_poll_reply_on_change = ((kArtPoll->Flags & artnet::Flags::kSendArtpOnChange) == artnet::Flags::kSendArtpOnChange);

    // If any controller requests diagnostics, the node will send diagnostics. (ArtPoll->Flags->2).
    if (kArtPoll->Flags & artnet::Flags::kSendDiagMessages)
    {
        state_.send_art_diag_data = true;

        if (state_.art.poll_ip == 0)
        {
            state_.art.poll_ip = ip_address_from_;
        }
        else if (!state_.is_multiple_controllers_req_diag && (state_.art.poll_ip != ip_address_from_))
        {
            // If there are multiple controllers requesting diagnostics, diagnostics shall be broadcast.
            state_.art.diag_ip = network::GetBroadcastIp();
            state_.is_multiple_controllers_req_diag = true;
        }

        if (state_.is_multiple_controllers_req_diag)
        {
            // The lowest minimum value of Priority shall be used. (Ignore ArtPoll->DiagPriority).
            state_.diag_priority = std::min(state_.diag_priority, kArtPoll->DiagPriority);
        }
        else
        {
            state_.diag_priority = kArtPoll->DiagPriority;
        }

        // If there are multiple controllers requesting diagnostics, diagnostics shall be broadcast. (Ignore ArtPoll->Flags->3).
        if (!state_.is_multiple_controllers_req_diag && (kArtPoll->Flags & artnet::Flags::kSendDiagUnicast))
        {
            state_.art.diag_ip = ip_address_from_;
        }
        else
        {
            state_.art.diag_ip = network::GetBroadcastIp();
        }
    }
    else
    {
        state_.send_art_diag_data = false;
        state_.art.diag_ip = 0;
    }

    uint16_t target_port_address_top = 32767; // TODO (a)
    uint16_t target_port_address_bottom = 0;

    if (kArtPoll->Flags & artnet::Flags::kUseTargetPortAddress)
    {
        target_port_address_top = static_cast<uint16_t>((static_cast<uint16_t>(kArtPoll->TargetPortAddressTopHi) >> 8) | kArtPoll->TargetPortAddressTopLo);
        target_port_address_bottom =
            static_cast<uint16_t>((static_cast<uint16_t>(kArtPoll->TargetPortAddressBottomHi) >> 8) | kArtPoll->TargetPortAddressBottomLo);
    }

    for (auto& entry : state_.art.poll_reply_queue)
    {
        if ((entry.art_poll_reply_ip_address == ip_address_from_) && (entry.art_poll_millis != 0)) [[unlikely]]
        {
            DEBUG_PRINTF("[ArtPollReply already queued for " IPSTR, IP2STR(entry.art_poll_reply_ip_address));
            return;
        }

        if (entry.art_poll_millis == 0)
        {
            entry.art_poll_millis = hal::Millis();
            entry.art_poll_reply_ip_address = ip_address_from_;
            entry.art_poll_reply.target_port_address_top = target_port_address_top;
            entry.art_poll_reply.target_port_address_bottom = target_port_address_bottom;
            DEBUG_PRINTF("[ArtPollReply queued for " IPSTR, IP2STR(entry.art_poll_reply_ip_address));
            return;
        }
    }
}
