/**
 * @file network_iface.h
 *
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

#ifndef NETWORK_IFACE_H_
#define NETWORK_IFACE_H_

#include <cstdint>

namespace network::iface
{
inline constexpr uint32_t kMacSize = 6;
inline constexpr uint32_t kHostnameSize = 64;   ///< Including a terminating null byte.
inline constexpr uint32_t kDomainnameSize = 64; ///< Including a terminating null byte.
inline constexpr uint32_t kNameserversCount = 3;

void EthernetInput(const uint8_t*, uint32_t);

void CopyMacAddressTo(uint8_t* mac_address);

void SetHostname(const char* hostname);
void SetHostnameAuto();
const char* HostName();

void SetDomainName(const char* domainname);
const char* DomainName();

[[nodiscard]] inline constexpr const char* InterfaceName()
{
    return "eth0";
}

inline constexpr uint32_t InterfaceIndex()
{
    return 1;
}

uint32_t NameServer(uint32_t index);
uint32_t NameServerCount();

// Zeroconfig / AutoIp
inline bool IsAutoIpCapable()
{
    return true;
}

void SetAutoIp();
bool AutoIp();

// DHCP
inline constexpr bool IsDhcpCapable()
{
    return true;
}

inline constexpr bool IsDhcpKnown()
{
    return true;
}

void EnableDhcp();
bool Dhcp();

inline char AddressingMode()
{
    if (AutoIp()) return 'Z';                     // Zeroconf
    if (IsDhcpKnown()) return Dhcp() ? 'D' : 'S'; // DHCP or Static
    return 'U';                                   // Unknown
}

struct Counters
{
    uint64_t rx_ok = 0, rx_err = 0, rx_drp = 0, rx_ovr = 0;
    uint64_t tx_ok = 0, tx_err = 0, tx_drp = 0, tx_ovr = 0;
};

void GetCounters(Counters& out);
} // namespace network::iface

#endif // NETWORK_IFACE_H_
