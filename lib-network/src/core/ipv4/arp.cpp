/**
 * @file arp.cpp
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
/**
 * https://datatracker.ietf.org/doc/html/rfc826
 * An Ethernet Address Resolution Protocol
 *                -- or --
 * Converting Network Protocol Addresses
 */

#if defined(DEBUG_NET_ARP)
#undef NDEBUG
#endif

#if !defined(CONFIG_REMOTECONFIG_MINIMUM)
#pragma GCC push_options
#pragma GCC optimize("O2")
#pragma GCC optimize("no-tree-loop-distribute-patterns")
#pragma GCC optimize("-fprefetch-loop-arrays")
#endif

#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <cassert>

#include "net_memcpy.h"
#include "net_private.h"
#include "net_config.h"
#include "net/netif.h"
#include "net/arp.h"
#include "net/acd.h"
#include "net/protocol/arp.h"
#include "net/protocol/udp.h"
#include "softwaretimers.h"
#include "firmware/debug/debug_debug.h"

#if !defined ARP_MAX_RECORDS
static constexpr auto kMaxRecords = 16;
#else
static constexpr auto kMaxRecords = ARP_MAX_RECORDS;
#endif

namespace net::globals
{
extern uint32_t on_network_mask;
} // namespace net::globals

namespace net::arp
{
static constexpr uint32_t kTimerInterval = 1000;     ///< 1 second
static constexpr uint32_t kMaxProbing = 2;           ///< 2 * 1 second
static constexpr uint32_t kMaxReachable = (10 * 60); ///< (10 * 60) * 1 second = 10 minutes
static constexpr uint32_t kMaxStale = (5 * 60);      ///< ( 5 * 60) * 1 second =  5 minutes

enum class State
{
    kStateEmpty,
    kStateProbe,
    kStateReachable,
    kStateStale,
};

struct Packet
{
    uint8_t* p;
    uint32_t size;
#if defined CONFIG_NET_ENABLE_PTP
    bool isTimestamp;
#endif
};

struct Record
{
    uint32_t ip;
    Packet packet;
    uint8_t mac_address[ETH_ADDR_LEN];
    uint16_t age;
    State state;
};

static net::arp::Record s_arp_records[kMaxRecords] SECTION_NETWORK ALIGNED;
static struct t_arp s_arp_request SECTION_NETWORK ALIGNED;
static struct t_arp s_arp_reply SECTION_NETWORK ALIGNED;

#ifndef NDEBUG
static constexpr char kState[4][12] = {
    "EMPTY",
    "PROBE",
    "REACHABLE",
    "STALE",
};

void static CacheRecordDump(net::arp::Record* record)
{
    printf("%p %-4d %c " MACSTR " %-10s " IPSTR "\n", record, record->age, record->packet.p == nullptr ? '-' : 'Q', MAC2STR(record->mac_address),
           kState[static_cast<unsigned>(record->state)], IP2STR(record->ip));
}

void static CacheDump()
{
    uint32_t index = 0;
    for (auto& record : s_arp_records)
    {
        printf("%p %02d %-4d" MACSTR " %-10s " IPSTR "\n", &record, index++, record.age, MAC2STR(record.mac_address),
               kState[static_cast<unsigned>(record.state)], IP2STR(record.ip));
        if (index == 6)
        {
            return;
        }
    }
}
#else
void static CacheRecordDump([[maybe_unused]] net::arp::Record* record) {}
void static CacheDump() {}
#endif

static net::arp::Record* FindRecord(uint32_t destination_ip, [[maybe_unused]] arp::Flags flag)
{
    DEBUG_ENTRY();

    net::arp::Record* stale = nullptr;
    net::arp::Record* reachable = nullptr;
    uint32_t age_stale = 0;
    uint32_t age_reachable = 0;

    for (auto& record : s_arp_records)
    {
        if (record.ip == destination_ip)
        {
            DEBUG_EXIT();
            return &record;
        }

        if (flag == arp::Flags::FLAG_UPDATE)
        {
            continue;
        }

        if (record.state == net::arp::State::kStateEmpty)
        {
            record.ip = destination_ip;
            DEBUG_EXIT();
            return &record;
        }

        if (record.state == net::arp::State::kStateReachable)
        {
            if (record.age > age_reachable)
            {
                age_reachable = record.age;
                reachable = &record;
            }
            continue;
        }

        if (record.state == net::arp::State::kStateStale)
        {
            if (record.age > age_stale)
            {
                age_stale = record.age;
                stale = &record;
            }
            continue;
        }
    }

    if (stale != nullptr)
    {
        DEBUG_EXIT();
        return stale;
    }

    if (reachable != nullptr)
    {
        DEBUG_EXIT();
        return reachable;
    }

    DEBUG_EXIT();
    return nullptr;
}

static void CacheUpdate(const uint8_t* mac_address, uint32_t ip, arp::Flags flag)
{
    DEBUG_ENTRY();
    DEBUG_PRINTF(MACSTR " " IPSTR " flag=%d", MAC2STR(mac_address), IP2STR(ip), flag);

    auto* record = FindRecord(ip, flag);

    if (record == nullptr)
    {
        assert(flag == arp::Flags::FLAG_UPDATE);
        DEBUG_EXIT();
        return;
    }

    record->state = net::arp::State::kStateReachable;
    record->age = 0;
    std::memcpy(record->mac_address, mac_address, ETH_ADDR_LEN);

    CacheRecordDump(record);

    if (record->packet.p != nullptr)
    {
        auto* udp = reinterpret_cast<struct t_udp*>(record->packet.p);
        std::memcpy(udp->ether.dst, record->mac_address, ETH_ADDR_LEN);
        udp->ip4.chksum = 0;
#if !defined(CHECKSUM_BY_HARDWARE)
        udp->ip4.chksum = Chksum(reinterpret_cast<void*>(&udp->ip4), sizeof(udp->ip4));
#endif
#if defined CONFIG_NET_ENABLE_PTP
        if (!record->packet.isTimestamp)
        {
#endif
            emac_eth_send(record->packet.p, record->packet.size);
#if defined CONFIG_NET_ENABLE_PTP
        }
        else
        {
            emac_eth_send_timestamp(record->packet.p, record->packet.size);
        }
#endif
        delete[] record->packet.p;
        record->packet.p = nullptr;
    }

    DEBUG_EXIT();
}

static void SendRequest(uint32_t ip)
{
    DEBUG_PRINTF(IPSTR, IP2STR(ip));

    net::memcpy_ip(s_arp_request.arp.target_ip, ip);

    emac_eth_send(reinterpret_cast<void*>(&s_arp_request), sizeof(struct t_arp));
}

template <net::arp::EthSend S> static void Query(uint32_t destination_ip, struct t_udp* packet, uint32_t size, [[maybe_unused]] arp::Flags flag)
{
    DEBUG_ENTRY();
    DEBUG_PRINTF(IPSTR " %c", IP2STR(destination_ip), flag == arp::Flags::FLAG_UPDATE ? 'U' : 'I');

    auto* record_found = FindRecord(destination_ip, flag);
    assert(record_found != nullptr);

    CacheRecordDump(record_found);

    if (record_found->state == net::arp::State::kStateEmpty)
    {
        if (record_found->packet.p != nullptr)
        {
            delete[] record_found->packet.p;
        }

        record_found->packet.p = new uint8_t[size];
        assert(record_found->packet.p != nullptr);

        net::memcpy(record_found->packet.p, packet, size);
        record_found->packet.size = size;
#if defined CONFIG_NET_ENABLE_PTP
        record_found->packet.isTimestamp = (S != net::arp::EthSend::kIsNormal);
#endif
        record_found->state = net::arp::State::kStateProbe;
        record_found->age = 0;
        SendRequest(destination_ip);
    }

    DEBUG_EXIT();
}

static void CacheCleanRecord(net::arp::Record& record)
{
    if (record.packet.p != nullptr)
    {
        delete[] record.packet.p;
    }
    std::memset(&record, 0, sizeof(struct net::arp::Record));
}

static void SendRequestUnicast(uint32_t ip, const uint8_t* mac_address)
{
    DEBUG_PRINTF(IPSTR, IP2STR(ip));

    net::memcpy(s_arp_request.ether.dst, mac_address, ETH_ADDR_LEN);
    net::memcpy_ip(s_arp_request.arp.target_ip, ip);

    emac_eth_send(reinterpret_cast<void*>(&s_arp_request), sizeof(struct t_arp));

    memset(s_arp_request.ether.dst, 0xFF, ETH_ADDR_LEN);
}

static void Timer([[maybe_unused]] TimerHandle_t handle)
{
    for (auto& record : s_arp_records)
    {
        const auto kState = record.state;
        if (kState != net::arp::State::kStateEmpty)
        {
            record.age++;

            switch (kState)
            {
                case net::arp::State::kStateProbe:
                    if (record.age > net::arp::kMaxProbing)
                    {
                        CacheCleanRecord(record);
                    }
                    break;

                case net::arp::State::kStateReachable:
                    if (record.age > net::arp::kMaxReachable)
                    {
                        record.state = net::arp::State::kStateStale;
                        record.age = 0;
                    }
                    break;

                case net::arp::State::kStateStale:
                    if (record.age > net::arp::kMaxStale)
                    {
                        record.state = net::arp::State::kStateProbe;
                        SendRequestUnicast(record.ip, record.mac_address);
                    }
                    break;

                default:
                    break;
            }
        }
    }

    CacheDump();
}

static void SendReply(const struct t_arp* p_arp)
{
    DEBUG_ENTRY();

    // Ethernet header
    std::memcpy(s_arp_reply.ether.dst, p_arp->ether.src, ETH_ADDR_LEN);
    // ARP Header
    const auto kIpTarget = net::memcpy_ip(p_arp->arp.target_ip);
    std::memcpy(s_arp_reply.arp.target_mac, p_arp->arp.sender_mac, ETH_ADDR_LEN);
    std::memcpy(s_arp_reply.arp.target_ip, p_arp->arp.sender_ip, IPv4_ADDR_LEN);
    net::memcpy_ip(s_arp_reply.arp.sender_ip, kIpTarget);

    emac_eth_send(reinterpret_cast<void*>(&s_arp_reply), sizeof(struct t_arp));

    DEBUG_EXIT();
}

// Public interface

void __attribute__((cold)) Init()
{
    DEBUG_ENTRY();

    for (auto& record : s_arp_records)
    {
        std::memset(&record, 0, sizeof(struct net::arp::Record));
    }

    // ARP Request template
    // Ethernet header
    std::memcpy(s_arp_request.ether.src, netif::globals::netif_default.hwaddr, ETH_ADDR_LEN);
    std::memset(s_arp_request.ether.dst, 0xFF, ETH_ADDR_LEN);
    s_arp_request.ether.type = __builtin_bswap16(ETHER_TYPE_ARP);

    // ARP Header
    s_arp_request.arp.hardware_type = __builtin_bswap16(ARP_HWTYPE_ETHERNET);
    s_arp_request.arp.protocol_type = __builtin_bswap16(ARP_PRTYPE_IPv4);
    s_arp_request.arp.hardware_size = ARP_HARDWARE_SIZE;
    s_arp_request.arp.protocol_size = ARP_PROTOCOL_SIZE;
    s_arp_request.arp.opcode = __builtin_bswap16(ARP_OPCODE_RQST);

    std::memcpy(s_arp_request.arp.sender_mac, netif::globals::netif_default.hwaddr, ETH_ADDR_LEN);
    net::memcpy_ip(s_arp_request.arp.sender_ip, netif::globals::netif_default.ip.addr);
    std::memset(s_arp_request.arp.target_mac, 0x00, ETH_ADDR_LEN);

    // ARP Reply Template
    // Ethernet header
    std::memcpy(s_arp_reply.ether.src, netif::globals::netif_default.hwaddr, ETH_ADDR_LEN);
    s_arp_reply.ether.type = __builtin_bswap16(ETHER_TYPE_ARP);

    // ARP Header
    s_arp_reply.arp.hardware_type = __builtin_bswap16(ARP_HWTYPE_ETHERNET);
    s_arp_reply.arp.protocol_type = __builtin_bswap16(ARP_PRTYPE_IPv4);
    s_arp_reply.arp.hardware_size = ARP_HARDWARE_SIZE;
    s_arp_reply.arp.protocol_size = ARP_PROTOCOL_SIZE;
    s_arp_reply.arp.opcode = __builtin_bswap16(ARP_OPCODE_REPLY);

    std::memcpy(s_arp_reply.arp.sender_mac, netif::globals::netif_default.hwaddr, ETH_ADDR_LEN);

    SoftwareTimerAdd(net::arp::kTimerInterval, Timer);

    DEBUG_EXIT();
}

__attribute__((hot)) void Input(const struct t_arp* arp)
{
    /*
     * RFC 826 Packet Reception:
     */
    if (__builtin_expect(
            ((arp->arp.hardware_type != __builtin_bswap16(ARP_HWTYPE_ETHERNET)) || (arp->arp.protocol_type != __builtin_bswap16(ARP_PRTYPE_IPv4)) ||
             (arp->arp.hardware_size != ARP_HARDWARE_SIZE) || (arp->arp.protocol_size != ARP_PROTOCOL_SIZE)),
            0))
    {
        DEBUG_EXIT();
        return;
    }

    net::acd::ArpReply(arp);

    // ARP packet directed to us?
    const auto kIpTarget = net::memcpy_ip(arp->arp.target_ip);
    const auto kToUs = ((kIpTarget == netif::globals::netif_default.ip.addr) || (kIpTarget == netif::globals::netif_default.secondary_ip.addr));
    // ARP packet from us?
    const auto kFromUs = (net::memcpy_ip(arp->arp.sender_ip) == netif::globals::netif_default.ip.addr);

    DEBUG_PRINTF("bToUs:%d, bFromUs:%d", kToUs, kFromUs);

    /*
     * ARP message directed to us?
     *  -> add IP address in ARP cache; assume requester wants to talk to us,
     *     can result in directly sending the queued packets for this host.
     * ARP message not directed to us?
     * ->  update the source IP address in the cache, if present
     */
    CacheUpdate(arp->arp.sender_mac, net::memcpy_ip(arp->arp.sender_ip), kToUs ? arp::Flags::FLAG_INSERT : arp::Flags::FLAG_UPDATE);

    switch (arp->arp.opcode)
    {
        case __builtin_bswap16(ARP_OPCODE_RQST):
            if (kToUs && !kFromUs)
            {
                SendReply(arp);
            }
            else
            {
                DEBUG_PUTS("ARP request was not for us");
            }
            break;
        case __builtin_bswap16(ARP_OPCODE_REPLY):
            /* Cache update is handled earlier */
            break;
        default:
            DEBUG_PRINTF("opcode %04x not handled", __builtin_bswap16(arp->arp.opcode));
            break;
    }
}

template <net::arp::EthSend S> static void SendImplementation(struct t_udp* packet, uint32_t size, uint32_t remote_ip)
{
    DEBUG_ENTRY();
    DEBUG_PRINTF(IPSTR, IP2STR(remote_ip));

    const auto& netif = netif::globals::netif_default;

    DEBUG_PRINTF(IPSTR, IP2STR(netif.ip.addr));

    if (__builtin_expect((netif.ip.addr == 0), 0))
    {
        return;
    }

    net::memcpy_ip(packet->ip4.dst, remote_ip);
    packet->ip4.chksum = 0;
#if !defined(CHECKSUM_BY_HARDWARE)
    packet->ip4.chksum = Chksum(reinterpret_cast<void*>(&packet->ip4), sizeof(packet->ip4));
#endif

    auto destination_ip = remote_ip;

    if (__builtin_expect((net::globals::on_network_mask != (remote_ip & net::globals::on_network_mask)), 0))
    {
        /* According to RFC 3297, chapter 2.6.2 (Forwarding Rules), a packet with
           a link-local source address must always be "directly to its destination
           on the same physical link. The host MUST NOT send the packet to any
           router for forwarding". */
        if (!net::is_linklocal_ip(remote_ip))
        {
            destination_ip = netif.gw.addr;
            DEBUG_PUTS("");
        }
    }

    for (auto& record : s_arp_records)
    {
        if (record.state >= net::arp::State::kStateReachable)
        {
            if (record.ip == destination_ip)
            {
                std::memcpy(packet->ether.dst, record.mac_address, ETH_ADDR_LEN);

                if constexpr (S == net::arp::EthSend::kIsNormal)
                {
                    emac_eth_send(reinterpret_cast<void*>(packet), size);
                }
#if defined CONFIG_NET_ENABLE_PTP
                else if constexpr (S == net::arp::EthSend::kIsTimestamp)
                {
                    emac_eth_send_timestamp(reinterpret_cast<void*>(packet), size);
                }
#endif
                DEBUG_EXIT();
                return;
            }
        }
    }

    Query<S>(destination_ip, packet, size, arp::Flags::FLAG_INSERT);

    DEBUG_EXIT();
    return;
}

void Send(struct t_udp* packet, uint32_t size, uint32_t remote_ip)
{
    SendImplementation<net::arp::EthSend::kIsNormal>(packet, size, remote_ip);
}

#if defined CONFIG_NET_ENABLE_PTP
void SendTimestamp(struct t_udp* packet, uint32_t size, uint32_t remote_ip)
{
    SendImplementation<net::arp::EthSend::kIsTimestamp>(packet, size, remote_ip);
}
#endif

/*
 *  The Sender IP is set to all zeros,
 *  which means it cannot map to the Sender MAC address.
 *  The Target MAC address is all zeros,
 *  which means it cannot map to the Target IP address.
 */
void AcdProbe(ip4_addr_t ipaddr)
{
    DEBUG_ENTRY();

    memset(s_arp_request.arp.sender_ip, 0, IPv4_ADDR_LEN);
    net::memcpy_ip(s_arp_request.arp.target_ip, ipaddr.addr);

    emac_eth_send(reinterpret_cast<void*>(&s_arp_request), sizeof(struct t_arp));

    net::memcpy_ip(s_arp_request.arp.sender_ip, netif::globals::netif_default.ip.addr);

    DEBUG_EXIT();
}

/*
 * The packet structure is identical to the ARP Probe above,
 * with the exception that a complete mapping exists.
 * Both the Sender MAC address and the Sender IP address create a complete ARP mapping,
 * and hosts on the network can use this pair of addresses in their ARP table.
 */
void AcdSendAnnouncement(ip4_addr_t ipaddr)
{
    net::memcpy_ip(s_arp_request.arp.target_ip, ipaddr.addr);
    net::memcpy_ip(s_arp_request.arp.sender_ip, ipaddr.addr);

    emac_eth_send(reinterpret_cast<void*>(&s_arp_request), sizeof(struct t_arp));
}
} // namespace net::arp
