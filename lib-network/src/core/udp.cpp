/**
 * @file udp.cpp
 *
 */
/* Copyright (C) 2018-2026 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include "core/protocol/ethernet.h"
#if defined(DEBUG_NET_UDP)
#undef NDEBUG
#endif

#if !defined(CONFIG_REMOTECONFIG_MINIMUM)
#pragma GCC push_options
#pragma GCC optimize("O2")
#pragma GCC optimize("no-tree-loop-distribute-patterns")
#endif

#include <cstdint>
#include <algorithm>
#include <cassert>

#include "core/netif.h"
#include "net_config.h"
#include "core/protocol/ieee.h"
#include "core/protocol/udp.h"
#include "core/ip4/arp.h"
#include "network_udp.h"
#include "net_private.h"
#include "net_memcpy.h"
#include "firmware/debug/debug_debug.h"

namespace network::udp
{
struct PortInfo
{
    UdpCallbackFunctionPtr callback;
    uint16_t port;
};

struct Data
{
    uint32_t from_ip;
    uint32_t size;
    uint8_t data[kDataSize];
    uint16_t from_port;
};

struct Port
{
    PortInfo info;
    Data data ALIGNED;
} ALIGNED;

static Port s_ports[UDP_MAX_PORTS_ALLOWED] SECTION_NETWORK ALIGNED;
static uint16_t s_id SECTION_NETWORK ALIGNED;
static uint8_t s_multicast_mac[network::ethernet::kAddressLength] SECTION_NETWORK ALIGNED;

void __attribute__((cold)) Init()
{
    // Multicast fixed part
    s_multicast_mac[0] = network::ethernet::kIP4MulticastAddr0;
    s_multicast_mac[1] = network::ethernet::kIP4MulticastAddr1;
    s_multicast_mac[2] = network::ethernet::kIP4MulticastAddr2;
}

void __attribute__((cold)) Shutdown()
{
    DEBUG_ENTRY();

    DEBUG_EXIT();
}

__attribute__((hot)) void Input(const struct Header* udp)
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

            const auto kDataLength = __builtin_bswap16(udp->udp.len) - kHeaderSize;
            const auto kSize = std::min(kDataSize, kDataLength);

            network::memcpy(data.data, udp->udp.data, kSize);
            data.from_ip = network::memcpy_ip(udp->ip4.src);
            data.from_port = __builtin_bswap16(udp->udp.source_port);
            data.size = kSize;

            emac_free_pkt();

            if (info.callback != nullptr)
            {
                info.callback(data.data, kSize, data.from_ip, data.from_port);
            }

            return;
        }
    }

    emac_free_pkt();

    DEBUG_PRINTF(IPSTR ":%d[%x] " MACSTR, udp->ip4.src[0], udp->ip4.src[1], udp->ip4.src[2], udp->ip4.src[3], kDestinationPort, kDestinationPort, MAC2STR(udp->ether.dst));
}

template <network::arp::EthSend S> static void SendImplementation(int index, const uint8_t* data, uint32_t size, uint32_t remote_ip, uint16_t remote_port)
{
    assert(index >= 0);
    assert(index < UDP_MAX_PORTS_ALLOWED);
    assert(s_ports[index].info.port != 0);

    auto* out_buffer = reinterpret_cast<Header*>(emac_eth_send_get_dma_buffer());

    // Ethernet
    std::memcpy(out_buffer->ether.src, netif::global::netif_default.hwaddr, network::ethernet::kAddressLength);
    out_buffer->ether.type = __builtin_bswap16(network::ethernet::Type::kIPv4);

    // IPv4
    out_buffer->ip4.ver_ihl = 0x45;
    out_buffer->ip4.tos = 0;
    out_buffer->ip4.flags_froff = __builtin_bswap16(network::ip4::Flags::kFlagDf);
    out_buffer->ip4.ttl = 64;
    out_buffer->ip4.proto = network::ip4::Proto::kUdp;
    out_buffer->ip4.id = ++s_id;
    out_buffer->ip4.len = __builtin_bswap16(static_cast<uint16_t>(size + kIPv4UdpHeadersSize));
    out_buffer->ip4.chksum = 0;
    network::memcpy_ip(out_buffer->ip4.src, netif::global::netif_default.ip.addr);

    // UDP
    out_buffer->udp.source_port = __builtin_bswap16(s_ports[index].info.port);
    out_buffer->udp.destination_port = __builtin_bswap16(remote_port);
    out_buffer->udp.len = __builtin_bswap16(static_cast<uint16_t>(size + kHeaderSize));
    out_buffer->udp.checksum = 0;

    size = std::min(kDataSize, size);

    network::memcpy(out_buffer->udp.data, data, size);

    if (remote_ip == network::kIpaddrBroadcast)
    {
        network::memset<0xFF, network::ethernet::kAddressLength>(out_buffer->ether.dst);
        network::memset<0xFF, network::ethernet::kAddressLength>(out_buffer->ip4.dst);
    }
    else if ((remote_ip & network::global::broadcast_mask) == network::global::broadcast_mask)
    {
        network::memset<0xFF, network::ethernet::kAddressLength>(out_buffer->ether.dst);
        network::memcpy_ip(out_buffer->ip4.dst, remote_ip);
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

            std::memcpy(out_buffer->ether.dst, s_multicast_mac, network::ethernet::kAddressLength);
            network::memcpy_ip(out_buffer->ip4.dst, remote_ip);
        }
        else
        {
            if constexpr (S == network::arp::EthSend::kIsNormal)
            {
                network::arp::Send(out_buffer, size + kUdpPacketHeadersSize, remote_ip);
            }
#if defined CONFIG_NET_ENABLE_PTP
            else if constexpr (S == network::arp::EthSend::kIsTimestamp)
            {
                network::arp::SendTimestamp(out_buffer, size + kUdpPacketHeadersSize, remote_ip);
            }
#endif
            return;
        }
    }

#if !defined(CHECKSUM_BY_HARDWARE)
    out_buffer->ip4.chksum = network::Chksum(reinterpret_cast<void*>(&out_buffer->ip4), sizeof(out_buffer->ip4));
#endif

    if constexpr (S == network::arp::EthSend::kIsNormal)
    {
        emac_eth_send(size + kUdpPacketHeadersSize);
    }
#if defined CONFIG_NET_ENABLE_PTP
    else if constexpr (S == network::arp::EthSend::kIsTimestamp)
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
    console::Error("network::udp::Begin");
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
    console::Error("network::udp::End");
#endif
    return -1;
}

void Send(int32_t index, const uint8_t* data, uint32_t size, uint32_t remote_ip, uint16_t remote_port)
{
    SendImplementation<network::arp::EthSend::kIsNormal>(index, data, size, remote_ip, remote_port);
}

#if defined CONFIG_NET_ENABLE_PTP
void SendWithTimestamp(int32_t index, const uint8_t* data, uint32_t size, uint32_t remote_ip, uint16_t remote_port)
{
    SendImplementation<network::arp::EthSend::kIsTimestamp>(index, data, size, remote_ip, remote_port);
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
} // namespace network::udp
// <---
