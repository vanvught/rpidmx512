/**
 * @file e131.h
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

#ifndef E131_H_
#define E131_H_

#include <cstdint>

#include "e117.h"
#include "ip4/ip4_address.h"

#if !defined(PACKED)
#define PACKED __attribute__((packed))
#endif

namespace e131
{
inline constexpr auto kMergeTimeoutSeconds = 10;
inline constexpr auto kPriorityTimeoutSeconds = 10;
inline constexpr auto kUniverseDiscoveryIntervalSeconds = 10;
inline constexpr auto kNetworkDataLossTimeoutSeconds = 2.5f;

struct OptionsMask
{
    enum class Mask : uint8_t
    {
        kPreviewData = 1U << 7,
        kStreamTerminated = 1U << 6,
        kForceSynchronization = 1U << 5
    };

    static constexpr bool Has(uint8_t value, Mask mask) noexcept { return (value & static_cast<uint8_t>(mask)) != 0; }
};

namespace universe
{
inline constexpr auto kDefault = 1;
inline constexpr auto kMax = 63999;
inline constexpr auto kDiscovery = 64214;
} // namespace universe
namespace priority
{
inline constexpr uint8_t kLowest = 1;
inline constexpr uint8_t kDefault = 100;
inline constexpr uint8_t kHighest = 200;
} // namespace priority
namespace vector
{
namespace root
{
inline constexpr uint32_t kData = 0x00000004;
inline constexpr uint32_t kExtended = 0x00000008;
} // namespace root
namespace data
{
inline constexpr auto kPacket = 0x00000002; ///< E1.31 Data Packet
} // namespace data
namespace extended
{
inline constexpr uint32_t kSynchronization = 0x00000001; ///< E1.31 Synchronization Packet
inline constexpr uint32_t kDiscovery = 0x00000002;       ///< E1.31 Universe Discovery
} // namespace extended
namespace dmp
{
inline constexpr auto kSetProperty = 0x02; ///< (Informative)
} // namespace dmp
namespace universe
{
inline constexpr uint32_t kDiscoveryUniverseList = 0x00000001;
} // namespace universe
} // namespace vector

inline constexpr uint32_t UniverseToMulticastIp(uint16_t universe)
{
    return network::ConvertToUint(239, 255, 0, 0) | (static_cast<uint32_t>(universe & 0xFF) << 24) | (static_cast<uint32_t>((universe & 0xFF00) << 8));
}

inline constexpr uint16_t kUdpPort = 5568;
inline constexpr auto kDmxLength = 512;
inline constexpr auto kSourceNameLength = 64;

///< E1.31 Framing Layer (See Section 6)
struct DataFrameLayer
{
    uint16_t flags_length;                  ///< Protocol flags and length. Low 12 bits = PDU length High 4 bits = 0x7
    uint32_t vector;                        ///< Identifies 1.31 data as DMP Protocol PDU. Fixed 0x00000002
    uint8_t source_name[kSourceNameLength]; ///< User Assigned Name of Source. UTF-8 [UTF-8] encoded string, null-terminated
    uint8_t priority;                       ///< Data priority if multiple sources. 0-200, default of 100
    uint16_t synchronization_address;       ///< Universe on which synchronization packets are transmitted
    uint8_t sequence_number;                ///< Sequence Number. To detect duplicate or out of order packets
    uint8_t options;                        ///< Options Flags Bit. 7 = Preview_Data Bit 6 = Stream_Terminated
    uint16_t universe;                      ///< Universe Number. Identifier for a distinct stream of DMX Data
} PACKED;

inline constexpr auto kDataFrameLayerSize = sizeof(struct DataFrameLayer);

///< DMP Layer (See Section 7)
///< In DMP terms the DMX packet is treated at the DMP layer as a set property message for an array of up to 513 one-octet virtually addressed properties.
struct DataDMPLayer
{
    uint16_t flags_length;                         ///< Protocol flags and length. Low 12 bits = PDU length High 4 bits = 0x7
    uint8_t vector;                                ///< Identifies DMP Set Property Message PDU. Fixed 0x02
    uint8_t type;                                  ///< Identifies format of address and data. Fixed 0xa1
    uint16_t first_address_property;               ///< Indicates DMX START Code is at DMP address 0. Fixed 0x0000
    uint16_t address_increment;                    ///< Indicates each property is 1 octet 0x0001
    uint16_t property_value_count;                 ///< Indicates 1+ the number of slots in packet. 0x0001 -- 0x0201
    uint8_t property_values[e131::kDmxLength + 1]; ///< DMX512-A START Code + data. START Code + Data
} PACKED;

inline constexpr auto kDataLayerSize = sizeof(struct DataDMPLayer);

/**
 *
 */
struct DataPacket
{
    e117::RootLayer root_layer; ///< E1.31 shall use the ACN Root Layer Protocol as defined in the ANSI E1.17 [ACN] “ACN Architecture” document.
    e131::DataFrameLayer frame_layer;
    e131::DataDMPLayer dmp_layer;
} PACKED;

inline constexpr uint32_t DataLayerLength(uint32_t x) noexcept
{
    return kDataLayerSize - 513U + x;
}

inline constexpr uint32_t DataFrameLayerLength(uint32_t x) noexcept
{
    return kDataFrameLayerSize + DataLayerLength(x);
}

inline constexpr uint32_t DataRootLayerLength(uint32_t x) noexcept
{
    return e117::kRootLayerSize - 16U + DataFrameLayerLength(x);
}

inline constexpr uint32_t DataPacketSize(uint32_t x) noexcept
{
    return e117::kRootLayerSize + kDataFrameLayerSize + DataLayerLength(x);
}

/**
 * 6.4 E1.31 Universe Discovery Packet Framing Layer
 */
struct DiscoveryFrameLayer
{
    uint16_t flags_length;                        ///< Protocol flags and length. Low 12 bits = PDU length High 4 bits = 0x7
    uint32_t vector;                              ///< Identifies 1.31 data as Universe Discovery Data. \ref E131_VECTOR_EXTENDED_DISCOVERY
    uint8_t source_name[e131::kSourceNameLength]; ///< User Assigned Name of Source. UTF-8 [UTF-8] encoded string, null-terminated
    uint32_t reserved;                            ///< Reserved
} PACKED;

inline constexpr auto kDiscoveryFrameLayerSize = sizeof(struct DiscoveryFrameLayer);

/**
 * 8 Universe Discovery Layer
 */
struct UniverseDiscoveryLayer
{
    uint16_t flags_length;           ///< Protocol flags and length. Low 12 bits = PDU length High 4 bits = 0x7
    uint32_t vector;                 ///< Identifies Universe Discovery data as universe list. \ref VECTOR_UNIVERSE_DISCOVERY_UNIVERSE_LIST
    uint8_t page;                    ///< Packet Number. Identifier indicating which packet of N this is—pages start numbering at 0.
    uint8_t last_page;               ///< Final Page. Page number of the final page to be transmitted.
    uint16_t list_of_universes[512]; ///< Sorted list of up to 512 16-bit universes. Universes upon which data is being transmitted.
} PACKED;

inline constexpr auto kDiscoveryLayerSize = sizeof(struct UniverseDiscoveryLayer);

/**
 *
 */
struct DiscoveryPacket
{
    e117::RootLayer root_layer; ///< E1.31 shall use the ACN Root Layer Protocol as defined in the ANSI E1.17 [ACN] “ACN Architecture” document.
    DiscoveryFrameLayer frame_layer;
    UniverseDiscoveryLayer universe_discovery_layer;
} PACKED;

inline constexpr uint32_t DiscoveryLayerLength(uint32_t x) noexcept
{
    return kDiscoveryLayerSize - ((512U - x) * 2U);
}

inline constexpr uint32_t DiscoveryFrameLayerLength(uint32_t x) noexcept
{
    return kDiscoveryFrameLayerSize + DiscoveryLayerLength(x);
}

inline constexpr uint32_t DiscoveryRootLayerLength(uint32_t x) noexcept
{
    return e117::kRootLayerSize - 16U + e131::DiscoveryFrameLayerLength(x);
}

inline constexpr uint32_t DiscoveryPacketSize(uint32_t x) noexcept
{
    return e117::kRootLayerSize + e131::kDiscoveryFrameLayerSize + e131::DiscoveryLayerLength(x);
}

/**
 * 6.3 E1.31 Synchronization Packet Framing Layer
 */
struct SynchronizationFrameLayer
{
    uint16_t flags_length;    ///< Protocol flags and length. Low 12 bits = PDU length High 4 bits = 0x7
    uint32_t vector;          ///< Identifies 1.31 data as DMP Protocol PDU. Fixed 0x00000002
    uint8_t sequence_number;  ///< Sequence Number. To detect duplicate or out of order packets
    uint16_t universe_number; ///< Universe on which synchronization packets
    uint16_t reserved;        ///< Reserved. (See Section 6.3.4)
} PACKED;

/**
 * 4.2 E1.31 Synchronization Packet
 */
struct SynchronizationPacket
{
    e117::RootLayer root_layer; ///< E1.31 shall use the ACN Root Layer Protocol as defined in the ANSI E1.17 [ACN] “ACN Architecture” document.
    SynchronizationFrameLayer frame_layer;
} PACKED;

inline constexpr auto kSynchronizationFrameLayerSize = sizeof(struct SynchronizationFrameLayer);
inline constexpr auto kSynchronizationRootLayerSize = (e117::kRootLayerSize - 16U + kSynchronizationFrameLayerSize);
inline constexpr auto kSynchronizationPacketSize = (e117::kRootLayerSize + kSynchronizationFrameLayerSize);

/**
 *
 */
struct RawFrameLayer
{
    uint16_t flags_length; ///< Protocol flags and length. Low 12 bits = PDU length High 4 bits = 0x7
    uint32_t vector;       ///< Identifies 1.31 data as DMP Protocol PDU.
} PACKED;

/**
 *
 */
struct RawPacket
{
    e117::RootLayer root_layer; ///< E1.31 shall use the ACN Root Layer Protocol as defined in the ANSI E1.17 [ACN] “ACN Architecture” document.
    RawFrameLayer frame_layer;
} PACKED;

static_assert(DataLayerLength(513) == kDataLayerSize, "DataLayerLength(513) should equal full size");
static_assert(DiscoveryLayerLength(512) == kDiscoveryLayerSize, "DiscoveryLayerLength(512) mismatch");
} // namespace e131

#endif  // E131_H_
