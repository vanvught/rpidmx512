#if !defined(DISPLAY_UDF)
/**
 * @file network_event.cpp
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

#include <cstdint>

#include "display.h"
#include "network.h"
#include "ip4/ip4_address.h"
#include "network_display.h"
#if !defined(NO_EMAC)
#include "emac/emac.h"
#endif

#if !defined(CONFIG_DISPLAY_LINE_IP)
static constexpr uint32_t LINE_IP = 2;
#else
static constexpr uint32_t LINE_IP = CONFIG_DISPLAY_LINE_IP;
#endif

namespace network::event
{
void __attribute__((weak)) Ipv4AddressChanged()
{
#if !defined(NO_EMAC)
    Display::Get()->ClearLine(LINE_IP);
    Display::Get()->Printf(LINE_IP, "" IPSTR "/%d %c", IP2STR(network::GetPrimaryIp()), network::GetNetmaskCIDR(), network::iface::AddressingMode());
#endif
}

void __attribute__((weak)) Ipv4NetmaskChanged()
{
#if !defined(NO_EMAC)
    Ipv4AddressChanged();
#endif
}

void __attribute__((weak)) Ipv4GatewayChanged() {}

void __attribute__((weak)) LinkUp()
{
#if !defined(NO_EMAC)
    net::emac::display::Status(true);
#endif
}

void __attribute__((weak)) LinkDown()
{
#if !defined(NO_EMAC)
    net::emac::display::Status(false);
#endif
}
} // namespace network::event
#endif
