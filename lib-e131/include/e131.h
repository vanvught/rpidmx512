/**
 * @file e131.h
 *
 */
/* Copyright (C) 2016-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "network.h"

namespace e131 {
static constexpr auto MERGE_TIMEOUT_SECONDS = 10;
static constexpr auto PRIORITY_TIMEOUT_SECONDS = 10;
static constexpr auto UNIVERSE_DISCOVERY_INTERVAL_SECONDS = 10;
static constexpr auto NETWORK_DATA_LOSS_TIMEOUT_SECONDS = 2.5f;

struct OptionsMask {
	static constexpr auto PREVIEW_DATA = (1U << 7);			///< Preview Data: Bit 7 (most significant bit)
	static constexpr auto STREAM_TERMINATED = (1U << 6);	///< Stream Terminated: Bit 6
	static constexpr auto FORCE_SYNCHRONIZATION = (1U << 5);///< Force Synchronization: Bit 5
};

namespace universe {
static constexpr auto DEFAULT = 1;
static constexpr auto MAX = 63999;
static constexpr auto DISCOVERY = 64214;
}  // namespace universe
namespace priority {
static constexpr uint8_t LOWEST  = 1;
static constexpr uint8_t DEFAULT = 100;
static constexpr uint8_t HIGHEST = 200;
}  // namespace priority
namespace vector {
namespace root {
static constexpr auto DATA = 0x00000004;
static constexpr auto EXTENDED = 0x00000008;
}  // namespace root
namespace data {
static constexpr auto PACKET = 0x00000002;			///< E1.31 Data Packet
}  // namespace data
namespace extended {
static constexpr auto SYNCHRONIZATION = 0x00000001;	///< E1.31 Synchronization Packet
static constexpr auto DISCOVERY = 0x00000002;		///< E1.31 Universe Discovery
}  // namespace extended
namespace dmp {
static constexpr auto SET_PROPERTY	= 0x02;			///< (Informative)
}  // namespace dmp
namespace universe {
static constexpr auto DISCOVERY_UNIVERSE_LIST = 0x00000001;
}  // namespace universe
}  // namespace vector

static constexpr uint32_t universe_to_multicast_ip(const uint16_t nUniverse) {
	return network::convert_to_uint(239, 255, 0, 0) | (static_cast<uint32_t>(nUniverse & 0xFF) << 24) | (static_cast<uint32_t>((nUniverse & 0xFF00) << 8));
}

static constexpr auto UDP_PORT = 5568;
static constexpr auto DMX_LENGTH = 512;
static constexpr auto CID_LENGTH = 16;
static constexpr auto SOURCE_NAME_LENGTH = 64;
static constexpr auto PACKET_IDENTIFIER_LENGTH = 12;
}  // namespace e131

#endif /* E131_H_ */
