/**
 * @file network.h
 *
 */
/* Copyright (C) 2017-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef NETWORK_H_
#define NETWORK_H_

#include <cstdint>

#define IP2STR(addr) (addr & 0xFF), ((addr >> 8) & 0xFF), ((addr >> 16) & 0xFF), ((addr >> 24) & 0xFF)
#define IPSTR "%d.%d.%d.%d"

#define MAC2STR(mac) static_cast<int>(mac[0]),static_cast<int>(mac[1]),static_cast<int>(mac[2]),static_cast<int>(mac[3]), static_cast<int>(mac[4]), static_cast<int>(mac[5])
#define MACSTR "%.2x:%.2x:%.2x:%.2x:%.2x:%.2x"

namespace network {
static constexpr auto STORE = 96;				///< Configuration store in bytes
static constexpr auto IP_SIZE = 4U;
static constexpr auto MAC_SIZE = 6U;
static constexpr auto HOSTNAME_SIZE = 64U;		///< Including a terminating null byte.
static constexpr auto DOMAINNAME_SIZE = 64U;	///< Including a terminating null byte.
static constexpr auto IP4_BROADCAST = 0xffffffff;
namespace dhcp {
enum class Mode: uint8_t {
	INACTIVE = 0x00,	///< The IP address was not obtained via DHCP
	ACTIVE = 0x01,		///< The IP address was obtained via DHCP
	UNKNOWN = 0x02		///< The system cannot determine if the address was obtained via DHCP
};
enum class ClientStatus {
	IDLE, RENEW, GOT_IP, RETRYING, FAILED
};
}  // namespace dhcp

static constexpr uint32_t convert_to_uint(const uint8_t a, const uint8_t b, const uint8_t c, const uint8_t d) {
	return static_cast<uint32_t>(a)       |
		   static_cast<uint32_t>(b) << 8  |
		   static_cast<uint32_t>(c) << 16 |
		   static_cast<uint32_t>(d) << 24;
}

inline bool is_netmask_valid(uint32_t nNetMask) {
	if (nNetMask == 0) {
		return false;
	}
	nNetMask = __builtin_bswap32(nNetMask);
	return !(nNetMask & (~nNetMask >> 1));

}
/**
 * The private address ranges are defined in RFC1918.
 */
inline bool is_private_ip(const uint32_t nIp) {
	const uint8_t n = (nIp >> 8) & 0xFF;

	switch (nIp & 0xFF) {
	case 10:
		return true;
		break;
	case 172:
		return (n >= 16) && (n < 32);
	case 192:
		return n == 168;
	default:
		break;
	}

	return false;
}

inline bool is_multicast_ip(const uint32_t nIp) {
	if ((nIp & 0xE0) != 0xE0) {
		return false;
	}

	if ((nIp & 0xFFFFEF) == 0x0000E0) { // 224.0.0.0 to 224.0.0.255 Local subnetwork
		return false;
	}

	if ((nIp & 0xFFFFEF) == 0x0100E0) { // 224.0.1.0 to 224.0.1.255 Internetwork control
		return false;
	}

	return true;
}

inline uint32_t cidr_to_netmask(const uint8_t nCIDR) {
	if (nCIDR != 0) {
		const auto nNetmask = __builtin_bswap32(static_cast<uint32_t>(~0x0) << (32 - nCIDR));
		return nNetmask;
	}

	return 0;
}

void display_emac_config();
void display_emac_start();
void display_emac_status(const bool isLinkUp);
void display_ip();
void display_netmask();
void display_gateway();
void display_hostname();
void display_dhcp_status(network::dhcp::ClientStatus nStatus);
void display_emac_shutdown();

void mdns_announcement();
void mdns_shutdown();
}  // namespace network

class NetworkStore {
public:
	virtual ~NetworkStore() {}

	virtual void SaveIp(uint32_t nIp)=0;
	virtual void SaveNetMask(uint32_t nNetMask)=0;
	virtual void SaveGatewayIp(uint32_t nGatewayIp)=0;
	virtual void SaveHostName(const char *pHostName, uint32_t nLength)=0;
	virtual void SaveDhcp(bool bIsDhcpUsed)=0;
};

#if defined (BARE_METAL)
# if defined (ESP8266)
#  include "esp8266/network.h"
# elif defined (NO_EMAC)
#  include "noemac/network.h"
# else
#  include "emac/network.h"
# endif
#else
# include "linux/network.h"
#endif

#endif /* NETWORK_H_ */
