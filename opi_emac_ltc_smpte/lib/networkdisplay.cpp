/**
 * @file networkdisplay.cpp
 *
 */
/* Copyright (C) 2021-2022 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "network.h"

#include "display.h"
#include "display7segment.h"

namespace network {
void display_emac_start() {
	Display::Get()->ClearLine(3);
	Display::Get()->Printf(3, "Ethernet start");
}

void display_ip() {
	Display::Get()->ClearLine(3);
	Display::Get()->Printf(3, IPSTR "/%d %c", IP2STR(Network::Get()->GetIp()), static_cast<int>(Network::Get()->GetNetmaskCIDR()), Network::Get()->GetAddressingMode());
}

void display_netmask() {
	display_ip();
}

void display_gateway() {
}

void display_hostname() {
}

void display_emac_shutdown() {
}

// DHCP Client
void display_dhcp_status(network::dhcp::ClientStatus nStatus) {
	switch (nStatus) {
	case network::dhcp::ClientStatus::IDLE:
		break;
	case network::dhcp::ClientStatus::RENEW:
		Display::Get()->Status(Display7SegmentMessage::INFO_DHCP);
		Display::Get()->ClearLine(3);
		Display::Get()->Printf(3, "DHCP renewing");
		break;
	case network::dhcp::ClientStatus::GOT_IP:
		Display::Get()->Status(Display7SegmentMessage::INFO_NONE);
		break;
	case network::dhcp::ClientStatus::RETRYING:
		Display::Get()->Status(Display7SegmentMessage::INFO_DHCP);
		Display::Get()->ClearLine(3);
		Display::Get()->Printf(3, "DHCP retrying");
		break;
	case network::dhcp::ClientStatus::FAILED:
		Display::Get()->Status(Display7SegmentMessage::ERROR_DHCP);
		break;
	default:
		break;
	}
}
}  // namespace network
