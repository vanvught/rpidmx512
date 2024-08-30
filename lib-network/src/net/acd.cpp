/**
 * @file acd.cpp
 */
/* Copyright (C) 2024 by Arjan van Vught mailto:info@gd32-dmx.org
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
 * The acp.cpp aims to be conform to RFC 5227.
 * https://datatracker.ietf.org/doc/html/rfc5227.html
 * IPv4 Address Conflict Detection
 */

#if defined (DEBUG_NET_ACD)
# undef NDEBUG
#endif

#pragma GCC push_options
#pragma GCC optimize ("Os")

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cassert>

#include "../../config/net_config.h"

#include "net/acd.h"
#include "net/protocol/acd.h"
#include "net/protocol/arp.h"
#include "net_memcpy.h"

#include "hardware.h"
#include "debug.h"

namespace net {
namespace acd {
static constexpr uint32_t ACD_TMR_INTERVAL = 100;
static constexpr uint32_t ACD_TICKS_PER_SECOND = (1000U / ACD_TMR_INTERVAL);
}  // namespace acd

static int32_t nTimerId;

static void acd_timer() {
	auto *acd = reinterpret_cast<struct acd::Acd *>(globals::netif_default.acd);
	assert(acd != nullptr);

	if (acd->lastconflict > 0) {
		acd->lastconflict--;
	}

	DEBUG_PRINTF("state=%u, ttw=%u", static_cast<uint32_t>(acd->state), acd->ttw);

	if (acd->ttw > 0) {
		acd->ttw--;
	}

	switch (acd->state) {
	case acd::State::ACD_STATE_PROBE_WAIT:
	case acd::State::ACD_STATE_PROBING:
		if (acd->ttw == 0) {
			acd->state = acd::State::ACD_STATE_PROBING;
			arp_acd_probe(acd->ipaddr);
			DEBUG_PUTS("PROBING Sent Probe");
			acd->sent_num++;
			if (acd->sent_num >= PROBE_NUM) {
				acd->state = acd::State::ACD_STATE_ANNOUNCE_WAIT;
				acd->sent_num = 0;
				acd->ttw = static_cast<uint16_t>(ANNOUNCE_WAIT * acd::ACD_TICKS_PER_SECOND);
			} else {
				acd->ttw = static_cast<uint16_t>(random() % (((PROBE_MAX - PROBE_MIN) * acd::ACD_TICKS_PER_SECOND)) + (PROBE_MIN * acd::ACD_TICKS_PER_SECOND));
			}
		}
		break;
	case acd::State::ACD_STATE_ANNOUNCE_WAIT:
	case acd::State::ACD_STATE_ANNOUNCING:
		if (acd->ttw == 0) {
			if (acd->sent_num == 0) {
				acd->state = acd::State::ACD_STATE_ANNOUNCING;
				acd->num_conflicts = 0;
			}
			arp_acd_send_announcement(acd->ipaddr);
			DEBUG_PUTS("ANNOUNCING Sent Announce");
			acd->ttw = static_cast<uint16_t>(ANNOUNCE_INTERVAL * acd::ACD_TICKS_PER_SECOND);
			acd->sent_num++;

			if (acd->sent_num >= ANNOUNCE_NUM) {
				acd->state = acd::State::ACD_STATE_ONGOING;
				acd->sent_num = 0;
				acd->ttw = 0;
				acd->acd_conflict_callback(acd::Callback::ACD_IP_OK);
			}
		}
		break;
	case acd::State::ACD_STATE_RATE_LIMIT:
		if (acd->ttw == 0) {
			acd_stop(acd);
			acd->acd_conflict_callback(acd::Callback::ACD_RESTART_CLIENT);
		}
		break;
	default:
		break;
	}
}

static void acd_restart(struct acd::Acd *acd) {
	acd->num_conflicts++;
	acd->acd_conflict_callback(acd::Callback::ACD_DECLINE);

	if (acd->num_conflicts >= MAX_CONFLICTS) {
		acd->state = acd::State::ACD_STATE_RATE_LIMIT;
		acd->ttw = static_cast<uint16_t>(RATE_LIMIT_INTERVAL * acd::ACD_TICKS_PER_SECOND);
		DEBUG_PUTS("rate limiting initiated. too many conflicts");
	}
	else {
		acd_stop(acd);
		acd->acd_conflict_callback(acd::Callback::ACD_RESTART_CLIENT);
	}
}

static void acd_handle_arp_conflict(struct acd::Acd *acd) {
	/* RFC5227, 2.4 "Ongoing Address Conflict Detection and Address Defense"
     allows three options where:
     a) means retreat on the first conflict,
     b) allows to keep an already configured address when having only one
        conflict in DEFEND_INTERVAL seconds and
     c) the host will not give up it's address and defend it indefinitely

     We use option b) when the acd module represents the netif address, since it
     helps to improve the chance that one of the two conflicting hosts may be
     able to retain its address. while we are flexible enough to help network
     performance

     We use option a) when the acd module does not represent the netif address,
     since we cannot have the acd module announcing or restarting. This
     situation occurs for the LL acd module when a routable address is used on
     the netif but the LL address is still open in the background. */

	if (acd->state == acd::State::ACD_STATE_PASSIVE_ONGOING) {
		DEBUG_PUTS("conflict when we are in passive mode -> back off");
		acd_stop(acd);
		acd->acd_conflict_callback(acd::Callback::ACD_DECLINE);
	}
	else {
		if (acd->lastconflict > 0) {
			DEBUG_PUTS("conflict within DEFEND_INTERVAL -> retreating");
			acd_restart(acd);
		} else {
			DEBUG_PUTS("we are defending, send ARP Announce");
			arp_acd_send_announcement(acd->ipaddr);
			acd->lastconflict = DEFEND_INTERVAL * acd::ACD_TICKS_PER_SECOND;
		}
	}
}

static void acd_put_in_passive_mode() {
	auto *acd = reinterpret_cast<struct acd::Acd *>(globals::netif_default.acd);
	assert(acd != nullptr);

	switch (acd->state) {
	case acd::State::ACD_STATE_OFF:
	case acd::State::ACD_STATE_PASSIVE_ONGOING:
	default:
		/* do nothing */
		break;
	case acd::State::ACD_STATE_PROBE_WAIT:
	case acd::State::ACD_STATE_PROBING:
	case acd::State::ACD_STATE_ANNOUNCE_WAIT:
	case acd::State::ACD_STATE_RATE_LIMIT:
		acd_stop(acd);
		acd->acd_conflict_callback(acd::Callback::ACD_DECLINE);
		break;
	case acd::State::ACD_STATE_ANNOUNCING:
	case acd::State::ACD_STATE_ONGOING:
		acd->state = acd::State::ACD_STATE_PASSIVE_ONGOING;
		break;
	}
}

/*
 * Public interface
 */

void acd_start(struct acd::Acd *acd, const ip4_addr_t ipaddr) {
	DEBUG_ENTRY
	assert(acd != nullptr);

	acd->ipaddr.addr = ipaddr.addr;
	acd->state = acd::State::ACD_STATE_PROBE_WAIT;
	acd->ttw = static_cast<uint16_t>(random() % (PROBE_WAIT * acd::ACD_TICKS_PER_SECOND));

	nTimerId = Hardware::Get()->SoftwareTimerAdd(acd::ACD_TMR_INTERVAL, acd_timer);
	assert(nTimerId >= 0);

	DEBUG_EXIT
}

void acd_stop(struct acd::Acd *acd) {
	DEBUG_ENTRY
	assert(acd != nullptr);

	acd->state = acd::State::ACD_STATE_OFF;

	assert(nTimerId >= 0);
	Hardware::Get()->SoftwareTimerDelete(nTimerId);
	nTimerId = -1;

	DEBUG_EXIT
}

void acd_network_changed_link_down() {
	DEBUG_ENTRY

	auto *acd = reinterpret_cast<struct acd::Acd *>(globals::netif_default.acd);
    acd_stop(acd);

	DEBUG_EXIT
}

void acd_arp_reply(struct t_arp *pArp) {
	DEBUG_ENTRY
	auto *acd = reinterpret_cast<struct acd::Acd *>(globals::netif_default.acd);

	switch (acd->state) {
	case acd::State::ACD_STATE_OFF:
	case acd::State::ACD_STATE_RATE_LIMIT:
	default:
		break;
	case acd::State::ACD_STATE_PROBE_WAIT:
	case acd::State::ACD_STATE_PROBING:
	case acd::State::ACD_STATE_ANNOUNCE_WAIT:
		/* RFC 5227 Section 2.1.1:
		 * from beginning to after ANNOUNCE_WAIT seconds we have a conflict if
		 * ip.sender == ipaddr (someone is already using the address)
		 * OR
		 * ip.dst == ipaddr && hw.src != own macAddress (someone else is probing it)
		 */
		if (((memcpy_ip(pArp->arp.sender_ip) == acd->ipaddr.addr))
				|| (!(memcpy_ip(pArp->arp.sender_ip) == 0)
						&& ((memcpy_ip(pArp->arp.target_ip)) == acd->ipaddr.addr)
						&& (memcmp(pArp->arp.sender_mac, globals::netif_default.hwaddr, ETH_ADDR_LEN) == 0)))
		{
			DEBUG_PUTS("Probe Conflict detected");
			acd_restart(acd);
		}
		break;
	case acd::State::ACD_STATE_ANNOUNCING:
	case acd::State::ACD_STATE_ONGOING:
	case acd::State::ACD_STATE_PASSIVE_ONGOING:
		/* RFC 5227 Section 2.4:
		 * in any state we have a conflict if
		 * ip.sender == ipaddr && hw.src != own macAddress (someone is using our address)
		 */
		if ((memcpy_ip(pArp->arp.sender_ip) == acd->ipaddr.addr)
				&& (memcmp(pArp->arp.sender_mac, globals::netif_default.hwaddr, ETH_ADDR_LEN) != 0)) {
			DEBUG_PUTS("Conflicting ARP-Packet detected");
			acd_handle_arp_conflict(acd);
		}
		break;
	}

	DEBUG_EXIT
}

void acd_add(struct acd::Acd *pAcd, acd_conflict_callback_t acd_conflict_callback) {
	DEBUG_ENTRY
	assert(pAcd != nullptr);
	assert(acd_conflict_callback != nullptr);

	pAcd->acd_conflict_callback = acd_conflict_callback;

	auto &netif = globals::netif_default;
	netif.acd = pAcd;

	DEBUG_EXIT
}

void acd_remove(struct acd::Acd *pAcd) {
	DEBUG_ENTRY

	assert(pAcd != nullptr);
	auto &netif = globals::netif_default;

	if (netif.acd == pAcd) {
		netif.acd = nullptr;

		DEBUG_EXIT
		return;
	}

	DEBUG_EXIT
}

void acd_netif_ip_addr_changed(const ip4_addr_t old_addr, const ip4_addr_t new_addr) {
	if ((old_addr.addr == 0) || (new_addr.addr == 0)) {
		return;
	}

	auto *acd = reinterpret_cast<struct acd::Acd*>(globals::netif_default.acd);

	if (acd->ipaddr.addr == old_addr.addr) {
		/* Did we change from a LL address to a routable address? */
		if (network::is_linklocal_ip(old_addr.addr) && !network::is_linklocal_ip(new_addr.addr)) {
			/* Put the module in passive conflict detection mode */
			acd_put_in_passive_mode();
		}
	}
}
}  // namespace net
