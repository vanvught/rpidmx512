/**
 * @file handleinput.cpp
 *
 */
/* Copyright (C) 2023-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include <cstdint>

#include "artnetnode.h"
#include "artnet.h"
#if (ARTNET_VERSION >= 4) && defined(E131_HAVE_DMXIN)
#include "e131bridge.h"
#endif

 #include "firmware/debug/debug_debug.h"

/**
 * A Controller or monitoring device on the network can
 * enable or disable individual DMX512 inputs on any of the network nodes.
 * This allows the Controller to directly control network traffic and
 * ensures that unused inputs are disabled and therefore not wasting bandwidth.
 */

void ArtNetNode::HandleInput()
{
    DEBUG_ENTRY();

    const auto* const kArtInput = reinterpret_cast<artnet::ArtInput*>(receive_buffer_);
    const auto kPortIndex = static_cast<uint32_t>(kArtInput->bind_index > 0 ? kArtInput->bind_index - 1 : 0);

    if (kArtInput->NumPortsLo == 1)
    {
        if (node_.port[kPortIndex].direction == dmxnode::PortDirection::kInput)
        {
            if (kArtInput->Input[0] & 0x01)
            {
                input_port_[kPortIndex].good_input |= static_cast<uint8_t>(artnet::GoodInput::kDisabled);
            }
            else
            {
                input_port_[kPortIndex].good_input &= static_cast<uint8_t>(~static_cast<uint8_t>(artnet::GoodInput::kDisabled));
            }
#if (ARTNET_VERSION >= 4) && defined(E131_HAVE_DMXIN)
            E131Bridge::SetInputDisabled(kPortIndex, kArtInput->Input[0] & 0x01);
#endif
        }
    }

    if (state_.send_art_poll_reply_on_change)
    {
        SendPollReply(0, ip_address_from_);
    }

    DEBUG_EXIT();
}
