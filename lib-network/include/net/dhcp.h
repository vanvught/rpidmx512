/**
 * @file dhcp.h
 *
 */
/* Copyright (C) 2018-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef NET_DHCP_H_
#define NET_DHCP_H_

#include <cstdint>
#include <cassert>

#include <cstdio>

#include "netif.h"
#include "acd.h"
#include "ip4_address.h"
#include "net/protocol/dhcp.h"
#include "net/protocol/iana.h"

#include "debug.h"

namespace net {
namespace dhcp {
static constexpr uint32_t DHCP_COARSE_TIMER_SECS = 60;
/** period (in milliseconds) of the application calling dhcp_coarse_tmr() */
static constexpr uint32_t DHCP_COARSE_TIMER_MSECS = (DHCP_COARSE_TIMER_SECS * 1000UL);
/** period (in milliseconds) of the application calling dhcp_fine_tmr() */
static constexpr uint32_t DHCP_FINE_TIMER_MSECS =  500;

#define DHCP_FLAG_SUBNET_MASK_GIVEN 0x01
#define DHCP_AUTOIP_COOP_TRIES     9

enum class AutoipCoopState {
  DHCP_AUTOIP_COOP_STATE_OFF  = 0,
  DHCP_AUTOIP_COOP_STATE_ON   = 1
};

typedef uint16_t dhcp_timeout_t;

struct Dhcp {
	int32_t nHandle;
	uint32_t xid;
	State state;
	uint8_t tries;
	uint8_t flags;

	dhcp_timeout_t request_timeout; /* #ticks with period DHCP_FINE_TIMER_SECS for request timeout */
	dhcp_timeout_t t1_timeout;  /* #ticks with period DHCP_COARSE_TIMER_SECS for renewal time */
	dhcp_timeout_t t2_timeout;  /* #ticks with period DHCP_COARSE_TIMER_SECS for rebind time */
	dhcp_timeout_t t1_renew_time;  /* #ticks with period DHCP_COARSE_TIMER_SECS until next renew try */
	dhcp_timeout_t t2_rebind_time; /* #ticks with period DHCP_COARSE_TIMER_SECS until next rebind try */
	dhcp_timeout_t lease_used; /* #ticks with period DHCP_COARSE_TIMER_SECS since last received DHCP ack */
	dhcp_timeout_t t0_timeout; /* #ticks with period DHCP_COARSE_TIMER_SECS for lease time */

	ip4_addr_t server_ip_addr;

	struct Offered {
		ip4_addr_t offered_ip_addr;
		ip4_addr_t offered_sn_mask;
		ip4_addr_t offered_gw_addr;

		uint32_t offered_t0_lease; /* lease period (in seconds) */
		uint32_t offered_t1_renew; /* recommended renew time (usually 50% of lease period) */
		uint32_t offered_t2_rebind; /* recommended rebind time (usually 87.5 of lease period)  */
	};

	Offered offered;

	acd::Acd acd;
};
}  // namespace dhcp

bool dhcp_start();
bool dhcp_renew();
bool dhcp_release();
void dhcp_stop();
void dhcp_release_and_stop();
void dhcp_inform();
void dhcp_network_changed_link_up();

uint32_t udp_recv2(int, const uint8_t **, uint32_t *, uint16_t *);
void dhcp_process(const dhcp::Message *const, const uint32_t nSize);

inline void dhcp_run() {
	auto *dhcp = reinterpret_cast<struct dhcp::Dhcp *>(globals::netif_default.dhcp);
	if (dhcp == nullptr) {
		return;
	}

	uint8_t *pResponse;
	uint32_t nFromIp;
	uint16_t nFromPort;

	const auto nSize = udp_recv2(dhcp->nHandle, const_cast<const uint8_t **>(&pResponse), &nFromIp, &nFromPort);

	if (__builtin_expect((nSize > 0), 0)) {
		if (nFromPort == net::iana::IANA_PORT_DHCP_SERVER) {
			const auto *const p = reinterpret_cast<dhcp::Message *>(pResponse);

			if (p->xid != dhcp->xid) {
				DEBUG_PRINTF("pDhcpMessage->xid=%u, dhcp->xid=%u", p->xid, dhcp->xid);
				return;
			}

			dhcp_process(p, nSize);
		}
	}
}

inline bool dhcp_supplied_address() {
	const auto *dhcp = reinterpret_cast<struct dhcp::Dhcp *>(globals::netif_default.dhcp);
	if ((dhcp != nullptr)) {
		return (dhcp->state == dhcp::State::STATE_BOUND) || (dhcp->state == dhcp::State::STATE_RENEWING) || (dhcp->state == dhcp::State::STATE_REBINDING);
	}
	return false;
}
}  // namespace net

#endif /* NET_DHCP_H_ */
