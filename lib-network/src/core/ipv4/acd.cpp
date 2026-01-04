/**
 * @file acd.cpp
 */
/* Copyright (C) 2024-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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
/* This code is inspired by the lwIP TCP/IP stack.
 * https://savannah.nongnu.org/projects/lwip/
 */
/**
 * The acp.cpp aims to be conform to RFC 5227.
 * https://datatracker.ietf.org/doc/html/rfc5227.html
 * IPv4 Address Conflict Detection
 */

#if defined(DEBUG_NET_ACD)
#undef NDEBUG
#endif

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cassert>

#include "net/netif.h"
#include "net_memcpy.h"
#include "net_config.h"
#include "net/arp.h"
#include "net/acd.h"
#include "net/protocol/acd.h"
#include "net/protocol/arp.h"

#include "softwaretimers.h"
#include "firmware/debug/debug_debug.h"

namespace net::acd
{
static constexpr uint32_t kAcdTmrInterval = 100;
static constexpr uint32_t kAcdTicksPerSecond = (1000U / kAcdTmrInterval);

static TimerHandle_t s_timer_id;

static void Timer([[maybe_unused]] TimerHandle_t handle)
{
    if (!netif::IsLinkUp())
    {
        return;
    }

    auto* acd = reinterpret_cast<struct acd::Acd*>(netif::globals::netif_default.acd);
    assert(acd != nullptr);

    if (acd->lastconflict > 0)
    {
        acd->lastconflict--;
    }

    DEBUG_PRINTF("state=%u, ttw=%u", static_cast<uint32_t>(acd->state), acd->ttw);

    if (acd->ttw > 0)
    {
        acd->ttw--;
    }

    switch (acd->state)
    {
        case acd::State::ACD_STATE_PROBE_WAIT:
        case acd::State::ACD_STATE_PROBING:
            if (acd->ttw == 0)
            {
                acd->state = acd::State::ACD_STATE_PROBING;
                arp::AcdProbe(acd->ipaddr);
                DEBUG_PUTS("PROBING Sent Probe");
                acd->sent_num++;
                if (acd->sent_num >= PROBE_NUM)
                {
                    acd->state = acd::State::ACD_STATE_ANNOUNCE_WAIT;
                    acd->sent_num = 0;
                    acd->ttw = static_cast<uint16_t>(ANNOUNCE_WAIT * acd::kAcdTicksPerSecond);
                }
                else
                {
                    acd->ttw = static_cast<uint16_t>(static_cast<uint32_t>(random()) % (((PROBE_MAX - PROBE_MIN) * acd::kAcdTicksPerSecond)) +
                                                     (PROBE_MIN * acd::kAcdTicksPerSecond));
                }
            }
            break;
        case acd::State::ACD_STATE_ANNOUNCE_WAIT:
        case acd::State::ACD_STATE_ANNOUNCING:
            if (acd->ttw == 0)
            {
                if (acd->sent_num == 0)
                {
                    acd->state = acd::State::ACD_STATE_ANNOUNCING;
                    acd->num_conflicts = 0;
                }
                arp::AcdSendAnnouncement(acd->ipaddr);
                DEBUG_PUTS("ANNOUNCING Sent Announce");
                acd->ttw = static_cast<uint16_t>(ANNOUNCE_INTERVAL * acd::kAcdTicksPerSecond);
                acd->sent_num++;

                if (acd->sent_num >= ANNOUNCE_NUM)
                {
                    acd->state = acd::State::ACD_STATE_ONGOING;
                    acd->sent_num = 0;
                    acd->ttw = 0;
                    Stop(acd);
                    acd->conflict_callback(acd::Callback::ACD_IP_OK);
                }
            }
            break;
        case acd::State::ACD_STATE_RATE_LIMIT:
            if (acd->ttw == 0)
            {
                Stop(acd);
                acd->conflict_callback(acd::Callback::ACD_RESTART_CLIENT);
            }
            break;
        default:
            break;
    }
}

static void Restart(struct acd::Acd* acd)
{
    acd->num_conflicts++;
    acd->conflict_callback(acd::Callback::ACD_DECLINE);

    if (acd->num_conflicts >= MAX_CONFLICTS)
    {
        acd->state = acd::State::ACD_STATE_RATE_LIMIT;
        acd->ttw = static_cast<uint16_t>(RATE_LIMIT_INTERVAL * acd::kAcdTicksPerSecond);
        DEBUG_PUTS("rate limiting initiated. too many conflicts");
    }
    else
    {
        Stop(acd);
        acd->conflict_callback(acd::Callback::ACD_RESTART_CLIENT);
    }
}

static void HandleArpConflict(struct acd::Acd* acd)
{
    /* RFC5227, 2.4 "Ongoing Address Conflict Detection and Address Defense"
     allows three options where:
     a) means retreat on the first conflict,
     b) allows to keep an already configured address when having only one
        conflict in DEFEND_INTERVAL seconds and
     c) the host will not give up it's address and defend it indefinitely

     We use option b) when the acd module represents the netif address, since it
     helps to improve the chance that one of the two conflicting hosts may be
     able to retain its address. while we are flexible enough to help network
     performance

     We use option a) when the acd module does not represent the netif address,
     since we cannot have the acd module announcing or restarting. This
     situation occurs for the LL acd module when a routable address is used on
     the netif but the LL address is still open in the background. */

    if (acd->state == acd::State::ACD_STATE_PASSIVE_ONGOING)
    {
        DEBUG_PUTS("conflict when we are in passive mode -> back off");
        Stop(acd);
        acd->conflict_callback(acd::Callback::ACD_DECLINE);
    }
    else
    {
        if (acd->lastconflict > 0)
        {
            DEBUG_PUTS("conflict within DEFEND_INTERVAL -> retreating");
            Restart(acd);
        }
        else
        {
            DEBUG_PUTS("we are defending, send ARP Announce");
            arp::AcdSendAnnouncement(acd->ipaddr);
            acd->lastconflict = DEFEND_INTERVAL * acd::kAcdTicksPerSecond;
        }
    }
}

static void PutInPassiveMode()
{
    auto* acd = reinterpret_cast<struct acd::Acd*>(netif::globals::netif_default.acd);
    assert(acd != nullptr);

    switch (acd->state)
    {
        case acd::State::ACD_STATE_OFF:
        case acd::State::ACD_STATE_PASSIVE_ONGOING:
        default:
            /* do nothing */
            break;
        case acd::State::ACD_STATE_PROBE_WAIT:
        case acd::State::ACD_STATE_PROBING:
        case acd::State::ACD_STATE_ANNOUNCE_WAIT:
        case acd::State::ACD_STATE_RATE_LIMIT:
            Stop(acd);
            acd->conflict_callback(acd::Callback::ACD_DECLINE);
            break;
        case acd::State::ACD_STATE_ANNOUNCING:
        case acd::State::ACD_STATE_ONGOING:
            acd->state = acd::State::ACD_STATE_PASSIVE_ONGOING;
            break;
    }
}

// Public interface

void Start(struct acd::Acd* acd, ip4_addr_t ipaddr)
{
    DEBUG_ENTRY();
    assert(acd != nullptr);

    acd->ipaddr.addr = ipaddr.addr;
    acd->state = acd::State::ACD_STATE_PROBE_WAIT;
    acd->ttw = static_cast<uint16_t>(static_cast<uint32_t>(random()) % (PROBE_WAIT * acd::kAcdTicksPerSecond));

    s_timer_id = SoftwareTimerAdd(acd::kAcdTmrInterval, Timer);
    assert(s_timer_id != kTimerIdNone);

    DEBUG_EXIT();
}

void Stop(struct acd::Acd* acd)
{
    DEBUG_ENTRY();
    assert(acd != nullptr);

    acd->state = acd::State::ACD_STATE_OFF;

    if (s_timer_id != kTimerIdNone)
    {
        SoftwareTimerDelete(s_timer_id);
        s_timer_id = kTimerIdNone;
    }

    DEBUG_EXIT();
}

void NetworkChangedLinkDown()
{
    DEBUG_ENTRY();

    auto* acd = reinterpret_cast<struct acd::Acd*>(netif::globals::netif_default.acd);

    if (acd == nullptr)
    {
        DEBUG_EXIT();
        return;
    }

    Stop(acd);

    DEBUG_EXIT();
}

//  Handles every incoming ARP Packet, called by arp_handle
void ArpReply(const struct t_arp* arp)
{
    DEBUG_ENTRY();
    auto* acd = reinterpret_cast<struct acd::Acd*>(netif::globals::netif_default.acd);

    if (acd == nullptr)
    {
        DEBUG_EXIT();
        return;
    }

    switch (acd->state)
    {
        case acd::State::ACD_STATE_OFF:
        case acd::State::ACD_STATE_RATE_LIMIT:
        default:
            break;
        case acd::State::ACD_STATE_PROBE_WAIT:
        case acd::State::ACD_STATE_PROBING:
        case acd::State::ACD_STATE_ANNOUNCE_WAIT:
            /* RFC 5227 Section 2.1.1:
             * from beginning to after ANNOUNCE_WAIT seconds we have a conflict if
             * ip.sender == ipaddr (someone is already using the address)
             * OR
             * ip.dst == ipaddr && hw.src != own macAddress (someone else is probing it)
             */
            if (((memcpy_ip(arp->arp.sender_ip) == acd->ipaddr.addr)) ||
                (!(memcpy_ip(arp->arp.sender_ip) == 0) && ((memcpy_ip(arp->arp.target_ip)) == acd->ipaddr.addr) &&
                 (memcmp(arp->arp.sender_mac, netif::globals::netif_default.hwaddr, ETH_ADDR_LEN) == 0)))
            {
                DEBUG_PUTS("Probe Conflict detected");
                Restart(acd);
            }
            break;
        case acd::State::ACD_STATE_ANNOUNCING:
        case acd::State::ACD_STATE_ONGOING:
        case acd::State::ACD_STATE_PASSIVE_ONGOING:
            /* RFC 5227 Section 2.4:
             * in any state we have a conflict if
             * ip.sender == ipaddr && hw.src != own macAddress (someone is using our address)
             */
            if ((memcpy_ip(arp->arp.sender_ip) == acd->ipaddr.addr) && (memcmp(arp->arp.sender_mac, netif::globals::netif_default.hwaddr, ETH_ADDR_LEN) != 0))
            {
                DEBUG_PUTS("Conflicting ARP-Packet detected");
                HandleArpConflict(acd);
            }
            break;
    }

    DEBUG_EXIT();
}

void Add(struct acd::Acd* acd, conflict_callback_t acd_conflict_callback)
{
    DEBUG_ENTRY();
    assert(acd != nullptr);
    assert(acd_conflict_callback != nullptr);

    acd->conflict_callback = acd_conflict_callback;

    auto& netif = netif::globals::netif_default;
    netif.acd = acd;

    DEBUG_EXIT();
}

void Remove(struct acd::Acd* acd)
{
    DEBUG_ENTRY();

    assert(acd != nullptr);
    auto& netif = netif::globals::netif_default;

    if (netif.acd == acd)
    {
        netif.acd = nullptr;

        DEBUG_EXIT();
        return;
    }

    DEBUG_EXIT();
}

void NetifIpAddrChanged(ip4_addr_t old_addr, ip4_addr_t new_addr)
{
    if ((old_addr.addr == 0) || (new_addr.addr == 0))
    {
        return;
    }

    auto* acd = reinterpret_cast<struct acd::Acd*>(netif::globals::netif_default.acd);

    if (acd->ipaddr.addr == old_addr.addr)
    {
        // Did we change from a LL address to a routable address?
        if (net::is_linklocal_ip(old_addr.addr) && !net::is_linklocal_ip(new_addr.addr))
        {
            // Put the module in passive conflict detection mode
            PutInPassiveMode();
        }
    }
}
} // namespace net::acd
