/**
 * @file network.cpp
 *
 */
/* Copyright (C) 2021-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#undef NDEBUG

#include "displayudf.h"
#include "net/protocol/dhcp.h"

void network_display_emac_config() {
	DisplayUdf::Get()->ShowEmacInit();
}

void network_display_emac_start() {
	DisplayUdf::Get()->ShowEmacStart();
}

void network_display_emac_status(const bool isLinkUp) {
	DisplayUdf::Get()->ShowEmacStatus(isLinkUp);
}

void network_display_netif_up() {
	DisplayUdf::Get()->ShowIpAddress();
}

void network_display_netif_down() {
	DisplayUdf::Get()->ShowEmacStatus(false);
}

void network_display_ip() {
	DisplayUdf::Get()->ShowIpAddress();
}

void network_display_netmask() {
	DisplayUdf::Get()->ShowNetmask();
}

void network_display_gateway() {
	DisplayUdf::Get()->ShowGatewayIp();
}

void network_display_hostname() {
	DisplayUdf::Get()->ShowHostName();
}

void network_display_emac_shutdown() {
	DisplayUdf::Get()->ShowShutdown();
}

void network_display_dhcp_status(net::dhcp::State state) {
	DisplayUdf::Get()->ShowDhcpStatus(state);
}
