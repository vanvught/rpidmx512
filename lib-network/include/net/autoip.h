/**
 * @file autoip.h
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
 * The autoip.cpp aims to be conform to RFC 3927.
 * https://datatracker.ietf.org/doc/html/rfc3927
 * Dynamic Configuration of IPv4 Link-Local Addresses
 */

#ifndef NET_AUTOIP_H_
#define NET_AUTOIP_H_

#include <cstdint>

#include "net/netif.h"
#include "net/acd.h"
#include "net/protocol/autoip.h"
#include "firmware/debug/debug_debug.h"

namespace net::autoip
{
struct Autoip
{
    ip4_addr_t llipaddr;
    State state;
    uint8_t tried_llipaddr;
    net::acd::Acd acd;
};

void Start();
void Stop();

inline bool SuppliedAddress()
{
    const auto* autoip = reinterpret_cast<struct autoip::Autoip*>(netif::globals::netif_default.autoip);

    return (autoip != nullptr) && (netif::globals::netif_default.ip.addr == autoip->llipaddr.addr) && (autoip->state == autoip::State::AUTOIP_STATE_BOUND);
}

inline void NetworkChangedLinkUp()
{
    DEBUG_ENTRY();

    auto* autoip = reinterpret_cast<struct autoip::Autoip*>(netif::globals::netif_default.autoip);

    if ((autoip != nullptr) && (autoip->state != autoip::State::AUTOIP_STATE_OFF))
    {
        net::acd::Start(&autoip->acd, autoip->llipaddr);
    }

    DEBUG_EXIT();
}

inline void NetworkChangedLinkDown()
{
    DEBUG_EXIT();

    const auto* autoip = reinterpret_cast<struct autoip::Autoip*>(netif::globals::netif_default.autoip);

    if ((autoip != nullptr) && (autoip->state != autoip::State::AUTOIP_STATE_OFF))
    {
        autoip::Stop();
    }

    DEBUG_EXIT();
}
} // namespace net::autoip

#endif // NET_AUTOIP_H_
