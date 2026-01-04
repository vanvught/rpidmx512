/**
 * @file dmxnode.h
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

#ifndef DMXNODE_H_
#define DMXNODE_H_

#include <cstdint>
#include <cstring>
#include <cstdio>
#include <cassert>

#include "configurationstore.h"

namespace dmxnode
{
inline constexpr uint16_t kAddressInvalid = 0xFFFF;
inline constexpr uint32_t kStartAddressDefault = 1;
inline constexpr uint32_t kUniverseSize = 512;
inline constexpr uint8_t kDmxMaxValue = 255;
/*
 * Art-Net
 */
inline constexpr uint32_t kNodeNameLength = 64; // LongName
static_assert(common::store::dmxnode::kNodeNameLength == kNodeNameLength);
inline constexpr uint32_t kLabelNameLength = 18; // ShortName
static_assert(common::store::dmxnode::kLabelNameLength == kLabelNameLength);
/*
 * sACN E1.31
 */
namespace priority
{
inline constexpr uint8_t kLowest = 1;
inline constexpr uint8_t kDefault = 100;
inline constexpr uint8_t kHighest = 200;
} // namespace priority

#if !defined(DMXNODE_PORTS) || (DMXNODE_PORTS == 0)
inline constexpr uint32_t kMaxPorts = 1; // ISO C++ forbids zero-size array
#else
inline constexpr uint32_t kMaxPorts = DMXNODE_PORTS; // From build config
#endif

#if !defined(CONFIG_DMXNODE_DMX_PORT_OFFSET)
inline constexpr uint32_t kDmxportOffset = 0; // Default if not overridden
#else
inline constexpr uint32_t kDmxportOffset = CONFIG_DMXNODE_DMX_PORT_OFFSET; // From build config
#endif

inline constexpr uint32_t kConfigPortCount =
    ((kMaxPorts - kDmxportOffset) <= common::store::dmxnode::kParamPorts) ? (kMaxPorts - kDmxportOffset) : common::store::dmxnode::kParamPorts;

enum class Personality
{
    kArtnet,
    kSacn,
    kNode
};

enum class MergeMode
{
    kHtp,
    kLtp
};

namespace mergemode
{
inline static constexpr const char kHtp[] = "htp";
inline static constexpr const char kLtp[] = "ltp";
inline static constexpr const char kHtpUpper[] = "HTP";
inline static constexpr const char kLtpUpper[] = "LTP";
} // namespace mergemode

enum class PortDirection
{
    kInput,
    kOutput,
    kDisable
};

enum class FailSafe
{
    kHold,
    kOff,
    kOn,
    kPlayback,
    kRecord
};

namespace failsafe
{
inline static constexpr const char kHold[] = "hold";
inline static constexpr const char kOff[] = "off";
inline static constexpr const char kOn[] = "on";
inline static constexpr const char kPlayback[] = "playback";
inline static constexpr const char kRecord[] = "record";
} // namespace failsafe

enum class OutputStyle
{
    kDelta,   ///< DMX frame is triggered
    kConstant ///< DMX output is continuous
};

enum class Rdm
{
    kDisable,
    kEnable
};

struct SlotInfo
{
    uint16_t category;
    uint8_t type;
};

inline Personality GetPersonality(const char* personality)
{
    assert(personality != nullptr);
    if (strncasecmp(personality, "node", 4) == 0)
    {
        return Personality::kNode;
    }

    if (strncasecmp(personality, "sacn", 4) == 0)
    {
        return Personality::kSacn;
    }

    return Personality::kArtnet;
}

[[nodiscard]] inline constexpr const char* GetPersonality(Personality personality)
{
    if (personality == Personality::kNode)
    {
        return "node";
    }

    if (personality == Personality::kSacn)
    {
        return "sacn";
    }

    return "artnet";
}

inline MergeMode GetMergeMode(const char* merge_mode)
{
    assert(merge_mode != nullptr);
    if (strncasecmp(merge_mode, mergemode::kLtp, sizeof(mergemode::kLtp) - 1) == 0)
    {
        return MergeMode::kLtp;
    }

    return MergeMode::kHtp;
}

[[nodiscard]] inline constexpr const char* GetMergeMode(MergeMode merge_mode, bool to_upper = false)
{
    if (to_upper)
    {
        return (merge_mode == MergeMode::kHtp) ? mergemode::kHtpUpper : mergemode::kLtpUpper;
    }
    return (merge_mode == MergeMode::kHtp) ? mergemode::kHtp : mergemode::kLtp;
}

inline const char* GetMergeMode(unsigned m, bool to_upper = false)
{
    return GetMergeMode(static_cast<MergeMode>(m), to_upper);
}

inline PortDirection GetPortDirection(const char* port_direction)
{
    assert(port_direction != nullptr);

    if (strncasecmp(port_direction, "input", 5) == 0)
    {
        return PortDirection::kInput;
    }

    if (strncasecmp(port_direction, "disable", 7) == 0)
    {
        return PortDirection::kDisable;
    }

    return PortDirection::kOutput;
}

[[nodiscard]] inline constexpr const char* GetPortDirection(PortDirection port_direction)
{
    if (port_direction == PortDirection::kInput)
    {
        return "input";
    }

    if (port_direction == PortDirection::kDisable)
    {
        return "disable";
    }

    return "output";
}

inline FailSafe GetFailsafe(const char* failsafe)
{
    if (strncasecmp(failsafe, failsafe::kHold, sizeof(failsafe::kHold) - 1) == 0)
    {
        return FailSafe::kHold;
    }

    if (strncasecmp(failsafe, failsafe::kOff, sizeof(failsafe::kOff) - 1) == 0)
    {
        return FailSafe::kOff;
    }

    if (strncasecmp(failsafe, failsafe::kOn, sizeof(failsafe::kOn) - 12) == 0)
    {
        return FailSafe::kOn;
    }

    if (strncasecmp(failsafe, failsafe::kPlayback, sizeof(failsafe::kPlayback) - 1) == 0)
    {
        return FailSafe::kPlayback;
    }

    if (strncasecmp(failsafe, failsafe::kRecord, sizeof(failsafe::kRecord) - 1) == 0)
    {
        return FailSafe::kRecord;
    }

    return FailSafe::kHold;
}

[[nodiscard]] inline constexpr const char* GetFailsafe(FailSafe failsafe)
{
    switch (failsafe)
    {
        case FailSafe::kHold:
            return failsafe::kHold;
            break;
        case FailSafe::kOff:
            return failsafe::kOff;
            break;
        case FailSafe::kOn:
            return failsafe::kOn;
            break;
        case FailSafe::kPlayback:
            return failsafe::kPlayback;
            break;
        case FailSafe::kRecord:
            return failsafe::kRecord;
            break;
    }

    return failsafe::kHold;
}

inline OutputStyle GetOutputStyle(const char* output_style)
{
    assert(output_style != nullptr);
    if (strncasecmp(output_style, "const", 5) == 0)
    {
        return OutputStyle::kConstant;
    }

    return OutputStyle::kDelta;
}

[[nodiscard]] inline constexpr const char* GetOutputStyle(OutputStyle output_style, bool to_upper = false)
{
    if (to_upper)
    {
        return (output_style == OutputStyle::kDelta) ? "DELTA" : "CONST";
    }
    return (output_style == OutputStyle::kDelta) ? "delta" : "const";
}

namespace scenes
{
inline constexpr auto kBytesNeeded = dmxnode::kMaxPorts * dmxnode::kUniverseSize;

void WriteStart();
void Write(uint32_t port_index, const uint8_t* data);
void WriteEnd();

void ReadStart();
void Read(uint32_t port_index, uint8_t* data);
void ReadEnd();
} // namespace scenes
} // namespace dmxnode

class DmxNode
{
   public:
    static DmxNode& Instance()
    {
        static DmxNode instance;
        return instance;
    }

    void SetShortName(uint32_t port_index, const char* name)
    {
        assert(port_index < dmxnode::kMaxPorts);
        auto& port = port_[port_index];

        if ((name == nullptr) || (name[0] == '\0')) return SetShortNameDefault(port_index);

        strncpy(port.label, name, dmxnode::kLabelNameLength - 1);
        port.label[dmxnode::kLabelNameLength - 1] = '\0';
    }

    void SetShortNameDefault(uint32_t port_index)
    {
        assert(port_index < dmxnode::kMaxPorts);
        auto& port = port_[port_index];

        snprintf(port.label, dmxnode::kLabelNameLength - 1, "Port %u", (1U + port_index));
        port.label[dmxnode::kLabelNameLength - 1] = '\0';
    }

    const char* GetShortName(uint32_t port_index) const
    {
        assert(port_index < dmxnode::kMaxPorts);
        const auto& port = port_[port_index];

        return port.label;
    }

    void SceneStore();
    void ScenePlayback();

   private:
    DmxNode()
    {
        for (uint32_t i = 0; i < dmxnode::kMaxPorts; i++)
        {
            SetShortNameDefault(i);
        }
    }

    struct
    {
        dmxnode::PortDirection port_direction{dmxnode::PortDirection::kDisable};
        bool is_transmitting{false};
        char label[dmxnode::kLabelNameLength];
    } port_[dmxnode::kMaxPorts];
};

#endif  // DMXNODE_H_
