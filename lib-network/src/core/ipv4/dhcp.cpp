/**
 * @file dhcp.cpp
 */
/* Copyright (C) 2018-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#if defined(DEBUG_NET_DHCP)
#undef NDEBUG
#endif

#include <cstdint>
#include <cstring>
#include <cassert>

#include "softwaretimers.h"
#include "net/udp.h"
#include "net/netif.h"
#include "net_memcpy.h"
#include "net_private.h"
#include "net/dhcp.h"
#include "net/protocol/dhcp.h"
#include "net/protocol/iana.h"
#if defined(CONFIG_NET_DHCP_USE_ACD)
#include "net/acd.h"
#endif
 #include "firmware/debug/debug_debug.h"

#define REBOOT_TRIES 2

static TimerHandle_t s_timer_id;

// https://tools.ietf.org/html/rfc1541
namespace net::dhcp
{
static dhcp::Message s_dhcp_message SECTION_NETWORK ALIGNED;

static void MessageInit()
{
    std::memset(&s_dhcp_message, 0, sizeof(dhcp::Message));

    s_dhcp_message.op = dhcp::OpCode::BOOTREQUEST;
    s_dhcp_message.htype = dhcp::HardwareType::HTYPE_10MB; // This is the current default
    s_dhcp_message.hlen = net::MAC_SIZE;
    memcpy(s_dhcp_message.chaddr, netif::globals::netif_default.hwaddr, net::MAC_SIZE);

    s_dhcp_message.options[0] = static_cast<uint8_t>((dhcp::MAGIC_COOKIE & 0xFF000000) >> 24);
    s_dhcp_message.options[1] = static_cast<uint8_t>((dhcp::MAGIC_COOKIE & 0x00FF0000) >> 16);
    s_dhcp_message.options[2] = static_cast<uint8_t>((dhcp::MAGIC_COOKIE & 0x0000FF00) >> 8);
    s_dhcp_message.options[3] = static_cast<uint8_t>(dhcp::MAGIC_COOKIE & 0x000000FF) >> 0;

    s_dhcp_message.options[4] = dhcp::Options::OPTION_MESSAGE_TYPE;
    s_dhcp_message.options[5] = 0x01;
}

static void UpdateMsg(uint8_t message_type)
{
    auto* dhcp = reinterpret_cast<struct dhcp::Dhcp*>(netif::globals::netif_default.dhcp);
    assert(dhcp != nullptr);

    // DHCP_REQUEST should reuse 'xid' from DHCPOFFER 
    if ((message_type != dhcp::Type::REQUEST) || (dhcp->state == dhcp::State::STATE_REBOOTING))
    {
        // reuse transaction identifier in retransmissions 
        if (dhcp->tries == 0)
        {
            auto xid = __builtin_bswap32(dhcp->xid);
            xid++;
            dhcp->xid = __builtin_bswap32(xid);
        }
    }

    s_dhcp_message.xid = dhcp->xid;

    if ((message_type == dhcp::Type::INFORM) || (message_type == dhcp::Type::DECLINE) || (message_type == dhcp::Type::RELEASE) ||
        ((message_type == dhcp::Type::REQUEST) && /* DHCP_STATE_BOUND not used for sending! */
         ((dhcp->state == dhcp::State::STATE_RENEWING) || dhcp->state == dhcp::State::STATE_REBINDING)))
    {
        const auto& netif = netif::globals::netif_default;
        net::memcpy_ip(s_dhcp_message.ciaddr, netif.ip.addr);
    }
}

static void SendDiscover()
{
    DEBUG_ENTRY();
    auto* dhcp = reinterpret_cast<struct dhcp::Dhcp*>(netif::globals::netif_default.dhcp);
    assert(dhcp != nullptr);

    UpdateMsg(dhcp::Type::DISCOVER);

    uint32_t k = 6;

    s_dhcp_message.options[k++] = dhcp::Type::DISCOVER;

    s_dhcp_message.options[k++] = dhcp::Options::OPTION_CLIENT_IDENTIFIER;
    s_dhcp_message.options[k++] = 0x07;
    s_dhcp_message.options[k++] = 0x01;
    s_dhcp_message.options[k++] = netif::globals::netif_default.hwaddr[0];
    s_dhcp_message.options[k++] = netif::globals::netif_default.hwaddr[1];
    s_dhcp_message.options[k++] = netif::globals::netif_default.hwaddr[2];
    s_dhcp_message.options[k++] = netif::globals::netif_default.hwaddr[3];
    s_dhcp_message.options[k++] = netif::globals::netif_default.hwaddr[4];
    s_dhcp_message.options[k++] = netif::globals::netif_default.hwaddr[5];

    s_dhcp_message.options[k++] = dhcp::Options::OPTION_PARAM_REQUEST;
    s_dhcp_message.options[k++] = 0x06; // length of request
    s_dhcp_message.options[k++] = dhcp::Options::OPTION_SUBNET_MASK;
    s_dhcp_message.options[k++] = dhcp::Options::OPTION_ROUTER;
    s_dhcp_message.options[k++] = dhcp::Options::OPTION_DOMAIN_NAME;
    s_dhcp_message.options[k++] = dhcp::Options::OPTION_LEASE_TIME;
    s_dhcp_message.options[k++] = dhcp::Options::OPTION_DHCP_T1_VALUE;
    s_dhcp_message.options[k++] = dhcp::Options::OPTION_DHCP_T2_VALUE;
    s_dhcp_message.options[k++] = dhcp::Options::OPTION_END;

    net::udp::Send(dhcp->handle, reinterpret_cast<uint8_t*>(&s_dhcp_message), static_cast<uint16_t>(k + sizeof(dhcp::Message) - dhcp::OPT_SIZE),
                   net::IPADDR_BROADCAST, net::iana::IANA_PORT_DHCP_SERVER);

    DEBUG_EXIT();
}

static void SendRequest()
{
    DEBUG_ENTRY();
    auto* dhcp = reinterpret_cast<struct dhcp::Dhcp*>(netif::globals::netif_default.dhcp);
    assert(dhcp != nullptr);

    uint32_t i;
    uint32_t k = 6;

    s_dhcp_message.options[k++] = dhcp::Type::REQUEST;

    s_dhcp_message.options[k++] = dhcp::Options::OPTION_CLIENT_IDENTIFIER;
    s_dhcp_message.options[k++] = 0x07;
    s_dhcp_message.options[k++] = 0x01;
    s_dhcp_message.options[k++] = netif::globals::netif_default.hwaddr[0];
    s_dhcp_message.options[k++] = netif::globals::netif_default.hwaddr[1];
    s_dhcp_message.options[k++] = netif::globals::netif_default.hwaddr[2];
    s_dhcp_message.options[k++] = netif::globals::netif_default.hwaddr[3];
    s_dhcp_message.options[k++] = netif::globals::netif_default.hwaddr[4];
    s_dhcp_message.options[k++] = netif::globals::netif_default.hwaddr[5];

    s_dhcp_message.options[k++] = dhcp::Options::OPTION_REQUESTED_IP;
    s_dhcp_message.options[k++] = 0x04;
    memcpy_ip(&s_dhcp_message.options[k], dhcp->offered.offered_ip_addr.addr);
    k = k + 4;

    s_dhcp_message.options[k++] = dhcp::Options::OPTION_SERVER_IDENTIFIER;
    s_dhcp_message.options[k++] = 0x04;
    memcpy_ip(&s_dhcp_message.options[k], dhcp->server_ip_addr.addr);
    k = k + 4;

    s_dhcp_message.options[k++] = dhcp::Options::OPTION_HOSTNAME;
    s_dhcp_message.options[k++] = 0; // length of hostname
    for (i = 0; netif::globals::netif_default.hostname[i] != 0; i++)
    {
        s_dhcp_message.options[k++] = netif::globals::netif_default.hostname[i];
    }
    s_dhcp_message.options[k - (i + 1)] = static_cast<uint8_t>(i); // length of hostname

    s_dhcp_message.options[k++] = dhcp::Options::OPTION_PARAM_REQUEST;
    s_dhcp_message.options[k++] = 0x06; // length of request
    s_dhcp_message.options[k++] = dhcp::Options::OPTION_SUBNET_MASK;
    s_dhcp_message.options[k++] = dhcp::Options::OPTION_ROUTER;
    s_dhcp_message.options[k++] = dhcp::Options::OPTION_DNS_SERVER;
    s_dhcp_message.options[k++] = dhcp::Options::OPTION_DOMAIN_NAME;
    s_dhcp_message.options[k++] = dhcp::Options::OPTION_DHCP_T1_VALUE;
    s_dhcp_message.options[k++] = dhcp::Options::OPTION_DHCP_T2_VALUE;
    s_dhcp_message.options[k++] = dhcp::Options::OPTION_END;

    net::udp::Send(dhcp->handle, reinterpret_cast<uint8_t*>(&s_dhcp_message), static_cast<uint16_t>(k + sizeof(dhcp::Message) - dhcp::OPT_SIZE),
                   net::IPADDR_BROADCAST, net::iana::IANA_PORT_DHCP_SERVER);

    DEBUG_EXIT();
}

static void SendRelease(uint32_t destination_ip)
{
    DEBUG_ENTRY();
    DEBUG_PRINTF(IPSTR, IP2STR(destination_ip));

    auto* dhcp = reinterpret_cast<struct dhcp::Dhcp*>(netif::globals::netif_default.dhcp);
    assert(dhcp != nullptr);

    uint32_t k = 6;

    s_dhcp_message.options[k++] = dhcp::Type::RELEASE;

    s_dhcp_message.options[k++] = dhcp::Options::OPTION_SERVER_IDENTIFIER;
    s_dhcp_message.options[k++] = 0x04;
    net::memcpy_ip(&s_dhcp_message.options[k], dhcp->server_ip_addr.addr);
    k = k + 4;

    s_dhcp_message.options[k++] = dhcp::Options::OPTION_END;

    net::udp::Send(dhcp->handle, reinterpret_cast<uint8_t*>(&s_dhcp_message), static_cast<uint16_t>(k + sizeof(dhcp::Message) - dhcp::OPT_SIZE),
                   destination_ip, net::iana::IANA_PORT_DHCP_SERVER);

    DEBUG_EXIT();
}

void Input(const uint8_t* buffer, uint32_t size, [[maybe_unused]] uint32_t from_ip, uint16_t from_port)
{
    DEBUG_ENTRY();

    if (from_port == net::iana::IANA_PORT_DHCP_SERVER)
    {
        auto* dhcp = reinterpret_cast<struct dhcp::Dhcp*>(netif::globals::netif_default.dhcp);
        assert(dhcp != nullptr);

        const auto* const kP = reinterpret_cast<const dhcp::Message*>(buffer);

        if (kP->xid != dhcp->xid)
        {
            DEBUG_PRINTF("pDhcpMessage->xid=%u, dhcp->xid=%u", kP->xid, dhcp->xid);
            return;
        }

        dhcp::Process(kP, size);

        DEBUG_EXIT();
        return;
    }

    DEBUG_EXIT();
}

void Inform()
{
    DEBUG_ENTRY();

    const auto kHandle = net::udp::Begin(net::iana::IANA_PORT_DHCP_CLIENT, nullptr);
#ifndef NDEBUG
    if (kHandle < 0)
    {
        console::Error("DHCP Inform");
        return;
    }
#endif

    MessageInit();
    net::memcpy_ip(&s_dhcp_message.ciaddr[0], netif::globals::netif_default.ip.addr);

    uint32_t k = 6;

    s_dhcp_message.options[k++] = dhcp::Type::INFORM;

    s_dhcp_message.options[k++] = dhcp::Options::OPTION_CLIENT_IDENTIFIER;
    s_dhcp_message.options[k++] = 0x07;
    s_dhcp_message.options[k++] = 0x01;
    s_dhcp_message.options[k++] = netif::globals::netif_default.hwaddr[0];
    s_dhcp_message.options[k++] = netif::globals::netif_default.hwaddr[1];
    s_dhcp_message.options[k++] = netif::globals::netif_default.hwaddr[2];
    s_dhcp_message.options[k++] = netif::globals::netif_default.hwaddr[3];
    s_dhcp_message.options[k++] = netif::globals::netif_default.hwaddr[4];
    s_dhcp_message.options[k++] = netif::globals::netif_default.hwaddr[5];

    s_dhcp_message.options[k++] = dhcp::Options::OPTION_END;

    net::udp::Send(kHandle, reinterpret_cast<uint8_t*>(&s_dhcp_message), static_cast<uint16_t>(k + sizeof(dhcp::Message) - dhcp::OPT_SIZE),
                   net::IPADDR_BROADCAST, net::iana::IANA_PORT_DHCP_SERVER);
    net::udp::End(net::iana::IANA_PORT_DHCP_CLIENT);

    DEBUG_EXIT();
}

#define SET_TIMEOUT_FROM_OFFERED(result, offered, min, max)                                       \
    do                                                                                            \
    {                                                                                             \
        uint32_t timeout = (offered + dhcp::kCoarseTimerSecs / 2) / dhcp::kCoarseTimerSecs; \
        if (timeout > max)                                                                        \
        {                                                                                         \
            timeout = max;                                                                        \
        }                                                                                         \
        if (timeout == min)                                                                       \
        {                                                                                         \
            timeout = 1;                                                                          \
        }                                                                                         \
        result = static_cast<dhcp::dhcp_timeout_t>(timeout);                                      \
    } while (0)

#define DHCP_SET_TIMEOUT_FROM_OFFERED_T0_LEASE(res, dhcp) SET_TIMEOUT_FROM_OFFERED(res, (dhcp)->offered.offered_t0_lease, 0, 0xffff)
#define DHCP_SET_TIMEOUT_FROM_OFFERED_T1_RENEW(res, dhcp) SET_TIMEOUT_FROM_OFFERED(res, (dhcp)->offered.offered_t1_renew, 0, 0xffff)
#define DHCP_SET_TIMEOUT_FROM_OFFERED_T2_REBIND(res, dhcp) SET_TIMEOUT_FROM_OFFERED(res, (dhcp)->offered.offered_t2_rebind, 0, 0xffff)

#define DHCP_NEXT_TIMEOUT_THRESHOLD ((60 + net::dhcp::kCoarseTimerSecs / 2) / net::dhcp::kCoarseTimerSecs)
#define DHCP_REQUEST_BACKOFF_SEQUENCE(tries) (((tries) < 6U ? 1U << (tries) : 60U) * 1000U)

static void SetState(struct dhcp::Dhcp* dhcp, dhcp::State new_state)
{
    if (new_state != dhcp->state)
    {
        DEBUG_PRINTF("%u -> %u", static_cast<unsigned>(dhcp->state), static_cast<unsigned>(new_state));

        dhcp->state = new_state;
        dhcp->tries = 0;
        dhcp->request_timeout = 0;
    }
}

static void Bind()
{
    DEBUG_ENTRY();

    auto* dhcp = reinterpret_cast<struct dhcp::Dhcp*>(netif::globals::netif_default.dhcp);
    assert(dhcp != nullptr);

    /* reset time used of lease */
    dhcp->lease_used = 0;

    if (dhcp->offered.offered_t0_lease != 0xffffffffUL)
    {
        /* set renewal period timer */
        DHCP_SET_TIMEOUT_FROM_OFFERED_T0_LEASE(dhcp->t0_timeout, dhcp);
    }

    /* temporary DHCP lease? */
    if (dhcp->offered.offered_t1_renew != 0xffffffffUL)
    {
        /* set renewal period timer */
        DHCP_SET_TIMEOUT_FROM_OFFERED_T1_RENEW(dhcp->t1_timeout, dhcp);
        dhcp->t1_renew_time = dhcp->t1_timeout;
    }
    /* set renewal period timer */
    if (dhcp->offered.offered_t2_rebind != 0xffffffffUL)
    {
        DHCP_SET_TIMEOUT_FROM_OFFERED_T2_REBIND(dhcp->t2_timeout, dhcp);
        dhcp->t2_rebind_time = dhcp->t2_timeout;
    }

    /* If we have sub 1 minute lease, t2 and t1 will kick in at the same time. */
    if ((dhcp->t1_timeout >= dhcp->t2_timeout) && (dhcp->t2_timeout > 0))
    {
        dhcp->t1_timeout = 0;
    }

    ip4_addr_t sn_mask;

    if (dhcp->flags & DHCP_FLAG_SUBNET_MASK_GIVEN)
    {
        /* copy offered network mask */
        sn_mask.addr = dhcp->offered.offered_sn_mask.addr;
    }
    else
    {
        // subnet mask not given, choose a safe subnet mask given the network class
        const uint8_t kFirstOctet = dhcp->offered.offered_ip_addr.addr & 0xFF;
        if (kFirstOctet <= 127)
        {
            sn_mask.addr = 0xFF;
        }
        else if (kFirstOctet >= 192)
        {
            sn_mask.addr = 0xFFFFFF;
        }
        else
        {
            sn_mask.addr = 0xFFFF;
        }
    }

    ip4_addr_t gw_addr;
    gw_addr.addr = dhcp->offered.offered_gw_addr.addr;

    SetState(dhcp, dhcp::State::STATE_BOUND);

    netif::SetFlags(netif::Netif::kNetifFlagDhcpOk);
    netif::SetAddr(dhcp->offered.offered_ip_addr, sn_mask, gw_addr);

    DEBUG_EXIT();
}

static void Rebind()
{
    DEBUG_ENTRY();

    DEBUG_EXIT();
}

#if defined(CONFIG_NET_DHCP_USE_ACD)
static void SendDecline()
{
    DEBUG_ENTRY();
    auto* dhcp = reinterpret_cast<struct dhcp::Dhcp*>(netif::globals::netif_default.dhcp);
    assert(dhcp != nullptr);

    UpdateMsg(dhcp::Type::DECLINE);

    uint32_t k = 6;

    s_dhcp_message.options[k++] = dhcp::Type::DECLINE;

    s_dhcp_message.options[k++] = dhcp::Options::OPTION_REQUESTED_IP;
    s_dhcp_message.options[k++] = 0x04;
    net::memcpy_ip(&s_dhcp_message.options[k], dhcp->offered.offered_ip_addr.addr);
    k = k + 4;
    s_dhcp_message.options[k++] = dhcp::Options::OPTION_END;

    net::udp::Send(dhcp->handle, reinterpret_cast<uint8_t*>(&s_dhcp_message), static_cast<uint16_t>(k + sizeof(dhcp::Message) - dhcp::OPT_SIZE),
                   net::IPADDR_BROADCAST, net::iana::IANA_PORT_DHCP_SERVER);

    DEBUG_EXIT();
}

static void Decline()
{
    DEBUG_ENTRY();

    auto* dhcp = reinterpret_cast<struct dhcp::Dhcp*>(netif::globals::netif_default.dhcp);
    assert(dhcp != nullptr);

    SetState(dhcp, dhcp::State::STATE_BACKING_OFF);
    /* per section 4.4.4, broadcast DECLINE messages */
    SendDecline();

    DEBUG_EXIT();
    return;
}

static void ConflictCallback(net::acd::Callback callback)
{
    auto* dhcp = reinterpret_cast<struct dhcp::Dhcp*>(netif::globals::netif_default.dhcp);
    assert(dhcp != nullptr);
    assert(dhcp->state != dhcp::State::STATE_OFF);

    uint16_t msecs;

    switch (callback)
    {
        case net::acd::Callback::ACD_IP_OK:
            Bind();
            break;
        case net::acd::Callback::ACD_RESTART_CLIENT:
            /* wait 10s before restarting
             * According to RFC2131 section 3.1 point 5:
             * If the client detects that the address is already in use (e.g., through
             * the use of ARP), the client MUST send a DHCPDECLINE message to the
             * server and restarts the configuration process.  The client SHOULD wait
             * a minimum of ten seconds before restarting the configuration process to
             * avoid excessive network traffic in case of looping. */
            SetState(dhcp, dhcp::State::STATE_BACKING_OFF);
            msecs = 10U * 1000U;
            dhcp->request_timeout = static_cast<uint16_t>((msecs + dhcp::kFineTimerMsecs - 1) / dhcp::kFineTimerMsecs);
            break;
        case net::acd::Callback::ACD_DECLINE:
            /* remove IP address from interface
             * (prevents routing from selecting this interface) */
            ip4_addr_t any;
            any.addr = 0;
            netif::SetAddr(any, any, any);
            /* Let the DHCP server know we will not use the address */
            Decline();
            netif::ClearFlags(Netif::kNetifFlagDhcpOk);
            break;
        default:
            break;
    }
}

static void Check()
{
    DEBUG_ENTRY();
    auto* dhcp = reinterpret_cast<struct dhcp::Dhcp*>(netif::globals::netif_default.dhcp);

    SetState(dhcp, dhcp::State::STATE_CHECKING);

    net::acd::Start(&dhcp->acd, dhcp->offered.offered_ip_addr);

    DEBUG_EXIT();
}
#endif

static void Discover()
{
    DEBUG_ENTRY();
    auto* dhcp = reinterpret_cast<struct dhcp::Dhcp*>(netif::globals::netif_default.dhcp);
    assert(dhcp != nullptr);

#if defined(CONFIG_NET_DHCP_USE_AUTOIP)
    if (dhcp->tries >= DHCP_AUTOIP_COOP_TRIES)
    {
        autoip::Start();
    }
#endif

    dhcp->offered.offered_ip_addr.addr = 0;

    SetState(dhcp, dhcp::State::STATE_SELECTING);

    SendDiscover();

    if (dhcp->tries < 255)
    {
        dhcp->tries++;
    }

    const auto kMsecs = DHCP_REQUEST_BACKOFF_SEQUENCE(dhcp->tries);
    dhcp->request_timeout = static_cast<uint16_t>((kMsecs + dhcp::kFineTimerMsecs - 1) / dhcp::kFineTimerMsecs);
}

static void Reboot()
{
    DEBUG_ENTRY();
    auto* dhcp = reinterpret_cast<struct dhcp::Dhcp*>(netif::globals::netif_default.dhcp);
    assert(dhcp != nullptr);

    SetState(dhcp, dhcp::State::STATE_REBOOTING);

    SendRequest();

    if (dhcp->tries < 255)
    {
        dhcp->tries++;
    }

    const auto kMsecs = static_cast<uint16_t>(dhcp->tries < 10U ? dhcp->tries * 1000U : 10U * 1000U);
    dhcp->request_timeout = static_cast<uint16_t>((kMsecs + dhcp::kFineTimerMsecs - 1) / dhcp::kFineTimerMsecs);

    DEBUG_EXIT();
}

static void Select()
{
    DEBUG_ENTRY();
    auto* dhcp = reinterpret_cast<struct dhcp::Dhcp*>(netif::globals::netif_default.dhcp);

    SetState(dhcp, dhcp::State::STATE_REQUESTING);

    SendRequest();

    if (dhcp->tries < 255)
    {
        dhcp->tries++;
    }

    const auto kMsecs = static_cast<uint16_t>((dhcp->tries < 6 ? 1 << dhcp->tries : 60U) * 1000U);
    dhcp->request_timeout = static_cast<uint16_t>((kMsecs + dhcp::kFineTimerMsecs - 1) / dhcp::kFineTimerMsecs);
}

static void Timeout()
{
    auto* dhcp = reinterpret_cast<struct dhcp::Dhcp*>(netif::globals::netif_default.dhcp);
    assert(dhcp != nullptr);

    /* back-off period has passed, or server selection timed out */
    if ((dhcp->state == dhcp::State::STATE_BACKING_OFF) || (dhcp->state == dhcp::State::STATE_SELECTING))
    {
        Discover();
        /* receiving the requested lease timed out */
    }
    else if (dhcp->state == dhcp::State::STATE_REQUESTING)
    {
        if (dhcp->tries <= 5)
        {
            Select();
        }
        else
        {
            dhcp::ReleaseAndStop();
            dhcp::Start();
        }
    }
    else if (dhcp->state == dhcp::State::STATE_REBOOTING)
    {
        if (dhcp->tries < REBOOT_TRIES)
        {
            Reboot();
        }
        else
        {
            Discover();
        }
    }
}

static void T1Timeout()
{
    auto* dhcp = reinterpret_cast<struct dhcp::Dhcp*>(netif::globals::netif_default.dhcp);

    if ((dhcp->state == dhcp::State::STATE_REQUESTING) || (dhcp->state == dhcp::State::STATE_BOUND) || (dhcp->state == dhcp::State::STATE_RENEWING))
    {
        /* just retry to renew - note that the rebind timer (t2) will
         * eventually time-out if renew tries fail. */
        /* This slightly different to RFC2131: DHCPREQUEST will be sent from state
         DHCP_STATE_RENEWING, not DHCP_STATE_BOUND */
        dhcp::Renew();
        /* Calculate next timeout */
        if (static_cast<uint32_t>((dhcp->t2_timeout - dhcp->lease_used) / 2) >= DHCP_NEXT_TIMEOUT_THRESHOLD)
        {
            dhcp->t1_renew_time = static_cast<net::dhcp::dhcp_timeout_t>((dhcp->t2_timeout - dhcp->lease_used) / 2);
        }
    }
}

static void T2Timeout()
{
    auto* dhcp = reinterpret_cast<struct dhcp::Dhcp*>(netif::globals::netif_default.dhcp);

    if ((dhcp->state == dhcp::State::STATE_REQUESTING) || (dhcp->state == dhcp::State::STATE_BOUND) || (dhcp->state == dhcp::State::STATE_RENEWING) ||
        (dhcp->state == dhcp::State::STATE_REBINDING))
    {
        /* just retry to rebind */
        /* This slightly different to RFC2131: DHCPREQUEST will be sent from state
         DHCP_STATE_REBINDING, not DHCP_STATE_BOUND */
        Rebind();
        /* Calculate next timeout */
        if (static_cast<uint32_t>((dhcp->t0_timeout - dhcp->lease_used) / 2) >= DHCP_NEXT_TIMEOUT_THRESHOLD)
        {
            dhcp->t2_rebind_time = static_cast<net::dhcp::dhcp_timeout_t>((dhcp->t0_timeout - dhcp->lease_used) / 2);
        }
    }
}

void CoarseTmr([[maybe_unused]] TimerHandle_t handle)
{
    auto* dhcp = reinterpret_cast<struct dhcp::Dhcp*>(netif::globals::netif_default.dhcp);

    if ((dhcp != nullptr) && (dhcp->state != dhcp::State::STATE_OFF))
    {
        /* compare lease time to expire timeout */
        if (dhcp->t0_timeout && (++dhcp->lease_used == dhcp->t0_timeout))
        {
            /* this clients' lease time has expired */
            dhcp::ReleaseAndStop();
            dhcp::Start();
            /* timer is active (non zero), and triggers (zeroes) now? */
        }
        else if (dhcp->t2_rebind_time && (dhcp->t2_rebind_time-- == 1))
        {
            /* this clients' rebind timeout triggered */
            T2Timeout();
            /* timer is active (non zero), and triggers (zeroes) now */
        }
        else if (dhcp->t1_renew_time && (dhcp->t1_renew_time-- == 1))
        {
            /* this clients' renewal timeout triggered */
            T1Timeout();
        }
    }
}

static void FineTmr([[maybe_unused]] TimerHandle_t handle)
{
    auto* dhcp = reinterpret_cast<struct dhcp::Dhcp*>(netif::globals::netif_default.dhcp);

    if (dhcp != nullptr)
    {
        /* timer is active (non zero), and is about to trigger now */
        if (dhcp->request_timeout > 1)
        {
            dhcp->request_timeout--;
        }
        else if (dhcp->request_timeout == 1)
        {
            dhcp->request_timeout--;
            /* this client's request timeout triggered */
            Timeout();
        }
    }
}

static void HandleOffer(const dhcp::Message* const kResponse)
{
    DEBUG_ENTRY();
    auto* dhcp = reinterpret_cast<struct dhcp::Dhcp*>(netif::globals::netif_default.dhcp);

    if (dhcp->server_ip_addr.addr != 0)
    {
        dhcp->request_timeout = 0; /* stop timer */
        dhcp->offered.offered_ip_addr.addr = net::memcpy_ip(kResponse->yiaddr);
        DEBUG_PRINTF(IPSTR " -> " IPSTR, IP2STR(dhcp->server_ip_addr.addr), IP2STR(dhcp->offered.offered_ip_addr.addr));
        Select();
    }
    else
    {
        DEBUG_PUTS("did not get server ID!");
    }

    DEBUG_EXIT();
}

static void HandleAck(const dhcp::Message* const kResponse)
{
    DEBUG_ENTRY();
    auto* dhcp = reinterpret_cast<struct dhcp::Dhcp*>(netif::globals::netif_default.dhcp);

    DEBUG_PRINTF("t0=%u, t1=%u, t2=%u", dhcp->offered.offered_t0_lease, dhcp->offered.offered_t1_renew, dhcp->offered.offered_t2_rebind);

    if (dhcp->offered.offered_t1_renew == 0)
    {
        /* calculate safe periods for renewal */
        dhcp->offered.offered_t1_renew = dhcp->offered.offered_t0_lease / 2;
    }

    if (dhcp->offered.offered_t2_rebind == 0)
    {
        /* calculate safe periods for rebinding (offered_t0_lease * 0.875 -> 87.5%)*/
        dhcp->offered.offered_t2_rebind = (dhcp->offered.offered_t0_lease * 7U) / 8U;
    }

    dhcp->offered.offered_ip_addr.addr = net::memcpy_ip(kResponse->yiaddr);

    if (dhcp->offered.offered_sn_mask.addr != 0)
    {
        dhcp->flags |= DHCP_FLAG_SUBNET_MASK_GIVEN;
    }
    else
    {
        dhcp->flags &= static_cast<uint8_t>(~DHCP_FLAG_SUBNET_MASK_GIVEN);
    }

    DEBUG_PRINTF("t0=%u, t1=%u, t2=%u", dhcp->offered.offered_t0_lease, dhcp->offered.offered_t1_renew, dhcp->offered.offered_t2_rebind);
    DEBUG_EXIT();
}

static void HandleNak()
{
    DEBUG_ENTRY();
    auto* dhcp = reinterpret_cast<struct dhcp::Dhcp*>(netif::globals::netif_default.dhcp);

    // Change to a defined state - set this before assigning the address
    // to ensure the callback can use dhcp_supplied_address()
    SetState(dhcp, dhcp::State::STATE_BACKING_OFF);
    // remove IP address from interface (must no longer be used, as per RFC2131)
    ip4_addr_t any;
    any.addr = 0;
    netif::SetAddr(any, any, any);
    // We can immediately restart discovery
    Discover();

    DEBUG_EXIT();
}

bool Start()
{
    DEBUG_ENTRY();
    auto* dhcp = reinterpret_cast<struct dhcp::Dhcp*>(netif::globals::netif_default.dhcp);

    if (dhcp == nullptr)
    {
        dhcp = new (struct dhcp::Dhcp);
        assert(dhcp != nullptr);
        netif::globals::netif_default.dhcp = dhcp;

        s_timer_id = SoftwareTimerAdd(dhcp::kFineTimerMsecs, FineTmr);
        assert(s_timer_id >= 0);
    }

    std::memset(dhcp, 0, sizeof(struct dhcp::Dhcp));
    dhcp->handle = net::udp::Begin(net::iana::IANA_PORT_DHCP_CLIENT, dhcp::Input);

#ifndef NDEBUG
    if (dhcp->handle < 0)
    {
        console::Error("DHCP Start");
        DEBUG_EXIT();
        return false;
    }
#endif

    MessageInit();

#if defined(CONFIG_NET_DHCP_USE_ACD)
    net::acd::Add(&dhcp->acd, ConflictCallback);
#endif

    if (!netif::IsLinkUp())
    {
        SetState(dhcp, dhcp::State::STATE_INIT);
        return false;
    }

    Discover();

    DEBUG_EXIT();
    return true;
}

void ReleaseAndStop()
{
    DEBUG_ENTRY();
    auto* dhcp = reinterpret_cast<struct dhcp::Dhcp*>(netif::globals::netif_default.dhcp);

    if (dhcp == nullptr)
    {
        DEBUG_EXIT();
        return;
    }

    if (dhcp->state == dhcp::State::STATE_OFF)
    {
        return;
    }

    ip4_addr_t server_ip_addr;

    server_ip_addr.addr = dhcp->server_ip_addr.addr;

    /* clean old DHCP offer */
    dhcp->server_ip_addr.addr = 0;
    std::memset(&dhcp->offered, 0, sizeof(struct dhcp::Dhcp::Offered));
    dhcp->t1_renew_time = dhcp->t2_rebind_time = dhcp->lease_used = dhcp->t0_timeout = 0;

    if (dhcp::SuppliedAddress())
    {
        SetState(dhcp, dhcp::State::STATE_OFF);

        SendRelease(server_ip_addr.addr);

        net::udp::End(net::iana::IANA_PORT_DHCP_CLIENT);

        /* remove IP address from interface (prevents routing from selecting this interface) */
        ip4_addr_t any;
        any.addr = 0;
        netif::SetAddr(any, any, any);
    }

#if defined(CONFIG_NET_DHCP_USE_ACD)
    acd::Remove(&dhcp->acd);
#endif

    delete reinterpret_cast<struct dhcp::Dhcp*>(netif::globals::netif_default.dhcp);
    netif::globals::netif_default.dhcp = nullptr;
    netif::ClearFlags(netif::Netif::kNetifFlagDhcpOk);
}

void NetworkChangedLinkUp()
{
    DEBUG_ENTRY();
    auto* dhcp = reinterpret_cast<struct dhcp::Dhcp*>(netif::globals::netif_default.dhcp);

    if (dhcp == nullptr)
    {
        DEBUG_EXIT();
        return;
    }

    switch (dhcp->state)
    {
        case dhcp::State::STATE_REBINDING:
        case dhcp::State::STATE_RENEWING:
        case dhcp::State::STATE_BOUND:
        case dhcp::State::STATE_REBOOTING:
            dhcp->tries = 0;
            Reboot();
            break;
        case dhcp::State::STATE_OFF:
            break;
        default:
            dhcp->tries = 0;
            Discover();
            break;
    }
}

void Process(const dhcp::Message* const kResponse, uint32_t size)
{
    DEBUG_ENTRY();

    assert(kResponse != nullptr);

    uint8_t msg_type = 0;
    uint8_t opt_len = 0;

    auto* dhcp = reinterpret_cast<struct dhcp::Dhcp*>(netif::globals::netif_default.dhcp);
    assert(dhcp != nullptr);

    std::memset(&dhcp->offered, 0, sizeof(struct dhcp::Dhcp::Offered));

    dhcp->server_ip_addr.addr = 0;

    const auto* p = reinterpret_cast<const uint8_t*>(kResponse);
    p = p + sizeof(dhcp::Message) - dhcp::OPT_SIZE + 4;
    auto* e = p + size;

    while (p < e)
    {
        switch (*p)
        {
            case dhcp::Options::OPTION_END:
                p = e;
                break;
            case dhcp::Options::OPTION_PAD_OPTION:
                p++;
                break;
            case dhcp::Options::OPTION_MESSAGE_TYPE:
                p++;
                p++;
                msg_type = *p++;
                break;
            case dhcp::Options::OPTION_SUBNET_MASK:
                p++;
                p++;
                dhcp->offered.offered_sn_mask.addr = net::memcpy_ip(p);
                p = p + 4;
                break;
            case dhcp::Options::OPTION_ROUTER:
                p++;
                p++;
                dhcp->offered.offered_gw_addr.addr = net::memcpy_ip(p);
                p = p + 4;
                break;
            case dhcp::Options::OPTION_SERVER_IDENTIFIER:
                p++;
                p++;
                dhcp->server_ip_addr.addr = net::memcpy_ip(p);
                p += 4;
                break;
            case dhcp::Options::OPTION_LEASE_TIME:
                p++;
                p++;
                dhcp->offered.offered_t0_lease = __builtin_bswap32(net::memcpy_ip(p));
                p += 4;
                break;
            case dhcp::Options::OPTION_DHCP_T1_VALUE:
                p++;
                p++;
                dhcp->offered.offered_t1_renew = __builtin_bswap32(net::memcpy_ip(p));
                p += 4;
                break;
            case dhcp::Options::OPTION_DHCP_T2_VALUE:
                p++;
                p++;
                dhcp->offered.offered_t2_rebind = __builtin_bswap32(net::memcpy_ip(p));
                p += 4;
                break;
            default:
                p++;
                opt_len = *p++;
                p += opt_len;
                break;
        }
    }

    if (msg_type == dhcp::Type::OFFER)
    {
        dhcp->offered.offered_ip_addr.addr = net::memcpy_ip(kResponse->yiaddr);
    }

    DEBUG_PRINTF("msg_type=%u", msg_type);

    if (msg_type == dhcp::Type::ACK)
    {
        // in requesting state or just reconnected to the network?
        if ((dhcp->state == dhcp::State::STATE_REQUESTING) || (dhcp->state == dhcp::State::STATE_REBOOTING))
        {
            HandleAck(kResponse);
#if defined(CONFIG_NET_DHCP_USE_ACD)
            Check();
#else
            Bind();
#endif
        }
        else if ((dhcp->state == dhcp::State::STATE_REBINDING) || (dhcp->state == dhcp::State::STATE_RENEWING))
        {
            HandleAck(kResponse);
            Bind();
        }
    }
    else if ((msg_type == dhcp::Type::NAK) && ((dhcp->state == dhcp::State::STATE_REBOOTING) || (dhcp->state == dhcp::State::STATE_REQUESTING) ||
                                               (dhcp->state == dhcp::State::STATE_REBINDING) || (dhcp->state == dhcp::State::STATE_RENEWING)))
    {
        HandleNak();
    }
    else if ((msg_type == dhcp::Type::OFFER) && (dhcp->state == dhcp::State::STATE_SELECTING))
    {
        HandleOffer(kResponse);
    }

    DEBUG_EXIT();
}
} // namespace net::dhcp
