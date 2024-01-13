/**
 * json_get_phystatus.cpp
 *
 */
/* Copyright (C) 2023 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include <cstdio>

#include "emac/phy.h"

namespace remoteconfig {
namespace net {
uint32_t json_get_phystatus(char *pOutBuffer, const uint32_t nOutBufferSize) {
	::net::PhyStatus phyStatus;
	::net::phy_customized_status(phyStatus);

	const auto nLength = static_cast<uint32_t>(snprintf(pOutBuffer, nOutBufferSize,
						"{\"link\":\"%s\",\"speed\":\"%s\",\"duplex\":\"%s\",\"autonegotiation\":\"%s\"}",
						::net::phy_string_get_link(phyStatus.link),
						::net::phy_string_get_speed(phyStatus.speed),
						::net::phy_string_get_duplex(phyStatus.duplex),
						::net::phy_string_get_autonegotiation(phyStatus.bAutonegotiation)));
	return nLength;
}
}  // namespace net
}  // namespace remoteconfig
