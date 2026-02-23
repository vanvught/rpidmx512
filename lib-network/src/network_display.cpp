#if !defined(DISPLAY_UDF)
/**
 * @file network_display.cpp
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
#include "core/protocol/dhcp.h"

#if !defined(CONFIG_DISPLAY_LINE_IP)
static constexpr uint32_t LINE_IP = 2;
#else
static constexpr uint32_t LINE_IP = CONFIG_DISPLAY_LINE_IP;
#endif

namespace net::emac::display
{
void __attribute__((weak)) Config()
{
#if !defined(NO_EMAC)
    Display::Get()->ClearLine(LINE_IP);
    Display::Get()->PutString("Ethernet config");
#endif
}

void __attribute__((weak)) Start()
{
#if !defined(NO_EMAC)
    Display::Get()->ClearLine(LINE_IP);
    Display::Get()->PutString("Ethernet start");
#endif
}

void __attribute__((weak)) Status([[maybe_unused]] bool isLinkUp)
{
#if !defined(NO_EMAC)
    Display::Get()->ClearLine(LINE_IP);
    Display::Get()->PutString("Ethernet Link ");
    if (isLinkUp)
    {
        Display::Get()->PutString("UP");
    }
    else
    {
        Display::Get()->PutString("DOWN");
    }
#endif
}
} // namespace net::emac::display

namespace network::display
{
void __attribute__((weak)) Hostname() {}

void __attribute__((weak)) EmacShutdown()
{
#if !defined(NO_EMAC)
    Display::Get()->ClearLine(LINE_IP);
    Display::Get()->PutString("Ethernet shutdown");
#endif
}

void __attribute__((weak)) DhcpStatus([[maybe_unused]] network::dhcp::State state)
{
#if !defined(NO_EMAC)
    Display::Get()->ClearLine(LINE_IP);

    switch (state)
    {
        case network::dhcp::State::kOff:
            break;
        case network::dhcp::State::kRenewing:
            Display::Get()->PutString("DHCP renewing");
            break;
        case network::dhcp::State::kBound:
            Display::Get()->PutString("Got IP");
            break;
        case network::dhcp::State::kRequesting:
            Display::Get()->PutString("DHCP requesting");
            break;
        case network::dhcp::State::kBackingOff:
            Display::Get()->PutString("DHCP Error");
            break;
        default:
            break;
    }
#endif
}
} // namespace network::display
#endif