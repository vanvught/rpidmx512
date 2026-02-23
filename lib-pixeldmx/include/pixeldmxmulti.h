/**
 * @file pixeldmxmulti.h
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

#ifndef PIXELDMXMULTI_H_
#define PIXELDMXMULTI_H_

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

#include "dmxnodedata.h"
#include "pixeloutputmulti.h"
#include "pixeldmxconfiguration.h"
#include "logic_analyzer.h"
#if defined(PIXELDMXSTARTSTOP_GPIO)
#include "hal_gpio.h"
#endif
 #include "firmware/debug/debug_debug.h"

namespace pixeldmxmulti
{
#if !defined(CONFIG_DMXNODE_PIXEL_MAX_PORTS)
#error
#endif
static constexpr auto kMaxPorts = CONFIG_DMXNODE_PIXEL_MAX_PORTS;
} // namespace pixeldmxmulti

class PixelDmxMulti final : public PixelDmxConfiguration
{
   public:
    PixelDmxMulti()
    {
        DEBUG_ENTRY();

        assert(s_this == nullptr);
        s_this = this;

        ApplyConfiguration();

#if defined(PIXELDMXSTARTSTOP_GPIO)
        FUNC_PREFIX(GpioFsel(PIXELDMXSTARTSTOP_GPIO, GPIO_FSEL_OUTPUT));
        FUNC_PREFIX(GpioClr(PIXELDMXSTARTSTOP_GPIO));
#endif

        DEBUG_EXIT();
    }

    ~PixelDmxMulti()
    {
        DEBUG_ENTRY();

      DEBUG_EXIT();
    }

    void ApplyConfiguration()
    {
        started_[0] = 0;
        started_[1] = 0;

        PixelDmxConfiguration::Validate(pixeldmxmulti::kMaxPorts);

#ifndef NDEBUG
        PixelDmxConfiguration::Print();
#endif

        output_type_.ApplyConfiguration();
        output_type_.Blackout();
    }

    void Start(uint32_t port_index)
    {
        const auto kIndex = (port_index <= 31) ? 0 : 1;
        DEBUG_PRINTF("%u [%u]", port_index, kIndex);

        if (kIndex == 0)
        {
            started_[0] |= (1U << port_index);
        }
        else
        {
            started_[1] |= (1U << (port_index - 32));
        }

#if defined(PIXELDMXSTARTSTOP_GPIO)
        if ((started_[0] != 0) || (started_[1] != 0))
        {
            FUNC_PREFIX(GpioSet(PIXELDMXSTARTSTOP_GPIO));
        }
#endif
    }

    void Stop(uint32_t port_index)
    {
        const auto kIndex = (port_index <= 31) ? 0 : 1;
        DEBUG_PRINTF("%u [%u]", port_index, kIndex);

        if (kIndex == 0)
        {
            if (started_[0] & (1U << port_index))
            {
                started_[0] &= ~(1U << port_index);
            }
        }
        else
        {
            if (started_[1] & (1U << (port_index - 32)))
            {
                started_[1] &= ~(1U << (port_index - 32));
            }
        }

#if defined(PIXELDMXSTARTSTOP_GPIO)
        if ((started_[0] == 0) && (started_[1] == 0))
        {
            FUNC_PREFIX(GpioClr(PIXELDMXSTARTSTOP_GPIO));
        }
#endif
    }

    template <bool doUpdate> void SetData(uint32_t port_index, const uint8_t* data, uint32_t length)
    {
        logic_analyzer::Ch0Set();

        SetData(port_index, data, length);

        auto& port_info = PixelDmxConfiguration::GetPortInfo();

        if constexpr (doUpdate)
        {
            if (port_index == port_info.protocol_port_index_last)
            {
                logic_analyzer::Ch1Set();

                for (uint32_t index = 0; index <= port_info.protocol_port_index_last; index++)
                {
                    logic_analyzer::Ch2Set();
                    SetData(index, dmxnode::Data::Backup(index), dmxnode::Data::GetLength(index));
                    logic_analyzer::Ch2Clear();
                }

                output_type_.Update();

                logic_analyzer::Ch1Clear();
            }
        }

        logic_analyzer::Ch0Clear();
    }

    void Sync([[maybe_unused]] uint32_t port_index)
    {
        logic_analyzer::Ch2Set();

        need_sync_ = true;

        logic_analyzer::Ch2Clear();
    }

    void Sync()
    {
        if (!need_sync_)
        {
            return;
        }

        logic_analyzer::Ch1Set();

        output_type_.Update();

        need_sync_ = false;

        logic_analyzer::Ch1Clear();
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

    void Print() { PixelDmxConfiguration::Get().Print(); }

    // Art-Net PollReply
    uint32_t GetUserData() { return output_type_.GetUserData(); }

    uint32_t GetRefreshRate() { return PixelConfiguration::GetRefreshRate(); }

    // RDMNet LLRP Device Only
    bool SetDmxStartAddress([[maybe_unused]] uint16_t dmx_start_address) { return false; }
    uint16_t GetDmxStartAddress() { return dmxnode::kAddressInvalid; }

    uint16_t GetDmxFootprint() { return 0; }

    bool GetSlotInfo([[maybe_unused]] uint16_t slot_offset, dmxnode::SlotInfo& slot_info)
    {
        slot_info.type = 0x00;       // ST_PRIMARY
        slot_info.category = 0x0001; // SD_INTENSITY
        return true;
    }

    static PixelDmxMulti& Get()
    {
        assert(s_this != nullptr); // Ensure that s_this is valid
        return *s_this;
    }

   private:
    void SetData(uint32_t port_index, const uint8_t* data, uint32_t length)
    {
        assert(data != nullptr);
        assert(length <= dmxnode::kUniverseSize);

#if defined(NODE_DDP_DISPLAY)
        const auto kOutIndex = (port_index / 4);
        const auto kSwitch = port_index - (kOutIndex * 4);
#else
        const auto kUniverses = PixelDmxConfiguration::GetUniverses();
        const auto kOutIndex = (port_index / kUniverses);
        const auto kSwitch = port_index - (kOutIndex * kUniverses);
#endif
        auto& port_info = PixelDmxConfiguration::GetPortInfo();

        const auto kGroups = PixelDmxConfiguration::GetGroups();
        const auto kBeginIndex = port_info.begin_index_port[kSwitch];
        const auto kChannelsPerPixel = PixelDmxConfiguration::GetLedsPerPixel();
        const auto kEndIndex = std::min(kGroups, (kBeginIndex + (length / kChannelsPerPixel)));
        const auto kGroupingCount = PixelDmxConfiguration::GetGroupingCount();
        const auto kPixelType = PixelDmxConfiguration::GetType();
        const auto kIsRtzProtocol = PixelDmxConfiguration::IsRTZProtocol();

        uint32_t d = 0;

        if (kChannelsPerPixel == 3)
        {
            // Define a lambda to handle pixel setting based on color order
            auto set_pixels_colour_rtz = [&](uint32_t portindex, uint32_t pixelindex, uint32_t r, uint32_t g, uint32_t b)
            {
#if defined(CONFIG_PIXELDMX_ENABLE_GAMMATABLE)
                const auto kGammaTable = PixelDmxConfiguration::GetGammaTable();
                r = kGammaTable[r];
                g = kGammaTable[g];
                b = kGammaTable[b];
#endif
                output_type_.SetColourRTZ(portindex, pixelindex, r, g, b);
            };

            // Define a lambda to handle pixel setting based on color order
            auto set_pixels_colour3 = [&](uint32_t portindex, uint32_t pixelindex, uint32_t r, uint32_t g, uint32_t b)
            {
#if defined(CONFIG_PIXELDMX_ENABLE_GAMMATABLE)
                const auto kGammaTable = PixelDmxConfiguration::GetGammaTable();
                r = kGammaTable[r];
                g = kGammaTable[g];
                b = kGammaTable[b];
#endif

                switch (kPixelType)
                {
                    case pixel::Type::WS2801:
                        output_type_.SetColourWS2801(portindex, pixelindex, r, g, b);
                        break;
                    case pixel::Type::APA102:
                    case pixel::Type::SK9822:
                        output_type_.SetPixel4Bytes(portindex, 1 + pixelindex, PixelDmxConfiguration::GetGlobalBrightness(), b, g, r);
                        break;
                    case pixel::Type::P9813:
                    {
                        const auto kFlag = static_cast<uint8_t>(0xC0 | ((~b & 0xC0) >> 2) | ((~r & 0xC0) >> 4) | ((~r & 0xC0) >> 6));
                        output_type_.SetPixel4Bytes(portindex, 1 + pixelindex, kFlag, b, g, r);
                    }
                    break;
                    default:
                        assert(0);
                        __builtin_unreachable();
                        break;
                }
            };

            constexpr uint32_t kChannelMap[6][3] = {
                {0, 1, 2}, // RGB
                {0, 2, 1}, // RBG
                {1, 0, 2}, // GRB
                {2, 0, 1}, // GBR
                {1, 2, 0}, // BRG
                {2, 1, 0}  // BGR
            };

            const auto kMapIndex = static_cast<uint32_t>(PixelDmxConfiguration::GetMap());
            // Ensure mapIndex is within valid bounds
            assert(kMapIndex < sizeof(kChannelMap) / sizeof(kChannelMap[0])); // Runtime check
            auto const& map = kChannelMap[kMapIndex];

            if (kIsRtzProtocol)
            {
                for (uint32_t j = kBeginIndex; (j < kEndIndex) && (d < length); j++)
                {
                    auto const kPixelIndexStart = j * kGroupingCount;
                    for (uint32_t k = 0; k < kGroupingCount; k++)
                    {
                        set_pixels_colour_rtz(kOutIndex, kPixelIndexStart + k, data[d + map[0]], data[d + map[1]], data[d + map[2]]);
                    }
                    d += 3; // Increment by 3 since we're processing 3 channels per pixel
                }
            }
            else
            {
                for (uint32_t j = kBeginIndex; (j < kEndIndex) && (d < length); j++)
                {
                    auto const kPixelIndexStart = j * kGroupingCount;
                    for (uint32_t k = 0; k < kGroupingCount; k++)
                    {
                        set_pixels_colour3(kOutIndex, kPixelIndexStart + k, data[d + map[0]], data[d + map[1]], data[d + map[2]]);
                    }
                    d += 3; // Increment by 3 since we're processing 3 channels per pixel
                }
            }
        }
        else
        {
            assert(kChannelsPerPixel == 4);
            assert(kIsRtzProtocol);
            for (uint32_t j = kBeginIndex; (j < kEndIndex) && (d < length); j++)
            {
                auto const kPixelIndexStart = (j * kGroupingCount);
                for (uint32_t k = 0; k < kGroupingCount; k++)
                {
                    output_type_.SetColourRTZ(kOutIndex, kPixelIndexStart + k, data[d], data[d + 1], data[d + 2], data[d + 3]);
                }
                d = d + 4; // Increment by 4 since we're processing 4 channels per pixel
            }
        }
    }

   private:
    PixelOutputType output_type_;

    uint32_t started_[2]; ///< Support for 16x4 = 64 ports.
    bool blackout_{false};
    bool need_sync_{false};

    static inline PixelDmxMulti* s_this;
};

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC pop_options
#endif
#if defined(_NDEBUG)
#undef _NDEBUG
#define NDEBUG
#endif

#endif  // PIXELDMXMULTI_H_
