/**
 * @file ethernet.cpp
 */
/* Copyright (C) 2025-2026 by Arjan van Vught mailto:info@gd32-dmx.org
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

#if defined(DEBUG_NETWORK_IFACE)
#undef NDEBUG
#endif

#if !defined(CONFIG_REMOTECONFIG_MINIMUM)
#pragma GCC push_options
#pragma GCC optimize("O3")
#pragma GCC optimize("no-tree-loop-distribute-patterns")
#endif

#include <cstdint>
#include <cstdint>

#include "../src/core/net_private.h"
#include "../src/core/net_memcpy.h"
#include "core/ip4/arp.h"
#include "core/protocol/ieee.h"
#include "core/protocol/ethernet.h"
#include "firmware/debug/debug_debug.h"

namespace network
{
namespace igmp
{
bool LookupGroup(uint32_t);
}
namespace ptp
{
#if defined(CONFIG_NET_ENABLE_PTP)
// Can only be used for PTP level 2 messages
__attribute__((weak)) void Input([[maybe_unused]] const uint8_t*, [[maybe_unused]] const uint32_t) {}
#endif
} // namespace ptp

namespace iface
{
void EthernetInput(const uint8_t* buffer, [[maybe_unused]] uint32_t length)
{
    const auto* const kEther = reinterpret_cast<const struct network::ethernet::Header*>(buffer);

    switch (kEther->type)
    {
#if defined(CONFIG_NET_ENABLE_PTP)
        case __builtin_bswap16(network::ethernet::Type::kPtp):
            network::ptp::Input(const_cast<const uint8_t*>(buffer), length);
            break;
#endif
        case __builtin_bswap16(network::ethernet::Type::kIPv4):
        {
            const auto* const kIp4 = reinterpret_cast<const struct network::ip4::Header*>(buffer);

            DEBUG_PRINTF(IPSTR " " IPSTR, kIp4->ip4.dst[0], kIp4->ip4.dst[1], kIp4->ip4.dst[2], kIp4->ip4.dst[3], kIp4->ip4.src[0], kIp4->ip4.src[1], kIp4->ip4.src[2], kIp4->ip4.src[3]);

            if ((kEther->dst[0] == network::ethernet::kIP4MulticastAddr0) && (kEther->dst[1] == network::ethernet::kIP4MulticastAddr1) && (kEther->dst[2] == network::ethernet::kIP4MulticastAddr2))
            {
                if (!network::igmp::LookupGroup(network::memcpy_ip(kIp4->ip4.dst)))
                {
                    emac_free_pkt();
                    DEBUG_PUTS("IGMP not for us");
                    return;
                }
            }

            switch (kIp4->ip4.proto)
            {
                case ip4::Proto::kUdp:
                    network::udp::Input(reinterpret_cast<const struct network::udp::Header*>(kIp4));
                    // NOTE: emac_free_pkt(); is done in net::udp::Input
                    return;
                    break;
                case ip4::Proto::kIgmp:
                    network::igmp::Input(reinterpret_cast<const struct network::igmp::Header*>(kIp4));
                    break;
                case ip4::Proto::kIcmp:
                    network::icmp::Input(const_cast<struct network::icmp::Header*>(reinterpret_cast<const struct network::icmp::Header*>(kIp4)));
                    break;
#if defined(ENABLE_HTTPD)
                case ip4::Proto::kTcp:
                    network::tcp::Input(const_cast<struct network::tcp::Header*>(reinterpret_cast<const struct network::tcp::Header*>(kIp4)));
                    break;
#endif
                default:
                    break;
            }
        }
        break;
        case __builtin_bswap16(network::ethernet::Type::kArp):
            network::arp::Input(reinterpret_cast<const struct network::arp::Header*>(buffer));
            break;
        default:
            DEBUG_PRINTF("type %04x is not implemented", __builtin_bswap16(kEther->type));
            break;
    }

    emac_free_pkt();
}
} // namespace iface
} // namespace network

#if !defined(CONFIG_REMOTECONFIG_MINIMUM)
#pragma GCC pop_options
#endif