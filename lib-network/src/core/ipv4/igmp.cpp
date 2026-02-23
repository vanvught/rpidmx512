/**
 * @file igmp.cpp
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

#if defined(DEBUG_NETWORK_IGMP)
#undef NDEBUG
#endif

#if !defined(CONFIG_REMOTECONFIG_MINIMUM)
#pragma GCC push_options
#pragma GCC optimize("O2")
#pragma GCC optimize("no-tree-loop-distribute-patterns")
#endif

#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <cassert>

#include "net_config.h"
#include "../src/core/net_memcpy.h"
#include "../src/core/net_private.h"
#include "core/netif.h"
#include "core/ip4/igmp.h"
#include "core/protocol/ieee.h"
#include "core/protocol/igmp.h"
#include "softwaretimers.h"
#include "firmware/debug/debug_debug.h"

/*
 * https://www.rfc-editor.org/rfc/rfc2236.html
 * Internet Group Management Protocol, Version 2
 */

namespace network::igmp
{
static constexpr uint32_t kIgmpTmrInterval = 100; /* Milliseconds */
static constexpr uint32_t kIgmpJoinDelayingMemberTmr = (500 / kIgmpTmrInterval);

enum State
{
    kNonMember,
    kDelayingMember,
    kIdleMember
};

struct GroupInfo
{
    uint32_t group_address;
    uint16_t timer; // 1/10 seconds
    State state;
};

typedef union pcast32
{
    uint32_t u32;
    uint8_t u8[4];
} _pcast32;

static struct Header s_report SECTION_NETWORK ALIGNED;
static struct Header s_leave SECTION_NETWORK ALIGNED;
static uint8_t s_multicast_mac[network::ethernet::kAddressLength] SECTION_NETWORK ALIGNED;
static struct GroupInfo s_groups[IGMP_MAX_JOINS_ALLOWED] SECTION_NETWORK ALIGNED;
static uint16_t s_id SECTION_NETWORK ALIGNED;
static TimerHandle_t s_timer_id;

static void SendReport(uint32_t group_address)
{
    DEBUG_ENTRY();
    _pcast32 multicast_ip;

    multicast_ip.u32 = group_address;

    s_multicast_mac[3] = multicast_ip.u8[1] & 0x7F;
    s_multicast_mac[4] = multicast_ip.u8[2];
    s_multicast_mac[5] = multicast_ip.u8[3];

    DEBUG_PRINTF(IPSTR " " MACSTR, IP2STR(group_address), MAC2STR(s_multicast_mac));

    // Ethernet
    std::memcpy(s_report.ether.dst, s_multicast_mac, network::ethernet::kAddressLength);
    // IPv4
    s_report.ip4.id = ++s_id;
    network::memcpy_ip(s_report.ip4.src, netif::global::netif_default.ip.addr);
    std::memcpy(s_report.ip4.dst, multicast_ip.u8, network::ip4::kAddressLength);
    s_report.ip4.chksum = 0;
#if !defined(CHECKSUM_BY_HARDWARE)
    s_report.ip4.chksum = Chksum(reinterpret_cast<void*>(&s_report.ip4), 24); // TODO(avv)
#endif
    // IGMP
    std::memcpy(s_report.igmp.report.igmp.group_address, multicast_ip.u8, network::ip4::kAddressLength);
    s_report.igmp.report.igmp.checksum = 0;
    s_report.igmp.report.igmp.checksum = Chksum(reinterpret_cast<void*>(&s_report.ip4), kIPv4IgmpReportHeadersSize);

    emac_eth_send(reinterpret_cast<void*>(&s_report), kReportPacketSize);

    DEBUG_EXIT();
}

static void StartTimer(struct GroupInfo& group, uint32_t max_time)
{
    group.timer = static_cast<uint16_t>((max_time > 2U ? (static_cast<uint32_t>(random()) % max_time) : 1U));

    if (group.timer == 0)
    {
        group.timer = 1;
    }
}

static void Timeout(struct GroupInfo& group)
{
    if ((group.state == kDelayingMember) && (group.group_address != 0x010000e0))
    { // FIXME all-systems
        group.state = kIdleMember;
        SendReport(group.group_address);
    }
}

static void Timer([[maybe_unused]] TimerHandle_t handle)
{
    for (auto& group : s_groups)
    {
        if (group.timer > 0)
        {
            group.timer--;
            if (group.timer == 0)
            {
                Timeout(group);
            }
        }
    }
}

void __attribute__((cold)) Init()
{
    s_multicast_mac[0] = 0x01;
    s_multicast_mac[1] = 0x00;
    s_multicast_mac[2] = 0x5E;

    // Ethernet
    std::memcpy(s_report.ether.src, netif::global::netif_default.hwaddr, network::ethernet::kAddressLength);
    s_report.ether.type = __builtin_bswap16(network::ethernet::Type::kIPv4);

    // IPv4
    s_report.ip4.ver_ihl = 0x46; // TODO(avv):
    s_report.ip4.tos = 0;
    s_report.ip4.flags_froff = __builtin_bswap16(network::ip4::Flags::kFlagDf);
    s_report.ip4.ttl = 1;
    s_report.ip4.proto = network::ip4::Proto::kIgmp;
    s_report.ip4.len = __builtin_bswap16(kIPv4IgmpReportHeadersSize);
    // IPv4 options
    s_report.igmp.report.ip4_options = 0x00000494; // TODO(avv):

    // IGMP
    s_report.igmp.report.igmp.type = Type::kReport;
    s_report.igmp.report.igmp.max_resp_time = 0;

    // Ethernet
    s_leave.ether.dst[0] = 0x01;
    s_leave.ether.dst[1] = 0x00;
    s_leave.ether.dst[2] = 0x5E;
    s_leave.ether.dst[3] = 0x00;
    s_leave.ether.dst[4] = 0x00;
    s_leave.ether.dst[5] = 0x02;
    std::memcpy(s_leave.ether.src, netif::global::netif_default.hwaddr, network::ethernet::kAddressLength);
    s_leave.ether.type = __builtin_bswap16(network::ethernet::Type::kIPv4);

    // IPv4
    s_leave.ip4.ver_ihl = 0x46; // TODO(avv):
    s_leave.ip4.tos = 0;
    s_leave.ip4.flags_froff = __builtin_bswap16(network::ip4::Flags::kFlagDf);
    s_leave.ip4.ttl = 1;
    s_leave.ip4.proto = network::ip4::Proto::kIgmp;
    s_leave.ip4.len = __builtin_bswap16(kIPv4IgmpReportHeadersSize);
    s_leave.ip4.dst[0] = 0xE0; // 224
    s_leave.ip4.dst[1] = 0x00; // 0
    s_leave.ip4.dst[2] = 0x00; // 0
    s_leave.ip4.dst[3] = 0x02; // 2
    // IPv4 options
    s_leave.igmp.report.ip4_options = 0x00000494; // TODO (avv)

    // IGMP
    s_leave.igmp.report.igmp.type = Type::kLeave;
    s_leave.igmp.report.igmp.max_resp_time = 0;

    s_timer_id = SoftwareTimerAdd(kIgmpTmrInterval, Timer);
    assert(s_timer_id >= 0);

#if defined(CONFIG_EMAC_HASH_MULTICAST_FILTER)
    emac::multicast::EnableHashFilter();
#endif
}

static void Leave(uint32_t);

void __attribute__((cold)) Shutdown()
{
    DEBUG_ENTRY();

    for (auto& group : s_groups)
    {
        if (group.group_address != 0)
        {
            DEBUG_PRINTF(IPSTR, IP2STR(group.group_address));

            Leave(group.group_address);
        }
    }

#if defined(CONFIG_EMAC_HASH_MULTICAST_FILTER)
    emac::multicast::DisableHashFilter();
#endif

    DEBUG_EXIT();
}

static void SendLeave(uint32_t group_address)
{
    DEBUG_ENTRY();
    DEBUG_PRINTF(IPSTR " " MACSTR, IP2STR(group_address), MAC2STR(s_multicast_mac));

    // IPv4
    s_leave.ip4.id = s_id;
    s_leave.ip4.chksum = 0;
#if !defined(CHECKSUM_BY_HARDWARE)
    s_leave.ip4.chksum = Chksum(reinterpret_cast<void*>(&s_leave.ip4), 24); // TODO(avv):
#endif
    network::memcpy_ip(s_leave.ip4.src, netif::global::netif_default.ip.addr);
    // IGMP
    network::memcpy_ip(s_leave.igmp.report.igmp.group_address, group_address);
    s_leave.igmp.report.igmp.checksum = 0;
#if !defined(CHECKSUM_BY_HARDWARE)
    s_leave.igmp.report.igmp.checksum = Chksum(reinterpret_cast<void*>(&s_leave.ip4), kIPv4IgmpReportHeadersSize);
#endif

    emac_eth_send(reinterpret_cast<void*>(&s_leave), kReportPacketSize);

    s_id++;

    DEBUG_EXIT();
}

__attribute__((hot)) void Input(const struct Header* p_igmp)
{
    DEBUG_ENTRY();

    if ((p_igmp->ip4.ver_ihl == 0x45) && (p_igmp->igmp.igmp.type == Type::kQuery))
    {
        DEBUG_PRINTF(IPSTR, p_igmp->ip4.dst[0], p_igmp->ip4.dst[1], p_igmp->ip4.dst[2], p_igmp->ip4.dst[3]);

        auto is_general_request = false;

        _pcast32 igmp_generic_address;
        igmp_generic_address.u32 = 0x010000e0;

        if (memcmp(p_igmp->ip4.dst, igmp_generic_address.u8, 4) == 0)
        {
            is_general_request = true;
        }

        for (auto& group : s_groups)
        {
            if (group.group_address == 0)
            {
                continue;
            }

            _pcast32 group_address;
            group_address.u32 = group.group_address;

            if (is_general_request || (memcmp(p_igmp->ip4.dst, group_address.u8, network::ip4::kAddressLength) == 0))
            {
                if (group.state == kDelayingMember)
                {
                    if (p_igmp->igmp.igmp.max_resp_time < group.timer)
                    {
                        group.timer = (1 + p_igmp->igmp.igmp.max_resp_time / 2);
                    }
                }
                else
                { // s_groups[s_joins_allowed_index].state == IDLE_MEMBER
                    group.state = kDelayingMember;
                    group.timer = (1 + p_igmp->igmp.igmp.max_resp_time / 2);
                }
            }
        }
    }

    DEBUG_EXIT();
}

static void DelayingMember(struct GroupInfo& group, uint32_t maxresp)
{
    if ((group.state == kIdleMember) || ((group.state == kDelayingMember) && ((group.timer == 0) || (maxresp < group.timer))))
    {
        StartTimer(group, maxresp);
        group.state = kDelayingMember;
    }
}

#if defined(CONFIG_EMAC_HASH_MULTICAST_FILTER)
static void ResetHash()
{
    emac::multicast::ResetHash();

    for (auto& group : s_groups)
    {
        if (group.group_address != 0)
        {
            _pcast32 multicast_ip;
            multicast_ip.u32 = group.group_address;
            const uint8_t kMacAddr[6] = {0x01, 0x00, 0x5E, static_cast<uint8_t>(multicast_ip.u8[1] & 0x7F), multicast_ip.u8[2], multicast_ip.u8[3]};

            emac::multicast::SetHash(kMacAddr);
        }
    }
}
#endif

void static Join(uint32_t group_address)
{
    DEBUG_ENTRY();
    DEBUG_PRINTF(IPSTR, IP2STR(group_address));

    if ((group_address & 0xE0) != 0xE0)
    {
        DEBUG_ENTRY();
        return;
    }

    for (int i = 0; i < IGMP_MAX_JOINS_ALLOWED; i++)
    {
        if (s_groups[i].group_address == group_address)
        {
            DEBUG_EXIT();
            return;
        }

        if (s_groups[i].group_address == 0)
        {
            s_groups[i].group_address = group_address;
            s_groups[i].state = kDelayingMember;
            s_groups[i].timer = 2; // TODO(avv):

#if defined(CONFIG_EMAC_HASH_MULTICAST_FILTER)
            _pcast32 multicast_ip;
            multicast_ip.u32 = group_address;
            const uint8_t kMacAddr[6] = {0x01, 0x00, 0x5E, static_cast<uint8_t>(multicast_ip.u8[1] & 0x7F), multicast_ip.u8[2], multicast_ip.u8[3]};
            DEBUG_PRINTF(MACSTR, MAC2STR(kMacAddr));
            emac::multicast::SetHash(kMacAddr);
#endif
            SendReport(group_address);

            DEBUG_EXIT();
            return;
        }
    }

#ifndef NDEBUG
    console::Error("igmp::Join");
#endif
    DEBUG_ENTRY();
}

static void Leave(uint32_t group_address)
{
    DEBUG_ENTRY();
    DEBUG_PRINTF(IPSTR, IP2STR(group_address));

    for (auto& group : s_groups)
    {
        if (group.group_address == group_address)
        {
            SendLeave(group.group_address);

            group.group_address = 0;
            group.state = kNonMember;
            group.timer = 0;

#if defined(CONFIG_EMAC_HASH_MULTICAST_FILTER)
            ResetHash();
#endif
            DEBUG_EXIT();
            return;
        }
    }

#ifndef NDEBUG
    console::Error("igmp::Leave: ");
    printf(IPSTR "\n", IP2STR(group_address));
#endif
    DEBUG_EXIT();
}

// --> Public

void JoinGroup([[maybe_unused]] int32_t handle, uint32_t group_address)
{
    Join(group_address);
}

void LeaveGroup([[maybe_unused]] int32_t handle, uint32_t group_address)
{
    Leave(group_address);
}

bool LookupGroup(uint32_t group_address)
{
    DEBUG_ENTRY();
    DEBUG_PRINTF(IPSTR, IP2STR(group_address));

    for (auto& group : s_groups)
    {
        if (group.group_address == group_address)
        {
            DEBUG_EXIT();
            return true;
        }
    }

    DEBUG_EXIT();
    return (group_address == network::ConvertToUint(224, 0, 0, 1));
}

void ReportGroups()
{
    for (auto& group : s_groups)
    {
        DelayingMember(group, kIgmpJoinDelayingMemberTmr);
    }
}
// <---
} // namespace network::igmp

#if !defined(CONFIG_REMOTECONFIG_MINIMUM)
#pragma GCC pop_options
#endif
