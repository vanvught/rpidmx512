/**
 * @file artnetnode4_inline_impl.h
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

#ifndef ARTNETNODE4_INLINE_IMPL_H_
#define ARTNETNODE4_INLINE_IMPL_H_

#include <cstdint>

#include "artnetnode.h"
 #include "firmware/debug/debug_debug.h"
#include "dmxnode.h"
#include "e131bridge.h"
#include "artnetstore.h"
#include "artnetdisplay.h"

inline void ArtNetNode::SetUniverse4(uint32_t port_index)
{
    DEBUG_ENTRY();

    if (node_.port[port_index].protocol != artnet::PortProtocol::kSacn)
    {
        DEBUG_EXIT();
        return;
    }

    auto universe = node_.port[port_index].port_address;

    if (IsMapUniverse0())
    {
        universe++;
    }

    if (universe == 0)
    {
        DEBUG_EXIT();
        return;
    }

    E131Bridge::SetUniverse(port_index, universe);

    DEBUG_EXIT();
}

inline void ArtNetNode::SetDirection4(uint32_t port_index)
{
    DEBUG_ENTRY();

    if (node_.port[port_index].protocol != artnet::PortProtocol::kSacn)
    {
        DEBUG_EXIT();
        return;
    }

    E131Bridge::SetDirection(port_index, node_.port[port_index].direction);

    DEBUG_EXIT();
}

inline void ArtNetNode::SetPortProtocol4(uint32_t port_index, artnet::PortProtocol port_protocol)
{
    DEBUG_PRINTF("port_index=%u, PortProtocol=%s", port_index, artnet::GetProtocolMode(port_protocol, false));

    assert(port_index < dmxnode::kMaxPorts);

    if (node_.port[port_index].protocol == port_protocol)
    {
        DEBUG_EXIT();
        return;
    }

    node_.port[port_index].protocol = port_protocol;

    if (port_protocol == artnet::PortProtocol::kSacn)
    {
        if (node_.port[port_index].direction == dmxnode::PortDirection::kOutput)
        {
            output_port_[port_index].good_output |= artnet::GoodOutput::kOutputIsSacn;
        }

        SetUniverse4(port_index);
        E131Bridge::SetDirection(port_index, node_.port[port_index].direction);
    }
    else
    {
        if (node_.port[port_index].direction == dmxnode::PortDirection::kOutput)
        {
            output_port_[port_index].good_output &= static_cast<uint8_t>(~artnet::GoodOutput::kOutputIsSacn);
        }

        E131Bridge::SetDirection(port_index, dmxnode::PortDirection::kDisable);
    }

    if (state_.status == artnet::Status::kOn)
    {
        artnet::store::SaveProtocol(port_index, port_protocol);
        artnet::display::Protocol(port_index, port_protocol);
    }

    DEBUG_EXIT();
}

inline artnet::PortProtocol ArtNetNode::GetPortProtocol4(uint32_t port_index) const
{
    assert(port_index < dmxnode::kMaxPorts);
    return node_.port[port_index].protocol;
}

inline void ArtNetNode::SetPriority4(uint32_t port_index, uint8_t priority)
{
    E131Bridge::SetPriority(port_index, priority);
}

inline void ArtNetNode::SetPriority4(uint32_t priority)
{
    art_poll_reply_.AcnPriority = static_cast<uint8_t>(priority);

    for (uint32_t port_index = 0; port_index < dmxnode::kMaxPorts; port_index++)
    {
        E131Bridge::SetPriority(port_index, static_cast<uint8_t>(priority));
    }
}

inline uint8_t ArtNetNode::GetPriority4(uint32_t port_index) const
{
    return E131Bridge::GetPriority(port_index);
}

inline uint8_t ArtNetNode::GetGoodOutput4(uint32_t port_index)
{
    assert(port_index < dmxnode::kMaxPorts);

    uint16_t universe;
    const auto kIsActive = E131Bridge::GetUniverse(port_index, universe, dmxnode::PortDirection::kOutput);

    DEBUG_PRINTF("Port %u, Active %c, Universe %d, %s", port_index, kIsActive ? 'Y' : 'N', universe,
                 dmxnode::GetMergeMode(E131Bridge::GetMergeMode(port_index), true));

    if (kIsActive)
    {
        uint8_t status = artnet::GoodOutput::kOutputIsSacn;
        status = status | (E131Bridge::IsTransmitting(port_index) ? artnet::GoodOutput::kDataIsBeingTransmitted : artnet::GoodOutput::kOutputNone);
        status = status | (E131Bridge::IsMerging(port_index) ? artnet::GoodOutput::kOutputIsMerging : artnet::GoodOutput::kOutputNone);
        return status;
    }

    return 0;
}

inline void ArtNetNode::SetLedBlinkMode4(hal::statusled::Mode mode)
{
    static hal::statusled::Mode s_mode;

    if (s_mode != mode)
    {
        s_mode = mode;
        DEBUG_PRINTF("mode=%u", static_cast<uint32_t>(mode));
    }

    E131Bridge::SetEnableDataIndicator(mode == hal::statusled::Mode::NORMAL);

    for (uint32_t port_index = 0; port_index < dmxnode::kMaxPorts; port_index++)
    {
        if (E131Bridge::IsTransmitting(port_index))
        {
            return;
        }
    }

    hal::statusled::SetMode(mode);
}

#endif  // ARTNETNODE4_INLINE_IMPL_H_
