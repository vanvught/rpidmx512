/**
 * @file pixeldmx.h
 */
/* Copyright (C) 2016-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef PIXELDMX_H_
#define PIXELDMX_H_

#if defined(DEBUG_PIXELDMX)
#if defined(NDEBUG)
#undef NDEBUG
#define _NDEBUG
#endif
#endif

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC push_options
#pragma GCC optimize("O3")
#pragma GCC optimize("no-tree-loop-distribute-patterns")
#pragma GCC optimize("-fprefetch-loop-arrays")
#endif

#include <cstdint>
#include <algorithm>
#include <cassert>

#include "pixeloutput.h"
#include "pixeldmxconfiguration.h"
#include "pixeldmxstore.h"
#if defined(PIXELDMXSTARTSTOP_GPIO)
#include "hal_gpio.h"
#endif
#include "dmxnode.h"
 #include "firmware/debug/debug_debug.h"

#if defined(OUTPUT_DMX_PIXEL) && defined(RDM_RESPONDER) && !defined(NODE_ARTNET)
#include "dmxnodeoutputrdmpixel.h"
#define OVERRIDE override
class PixelDmx final : public DmxNodeOutputRdmPixel, public PixelDmxConfiguration
{
#else
#define OVERRIDE
#define SETDATA
class PixelDmx final : public PixelDmxConfiguration
{
#endif
   public:
    PixelDmx()
    {
        DEBUG_ENTRY();

        assert(s_this == nullptr);
        s_this = this;

#if defined(PIXELDMXSTARTSTOP_GPIO)
        FUNC_PREFIX(GpioFsel(PIXELDMXSTARTSTOP_GPIO, GPIO_FSEL_OUTPUT));
        FUNC_PREFIX(GpioClr(PIXELDMXSTARTSTOP_GPIO));
#endif

        ApplyConfiguration();

        DEBUG_EXIT();
    }

    ~PixelDmx() OVERRIDE
    {
        DEBUG_ENTRY();

        DEBUG_EXIT();
    }

    void ApplyConfiguration()
    {
        DEBUG_ENTRY();
        PixelDmxConfiguration::Validate(1);

#ifndef NDEBUG
        PixelDmxConfiguration::Print();
#endif

         output_type_.ApplyConfiguration();
        output_type_.Blackout();

        DEBUG_EXIT();
    }

    void Start([[maybe_unused]] uint32_t port_index) OVERRIDE
    {
        if (started_)
        {
            return;
        }

        started_ = true;

#if defined(PIXELDMXSTARTSTOP_GPIO)
        FUNC_PREFIX(GpioSet(PIXELDMXSTARTSTOP_GPIO));
#endif
    }

    void Stop([[maybe_unused]] uint32_t port_index) OVERRIDE
    {
        if (!started_)
        {
            return;
        }

        started_ = false;

#if defined(PIXELDMXSTARTSTOP_GPIO)
        FUNC_PREFIX(GpioClr(PIXELDMXSTARTSTOP_GPIO));
#endif
    }

#if defined(SETDATA)
    template <bool do_update> void SetData(uint32_t port_index, const uint8_t* data, uint32_t length) { SetDataImpl<do_update>(port_index, data, length); }

    template <bool do_update> void SetDataImpl([[maybe_unused]] uint32_t port_index, const uint8_t* data, uint32_t length)
    {
#else
    void SetDataImpl([[maybe_unused]] uint32_t port_index, const uint8_t* data, uint32_t length, bool do_update) OVERRIDE
    {

#endif
        assert(data != nullptr);
        assert(length <= dmxnode::kUniverseSize);

        if (output_type_.IsUpdating())
        {
			puts("output_type_.IsUpdating()");
            return;
        }

        auto& port_info = PixelDmxConfiguration::GetPortInfo();
        uint32_t d = 0;

#if !defined(DMXNODE_PORTS)
        static constexpr uint32_t kSwitch = 0;
#else
        const auto kSwitch = port_index & 0x03;
#endif
        const auto kGroups = PixelDmxConfiguration::GetGroups();
#if !defined(DMXNODE_PORTS)
        static constexpr uint32_t kBeginIndex = 0;
#else
        const auto kBeginIndex = port_info.begin_index_port[kSwitch];
#endif
        const auto kChannelsPerPixel = PixelDmxConfiguration::GetLedsPerPixel();
        const auto kEndIndex = std::min(kGroups, (kBeginIndex + (length / kChannelsPerPixel)));

        if ((kSwitch == 0) && (kGroups < port_info.begin_index_port[1]))
        {
			assert(PixelDmxConfiguration::GetDmxStartAddress() != 0);
            d = (PixelDmxConfiguration::GetDmxStartAddress() - 1U);
        }

        const auto kGroupingCount = PixelDmxConfiguration::GetGroupingCount();

        if (kChannelsPerPixel == 3)
        {
            switch (PixelDmxConfiguration::GetMap())
            {
                case pixel::Map::RGB:
                    for (uint32_t j = kBeginIndex; (j < kEndIndex) && (d < length); j++)
                    {
                        auto const kPixelIndexStart = (j * kGroupingCount);
                        for (uint32_t k = 0; k < kGroupingCount; k++)
                        {
                            output_type_.SetPixel(kPixelIndexStart + k, data[d + 0], data[d + 1], data[d + 2]);
                        }
                        d = d + 3;
                    }
                    break;
                case pixel::Map::RBG:
                    for (uint32_t j = kBeginIndex; (j < kEndIndex) && (d < length); j++)
                    {
                        auto const kPixelIndexStart = (j * kGroupingCount);
                        for (uint32_t k = 0; k < kGroupingCount; k++)
                        {
                            output_type_.SetPixel(kPixelIndexStart + k, data[d + 0], data[d + 2], data[d + 1]);
                        }
                        d = d + 3;
                    }
                    break;
                case pixel::Map::GRB:
                    for (uint32_t j = kBeginIndex; (j < kEndIndex) && (d < length); j++)
                    {
                        auto const kPixelIndexStart = (j * kGroupingCount);
                        for (uint32_t k = 0; k < kGroupingCount; k++)
                        {
                            output_type_.SetPixel(kPixelIndexStart + k, data[d + 1], data[d + 0], data[d + 2]);
                        }
                        d = d + 3;
                    }
                    break;
                case pixel::Map::GBR:
                    for (uint32_t j = kBeginIndex; (j < kEndIndex) && (d < length); j++)
                    {
                        auto const kPixelIndexStart = (j * kGroupingCount);
                        for (uint32_t k = 0; k < kGroupingCount; k++)
                        {
                            output_type_.SetPixel(kPixelIndexStart + k, data[d + 2], data[d + 0], data[d + 1]);
                        }
                        d = d + 3;
                    }
                    break;
                case pixel::Map::BRG:
                    for (uint32_t j = kBeginIndex; (j < kEndIndex) && (d < length); j++)
                    {
                        auto const kPixelIndexStart = (j * kGroupingCount);
                        for (uint32_t k = 0; k < kGroupingCount; k++)
                        {
                            output_type_.SetPixel(kPixelIndexStart + k, data[d + 1], data[d + 2], data[d + 0]);
                        }
                        d = d + 3;
                    }
                    break;
                case pixel::Map::BGR:
                    for (uint32_t j = kBeginIndex; (j < kEndIndex) && (d < length); j++)
                    {
                        auto const kPixelIndexStart = (j * kGroupingCount);
                        for (uint32_t k = 0; k < kGroupingCount; k++)
                        {
                            output_type_.SetPixel(kPixelIndexStart + k, data[d + 2], data[d + 1], data[d + 0]);
                        }
                        d = d + 3;
                    }
                    break;
                default:
                    assert(0);
                    __builtin_unreachable();
                    break;
            }
        }
        else
        {
            assert(kChannelsPerPixel == 4);
            for (auto j = kBeginIndex; (j < kEndIndex) && (d < length); j++)
            {
                auto const kPixelIndexStart = (j * kGroupingCount);
                for (uint32_t k = 0; k < kGroupingCount; k++)
                {
                    output_type_.SetPixel(kPixelIndexStart + k, data[d], data[d + 1], data[d + 2], data[d + 3]);
                }
                d = d + 4;
            }
        }

#if !defined(DMXNODE_PORTS)
        if (do_update)
        {
            if (__builtin_expect((blackout_), 0))
            {
                return;
            }
            output_type_.Update();
        }
#else
#if !defined(SETDATA)
#error
#endif
        if constexpr (do_update)
        {
            if (port_index == port_info.protocol_port_index_last)
            {
                if (__builtin_expect((blackout_), 0))
                {
                    return;
                }
                output_type_.Update();
            }
        }
#endif
    }

    void Sync([[maybe_unused]] uint32_t port_index) {}

    void Sync()
    {
        output_type_.Update();
    }

#if defined(OUTPUT_HAVE_STYLESWITCH)
    void SetOutputStyle([[maybe_unused]] uint32_t port_index, [[maybe_unused]] dmxnode::OutputStyle output_style) {}
    dmxnode::OutputStyle GetOutputStyle([[maybe_unused]] uint32_t port_index) const { return dmxnode::OutputStyle::kDelta; }
#endif

    void Blackout(bool blackout = true)
    {
        blackout_ = blackout;

        while (output_type_.IsUpdating())
        {
            // wait for completion
        }

        if (blackout)
        {
            output_type_.Blackout();
        }
        else
        {
            output_type_.Update();
        }
    }

    void FullOn()
    {
        while (output_type_.IsUpdating())
        {
            // wait for completion
        }

        output_type_.FullOn();
    }

    void Print() OVERRIDE { PixelDmxConfiguration::Get().Print(); }

    // RDM
    bool SetDmxStartAddress(uint16_t dmx_start_address) OVERRIDE
    {
        if (dmx_start_address == PixelDmxConfiguration::GetDmxStartAddress())
        {
            return true;
        }

        if ((dmx_start_address + PixelDmxConfiguration::GetDmxFootprint()) > dmxnode::kUniverseSize)
        {
            return false;
        }

        if ((dmx_start_address != 0) && (dmx_start_address <= dmxnode::kUniverseSize))
        {
            PixelDmxConfiguration::SetDmxStartAddress(dmx_start_address);
            dmxled_store::SaveDmxStartAddress(dmx_start_address);
            return true;
        }

        return false;
    }

    uint16_t GetDmxStartAddress() OVERRIDE { return PixelDmxConfiguration::Get().GetDmxStartAddress(); }

    uint16_t GetDmxFootprint() OVERRIDE { return PixelDmxConfiguration::Get().GetDmxFootprint(); }

    bool GetSlotInfo(uint16_t slotoffset, dmxnode::SlotInfo& slotinfo) OVERRIDE
    {
        auto& pixeldmx_configuration = PixelDmxConfiguration::Get();

        if (slotoffset > pixeldmx_configuration.GetDmxFootprint())
        {
            return false;
        }

        slotinfo.type = 0x00; // ST_PRIMARY

        switch (slotoffset % pixeldmx_configuration.GetLedsPerPixel())
        {
            case 0:
                slotinfo.category = 0x0205; // SD_COLOR_ADD_RED
                break;
            case 1:
                slotinfo.category = 0x0206; // SD_COLOR_ADD_GREEN
                break;
            case 2:
                slotinfo.category = 0x0207; // SD_COLOR_ADD_BLUE
                break;
            case 3:
                slotinfo.category = 0x0212; // SD_COLOR_ADD_WHITE
                break;
            default:
                __builtin_unreachable();
                break;
        }

        return true;
    }

    /*
     * Art-Net ArtPollReply
     */
    uint32_t GetUserData() { return 0; }
    uint32_t GetRefreshRate() { return 0; }

    static PixelDmx& Get()
    {
        assert(s_this != nullptr); // Ensure that s_this is valid
        return *s_this;
    }

   private:
    PixelOutputType output_type_;

    bool started_{false};
    bool blackout_{false};

    static inline PixelDmx* s_this;
};

#undef OVERRIDE
#undef SETDATA
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC pop_options
#endif
#if defined(_NDEBUG)
#undef _NDEBUG
#define NDEBUG
#endif

#endif  // PIXELDMX_H_
