/**
 * @file rdmconst.h
 *
 */
/* Copyright (C) 2019-2026 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef RDMCONST_H_
#define RDMCONST_H_

#include <cstdint>

namespace rdm {
namespace transmit {
// 3.1 Controller Timing Requirements
// 3.1.1 Controller Packet Timing
// time in µs
inline constexpr uint32_t kBreakTimeMin = 176;
inline constexpr uint32_t kBreakTimeTypical = 180;
inline constexpr uint32_t kBreakTimeMax = 352;
inline constexpr uint32_t kMabTimeMin = 12;
inline constexpr uint32_t kMabTimeTypical = 16;
inline constexpr uint32_t kMabTimeMax = 88;
inline constexpr uint32_t kDirectionTime = 94;
} // namespace transmit
namespace responder {
///< 3.2.2 Responder Packet spacing
inline constexpr uint32_t kPacketSpacing = 200;    ///< Min 176us, Max 2ms
inline constexpr uint32_t kDataDirectionDelay = 4; ///<
} // namespace responder

inline constexpr uint16_t kRootDevice = 0;
///< 5 Device Addressing
inline constexpr uint32_t kUidSize = 6; ///< 48-bit

// Unique identifier (UID) which consists of a 2 byte ESTA manufacturer ID, and a 4 byte device ID.
inline constexpr uint8_t kUidAll[kUidSize] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

///< 6.2.3 Message Length
inline constexpr uint32_t kMessageMinimumSize = 24; ///< Minimum size of RDM message without the checksum
inline constexpr uint32_t kMessageChecksumSize = 2; ///< Size of the checksum
inline constexpr uint32_t kMessageCountMax = 255;   ///< Message Count field for Responder Generated Messages

struct Manufacturer {
    static const char kName[];
    static const uint8_t kId[];
};
namespace control {
///< 7.6 Discovery Mute/Un-Mute Messages
///< 7.6.1 Control Field
///< The Reserved bits (Bits 4-15) are reserved for future implementation and shall be set to 0.
struct Field {
    static constexpr uint16_t kManagedProxyFlag = (1U << 0); ///< The Managed Proxy Flag (Bit 0) shall be set to 1 when the responder is a Proxy device.
    static constexpr uint16_t kSubDeviceFlag = (1U << 1);    ///< The Sub-Device Flag (Bit 1) shall be set to 1 when the responder supports Sub-Devices.
    static constexpr uint16_t kBootloaderFlag = (1U << 2);   ///< The Boot-Loader Flag (Bit 2) shall only be set to 1 when the device is incapable of normal operation until receiving a firmware upload.
    static constexpr uint16_t kProxiedDeviceFlag =
        (1U << 3); ///< The Proxied Device Flag (Bit 3) shall only be set to 1 when a Proxy is responding to Discovery on behalf of another device. This flag indicates that the response has come from a Proxy, rather than the actual device.
};
} // namespace control

namespace device {
///< 10.5.4 Get Manufacturer Label (MANUFACTURER_LABEL)
inline constexpr uint32_t kManufacturerLabelMaxLength = 32;
inline constexpr uint32_t kManufacturerIdLength = 2; ///<
///< 10.5.5 Get/Set Device Label (DEVICE_LABEL)
inline constexpr uint32_t kLabelMaxLength = 32;
///< 10.5.8 Get/Set Language (LANGUAGE)
inline constexpr uint32_t kSupportedLanguageLength = 2; ///< The Language Codes are 2 character alpha codes as defined by ISO 639-1.
///< 10.5.9 Get Software Version Label (SOFTWARE_VERSION_LABEL)
inline constexpr uint32_t kSoftwareVersionLabelMaxLength = 32;
///< 10.5.10 Get Boot Software Version ID (BOOT_SOFTWARE_VERSION_ID)
inline constexpr uint32_t kBootSoftwareVersionIdLength = 4; ///< The Boot Software Version ID is a 32-bit value determined by the Manufacturer.
///< 10.5.11 Get Boot Software Version Label (BOOT_SOFTWARE_VERSION_LABEL)
inline constexpr uint8_t kBootSoftwareVersionLabelMaxLength = 32;
///< 10.7.1 Get Sensor Definition (SENSOR_DEFINITION)
inline constexpr uint8_t kSensorsAll = 0xFF; ///< The sensor number 0xFF is used to address all sensors.
///< 10.8.1 Get/Set Device Hours (DEVICE_HOURS)
inline constexpr uint32_t kHoursSize = 4;

namespace identify {
struct State {
    ///< 10.11.1 Get/Set Identify Device (IDENTIFY_DEVICE)
    static constexpr uint8_t kOff = 0;
    static constexpr uint8_t kOn = 1;
};
} // namespace identify
} // namespace device
} // namespace rdm

#if !defined(PACKED)
#define PACKED __attribute__((packed))
#endif
#endif // RDMCONST_H_
