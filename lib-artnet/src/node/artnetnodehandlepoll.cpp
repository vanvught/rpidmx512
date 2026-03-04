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
            return "DMX output short detected. See good_output field.";
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
    // preventing: src/node/artnetnodehandlepoll.cpp:157:36: error: array subscript 2 is above array bounds of 'artnetnode::OutputPort [2]'
    // [-Werror=array-bounds=]
    if (__builtin_expect(port_index >= dmxnode::kMaxPorts, 0))
    {
#ifndef NDEBUG
        assert(0 && "port_index >= dmxnode::kMaxPorts");
        return;
#else
        __builtin_unreachable();
#endif
    }

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
#if defined(RDM_CONTROLLER)
        if (rdm_controller_.IsRunning(port_index))
        {
            GoodOutputBClear(port_index, artnet::GoodOutputB::kDiscoveryNotRunning);
        }
        else
        {
            GoodOutputBSet(port_index, artnet::GoodOutputB::kDiscoveryNotRunning);
        }
#endif
        art_poll_reply_.port_types[0] = artnet::PortType::kOutputArtnet;
        art_poll_reply_.good_output[0] = output_port_[port_index].good_output;
        art_poll_reply_.good_output_b[0] = output_port_[port_index].good_output_b;
        art_poll_reply_.good_input[0] = 0;
        art_poll_reply_.sw_out[0] = node_.port[port_index].sw;
        art_poll_reply_.sw_in[0] = 0;
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
        art_poll_reply_.port_types[0] = artnet::PortType::kInputArtnet;
        art_poll_reply_.good_output[0] = 0;
        art_poll_reply_.good_output_b[0] = 0;
        art_poll_reply_.good_input[0] = input_port_[port_index].good_input;
        art_poll_reply_.sw_out[0] = 0;
        art_poll_reply_.sw_in[0] = node_.port[port_index].sw;
        DEBUG_EXIT();
        return;
    }
#endif
}

void ArtNetNode::SendPollReply(uint32_t port_index, uint32_t destination_ip, artnet::ArtPollQueue* queue)
{
    assert(port_index < dmxnode::kMaxPorts);

    if (node_.port[port_index].direction == dmxnode::PortDirection::kDisable)
    {
        return;
    }

    ip.u32 = network::GetPrimaryIp();
    memcpy(art_poll_reply_.ip_address, ip.u8, sizeof(art_poll_reply_.ip_address));
#if (ARTNET_VERSION >= 4)
    memcpy(art_poll_reply_.bind_ip, ip.u8, sizeof(art_poll_reply_.bind_ip));
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

    art_poll_reply_.net_switch = node_.port[port_index].net_switch;
    art_poll_reply_.sub_switch = node_.port[port_index].sub_switch;
    art_poll_reply_.bind_index = static_cast<uint8_t>(port_index + 1);
    art_poll_reply_.num_ports_lo = 1;

    const auto* const kPortName = DmxNode::Instance().GetPortName(port_index);
    memcpy(art_poll_reply_.port_name, kPortName, artnet::kPortNameLength);

    if (__builtin_expect((dmxnode_output_type_ != nullptr), 1))
    {
        const auto kRefreshRate = dmxnode_output_type_->GetRefreshRate();
        art_poll_reply_.refresh_rate_lo = static_cast<uint8_t>(kRefreshRate);
        art_poll_reply_.refresh_rate_hi = static_cast<uint8_t>(kRefreshRate >> 8);
        const auto kUserData = dmxnode_output_type_->GetUserData();
        art_poll_reply_.user_lo = static_cast<uint8_t>(kUserData);
        art_poll_reply_.user_hi = static_cast<uint8_t>(kUserData >> 8);
    }

    ProcessPollReply(port_index);

    state_.art.poll_reply_count++;
    if (state_.art.poll_reply_count == 10000)
    {
        state_.art.poll_reply_count = 0;
    }

    CreateNodeReport(art_poll_reply_.node_report, state_.report_code, state_.art.poll_reply_count);
    network::udp::Send(handle_, reinterpret_cast<const uint8_t*>(&art_poll_reply_), sizeof(artnet::ArtPollReply), destination_ip, artnet::kUdpPort);

    state_.is_changed = false;
}

void ArtNetNode::HandlePoll()
{
    const auto* const kArtPoll = reinterpret_cast<artnet::ArtPoll*>(receive_buffer_);

    state_.send_art_poll_reply_on_change = ((kArtPoll->flags & artnet::Flags::kSendArtpOnChange) == artnet::Flags::kSendArtpOnChange);

    // If any controller requests diagnostics, the node will send diagnostics. (ArtPoll->Flags->2).
    if (kArtPoll->flags & artnet::Flags::kSendDiagMessages)
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
            // The lowest minimum value of Priority shall be used. (Ignore ArtPoll->diag_priority).
            state_.diag_priority = std::min(state_.diag_priority, kArtPoll->diag_priority);
        }
        else
        {
            state_.diag_priority = kArtPoll->diag_priority;
        }

        // If there are multiple controllers requesting diagnostics, diagnostics shall be broadcast. (Ignore ArtPoll->Flags->3).
        if (!state_.is_multiple_controllers_req_diag && (kArtPoll->flags & artnet::Flags::kSendDiagUnicast))
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

    auto target_port_address_bottom = artnet::kPortAddressFirst;
    auto target_port_address_top = artnet::kPortAddressLast;

    if (kArtPoll->flags & artnet::Flags::kUseTargetPortAddress)
    {
        target_port_address_top =
            static_cast<uint16_t>((static_cast<uint16_t>(kArtPoll->target_port_address_top_hi) >> 8) | kArtPoll->target_port_address_top_lo);
        target_port_address_bottom =
            static_cast<uint16_t>((static_cast<uint16_t>(kArtPoll->target_port_address_bottom_hi) >> 8) | kArtPoll->target_port_address_bottom_lo);
    }

    PollReplyQueueAdd(target_port_address_bottom, target_port_address_top);
}

void ArtNetNode::PollReplyQueueAdd(uint16_t target_port_address_bottom, uint16_t target_port_address_top)
{
    for (auto& entry : state_.art.poll_reply_queue)
    {
        if ((entry.art_poll_reply_ip_address == ip_address_from_) && (entry.art_poll_millis != 0)) [[unlikely]]
        {
            DEBUG_PRINTF("PollReply already queued for " IPSTR, IP2STR(entry.art_poll_reply_ip_address));
            return;
        }

        if (entry.art_poll_millis == 0)
        {
            entry.art_poll_millis = hal::Millis();
            entry.art_poll_reply_ip_address = ip_address_from_;
            entry.art_poll_reply.target_port_address_top = target_port_address_top;
            entry.art_poll_reply.target_port_address_bottom = target_port_address_bottom;
            DEBUG_PRINTF("PollReply queued for " IPSTR, IP2STR(entry.art_poll_reply_ip_address));
            return;
        }
    }
}
