/**
 * NetworkInit.cpp
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

#ifdef DEBUG_NETWORK
#undef NDEBUG
#endif

#include <cstdio>

#include "common/utils/utils_flags.h"
#include "configstore.h"
#include "configurationstore.h"
#include "emac/emac.h"
#include "emac/phy.h"
#include "emac/net_link_check.h"
#include "emac/network.h"
#include "net/netif.h"
#if defined(CONFIG_NET_ENABLE_NTP_CLIENT) || defined(CONFIG_NET_ENABLE_PTP_NTP_CLIENT)
#include "net/apps/ntpclient.h"
#endif
#if !defined(CONFIG_NET_APPS_NO_MDNS)
#include "net/apps/mdns.h"
#endif
#include "network_display.h"
#include "network_event.h"
#include "../../config/net_config.h"
#include "firmware/debug/debug_debug.h"

using common::store::network::Flags;

namespace net
{
void NetInit();
} // namespace net
namespace global::network
{
net::phy::Link linkState;
} // namespace global::network

namespace network
{
static void NetifExtCallback(uint16_t reason, [[maybe_unused]] const netif::netif_ext_callback_args_t* args)
{
    DEBUG_ENTRY();

    if ((reason & netif::NetifReason::kIpv4AddressChanged) == netif::NetifReason::kIpv4AddressChanged)
    {
        printf("ip: " IPSTR " -> " IPSTR "\n", IP2STR(args->ipv4_changed.old_address.addr), IP2STR(netif::IpAddr()));

        network::event::Ipv4AddressChanged();
#if defined(CONFIG_NET_ENABLE_NTP_CLIENT)
        ntpclient::Start();
#endif
#if defined(CONFIG_NET_ENABLE_PTP_NTP_CLIENT)
        ntpclient::ptp::Start();
#endif
#if !defined(CONFIG_NET_APPS_NO_MDNS)
        mdns::Start();
#endif
    }

    if ((reason & netif::NetifReason::kIpv4NetmaskChanged) == netif::NetifReason::kIpv4NetmaskChanged)
    {
        printf("netmask: " IPSTR " -> " IPSTR "\n", IP2STR(args->ipv4_changed.old_netmask.addr), IP2STR(netif::Netmask()));

        network::event::Ipv4NetmaskChanged();
    }

    if ((reason & netif::NetifReason::kIpv4GatewayChanged) == netif::NetifReason::kIpv4GatewayChanged)
    {
        printf("gw: " IPSTR " -> " IPSTR "\n", IP2STR(args->ipv4_changed.old_gw.addr), IP2STR(netif::Gw()));

        network::event::Ipv4GatewayChanged();
    }

    if ((reason & netif::NetifReason::kLinkChanged) == netif::NetifReason::kLinkChanged)
    {
        if (args->link_changed.state == 0)
        { // Link down
            network::event::LinkDown();
            DEBUG_EXIT();
            return;
        }

        network::event::LinkUp();
        DEBUG_EXIT();
    }

    DEBUG_EXIT();
}
void Init()
{
    DEBUG_ENTRY();

    net::emac::display::Config();

    net::emac::Config();

    net::phy::CustomizedTiming();
    net::phy::CustomizedLed();

    net::emac::display::Start();

    net::emac::Start(netif::globals::netif_default.hwaddr, global::network::linkState);
    printf(MACSTR "\n", MAC2STR(netif::globals::netif_default.hwaddr));

    net::emac::display::Status(net::phy::Link::kStateUp == global::network::linkState);

    net::NetInit();

    netif::Init();
    netif::AddExtCallback(NetifExtCallback);

    net::ip4_addr_t ipaddr;
    net::ip4_addr_t netmask;
    net::ip4_addr_t gw;

    common::store::Network store;
    ConfigStore::Instance().Copy(&store, &ConfigurationStore::network);

    network::iface::SetHostname(reinterpret_cast<char*>(store.host_name));

    ipaddr.addr = ConfigStore::Instance().NetworkGet(&common::store::Network::local_ip);
    netmask.addr = ConfigStore::Instance().NetworkGet(&common::store::Network::netmask);
    gw.addr = ConfigStore::Instance().NetworkGet(&common::store::Network::gateway_ip);

    const auto kFlags = ConfigStore::Instance().NetworkGet(&common::store::Network::flags);

    net::Set(ipaddr, netmask, gw, !common::IsFlagSet(kFlags, Flags::Flag::kUseStaticIp));

#if defined(ENET_LINK_CHECK_USE_INT)
    net::link_interrupt_init();
#elif defined(ENET_LINK_CHECK_USE_PIN_POLL)
    net::link_pin_poll_init();
#elif defined(ENET_LINK_CHECK_REG_POLL)
    net::link_status_read();
#endif
    DEBUG_EXIT();
}
} // namespace network
