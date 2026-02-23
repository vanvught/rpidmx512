/**
 * @file artnetnode_inline_impl.h
 *
 */
/* Copyright (C) 2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef ARTNETNODE_INLINE_IMPL_H_
#define ARTNETNODE_INLINE_IMPL_H_

#include <cstdint>
#include <cstdio>

#include "artnet.h"
#include "artnetnode.h"
#include "artnetstore.h"
#include "artnetdisplay.h"
#include "dmxnode.h"
#include "dmxnode_outputtype.h"
#if (ARTNET_VERSION >= 4)
#include "e131bridge.h"
#endif
#include "hal_millis.h"
#include "hal_panelled.h"
#include "hal_statusled.h"
#include "network.h"

inline void ArtNetNode::SetPortAddress(uint32_t port_index)
{
    node_.port[port_index].port_address =
        artnet::MakePortAddress(node_.port[port_index].net_switch, node_.port[port_index].sub_switch, node_.port[port_index].sw);
}

inline void ArtNetNode::SetOutput(DmxNodeOutputType* dmx_node_output_type)
{
    dmxnode_output_type_ = dmx_node_output_type;
#if (ARTNET_VERSION >= 4)
    E131Bridge::SetOutput(dmx_node_output_type);
#endif
}

inline DmxNodeOutputType* ArtNetNode::GetOutput() const
{
    return dmxnode_output_type_;
}

inline const char* ArtNetNode::GetLongName() const
{
    return reinterpret_cast<const char*>(art_poll_reply_.LongName);
}

inline void ArtNetNode::SetDisableMergeTimeout(bool disable)
{
    state_.disable_merge_timeout = disable;
#if (ARTNET_VERSION >= 4)
    E131Bridge::SetDisableMergeTimeout(disable);
#endif
}

inline bool ArtNetNode::GetDisableMergeTimeout() const
{
    return state_.disable_merge_timeout;
}

inline uint16_t ArtNetNode::GetUniverse(uint32_t port_index) const
{
    assert(port_index < dmxnode::kMaxPorts);
    if (node_.port[port_index].protocol == artnet::PortProtocol::kArtnet)
    {
        return node_.port[port_index].port_address;
    }
#if (ARTNET_VERSION >= 4)
    if (node_.port[port_index].protocol == artnet::PortProtocol::kSacn)
    {
        return E131Bridge::GetUniverse(port_index);
    }
#endif

    return 0;
}

inline dmxnode::PortDirection ArtNetNode::GetDirection(uint32_t port_index) const
{
    assert(port_index < dmxnode::kMaxPorts);
    return node_.port[port_index].direction;
}

inline bool ArtNetNode::GetUniverse(uint32_t port_index, uint16_t& universe, dmxnode::PortDirection port_direction)
{
    if (node_.port[port_index].protocol == artnet::PortProtocol::kArtnet)
    {
        return ArtNetNode::GetPortAddress(port_index, universe, port_direction);
    }
#if (ARTNET_VERSION >= 4)
    if (node_.port[port_index].protocol == artnet::PortProtocol::kSacn)
    {
        return E131Bridge::GetUniverse(port_index, universe, port_direction);
    }
#endif
    return false;
}

inline dmxnode::MergeMode ArtNetNode::GetMergeMode(uint32_t port_index) const
{
    assert(port_index < dmxnode::kMaxPorts);
    if ((output_port_[port_index].good_output & artnet::GoodOutput::kMergeModeLtp) == artnet::GoodOutput::kMergeModeLtp)
    {
        return dmxnode::MergeMode::kLtp;
    }
    return dmxnode::MergeMode::kHtp;
}

inline void ArtNetNode::SetFailSafe(dmxnode::FailSafe fail_safe)
{
    switch (fail_safe)
    {
        case dmxnode::FailSafe::kHold:
            SetFailSafe(artnet::FailSafe::kLast);
            break;
        case dmxnode::FailSafe::kOff:
            SetFailSafe(artnet::FailSafe::kOff);
            break;
        case dmxnode::FailSafe::kOn:
            SetFailSafe(artnet::FailSafe::kOn);
            break;
        case dmxnode::FailSafe::kPlayback:
            SetFailSafe(artnet::FailSafe::kPlayback);
            break;
        case dmxnode::FailSafe::kRecord:
            SetFailSafe(artnet::FailSafe::kRecord);
            break;
        default:
            assert(0);
            __builtin_unreachable();
            break;
    }
}

inline dmxnode::FailSafe ArtNetNode::GetFailSafe()
{
    const auto kNetworkloss = (art_poll_reply_.Status3 & artnet::Status3::kNetworklossMask);
    switch (kNetworkloss)
    {
        case artnet::Status3::kNetworklossLastState:
            return dmxnode::FailSafe::kHold;
            break;
        case artnet::Status3::kNetworklossOffState:
            return dmxnode::FailSafe::kOff;
            break;
        case artnet::Status3::kNetworklossOnState:
            return dmxnode::FailSafe::kOn;
            break;
        case artnet::Status3::kNetworklossPlayback:
            return dmxnode::FailSafe::kPlayback;
            break;
        default:
            [[unlikely]] assert(0);
            __builtin_unreachable();
            break;
    }

    __builtin_unreachable();
    return dmxnode::FailSafe::kOff;
}

inline void ArtNetNode::SetRdm(uint32_t port_index, bool enable)
{
    assert(port_index < dmxnode::kMaxPorts);

    const auto kIsEnabled = !((output_port_[port_index].good_output_b & artnet::GoodOutputB::kRdmDisabled) == artnet::GoodOutputB::kRdmDisabled);

    if (kIsEnabled == enable)
    {
        return;
    }

    if (!enable)
    {
        output_port_[port_index].good_output_b |= artnet::GoodOutputB::kRdmDisabled;
#if defined(RDM_CONTROLLER)
        rdm_controller_.Disable(port_index);
#endif
    }
    else
    {
        output_port_[port_index].good_output_b &= static_cast<uint8_t>(~artnet::GoodOutputB::kRdmDisabled);
#if defined(RDM_CONTROLLER)
        rdm_controller_.Enable(port_index);
#endif
    }

    if (state_.status == artnet::Status::kOn)
    {
        artnet::store::SaveRdmEnabled(port_index, enable);
        artnet::display::RdmEnabled(port_index, enable);
    }
}

inline bool ArtNetNode::GetRdm(uint32_t port_index) const
{
    assert(port_index < dmxnode::kMaxPorts);
    return !((output_port_[port_index].good_output_b & artnet::GoodOutputB::kRdmDisabled) == artnet::GoodOutputB::kRdmDisabled);
}

inline void ArtNetNode::GoodOutputBSet(uint32_t port_index, uint8_t b)
{
    output_port_[port_index].good_output_b |= b;
}

inline void ArtNetNode::GoodOutputBClear(uint32_t port_index, uint8_t b)
{
    output_port_[port_index].good_output_b &= static_cast<uint8_t>(~b);
}

inline dmxnode::PortDirection ArtNetNode::GetPortDirection(uint32_t port_index) const
{
    assert(port_index < dmxnode::kMaxPorts);
    return node_.port[port_index].direction;
}

inline bool ArtNetNode::GetPortAddress(uint32_t port_index, uint16_t& address) const
{
    assert(port_index < dmxnode::kMaxPorts);

    if (node_.port[port_index].direction == dmxnode::PortDirection::kDisable)
    {
        return false;
    }

    address = node_.port[port_index].port_address;
    return true;
}

inline bool ArtNetNode::GetPortAddress(uint32_t port_index, uint16_t& address, dmxnode::PortDirection port_direction) const
{
    assert(port_index < dmxnode::kMaxPorts);

    if (port_direction == dmxnode::PortDirection::kDisable)
    {
        return false;
    }

    address = node_.port[port_index].port_address;
    return node_.port[port_index].direction == port_direction;
}

inline bool ArtNetNode::GetOutputPort(uint16_t universe, uint32_t& port_index)
{
    for (port_index = 0; port_index < dmxnode::kMaxPorts; port_index++)
    {
        if (node_.port[port_index].direction != dmxnode::PortDirection::kOutput)
        {
            continue;
        }
        if ((node_.port[port_index].protocol == artnet::PortProtocol::kArtnet) && (universe == node_.port[port_index].port_address))
        {
            return true;
        }
    }
    return false;
}

inline void ArtNetNode::RestartOutputPort(uint32_t port_index)
{
    if (output_port_[port_index].is_transmitting)
    {
        dmxnode_output_type_->Stop(port_index);
        dmxnode_output_type_->Start(port_index);
    }
}

inline void ArtNetNode::Run()
{
#if defined(ARTNET_HAVE_DMXIN)
    HandleDmxIn();
#endif

#if (ARTNET_VERSION >= 4)
    E131Bridge::Run();
#endif

    current_millis_ = hal::Millis();
    const auto kDeltaMillis = current_millis_ - packet_millis_;

    if (kDeltaMillis >= artnet::kNetworkDataLossTimeout * 1000)
    {
        SetNetworkDataLossCondition();
    }

    if (kDeltaMillis >= (1U * 1000U))
    {
        state_.receiving_dmx &= static_cast<uint8_t>(~(1U << static_cast<uint8_t>(dmxnode::PortDirection::kOutput)));
    }

#if (DMXNODE_PORTS > 0)
    if ((((art_poll_reply_.Status1 & artnet::Status1::kIndicatorMask) == artnet::Status1::kIndicatorNormalMode)) &&
        (hal::statusled::GetMode() != hal::statusled::Mode::FAST))
    {
#if (ARTNET_VERSION >= 4)
        if (state_.receiving_dmx != 0)
        {
            SetLedBlinkMode4(hal::statusled::Mode::DATA);
        }
        else
        {
            SetLedBlinkMode4(hal::statusled::Mode::NORMAL);
        }
#else
        if (state_.receiving_dmx != 0)
        {
            hal::statusled::SetMode(hal::statusled::Mode::DATA);
        }
        else
        {
            hal::statusled::SetMode(hal::statusled::Mode::NORMAL);
        }
#endif
    }
#endif

    auto& entry = state_.art.poll_reply_queue[state_.art.poll_reply_queue_index];

    if (__builtin_expect((entry.art_poll_millis != 0), 0))
    {
        if (state_.art.poll_reply_state == artnetnode::PollReplyState::kWaitingTimeout)
        {
            if ((current_millis_ - entry.art_poll_millis) > state_.art.poll_reply_delay_millis)
            {
                state_.art.poll_reply_state = artnetnode::PollReplyState::kRunning;
                state_.art.poll_reply_port_index = 0;
            }
        }
        else
        {
            SendPollReply(state_.art.poll_reply_port_index, entry.art_poll_reply_ip_address, &entry);

            state_.art.poll_reply_port_index++;

            if (state_.art.poll_reply_port_index == dmxnode::kMaxPorts)
            {
                entry.art_poll_millis = 0;
                state_.art.poll_reply_state = artnetnode::PollReplyState::kWaitingTimeout;
            }
        }
    }
    else
    {
        state_.art.poll_reply_queue_index++;
        if (state_.art.poll_reply_queue_index == artnetnode::kPollReplyQueueSize)
        {
            state_.art.poll_reply_queue_index = 0;
        }
    }

#if defined(RDM_CONTROLLER)
    if (__builtin_expect((state_.is_rdm_enabled), 0))
    {
        HandleRdmIn();

        rdm_controller_.Run();
    }
#endif
}

inline void ArtNetNode::SendDiag([[maybe_unused]] const artnet::PriorityCodes kPriorityCode, [[maybe_unused]] const char* format, ...)
{
#if defined(ARTNET_ENABLE_SENDDIAG)
    if (!state_.send_art_diag_data)
    {
        return;
    }

    if (static_cast<uint8_t>(kPriorityCode) < state_.diag_priority)
    {
        return;
    }

    diag_data_.Priority = static_cast<uint8_t>(kPriorityCode);

    va_list arp;

    va_start(arp, format);

    auto i = vsnprintf(reinterpret_cast<char*>(diag_data_.data), sizeof(diag_data_.data) - 1, format, arp);

    va_end(arp);

    diag_data_.data[sizeof(diag_data_.data) - 1] = '\0'; // Just be sure we have a last '\0'
    diag_data_.LengthLo = static_cast<uint8_t>(i + 1);   // Text length including the '\0'

    const uint16_t kSize = sizeof(struct artnet::ArtDiagData) - sizeof(diag_data_.data) + diag_data_.LengthLo;

    network::udp::Send(handle_, reinterpret_cast<const uint8_t*>(&diag_data_), kSize, state_.art.diag_ip, artnet::kUdpPort);
#endif
}

#endif  // ARTNETNODE_INLINE_IMPL_H_
