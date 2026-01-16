/**
 * @file arp.cpp
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
/**
 * https://datatracker.ietf.org/doc/html/rfc826
 * An Ethernet Address Resolution Protocol
 *                -- or --
 * Converting Network Protocol Addresses
 */

#if defined(DEBUG_NETWORK_ARP)
#undef NDEBUG
#endif

#if !defined(CONFIG_REMOTECONFIG_MINIMUM)
#pragma GCC push_options
#pragma GCC optimize("O2")
#pragma GCC optimize("no-tree-loop-distribute-patterns")
#endif

#include <cstdint>
#include <cstring>
#include <cassert>

#include "../src/core/net_memcpy.h"
#include "../src/core/net_private.h"
#include "net_config.h"
#include "core/netif.h"
#include "core/ip4/arp.h"
#include "core/ip4/acd.h"
#include "core/protocol/ethernet.h"
#include "core/protocol/arp.h"
#include "softwaretimers.h"
#include "../src/core/network_memory.h"
#include "firmware/debug/debug_debug.h"
#include "firmware/debug/debug_dump.h"

#if !defined ARP_MAX_RECORDS
static constexpr auto kMaxRecords = 16;
#else
static constexpr auto kMaxRecords = ARP_MAX_RECORDS;
#endif

namespace network::globals
{
extern uint32_t on_network_mask;
} // namespace network::globals

namespace network::arp
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
    uint8_t mac_address[network::ethernet::kAddressLength];
    uint16_t age;
    State state;
};

static network::arp::Record s_arp_records[kMaxRecords] SECTION_NETWORK ALIGNED;
static struct network::arp::Header s_arp_request SECTION_NETWORK ALIGNED;
static struct network::arp::Header s_arp_reply SECTION_NETWORK ALIGNED;

#ifndef NDEBUG
static constexpr char kStates[4][12] = {
    "EMPTY",
    "PROBE",
    "REACHABLE",
    "STALE",
};

void static CacheRecordDump(network::arp::Record* record)
{
    printf("%p %-4d %c " MACSTR " %-10s " IPSTR "\n", record, record->age, record->packet.p == nullptr ? '-' : 'Q', MAC2STR(record->mac_address), kStates[static_cast<unsigned>(record->state)], IP2STR(record->ip));
}

void static CacheDump()
{
    uint32_t index = 0;
    for (auto& record : s_arp_records)
    {
        printf("%p %02d %-4d" MACSTR " %-10s " IPSTR "\n", &record, index++, record.age, MAC2STR(record.mac_address), kStates[static_cast<unsigned>(record.state)], IP2STR(record.ip));
        if (index == 6)
        {
            return;
        }
    }
}
#else
void static CacheRecordDump([[maybe_unused]] network::arp::Record* record) {}
void static CacheDump() {}
#endif

static network::arp::Record* FindRecord(uint32_t destination_ip, [[maybe_unused]] arp::Flags flag)
{
    DEBUG_ENTRY();

    network::arp::Record* stale = nullptr;
    network::arp::Record* reachable = nullptr;
    uint32_t age_stale = 0;
    uint32_t age_reachable = 0;

    for (auto& record : s_arp_records)
    {
        if (record.ip == destination_ip)
        {
            DEBUG_EXIT();
            return &record;
        }

        if (flag == arp::Flags::kFlagUpdate)
        {
            continue;
        }

        if (record.state == network::arp::State::kStateEmpty)
        {
            record.ip = destination_ip;
            DEBUG_EXIT();
            return &record;
        }

        if (record.state == network::arp::State::kStateReachable)
        {
            if (record.age > age_reachable)
            {
                age_reachable = record.age;
                reachable = &record;
            }
            continue;
        }

        if (record.state == network::arp::State::kStateStale)
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
        assert(flag == arp::Flags::kFlagUpdate);
        DEBUG_EXIT();
        return;
    }

    record->state = network::arp::State::kStateReachable;
    record->age = 0;
    std::memcpy(record->mac_address, mac_address, network::ethernet::kAddressLength);

    CacheRecordDump(record);

    if (record->packet.p != nullptr)
    {
        auto* udp = reinterpret_cast<struct network::udp::Header*>(record->packet.p);
        std::memcpy(udp->ether.dst, record->mac_address, network::ethernet::kAddressLength);
        udp->ip4.chksum = 0;
#if !defined(CHECKSUM_BY_HARDWARE)
        udp->ip4.chksum = Chksum(reinterpret_cast<void*>(&udp->ip4), sizeof(udp->ip4));
#endif
#if defined CONFIG_NET_ENABLE_PTP
        if (!record->packet.isTimestamp)
        {
#endif
            debug::Dump(record->packet.p, record->packet.size);
            emac_eth_send(record->packet.p, record->packet.size);
#if defined CONFIG_NET_ENABLE_PTP
        }
        else
        {
            emac_eth_send_timestamp(record->packet.p, record->packet.size);
        }
#endif
        network::memory::Allocator::Instance().Free(record->packet.p);
        record->packet.p = nullptr;
    }

    DEBUG_EXIT();
}

static void SendRequest(uint32_t ip)
{
    DEBUG_PRINTF(IPSTR, IP2STR(ip));

    network::memcpy_ip(s_arp_request.arp.target_ip, ip);

    emac_eth_send(reinterpret_cast<void*>(&s_arp_request), sizeof(struct network::arp::Header));
}

template <network::arp::EthSend S> static void Query(uint32_t destination_ip, void* packet, uint32_t size, [[maybe_unused]] arp::Flags flag)
{
    DEBUG_ENTRY();
    DEBUG_PRINTF(IPSTR " %c", IP2STR(destination_ip), flag == arp::Flags::kFlagUpdate ? 'U' : 'I');

    auto* record_found = FindRecord(destination_ip, flag);
    assert(record_found != nullptr);

    CacheRecordDump(record_found);

    if (record_found->state == network::arp::State::kStateEmpty)
    {
        if (record_found->packet.p != nullptr)
        {
            network::memory::Allocator::Instance().Free(record_found->packet.p);
        }

        printf("size=%u\n", size);
        assert(size <= network::memory::kBlockSize);
        record_found->packet.p = network::memory::Allocator::Instance().Allocate();
        assert(record_found->packet.p != nullptr);

        network::memcpy(record_found->packet.p, packet, size);
        record_found->packet.size = size;
#if defined CONFIG_NET_ENABLE_PTP
        record_found->packet.isTimestamp = (S != network::arp::EthSend::kIsNormal);
#endif
        record_found->state = network::arp::State::kStateProbe;
        record_found->age = 0;
        SendRequest(destination_ip);
    }

    DEBUG_EXIT();
}

static void CacheCleanRecord(network::arp::Record& record)
{
    if (record.packet.p != nullptr)
    {
        delete[] record.packet.p;
    }
    std::memset(&record, 0, sizeof(struct network::arp::Record));
}

static void SendRequestUnicast(uint32_t ip, const uint8_t* mac_address)
{
    DEBUG_PRINTF(IPSTR, IP2STR(ip));

    network::memcpy(s_arp_request.ether.dst, mac_address, network::ethernet::kAddressLength);
    network::memcpy_ip(s_arp_request.arp.target_ip, ip);

    emac_eth_send(reinterpret_cast<void*>(&s_arp_request), sizeof(struct network::arp::Header));

    network::memset<0xFF, network::ethernet::kAddressLength>(s_arp_request.ether.dst);
}

static void Timer([[maybe_unused]] TimerHandle_t handle)
{
    for (auto& record : s_arp_records)
    {
        const auto kState = record.state;
        if (kState != network::arp::State::kStateEmpty)
        {
            record.age++;

            switch (kState)
            {
                case network::arp::State::kStateProbe:
                    if (record.age > network::arp::kMaxProbing)
                    {
                        CacheCleanRecord(record);
                    }
                    break;

                case network::arp::State::kStateReachable:
                    if (record.age > network::arp::kMaxReachable)
                    {
                        record.state = network::arp::State::kStateStale;
                        record.age = 0;
                    }
                    break;

                case network::arp::State::kStateStale:
                    if (record.age > network::arp::kMaxStale)
                    {
                        record.state = network::arp::State::kStateProbe;
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

static void SendReply(const struct network::arp::Header* p_arp)
{
    DEBUG_ENTRY();

    // Ethernet header
    std::memcpy(s_arp_reply.ether.dst, p_arp->ether.src, network::ethernet::kAddressLength);
    // ARP Header
    const auto kIpTarget = network::memcpy_ip(p_arp->arp.target_ip);
    std::memcpy(s_arp_reply.arp.target_mac, p_arp->arp.sender_mac, network::ethernet::kAddressLength);
    std::memcpy(s_arp_reply.arp.target_ip, p_arp->arp.sender_ip, network::ip4::kAddressLength);
    network::memcpy_ip(s_arp_reply.arp.sender_ip, kIpTarget);

    emac_eth_send(reinterpret_cast<void*>(&s_arp_reply), sizeof(struct network::arp::Header));

    DEBUG_EXIT();
}

// Public interface

void __attribute__((cold)) Init()
{
    DEBUG_ENTRY();

    for (auto& record : s_arp_records)
    {
        std::memset(&record, 0, sizeof(struct network::arp::Record));
    }

    // ARP Request template
    // Ethernet header
    std::memcpy(s_arp_request.ether.src, netif::global::netif_default.hwaddr, network::ethernet::kAddressLength);
    std::memset(s_arp_request.ether.dst, 0xFF, network::ethernet::kAddressLength);
    s_arp_request.ether.type = __builtin_bswap16(network::ethernet::Type::kArp);

    // ARP Header
    s_arp_request.arp.hardware_type = __builtin_bswap16(kHwtypeEthernet);
    s_arp_request.arp.protocol_type = __builtin_bswap16(kPrtypeIPv4);
    s_arp_request.arp.hardware_size = kHardwareSize;
    s_arp_request.arp.protocol_size = kProtocolSize;
    s_arp_request.arp.opcode = __builtin_bswap16(network::arp::OpCode::kRqstRqst);

    std::memcpy(s_arp_request.arp.sender_mac, netif::global::netif_default.hwaddr, network::ethernet::kAddressLength);
    network::memcpy_ip(s_arp_request.arp.sender_ip, netif::global::netif_default.ip.addr);
    std::memset(s_arp_request.arp.target_mac, 0x00, network::ethernet::kAddressLength);

    // ARP Reply Template
    // Ethernet header
    std::memcpy(s_arp_reply.ether.src, netif::global::netif_default.hwaddr, network::ethernet::kAddressLength);
    s_arp_reply.ether.type = __builtin_bswap16(network::ethernet::Type::kArp);

    // ARP Header
    s_arp_reply.arp.hardware_type = __builtin_bswap16(kHwtypeEthernet);
    s_arp_reply.arp.protocol_type = __builtin_bswap16(kPrtypeIPv4);
    s_arp_reply.arp.hardware_size = kHardwareSize;
    s_arp_reply.arp.protocol_size = kProtocolSize;
    s_arp_reply.arp.opcode = __builtin_bswap16(network::arp::OpCode::kRqstReply);

    std::memcpy(s_arp_reply.arp.sender_mac, netif::global::netif_default.hwaddr, network::ethernet::kAddressLength);

    SoftwareTimerAdd(network::arp::kTimerInterval, Timer);

    DEBUG_EXIT();
}

__attribute__((hot)) void Input(const struct network::arp::Header* arp)
{
    /*
     * RFC 826 Packet Reception:
     */
    if (__builtin_expect(
            ((arp->arp.hardware_type != __builtin_bswap16(kHwtypeEthernet)) || (arp->arp.protocol_type != __builtin_bswap16(kPrtypeIPv4)) || (arp->arp.hardware_size != kHardwareSize) || (arp->arp.protocol_size != kProtocolSize)), 0))
    {
        DEBUG_EXIT();
        return;
    }

    network::acd::ArpReply(arp);

    // ARP packet directed to us?
    const auto kIpTarget = network::memcpy_ip(arp->arp.target_ip);
    const auto kToUs = ((kIpTarget == netif::global::netif_default.ip.addr) || (kIpTarget == netif::global::netif_default.secondary_ip.addr));
    // ARP packet from us?
    const auto kFromUs = (network::memcpy_ip(arp->arp.sender_ip) == netif::global::netif_default.ip.addr);

    DEBUG_PRINTF("bToUs:%d, bFromUs:%d", kToUs, kFromUs);

    /*
     * ARP message directed to us?
     *  -> add IP address in ARP cache; assume requester wants to talk to us,
     *     can result in directly sending the queued packets for this host.
     * ARP message not directed to us?
     * ->  update the source IP address in the cache, if present
     */
    CacheUpdate(arp->arp.sender_mac, network::memcpy_ip(arp->arp.sender_ip), kToUs ? arp::Flags::kFlagInsert : arp::Flags::kFlagUpdate);

    switch (arp->arp.opcode)
    {
        case __builtin_bswap16(network::arp::OpCode::kRqstRqst):
            if (kToUs && !kFromUs)
            {
                SendReply(arp);
            }
            else
            {
                DEBUG_PUTS("ARP request was not for us");
            }
            break;
        case __builtin_bswap16(network::arp::OpCode::kRqstReply):
            /* Cache update is handled earlier */
            break;
        default:
            DEBUG_PRINTF("opcode %04x not handled", __builtin_bswap16(arp->arp.opcode));
            break;
    }
}

template <network::arp::EthSend S> static void SendImplementation(void* packet, uint32_t size, uint32_t remote_ip)
{
    DEBUG_ENTRY();
    DEBUG_PRINTF(IPSTR, IP2STR(remote_ip));

    const auto& netif = netif::global::netif_default;

    DEBUG_PRINTF(IPSTR, IP2STR(netif.ip.addr));

    if (__builtin_expect((netif.ip.addr == 0), 0))
    {
        return;
    }

    auto* p = reinterpret_cast<struct network::ip4::Header*>(packet);

    network::memcpy_ip(p->ip4.dst, remote_ip);
    p->ip4.chksum = 0;
#if !defined(CHECKSUM_BY_HARDWARE)
    p->ip4.chksum = Chksum(reinterpret_cast<void*>(&p->ip4), sizeof(p->ip4));
#endif

    auto destination_ip = remote_ip;

    if (__builtin_expect((network::global::on_network_mask != (remote_ip & network::global::on_network_mask)), 0))
    {
        /* According to RFC 3297, chapter 2.6.2 (Forwarding Rules), a packet with
           a link-local source address must always be "directly to its destination
           on the same physical link. The host MUST NOT send the packet to any
           router for forwarding". */
        if (!network::IsLinklocalIp(remote_ip))
        {
            destination_ip = netif.gw.addr;
            DEBUG_PUTS("");
        }
    }

    for (auto& record : s_arp_records)
    {
        if (record.state >= network::arp::State::kStateReachable)
        {
            if (record.ip == destination_ip)
            {
                std::memcpy(p->ether.dst, record.mac_address, network::ethernet::kAddressLength);

                if constexpr (S == network::arp::EthSend::kIsNormal)
                {
                    emac_eth_send(packet, size);
                }
#if defined CONFIG_NET_ENABLE_PTP
                else if constexpr (S == network::arp::EthSend::kIsTimestamp)
                {
                    emac_eth_send_timestamp(packet, size);
                }
#endif
                DEBUG_EXIT();
                return;
            }
        }
    }

    Query<S>(destination_ip, packet, size, arp::Flags::kFlagInsert);

    DEBUG_EXIT();
    return;
}

void Send(void* packet, uint32_t size, uint32_t remote_ip)
{
    SendImplementation<network::arp::EthSend::kIsNormal>(packet, size, remote_ip);
}

#if defined CONFIG_NET_ENABLE_PTP
void SendTimestamp(void* packet, uint32_t size, uint32_t remote_ip)
{
    SendImplementation<network::arp::EthSend::kIsTimestamp>(packet, size, remote_ip);
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

    network::memset<0, network::ip4::kAddressLength>(s_arp_request.arp.sender_ip);
    network::memcpy_ip(s_arp_request.arp.target_ip, ipaddr.addr);

    emac_eth_send(reinterpret_cast<void*>(&s_arp_request), sizeof(struct network::arp::Header));

    network::memcpy_ip(s_arp_request.arp.sender_ip, netif::global::netif_default.ip.addr);

    DEBUG_EXIT();
}

// The packet structure is identical to the ARP Probe above,
// with the exception that a complete mapping exists.
// Both the Sender MAC address and the Sender IP address create a complete ARP mapping,
// and hosts on the network can use this pair of addresses in their ARP table.
void AcdSendAnnouncement(ip4_addr_t ipaddr)
{
    network::memcpy_ip(s_arp_request.arp.target_ip, ipaddr.addr);
    network::memcpy_ip(s_arp_request.arp.sender_ip, ipaddr.addr);

    emac_eth_send(reinterpret_cast<void*>(&s_arp_request), sizeof(struct network::arp::Header));
}
} // namespace network::arp
