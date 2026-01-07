/**
 * @file displayudfshowartnet.cpp
 *
 */
/* Copyright (C) 2019-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#if defined(DEBUG_DISPLAYUDF)
#undef NDEBUG
#endif

#include <cstdint>

#include "displayudf.h"
#include "artnetnode.h"
#include "artnet.h"
#include "dmxnode.h"
#include "ip4/ip4_address.h"
 #include "firmware/debug/debug_debug.h"

void DisplayUdf::ShowArtNetNode()
{
    DEBUG_ENTRY();
    DEBUG_PRINTF("dmxnode::kDmxportOffset=%u", dmxnode::kDmxportOffset);

    auto* artnet_node = ArtNetNode::Get();

    ShowUniverseArtNetNode();
#if defined(ARTNET_HAVE_DMXIN)
    ShowDestinationIpArtNetNode();
#endif
    Printf(labels_[static_cast<uint32_t>(displayudf::Labels::kAp)], "AP: %d", artnet_node->GetActiveOutputPorts() + artnet_node->GetActiveInputPorts());

    DEBUG_EXIT();
}

void DisplayUdf::ShowUniverseArtNetNode()
{
#if defined(DMX_MAX_PORTS)
    DEBUG_ENTRY();
    if constexpr (dmxnode::kConfigPortCount != 0)
    {
        auto* artnet_node = ArtNetNode::Get();
        uint16_t universe;

        for (uint32_t config_port_index = 0; config_port_index < dmxnode::kConfigPortCount; config_port_index++)
        {
            const auto kPortIndex = config_port_index + dmxnode::kDmxportOffset;

            if (kPortIndex >= dmxnode::kMaxPorts)
            {
                break;
            }

            const auto kLabelIndex = static_cast<uint32_t>(displayudf::Labels::kUniversePortA) + config_port_index;

            if (kLabelIndex != 0xFF)
            {
                if (artnet_node->GetPortAddress(kPortIndex, universe, dmxnode::PortDirection::kOutput))
                {
                    ClearEndOfLine();
                    Printf(labels_[kLabelIndex],
#if defined(OUTPUT_HAVE_STYLESWITCH)
                           "%c %d %s %s %c %s",
#else
                           "%c %d %s %s %s",
#endif
                           'A' + config_port_index, universe, dmxnode::GetMergeMode(artnet_node->GetMergeMode(kPortIndex), true),
#if (ARTNET_VERSION >= 4)
                           artnet::GetProtocolMode(artnet_node->GetPortProtocol4(kPortIndex), true),
#else
                           "Art-Net",
#endif
#if defined(OUTPUT_HAVE_STYLESWITCH)
                           artnet_node->GetOutputStyle(kPortIndex) == dmxnode::OutputStyle::kConstant ? 'C' : 'D',
#endif
                           artnet_node->GetRdm(kPortIndex) ? "RDM" : "");
                }
            }
        }
    }
    DEBUG_EXIT();
#endif
}

void DisplayUdf::ShowDestinationIpArtNetNode()
{
    DEBUG_ENTRY();
#if defined(ARTNET_HAVE_DMXIN)
    if constexpr (dmxnode::kConfigPortCount != 0)
    {
        auto* artnet_node = ArtNetNode::Get();

        for (uint32_t config_port_index = 0; config_port_index < dmxnode::kConfigPortCount; config_port_index++)
        {
            const auto kPortIndex = config_port_index + dmxnode::kDmxportOffset;

            if (kPortIndex >= dmxnode::kMaxPorts)
            {
                break;
            }

            Printf(labels_[static_cast<uint32_t>(displayudf::Labels::kDestinationIpPortA) + config_port_index], "%c: " IPSTR, 'A' + config_port_index,
                   IP2STR(artnet_node->GetDestinationIp(kPortIndex)));
        }
    }
#endif
    DEBUG_EXIT();
}
