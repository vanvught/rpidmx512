/**
 * @file autoip.h
 */
/* Copyright (C) 2024-2026 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef CORE_IP4_AUTOIP_H_
#define CORE_IP4_AUTOIP_H_

#include <cstdint>

#include "core/ip4/acd.h"
#include "core/protocol/autoip.h"

namespace network::autoip
{
struct Autoip
{
    ip4_addr_t llipaddr;
    State state;
    uint8_t tried_llipaddr;
    acd::Acd acd;
};

void Start();
void Stop();

bool SuppliedAddress();

void NetworkChangedLinkUp();
void NetworkChangedLinkDown();
} // namespace network::autoip

#endif // CORE_IP4_AUTOIP_H_
