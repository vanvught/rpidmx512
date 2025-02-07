/**
 * network_init.cpp
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
# undef NDEBUG
#endif

#include "networkparams.h"

#include "emac/emac.h"
#include "emac/phy.h"
#include "emac/mmi.h"
#include "emac/net_link_check.h"

#include "net/netif.h"
#include "net/autoip.h"
#include "net/dhcp.h"
#if defined (CONFIG_NET_ENABLE_NTP_CLIENT) || defined (CONFIG_NET_ENABLE_PTP_NTP_CLIENT)
# include "net/apps/ntp_client.h"
#endif
#if !defined(CONFIG_NET_APPS_NO_MDNS)
# include "net/apps/mdns.h"
#endif

#include "network_display.h"

#include "../../config/net_config.h"

namespace net {
void net_init();
}  // namespace net

#include "debug.h"

static void netif_ext_callback(const uint16_t reason, [[maybe_unused]] const net::netif_ext_callback_args_t *args) {
	DEBUG_ENTRY

	if ((reason & net::NetifReason::NSC_IPV4_ADDRESS_CHANGED) == net::NetifReason::NSC_IPV4_ADDRESS_CHANGED) {
		printf("ip: " IPSTR " -> " IPSTR "\n", IP2STR(args->ipv4_changed.old_address.addr), IP2STR(net::netif_ipaddr()));

		network_display_ip();
#if defined (CONFIG_NET_ENABLE_NTP_CLIENT)
		ntp_client_start();
#endif
#if defined (CONFIG_NET_ENABLE_PTP_NTP_CLIENT)
		ptp_ntp_start();
#endif
#if !defined(CONFIG_NET_APPS_NO_MDNS)
		mdns_start();
#endif
	}

	if ((reason & net::NetifReason::NSC_IPV4_NETMASK_CHANGED) == net::NetifReason::NSC_IPV4_NETMASK_CHANGED) {
		printf("netmask: " IPSTR " -> " IPSTR "\n", IP2STR(args->ipv4_changed.old_netmask.addr), IP2STR(net::netif_netmask()));

		network_display_netmask();
	}

	if ((reason & net::NetifReason::NSC_IPV4_GATEWAY_CHANGED) == net::NetifReason::NSC_IPV4_GATEWAY_CHANGED) {
		printf("gw: " IPSTR " -> " IPSTR "\n", IP2STR(args->ipv4_changed.old_gw.addr), IP2STR(net::netif_gw()));

		network_display_gateway();
	}

	if ((reason & net::NetifReason::NSC_LINK_CHANGED) == net::NetifReason::NSC_LINK_CHANGED) {
		if (args->link_changed.state == 0) {	// Link down
			network_display_netif_down();
			DEBUG_EXIT
			return;
		}

		network_display_netif_up();
		DEBUG_EXIT
	}

	DEBUG_EXIT
}

namespace global::network {
net::Link linkState;
}  // namespace global::network

void network_init() {
	DEBUG_ENTRY

	network_display_emac_config();

	emac_config();

	network_display_emac_start();

	emac_start(net::globals::netif_default.hwaddr, global::network::linkState);
	printf(MACSTR "\n", MAC2STR(net::globals::netif_default.hwaddr));

	network_display_emac_status(net::Link::STATE_UP == global::network::linkState);

	net::phy_customized_timing();
	net::phy_customized_led();

	NetworkParams params;
	params.Load();

	net::net_init();

	net::netif_init();
	net::netif_add_ext_callback(netif_ext_callback);
	net::netif_set_hostname(params.GetHostName());

	net::ip4_addr_t ipaddr;
	net::ip4_addr_t netmask;
	net::ip4_addr_t gw;

	ipaddr.addr = params.GetIpAddress();
	netmask.addr = params.GetNetMask();
	gw.addr = params.GetDefaultGateway();

	net::netif_set(global::network::linkState, ipaddr, netmask, gw, params.isDhcpUsed());

#if defined (ENET_LINK_CHECK_USE_INT)
	net::link_interrupt_init();
#elif defined (ENET_LINK_CHECK_USE_PIN_POLL)
	net::link_pin_poll_init();
#elif defined (ENET_LINK_CHECK_REG_POLL)
	net::link_status_read();
#endif
	DEBUG_EXIT
}
