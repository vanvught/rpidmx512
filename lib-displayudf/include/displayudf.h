/**
 * @file displayudf.h
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

#ifndef DISPLAYUDF_H_
#define DISPLAYUDF_H_

#include <cstdint>
#include <cstdarg>

#include "display.h"
#include "firmwareversion.h"
#if !defined(NO_EMAC)
#include "network.h"
#include "core/protocol/dhcp.h"
#endif
#if defined(NODE_ARTNET_MULTI)
#define NODE_ARTNET
#endif
#if defined(NODE_E131_MULTI)
#define NODE_E131
#endif
#if defined(NODE_ARTNET)
#include "artnetnode.h"
#endif
#if defined(NODE_E131)
#include "e131bridge.h"
#endif
#if defined(RDM_RESPONDER)
#include "rdmdeviceresponder.h"
#endif
#if defined(OUTPUT_DMX_SEND) || defined(OUTPUT_DMX_SEND_MULTI)
#include "dmx.h"
#include "dmxconst.h"
#endif
#if defined(RDM_RESPONDER) || defined(OUTPUT_DMX_MONITOR) || defined(OUTPUT_DMX_PCA9685) || defined(OUTPUT_DMX_PIXEL) || defined(OUTPUT_DMX_TLC59711)
#define HAVE_DMX_START_ADDRESS
#endif
#include "firmware/debug/debug_debug.h"

namespace displayudf
{
inline constexpr uint32_t kLabelMaxRows = 6;

enum class Labels : uint8_t
{
    kTitle,
    kBoardname,
    kVersion,
    kHostname,
    kIp,
    kNetmask,
    kDefaultGateway,
    kAp,
    kDmxStartAddress,
#if defined(DMX_MAX_PORTS)
    kUniversePortA,
#if (DMX_MAX_PORTS > 1)
    kUniversePortB,
#endif
#if (DMX_MAX_PORTS > 2)
    UNIVERSE_PORT_C,
#endif
#if (DMX_MAX_PORTS == 4)
    UNIVERSE_PORT_D,
#endif
#if defined(NODE_ARTNET) || defined(NODE_ARTNET_MULTI)
    kDestinationIpPortA,
#if (DMX_MAX_PORTS > 1)
    kDestinationIpPortB,
#endif
#if (DMX_MAX_PORTS > 2)
    DESTINATION_IP_PORT_C,
#endif
#if (DMX_MAX_PORTS == 4)
    DESTINATION_IP_PORT_D,
#endif
#endif
#endif
    kUnknown
};

namespace defaults
{
inline constexpr uint8_t kIntensity = 0x7F;
} // namespace defaults
} // namespace displayudf

class DisplayUdf final : public Display
{
   public:
    DisplayUdf();

    DisplayUdf(const DisplayUdf&) = delete;
    DisplayUdf& operator=(const DisplayUdf&) = delete;

    ~DisplayUdf() = default;

    void SetTitle(const char* format, ...);
    void Set(uint32_t line, displayudf::Labels label);

    uint8_t GetLabel(uint32_t index) const
    {
        if (index < static_cast<uint32_t>(displayudf::Labels::kUnknown))
        {
            return labels_[index];
        }

        return labels_[0];
    }

    void Show();

    /**
     * Art-Net
     */

#if defined(NODE_ARTNET)
    void ShowUniverseArtNetNode();
#endif
    /**
     * RDM Responder
     */

#if defined(RDM_RESPONDER)
    void ShowDmxStartAddress()
    {
        const auto nDmxStartAddress = RDMDeviceResponder::Get()->GetDmxStartAddress();
        const auto nDmxFootprint = RDMDeviceResponder::Get()->GetDmxFootPrint();
        Printf(labels_[static_cast<uint32_t>(displayudf::Labels::kDmxStartAddress)], "DMX S:%3u F:%3u", nDmxStartAddress, nDmxFootprint);
    }
#endif

    /**
     * Network
     */

#if !defined(NO_EMAC)
    void ShowEmacInit()
    {
        ClearEndOfLine();
        Printf(labels_[static_cast<uint32_t>(displayudf::Labels::kIp)], "Ethernet init");
    }

    void ShowEmacStart()
    {
        ClearEndOfLine();
        Printf(labels_[static_cast<uint32_t>(displayudf::Labels::kIp)], "Ethernet start");
    }

    void ShowEmacStatus(bool is_link_up)
    {
        ClearEndOfLine();
        Printf(labels_[static_cast<uint32_t>(displayudf::Labels::kIp)], "Ethernet Link %s", is_link_up ? "UP" : "DOWN");
    }

    void ShowIpAddress()
    {
        ClearEndOfLine();
        Printf(labels_[static_cast<uint32_t>(displayudf::Labels::kIp)], "" IPSTR "/%d %c", IP2STR(network::GetPrimaryIp()), network::GetNetmaskCIDR(), network::iface::AddressingMode());
    }

    void ShowNetmask()
    {
        ClearEndOfLine();
        Printf(labels_[static_cast<uint32_t>(displayudf::Labels::kNetmask)], "N: " IPSTR "", IP2STR(network::GetNetmask()));
        ShowIpAddress();
    }

    void ShowGatewayIp()
    {
        ClearEndOfLine();
        Printf(labels_[static_cast<uint32_t>(displayudf::Labels::kDefaultGateway)], "G: " IPSTR "", IP2STR(network::GetGatewayIp()));
    }

    void ShowHostName()
    {
        ClearEndOfLine();
        Write(labels_[static_cast<uint32_t>(displayudf::Labels::kHostname)], network::iface::HostName());
    }

    void ShowDhcpStatus(network::dhcp::State state)
    {
        switch (state)
        {
            case network::dhcp::State::kOff:
                break;
            case network::dhcp::State::kRenewing:
                ClearEndOfLine();
                Printf(labels_[static_cast<uint32_t>(displayudf::Labels::kIp)], "DHCP renewing");
                break;
            case network::dhcp::State::kBound:
                break;
            default:
                break;
        }
    }

    void ShowShutdown() { TextStatus("Network shutdown"); }
#endif

    static DisplayUdf* Get() { return s_this; }

   private:
    /**
     * Art-Net
     */

#if defined(NODE_ARTNET)
    void ShowArtNetNode();
    void ShowDestinationIpArtNetNode();
#endif

    /**
     * sACN E1.31
     */

#if defined(NODE_E131)
    void ShowE131Bridge();
#endif

   private:
    char title_[32];
    uint8_t labels_[static_cast<uint32_t>(displayudf::Labels::kUnknown)];

    inline static DisplayUdf* s_this;
};

#endif // DISPLAYUDF_H_
