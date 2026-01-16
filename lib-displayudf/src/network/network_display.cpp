/**
 * @file network_display.cpp
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

#include "displayudf.h"
#include "core/protocol/dhcp.h"

namespace net::emac::display
{
void Config()
{
    DisplayUdf::Get()->ShowEmacInit();
}

void Start()
{
    DisplayUdf::Get()->ShowEmacStart();
}
void Status(bool is_link_up)
{
    DisplayUdf::Get()->ShowEmacStatus(is_link_up);
}
} // namespace net::emac::display

namespace network::display
{
void Hostname()
{
    DisplayUdf::Get()->ShowHostName();
}

void EmacShutdown()
{
    DisplayUdf::Get()->ShowShutdown();
}

void DhcpStatus(network::dhcp::State state)
{
    DisplayUdf::Get()->ShowDhcpStatus(state);
}
} // namespace network::display

namespace network::event
{
void LinkUp()
{
    DisplayUdf::Get()->ShowIpAddress();
}

void LinkDown()
{
    DisplayUdf::Get()->ShowEmacStatus(false);
}

void Ipv4AddressChanged()
{
    DisplayUdf::Get()->ShowIpAddress();
}

void Ipv4NetmaskChanged()
{
    DisplayUdf::Get()->ShowNetmask();
}

void Ipv4GatewayChanged()
{
    DisplayUdf::Get()->ShowGatewayIp();
}
} // namespace network::event
