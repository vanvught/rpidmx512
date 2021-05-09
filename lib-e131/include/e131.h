/**
 * @file e131.h
 *
 */
/* Copyright (C) 2016-2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

namespace e131 {
static constexpr auto MERGE_TIMEOUT_SECONDS = 10;
static constexpr auto PRIORITY_TIMEOUT_SECONDS = 10;
static constexpr auto UNIVERSE_DISCOVERY_INTERVAL_SECONDS = 10;
static constexpr auto NETWORK_DATA_LOSS_TIMEOUT_SECONDS = 2.5f;

enum class Merge {
	HTP, LTP
};

enum class PortDir {
	INPUT, OUTPUT, DISABLE
};

struct OptionsMask {
	static constexpr auto PREVIEW_DATA = (1 << 7);			///< Preview Data: Bit 7 (most significant bit)
	static constexpr auto STREAM_TERMINATED = (1 << 6);		///< Stream Terminated: Bit 6
	static constexpr auto FORCE_SYNCHRONIZATION = (1 << 5);	///< Force Synchronization: Bit 5
};

namespace universe {
static constexpr auto DEFAULT = 1;
static constexpr auto MAX = 63999;
static constexpr auto DISCOVERY = 64214;
}  // namespace universe
namespace priority {
static constexpr auto LOWEST = 1;
static constexpr auto DEFAULT = 100;
static constexpr auto HIGHEST = 200;
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
}  // namespace e131

struct E131 {
	static constexpr auto UDP_PORT = 5568;
	static constexpr auto MAX_PORTS = 32;
	static constexpr auto MAX_UARTS = 4;
	static constexpr auto DMX_LENGTH = 512;
	static constexpr auto CID_LENGTH = 16;
	static constexpr auto SOURCE_NAME_LENGTH = 64;
	static constexpr auto PACKET_IDENTIFIER_LENGTH = 12;

	static e131::Merge GetMergeMode(const char *pMergeMode) {
		if (pMergeMode != nullptr) {
			if (((pMergeMode[0] | 0x20) == 'l')
					&& ((pMergeMode[1] | 0x20) == 't')
					&& ((pMergeMode[2] | 0x20) == 'p')) {
				return e131::Merge::LTP;
			}
		}
		return e131::Merge::HTP;
	}

	static const char* GetMergeMode(e131::Merge m, bool bToUpper = false) {
		if (bToUpper) {
			return (m == e131::Merge::HTP) ? "HTP" : "LTP";
		}
		return (m == e131::Merge::HTP) ? "htp" : "ltp";
	}

	static const char* GetMergeMode(uint8_t m, bool bToUpper = false) {
		return GetMergeMode(static_cast<e131::Merge>(m), bToUpper);
	}
};

#endif /* E131_H_ */
