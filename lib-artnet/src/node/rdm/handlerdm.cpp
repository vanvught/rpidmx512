/**
 * @file handlerdm.cpp
 *
 */
/* Copyright (C) 2023-2026 by Arjan van Vught mailto:info@gd32-dmx.org
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

#if defined(DEBUG_ARTNET_RDM)
#undef NDEBUG
#endif

#include <cstdint>

#include "artnetnode.h"
#include "firmware/debug/debug_debug.h"

/**
 * Node Output Gateway -> Reply with ArtTodData.
 *
 * This packet is used to request the Table of RDM Devices (TOD). A Node receiving this
 * packet must not interpret it as forcing full discovery. Full discovery is only initiated at
 * power on or when an ArtTodControl.AtcFlush is received. The response is ArtTodData.

 */
void ArtNetNode::HandleTodRequest()
{
    DEBUG_ENTRY();

    const auto* const kRequest = reinterpret_cast<artnet::ArtTodRequest*>(receive_buffer_);
    // The number of entries in Address that are used. Max value is 32.
    const auto kAddCount = kRequest->add_count & 0x1f;

    for (auto count = 0; count < kAddCount; count++)
    {
        // Address[count] This array defines the low byte of the Port-Address of the Output Gateway nodes that
        // must respond to this packet. This is combined with the 'Net' field above to form the 15 bit address.
        const auto kPortAddress = static_cast<uint16_t>((kRequest->net << 8)) | static_cast<uint16_t>((kRequest->address[count]));

        for (uint32_t port_index = 0; port_index < dmxnode::kMaxPorts; port_index++)
        {
            if ((output_port_[port_index].good_output_b & artnet::GoodOutputB::kRdmDisabled) == artnet::GoodOutputB::kRdmDisabled)
            {
                continue;
            }

            if ((kPortAddress == node_.port[port_index].port_address) && (node_.port[port_index].direction == dmxnode::PortDirection::kOutput))
            {
                SendArtTodData(port_index);
            }
        }
    }

#if defined(RDM_CONTROLLER)
    for (auto& entry : state_.art.tod_request_ip_list)
    {
        if (entry == ip_address_from_)
        {
            DEBUG_EXIT();
            return;
        }

        if (entry == 0)
        {
            entry = ip_address_from_;
            DEBUG_EXIT();
            return;
        }
    }
#endif

    DEBUG_EXIT();
}
