/**
 * @file udp.cpp
 *
 */
/* Copyright (C) 2018-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#if defined(DEBUG_NET_UDP)
#undef NDEBUG
#endif

#if !defined(CONFIG_REMOTECONFIG_MINIMUM)
#pragma GCC push_options
#pragma GCC optimize("O2")
#pragma GCC optimize("no-tree-loop-distribute-patterns")
#pragma GCC optimize("-fprefetch-loop-arrays")
#endif

#include <cstdint>
#include <algorithm>
#include <cassert>

#include "net/netif.h"
#include "net_config.h"
#include "net/protocol/ieee.h"
#include "net/protocol/udp.h"
#include "net/arp.h"
#include "net/udp.h"
#include "net_private.h"
#include "net_memcpy.h"
#include "console.h"
#include "firmware/debug/debug_debug.h"

namespace net::globals
{
extern uint32_t broadcast_mask;
}
// namespace globals

namespace net::udp
{
struct PortInfo
{
    udp::UdpCallbackFunctionPtr callback;
    uint16_t port;
};

struct Data
{
    uint32_t from_ip;
    uint32_t size;
    uint8_t data[UDP_DATA_SIZE];
    uint16_t from_port;
};

struct Port
{
    PortInfo info;
    Data data ALIGNED;
} ALIGNED;

static Port s_ports[UDP_MAX_PORTS_ALLOWED] SECTION_NETWORK ALIGNED;
static uint16_t s_id SECTION_NETWORK ALIGNED;
static uint8_t s_multicast_mac[ETH_ADDR_LEN] SECTION_NETWORK ALIGNED;

void __attribute__((cold)) Init()
{
    // Multicast fixed part
    s_multicast_mac[0] = 0x01;
    s_multicast_mac[1] = 0x00;
    s_multicast_mac[2] = 0x5E;
}

void __attribute__((cold)) Shutdown()
{
    DEBUG_ENTRY();

    DEBUG_EXIT();
}

__attribute__((hot)) void Input(const struct t_udp* udp)
{
    const auto kDestinationPort = __builtin_bswap16(udp->udp.destination_port);

    for (uint32_t port_index = 0; port_index < UDP_MAX_PORTS_ALLOWED; port_index++)
    {
        const auto& info = s_ports[port_index].info;

        if (info.port == kDestinationPort)
        {
            auto& data = s_ports[port_index].data;

            if (__builtin_expect((data.size != 0), 0))
            {
                DEBUG_PRINTF("%d[%x]", kDestinationPort, kDestinationPort);
            }

            const auto kDataLength = static_cast<uint32_t>(__builtin_bswap16(udp->udp.len) - UDP_HEADER_SIZE);
            const auto kSize = std::min(static_cast<uint32_t>(UDP_DATA_SIZE), kDataLength);

            net::memcpy(data.data, udp->udp.data, kSize);
            data.from_ip = net::memcpy_ip(udp->ip4.src);
            data.from_port = __builtin_bswap16(udp->udp.source_port);
            data.size = kSize;

            emac_free_pkt();

            if (info.callback != nullptr)
            {
                info.callback(data.data, kDataLength, data.from_ip, data.from_port);
            }

            return;
        }
    }

    emac_free_pkt();

    DEBUG_PRINTF(IPSTR ":%d[%x] " MACSTR, udp->ip4.src[0], udp->ip4.src[1], udp->ip4.src[2], udp->ip4.src[3], kDestinationPort, kDestinationPort,
                 MAC2STR(udp->ether.dst));
}

template <net::arp::EthSend S> static void SendImplementation(int index, const uint8_t* data, uint32_t size, uint32_t remote_ip, uint16_t remote_port)
{
    assert(index >= 0);
    assert(index < UDP_MAX_PORTS_ALLOWED);
    assert(s_ports[index].info.port != 0);

    auto* out_buffer = reinterpret_cast<t_udp*>(emac_eth_send_get_dma_buffer());

    // Ethernet
    std::memcpy(out_buffer->ether.src, netif::globals::netif_default.hwaddr, ETH_ADDR_LEN);
    out_buffer->ether.type = __builtin_bswap16(ETHER_TYPE_IPv4);

    // IPv4
    out_buffer->ip4.ver_ihl = 0x45;
    out_buffer->ip4.tos = 0;
    out_buffer->ip4.flags_froff = __builtin_bswap16(IPv4_FLAG_DF);
    out_buffer->ip4.ttl = 64;
    out_buffer->ip4.proto = IPv4_PROTO_UDP;
    out_buffer->ip4.id = ++s_id;
    out_buffer->ip4.len = __builtin_bswap16(static_cast<uint16_t>(size + IPv4_UDP_HEADERS_SIZE));
    out_buffer->ip4.chksum = 0;
    net::memcpy_ip(out_buffer->ip4.src, netif::globals::netif_default.ip.addr);

    // UDP
    out_buffer->udp.source_port = __builtin_bswap16(s_ports[index].info.port);
    out_buffer->udp.destination_port = __builtin_bswap16(remote_port);
    out_buffer->udp.len = __builtin_bswap16(static_cast<uint16_t>(size + UDP_HEADER_SIZE));
    out_buffer->udp.checksum = 0;

    size = std::min(static_cast<uint32_t>(UDP_DATA_SIZE), size);

    net::memcpy(out_buffer->udp.data, data, size);

    if (remote_ip == net::IPADDR_BROADCAST)
    {
        net::memset<0xFF, ETH_ADDR_LEN>(out_buffer->ether.dst);
        net::memset<0xFF, IPv4_ADDR_LEN>(out_buffer->ip4.dst);
    }
    else if ((remote_ip & net::globals::broadcast_mask) == net::globals::broadcast_mask)
    {
        net::memset<0xFF, ETH_ADDR_LEN>(out_buffer->ether.dst);
        net::memcpy_ip(out_buffer->ip4.dst, remote_ip);
    }
    else
    {
        if ((remote_ip & 0xF0) == 0xE0)
        { // Multicast, we know the MAC Address
            typedef union pcast32
            {
                uint32_t u32;
                uint8_t u8[4];
            } _pcast32;
            _pcast32 multicast_ip;

            multicast_ip.u32 = remote_ip;
            s_multicast_mac[3] = multicast_ip.u8[1] & 0x7F;
            s_multicast_mac[4] = multicast_ip.u8[2];
            s_multicast_mac[5] = multicast_ip.u8[3];

            std::memcpy(out_buffer->ether.dst, s_multicast_mac, ETH_ADDR_LEN);
            net::memcpy_ip(out_buffer->ip4.dst, remote_ip);
        }
        else
        {
            if constexpr (S == net::arp::EthSend::kIsNormal)
            {
                net::arp::Send(out_buffer, size + UDP_PACKET_HEADERS_SIZE, remote_ip);
            }
#if defined CONFIG_NET_ENABLE_PTP
            else if constexpr (S == net::arp::EthSend::kIsTimestamp)
            {
                net::arp::SendTimestamp(out_buffer, size + UDP_PACKET_HEADERS_SIZE, remote_ip);
            }
#endif
            return;
        }
    }

#if !defined(CHECKSUM_BY_HARDWARE)
    out_buffer->ip4.chksum = Chksum(reinterpret_cast<void*>(&out_buffer->ip4), sizeof(out_buffer->ip4));
#endif

    if constexpr (S == net::arp::EthSend::kIsNormal)
    {
        emac_eth_send(size + UDP_PACKET_HEADERS_SIZE);
    }
#if defined CONFIG_NET_ENABLE_PTP
    else if constexpr (S == net::arp::EthSend::kIsTimestamp)
    {
        emac_eth_send_timestamp(size);
    }
#endif
    return;
}

int32_t Begin(uint16_t localport, UdpCallbackFunctionPtr callback)
{
    DEBUG_PRINTF("localport=%u", localport);

    for (auto i = 0; i < UDP_MAX_PORTS_ALLOWED; i++)
    {
        auto& info = s_ports[i].info;

        if (info.port == localport)
        {
            return i;
        }

        if (info.port == 0)
        {
            info.callback = callback;
            info.port = localport;

            DEBUG_PRINTF("i=%d, localport=%d[%x], callback=%p", i, localport, localport, callback);
            return i;
        }
    }

#ifndef NDEBUG
    console::Error("net::udp::Begin");
#endif
    return -1;
}

int32_t End(uint16_t localport)
{
    DEBUG_PRINTF("localport=%u[%x]", localport, localport);

    for (auto i = 0; i < UDP_MAX_PORTS_ALLOWED; i++)
    {
        auto& info = s_ports[i].info;

        if (info.port == localport)
        {
            info.callback = nullptr;
            info.port = 0;

            auto& data = s_ports[i].data;
            data.size = 0;
            return 0;
        }
    }

#ifndef NDEBUG
    console::Error("net::udp::End");
#endif
    return -1;
}

void Send(int32_t index, const uint8_t* data, uint32_t size, uint32_t remote_ip, uint16_t remote_port)
{
    SendImplementation<net::arp::EthSend::kIsNormal>(index, data, size, remote_ip, remote_port);
}

#if defined CONFIG_NET_ENABLE_PTP
void SendWithTimestamp(int32_t index, const uint8_t* data, uint32_t size, uint32_t remote_ip, uint16_t remote_port)
{
    SendImplementation<net::arp::EthSend::kIsTimestamp>(index, data, size, remote_ip, remote_port);
}
#endif

// Do not use - subject for removal 
uint32_t Recv(int32_t index, const uint8_t** data, uint32_t* from_ip, uint16_t* from_port)
{
    assert(index >= 0);
    assert(index < UDP_MAX_PORTS_ALLOWED);

    const auto& info = s_ports[index].info;

    if (__builtin_expect(info.callback != nullptr, 0))
    {
        return 0;
    }

    auto& d = s_ports[index].data;

    if (__builtin_expect((d.size == 0), 1))
    {
        return 0;
    }

    *data = d.data;
    *from_ip = d.from_ip;
    *from_port = d.from_port;

    const auto kSize = d.size;

    d.size = 0;

    return kSize;
}
} // namespace net::udp
// <---
