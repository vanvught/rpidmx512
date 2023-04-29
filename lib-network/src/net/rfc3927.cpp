/**
 * @file rfc3927.cpp
 *
 */
/* Copyright (C) 2020-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <cstdint>
#include <cstring>

#include "net.h"
#include "net_private.h"

#include "hardware.h"

#include "../../config/net_config.h"

/*
 * https://www.rfc-editor.org/rfc/rfc3927.html
 * Dynamic Configuration of IPv4 Link-Local Addresses
 */

static const uint32_t s_ip_begin = 0x0100FEA9;	// 169.254.0.1
static const uint32_t s_ip_end = 0xFFFEFEA9;	// 169.254.254.255

namespace net {
namespace globals {
extern struct IpInfo ipInfo;
extern uint8_t macAddress[ETH_ADDR_LEN];
}  // namespace globals
}  // namespace net

bool rfc3927() {
	DEBUG_ENTRY

	const auto mask = net::globals::macAddress[3] + (net::globals::macAddress[4] << 8);
	auto ip = s_ip_begin | static_cast<uint32_t>(mask << 16);

	DEBUG_PRINTF("ip=" IPSTR, IP2STR(ip));

	auto nCount = 0;
	const auto nMillis = Hardware::Get()->Millis();

	do  {
		DEBUG_PRINTF(IPSTR, IP2STR(ip));

		net::globals::ipInfo.ip.addr = ip;

		if (!arp_do_probe()) {
			net::globals::ipInfo.gw.addr = ip;
			net::globals::ipInfo.netmask.addr = 0x0000FFFF;

			DEBUG_EXIT
			return true;
		}

		ip = __builtin_bswap32(__builtin_bswap32(ip) + 1);

		if (ip == s_ip_end) {
			ip = s_ip_begin;
		}

		nCount++;
	} while ((nCount < 0xFF) && ((Hardware::Get()->Millis() - nMillis) < 500));

	net::globals::ipInfo.ip.addr = 0;
	net::globals::ipInfo.gw.addr = 0;
	net::globals::ipInfo.netmask.addr = 0;

	DEBUG_EXIT
	return false;
}
