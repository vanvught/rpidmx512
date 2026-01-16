/**
 * @file network.h
 *
 */
/* Copyright (C) 2017-2026 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef EMAC_NETWORK_H_
#define EMAC_NETWORK_H_

#if defined(NO_EMAC) || defined(ESP8266)
#error This file should not be included
#endif

#include <cstdint>

#include "../src/core/net_private.h"
#include "network_config.h" // IWYU pragma: keep
#include "network_iface.h"
#include "network_udp.h"  // IWYU pragma: keep
#include "network_igmp.h" // IWYU pragma: keep
#if defined(ENABLE_HTTPD)
#include "network_tcp.h" // IWYU pragma: keep
#endif
#include "emac/phy.h"
#if defined(ENET_LINK_CHECK_USE_PIN_POLL) || defined(ENET_LINK_CHECK_REG_POLL)
#include "emac/net_link_check.h"
#endif

uint32_t emac_eth_recv(uint8_t**);

namespace network
{
namespace global
{
extern net::phy::Link link_state;
}
void Init();

#if defined(CONFIG_NET_ENABLE_PTP)
namespace ptp
{
void Run();
}
#endif

inline void Run()
{
    uint8_t* ethernet_buffer;
    auto length = emac_eth_recv(&ethernet_buffer);

    if (__builtin_expect((length > 0), 0))
    {
        do
        {
            network::iface::EthernetInput(ethernet_buffer, length);
            length = emac_eth_recv(&ethernet_buffer);
        } while (length > 0);
    }
#if defined(ENABLE_HTTPD)
    network::tcp::Run();
#endif
#if defined(CONFIG_NET_ENABLE_PTP)
    network::ptp::Run();
#endif
#if defined(ENET_LINK_CHECK_USE_PIN_POLL)
    net::link::PinPoll();
#elif defined(ENET_LINK_CHECK_REG_POLL)
    const net::phy::Link link_state = net::link::StatusRead();
    if (link_state != global::link_state)
    {
        global::link_state = link_state;
        net::link::HandleChange(link_state);
    }
#endif
}
} // namespace network

#endif // EMAC_NETWORK_H_
