/*
 * ip4_address.h
 */

#ifndef IP4_ADDRESS_H_
#define IP4_ADDRESS_H_

#include <cstdint>

#define IP2STR(addr)  static_cast<int>(addr & 0xFF),  static_cast<int>((addr >> 8) & 0xFF),  static_cast<int>((addr >> 16) & 0xFF),  static_cast<int>((addr >> 24) & 0xFF)
#define IPSTR "%d.%d.%d.%d"

#define MAC2STR(mac) static_cast<int>(mac[0]),static_cast<int>(mac[1]),static_cast<int>(mac[2]),static_cast<int>(mac[3]), static_cast<int>(mac[4]), static_cast<int>(mac[5])
#define MACSTR "%.2x:%.2x:%.2x:%.2x:%.2x:%.2x"

namespace network {
static constexpr uint32_t STORE = 96;			///< Configuration store in bytes
static constexpr uint32_t MAC_SIZE = 6;
static constexpr uint32_t HOSTNAME_SIZE = 64;	///< Including a terminating null byte.
static constexpr uint32_t DOMAINNAME_SIZE = 64;	///< Including a terminating null byte.
static constexpr uint32_t NAMESERVERS_COUNT = 3;
static constexpr uint32_t IP4_ANY = 0x00000000;
static constexpr uint32_t IP4_BROADCAST = 0xffffffff;

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

inline bool is_linklocal_ip(const uint32_t nIp) {
	return (nIp & 0xFFFF) == 0xA9FE;
}

inline bool is_multicast_ip(const uint32_t nIp) {
		return (nIp & 0xF0) == 0xE0;
}

inline uint32_t cidr_to_netmask(const uint8_t nCIDR) {
	if (nCIDR != 0) {
		const auto nNetmask = __builtin_bswap32(static_cast<uint32_t>(~0x0) << (32 - nCIDR));
		return nNetmask;
	}

	return 0;
}
}  // namespace network

#endif /* IP4_ADDRESS_H_ */
