/**
 * @file pixeldmxconfiguration.h
 *
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
/**
 * Static Local Variables:
 * Since C++11, the initialization of function-local static variables, is guaranteed to be thread-safe.
 * This means that even if multiple threads attempt to access Get() simultaneously,
 * the C++ runtime ensures that the instance is initialized only once.
 */

#ifndef PIXELDMXCONFIGURATION_H_
#define PIXELDMXCONFIGURATION_H_

#include <cstdint>
#include <cstdio>
#include <algorithm>
#include <cassert>

#include "dmxnode.h"
#include "pixelconfiguration.h"
#include "pixeltype.h"

namespace pixeldmxconfiguration
{
struct PortInfo
{
    uint16_t begin_index_port[4];
    uint16_t protocol_port_index_last;
};
} // namespace pixeldmxconfiguration

class PixelDmxConfiguration : public PixelConfiguration
{
   public:
    PixelDmxConfiguration()
    {
        DEBUG_ENTRY();

        assert(s_this == nullptr);
        s_this = this;

        DEBUG_EXIT();
    }

    ~PixelDmxConfiguration() = default;

    PixelDmxConfiguration(const PixelDmxConfiguration&) = delete;
    PixelDmxConfiguration& operator=(const PixelDmxConfiguration&) = delete;

    void SetOutputPorts(uint16_t output_ports) { output_ports_ = output_ports; }
    uint32_t GetOutputPorts() const { return output_ports_; }

    void SetGroupingCount(uint16_t grouping_count) { grouping_count_ = grouping_count; }
    uint32_t GetGroupingCount() const { return grouping_count_; }

    uint32_t GetGroups() const { return groups_; }

    uint32_t GetUniverses() const { return universes_; }

    pixeldmxconfiguration::PortInfo& GetPortInfo() { return port_info_; }

    void SetDmxStartAddress(uint16_t dmx_start_address)
    {
        if ((dmx_start_address > 0) && (dmx_start_address <= dmxnode::kUniverseSize))
        {
            dmx_start_address_ = dmx_start_address;
            return;
        }
        dmx_start_address_ = dmxnode::kStartAddressDefault;
    }
    uint16_t GetDmxStartAddress() const { return dmx_start_address_; }

    uint16_t GetDmxFootprint() const { return dmx_footprint_; }

    void Validate(uint32_t ports_max)
    {
        DEBUG_ENTRY();

        PixelConfiguration::Validate();

        if (!PixelConfiguration::IsRTZProtocol())
        {
            if (!((PixelConfiguration::GetType() == pixel::Type::WS2801) || (PixelConfiguration::GetType() == pixel::Type::APA102) ||
                  (PixelConfiguration::GetType() == pixel::Type::SK9822)))
            {
                PixelConfiguration::SetType(pixel::Type::WS2801);
            }

            PixelConfiguration::Validate();
        }

        port_info_.begin_index_port[0] = 0;

        if (PixelConfiguration::GetType() == pixel::Type::SK6812W)
        {
            port_info_.begin_index_port[1] = 128;
            port_info_.begin_index_port[2] = 256;
            port_info_.begin_index_port[3] = 384;
        }
        else
        {
            port_info_.begin_index_port[1] = 170;
            port_info_.begin_index_port[2] = 340;
            port_info_.begin_index_port[3] = 510;
        }

        if ((grouping_count_ == 0) || (grouping_count_ > PixelConfiguration::GetCount()))
        {
            grouping_count_ = PixelConfiguration::GetCount();
        }

        groups_ = PixelConfiguration::GetCount() / grouping_count_;
        output_ports_ = std::min(ports_max, output_ports_);
        universes_ = (1U + (groups_ / (1U + port_info_.begin_index_port[1])));
        dmx_footprint_ = static_cast<uint16_t>(PixelConfiguration::GetLedsPerPixel() * groups_);
        if (dmx_start_address_ == 0) dmx_start_address_ = dmxnode::kStartAddressDefault;

        if (ports_max == 1)
        {
            port_info_.protocol_port_index_last = static_cast<uint16_t>(groups_ / (1U + port_info_.begin_index_port[1]));
        }
        else
        {
#if defined(NODE_DDP_DISPLAY)
            port_info_.protocol_port_index_last = static_cast<uint16_t>(((output_ports_ - 1U) * 4U) + universes_ - 1U);
#else
            port_info_.protocol_port_index_last = static_cast<uint16_t>((output_ports_ * universes_) - 1U);
#endif
        }

        DEBUG_EXIT();
    }

    void Print()
    {
        PixelConfiguration::Print();
        puts("Pixel DMX configuration");
        printf(" Outputs        : %u\n", output_ports_);
        printf(" Grouping count : %u [Groups : %u]\n", grouping_count_, groups_);
        printf(" Universes      : %u\n", universes_);
        printf(" DmxFootprint   : %u\n", dmx_footprint_);

#ifndef NDEBUG
        const auto& begin_index_port = port_info_.begin_index_port;
        printf(" %u:%u:%u:%u -> %u\n", begin_index_port[0], begin_index_port[1], begin_index_port[2], begin_index_port[3], port_info_.protocol_port_index_last);
#endif
    }

    static PixelDmxConfiguration& Get()
    {
        assert(s_this != nullptr);
        return *s_this;
    }

   private:
    uint32_t output_ports_{1};
    uint32_t grouping_count_{1};
    uint32_t groups_{pixel::defaults::kCount};
    uint32_t universes_{0};
    uint16_t dmx_start_address_{1};
    uint16_t dmx_footprint_{0};
    pixeldmxconfiguration::PortInfo port_info_;

    static inline PixelDmxConfiguration* s_this;
};

#endif  // PIXELDMXCONFIGURATION_H_
