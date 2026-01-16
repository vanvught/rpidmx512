/**
 * @file dhcp.h
 *
 */
/* Copyright (C) 2018-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef CORE_IP4_DHCP_H_
#define CORE_IP4_DHCP_H_

#include <cstdint>

#include "core/ip4/acd.h"
#include "ip4/ip4_address.h"
#include "core/protocol/dhcp.h"

namespace network::dhcp
{
static constexpr uint32_t kCoarseTimerSecs = 60;
// period (in milliseconds) of the application calling dhcp_coarse_tmr()
static constexpr uint32_t kCoarseTimerMsecs = (kCoarseTimerSecs * 1000UL);
// period (in milliseconds) of the application calling dhcp_fine_tmr()
static constexpr uint32_t kFineTimerMsecs = 500;

static constexpr uint8_t kFlagSubnetMaskGiven = 0x01;

using dhcp_timeout_t = uint16_t;

struct Dhcp
{
    int32_t handle;
    uint32_t xid;
    State state;
    uint8_t tries;
    uint8_t flags;

    dhcp_timeout_t request_timeout; // #ticks with period DHCP_FINE_TIMER_SECS for request timeout
    dhcp_timeout_t t1_timeout;      // #ticks with period DHCP_COARSE_TIMER_SECS for renewal time
    dhcp_timeout_t t2_timeout;      // #ticks with period DHCP_COARSE_TIMER_SECS for rebind time
    dhcp_timeout_t t1_renew_time;   // #ticks with period DHCP_COARSE_TIMER_SECS until next renew try
    dhcp_timeout_t t2_rebind_time;  // #ticks with period DHCP_COARSE_TIMER_SECS until next rebind try
    dhcp_timeout_t lease_used;      // #ticks with period DHCP_COARSE_TIMER_SECS since last received DHCP ack
    dhcp_timeout_t t0_timeout;      // #ticks with period DHCP_COARSE_TIMER_SECS for lease time

    ip4_addr_t server_ip_addr;

    struct Offered
    {
        ip4_addr_t offered_ip_addr;
        ip4_addr_t offered_sn_mask;
        ip4_addr_t offered_gw_addr;

        uint32_t offered_t0_lease;  // lease period (in seconds)
        uint32_t offered_t1_renew;  // recommended renew time (usually 50% of lease period)
        uint32_t offered_t2_rebind; // recommended rebind time (usually 87.5 of lease period)
    };

    Offered offered;

    acd::Acd acd;
};

bool Start();
bool Renew();
bool Release();
void Stop();
void ReleaseAndStop();
void Inform();
void NetworkChangedLinkUp();

bool SuppliedAddress();

void Process(const dhcp::Message* const, uint32_t size);
void Input(const uint8_t*, uint32_t, uint32_t, uint16_t);
} // namespace network::dhcp

#endif // CORE_IP4_DHCP_H_
