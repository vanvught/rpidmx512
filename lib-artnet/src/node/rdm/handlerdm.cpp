/**
 * @file handlerdm.cpp
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
 #include "firmware/debug/debug_debug.h"

/**
 * An Output Gateway must not interpret receipt of an ArtTodRequest
 * as an instruction to perform full RDM Discovery on the DMX512 physical layer;
 * it is just a request to send the ToD back to the controller.
 */
void ArtNetNode::HandleTodRequest()
{
    DEBUG_ENTRY();

    const auto* const kRequest = reinterpret_cast<artnet::ArtTodRequest*>(receive_buffer_);
    const auto kAddCount = kRequest->AddCount & 0x1f;

    for (auto count = 0; count < kAddCount; count++)
    {
        const auto kPortAddress = static_cast<uint16_t>((kRequest->Net << 8)) | static_cast<uint16_t>((kRequest->Address[count]));

        for (uint32_t port_index = 0; port_index < dmxnode::kMaxPorts; port_index++)
        {
            if ((output_port_[port_index].good_output_b & artnet::GoodOutputB::kRdmDisabled) == artnet::GoodOutputB::kRdmDisabled)
            {
                continue;
            }

            if ((kPortAddress == node_.port[port_index].port_address) && (node_.port[port_index].direction == dmxnode::PortDirection::kOutput))
            {
                SendTod(port_index);
            }
        }
    }

    DEBUG_EXIT();
}
