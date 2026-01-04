/**
 * @file dmxmonitor.h
 *
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

#ifndef DMXMONITOR_H_
#define DMXMONITOR_H_

#include <cstdint>

#include "dmxnode.h"

 #include "firmware/debug/debug_debug.h"

namespace dmxmonitor
{
enum class Format: uint8_t
{
    kHex,
    kPct,
    kDec,
};

#if defined(__linux__) || defined(__APPLE__)
inline constexpr uint32_t kDmxDefaultMaxChannels = 32;
#else
inline constexpr uint32_t kDmxDefaultMaxChannels = 512;
#endif

namespace output
{
namespace hdmi
{
static constexpr char kMaxPorts = 1;
} // namespace hdmi
namespace text
{
#if !defined(DMXNODE_PORTS)
static constexpr char kMaxPorts = 4;
#else
static constexpr char kMaxPorts = DMXNODE_PORTS;
#endif
} // namespace text
} // namespace output
} // namespace dmxmonitor

class DmxMonitor
{
   public:
    DmxMonitor();
    ~DmxMonitor() = default;

    void Print()
    {
        DEBUG_ENTRY();
        DEBUG_EXIT();
    }

    void Start(uint32_t port_index);
    void Stop(uint32_t port_index);

    template <bool doUpdate> void SetData(uint32_t port_index, const uint8_t* data, uint32_t length);

    void Sync([[maybe_unused]] uint32_t port_index)
    {
#if defined(__linux__) || defined(__APPLE__)
        Update(port_index, data_[port_index].data, data_[port_index].length);
#else
        Update();
#endif
    }

    void Sync() {}

    uint32_t GetUserData() { return 0; }
    uint32_t GetRefreshRate() { return 0; }

    void Blackout([[maybe_unused]] bool blackout)
    {
        DEBUG_ENTRY();
        DEBUG_EXIT();
    }

    void FullOn()
    {
        DEBUG_ENTRY();
        DEBUG_EXIT();
    }

#if defined(OUTPUT_HAVE_STYLESWITCH)
    void SetOutputStyle(uint32_t port_index, dmxnode::OutputStyle output_style)
    {
        DEBUG_ENTRY();

        assert(port_index < 32);
        outputstyle_ = static_cast<uint32_t>(output_style) << port_index;

        DEBUG_EXIT();
    }

    dmxnode::OutputStyle GetOutputStyle([[maybe_unused]] uint32_t port_index) const
    {
        DEBUG_ENTRY();
#if defined(__linux__) || defined(__APPLE__)
        assert(port_index < 32);
        return static_cast<dmxnode::OutputStyle>(outputstyle_ >> port_index);
#else
        return dmxnode::OutputStyle::kDelta;
#endif
        DEBUG_EXIT();
    }
#endif

    bool SetDmxStartAddress(uint16_t dmx_start_address);
    uint16_t GetDmxStartAddress() { return dmx_start_address_; }

    uint16_t GetMaxDmxChannels() { return max_channels_; }

    uint16_t GetDmxFootprint()
    {
#if defined(__linux__) || defined(__APPLE__)
        return max_channels_;
#else
        return dmxnode::kUniverseSize;
#endif
    }

    bool GetSlotInfo([[maybe_unused]] uint16_t slot_offset, dmxnode::SlotInfo& slot_info)
    {
        slot_info.type = 0x00;       // ST_PRIMARY
        slot_info.category = 0x0001; // SD_INTENSITY
        return true;
    }

    void SetFormat(dmxmonitor::Format format) { format_ = format; }
    dmxmonitor::Format GetFormat() const { return format_; }

#if defined(__linux__) || defined(__APPLE__)
    void Cls() {}
#else
    void Cls();
#endif

    static DmxMonitor& Instance()
    {
        assert(s_this != nullptr);
        return *s_this;
    }

#if defined(__linux__) || defined(__APPLE__)
    void SetMaxDmxChannels(uint16_t nMaxChannels) { max_channels_ = nMaxChannels; }

   private:
    void DisplayDateTime(const uint32_t port_index, const char* pString);
    void Update(const uint32_t port_index, const uint8_t* pData, const uint32_t nLength);
#else
   private:
    void Update();
#endif

   private:
    dmxmonitor::Format format_{dmxmonitor::Format::kHex};
    uint16_t dmx_start_address_{dmxnode::kStartAddressDefault};
    uint16_t max_channels_{dmxmonitor::kDmxDefaultMaxChannels};
#if defined(OUTPUT_HAVE_STYLESWITCH)
    uint32_t outputstyle_;
#endif
#if defined(__linux__) || defined(__APPLE__)
    bool started_[dmxmonitor::output::text::kMaxPorts];
    struct Data
    {
        uint8_t data[516];
        uint32_t length;
    };
    struct Data data_[dmxmonitor::output::text::kMaxPorts];
#else
    uint8_t data_[512];
    uint16_t slots_{0};
    bool started_{false};
#endif

    static inline DmxMonitor* s_this;
};

#endif  // DMXMONITOR_H_
