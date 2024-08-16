/**
 * @file arp.cpp
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
/**
 * https://datatracker.ietf.org/doc/html/rfc826
 * An Ethernet Address Resolution Protocol
 *                -- or --
 * Converting Network Protocol Addresses
 */

#if defined (DEBUG_NET_ARP)
# undef NDEBUG
#endif

#if !defined (CONFIG_REMOTECONFIG_MINIMUM)
# pragma GCC push_options
# pragma GCC optimize ("O2")
# pragma GCC optimize ("no-tree-loop-distribute-patterns")
#endif

#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <cassert>

#include "../../config/net_config.h"

#include "net_memcpy.h"
#include "net_private.h"

#include "net/arp.h"
#include "net/acd.h"
#include "net/protocol/arp.h"
#include "net/protocol/udp.h"
#include "netif.h"

#include "hardware.h"

#include "debug.h"

#if !defined ARP_MAX_RECORDS
static constexpr auto MAX_RECORDS = 16;
#else
static constexpr auto MAX_RECORDS = ARP_MAX_RECORDS;
#endif

namespace net {
namespace globals {
extern uint32_t nOnNetworkMask;
}  // namespace globals

namespace arp {
static constexpr uint32_t TIMER_INTERVAL	= 1000;			///< 1 second
static constexpr uint32_t MAX_PROBING 		= 2;			///< 2 * 1 second
static constexpr uint32_t MAX_REACHABLE 	= (10 * 60);	///< (10 * 60) * 1 second = 10 minutes
static constexpr uint32_t MAX_STALE 		= ( 5 * 60);	///< ( 5 * 60) * 1 second =  5 minutes

enum class State {
	STATE_EMPTY, STATE_PROBE, STATE_REACHABLE, STATE_STALE,
};

struct Packet {
	uint8_t *p;
	uint32_t nSize;
#if defined CONFIG_ENET_ENABLE_PTP
	bool isTimestamp;
#endif
};

struct Record {
	uint32_t nIp;
	Packet packet;
	uint8_t mac_address[ETH_ADDR_LEN];
	uint16_t nAge;
	State state;
};
}  // namespace arp

static net::arp::Record s_ArpRecords[MAX_RECORDS] SECTION_NETWORK ALIGNED;

static struct t_arp s_arp_request ALIGNED ;
static struct t_arp s_arp_reply ALIGNED;

#ifndef NDEBUG
static constexpr char STATE[4][12] = { "EMPTY", "PROBE", "REACHABLE", "STALE", };

void static arp_cache_record_dump(net::arp::Record *pRecord) {
	printf("%p %-4d %c " MACSTR " %-10s " IPSTR  "\n", pRecord, pRecord->nAge, pRecord->packet.p == nullptr ? '-' : 'Q' , MAC2STR(pRecord->mac_address), STATE[static_cast<unsigned>(pRecord->state)], IP2STR(pRecord->nIp));
}

void static arp_cache_dump() {
	uint32_t nIndex = 0;
	for (auto &record : s_ArpRecords) {
		printf("%p %02d %-4d" MACSTR " %-10s " IPSTR  "\n", &record, nIndex++, record.nAge, MAC2STR(record.mac_address), STATE[static_cast<unsigned>(record.state)], IP2STR(record.nIp));
		if (nIndex ==6) {
			return;
		}
	}
}
#else
void static arp_cache_record_dump([[maybe_unused]] net::arp::Record *pRecord) {}
void static arp_cache_dump() {}
#endif

static net::arp::Record *arp_find_record(const uint32_t nDestinationIp, [[maybe_unused]] const arp::Flags flag) {
	DEBUG_ENTRY

	net::arp::Record *pStale = nullptr;
	net::arp::Record *pReachable = nullptr;
	uint32_t nAgeStale = 0;
	uint32_t nAgeReachable = 0;

	for (auto &record : s_ArpRecords) {
		if (record.nIp == nDestinationIp) {
			DEBUG_EXIT
			return &record;
		}

		if (flag == arp::Flags::FLAG_UPDATE) {
			continue;
		}

		if (record.state == net::arp::State::STATE_EMPTY) {
			record.nIp = nDestinationIp;
			DEBUG_EXIT
			return &record;
		}

		if (record.state == net::arp::State::STATE_REACHABLE) {
			if (record.nAge > nAgeReachable) {
				nAgeReachable = record.nAge;
				pReachable = &record;
			}
			continue;
		}


		if (record.state == net::arp::State::STATE_STALE) {
			if (record.nAge > nAgeStale) {
				nAgeStale = record.nAge;
				pStale = &record;
			}
			continue;
		}
	}

	if (pStale != nullptr) {
		DEBUG_EXIT
		return pStale;
	}

	if (pReachable != nullptr) {
		DEBUG_EXIT
		return pReachable;
	}

	DEBUG_EXIT
	return nullptr;
}

static void arp_cache_update(const uint8_t *pMacAddress, const uint32_t nIp, const arp::Flags flag) {
	DEBUG_ENTRY
	DEBUG_PRINTF(MACSTR " " IPSTR " flag=%d", MAC2STR(pMacAddress), IP2STR(nIp), flag);

	auto *record = arp_find_record(nIp, flag);

	if (record == nullptr) {
		assert(flag == arp::Flags::FLAG_UPDATE);
		DEBUG_EXIT
		return;
	}

	record->state = net::arp::State::STATE_REACHABLE;
	record->nAge = 0;
	std::memcpy(record->mac_address, pMacAddress, ETH_ADDR_LEN);

	arp_cache_record_dump(record);

	if (record->packet.p != nullptr) {
		auto *udp = reinterpret_cast<struct t_udp *>(record->packet.p);
		std::memcpy(udp->ether.dst, record->mac_address, ETH_ADDR_LEN);
		udp->ip4.chksum = 0;
#if !defined (CHECKSUM_BY_HARDWARE)
		udp->ip4.chksum = net_chksum(reinterpret_cast<void *>(&udp->ip4), sizeof(udp->ip4));
#endif
#if defined CONFIG_ENET_ENABLE_PTP
		if (!record->packet.isTimestamp) {
#endif
			emac_eth_send(record->packet.p, record->packet.nSize);
#if defined CONFIG_ENET_ENABLE_PTP
		} else {
			emac_eth_send_timestamp(record->packet.p, record->packet.nSize);
		}
#endif
		delete [] record->packet.p;
		record->packet.p = nullptr;
	}

	DEBUG_EXIT
}

static void arp_send_request(const uint32_t nIp) {
	DEBUG_PRINTF(IPSTR, IP2STR(nIp));

	net::memcpy_ip(s_arp_request.arp.target_ip, nIp);

	emac_eth_send(reinterpret_cast<void *>(&s_arp_request), sizeof(struct t_arp));
}

template<net::arp::EthSend S>
static void arp_query(const uint32_t nDestinationIp, struct t_udp *pPacket, const uint32_t nSize, [[maybe_unused]] const arp::Flags flag) {
	DEBUG_ENTRY
	DEBUG_PRINTF(IPSTR " %c", IP2STR(nDestinationIp), flag == arp::Flags::FLAG_UPDATE ? 'U' : 'I');

	auto *recordFound = arp_find_record(nDestinationIp, flag);
	assert(recordFound != nullptr);

	arp_cache_record_dump(recordFound);

	if (recordFound->state == net::arp::State::STATE_EMPTY) {
		recordFound->state = net::arp::State::STATE_PROBE;
		recordFound->nAge = 0;
		arp_send_request(nDestinationIp);
	}

	if (recordFound->state == net::arp::State::STATE_PROBE) {
		if (recordFound->packet.p != nullptr) {
			delete[] recordFound->packet.p;
		}

		recordFound->packet.p = new uint8_t[nSize];
		assert(recordFound->packet.p != nullptr);

		net::memcpy(recordFound->packet.p, pPacket, nSize);
		recordFound->packet.nSize = nSize;
#if defined CONFIG_ENET_ENABLE_PTP
		recordFound->packet.isTimestamp = (S != net::arp::EthSend::IS_NORMAL);
#endif
	}

	DEBUG_EXIT
}

static void arp_cache_clean_record(net::arp::Record& record) {
	if (record.packet.p != nullptr) {
		delete[] record.packet.p;
	}
	memset(&record, 0, sizeof(struct net::arp::Record));
}

static void arp_send_request_unicast(const uint32_t nIp, const uint8_t *pMacAddress) {
	DEBUG_PRINTF(IPSTR, IP2STR(nIp));

	net::memcpy(s_arp_request.ether.dst, pMacAddress , ETH_ADDR_LEN);
	net::memcpy_ip(s_arp_request.arp.target_ip, nIp);

	emac_eth_send(reinterpret_cast<void *>(&s_arp_request), sizeof(struct t_arp));

	memset(s_arp_request.ether.dst, 0xFF , ETH_ADDR_LEN);
}

static void arp_timer() {
	for (auto &record : s_ArpRecords) {
		const auto state = record.state;
		if (state != net::arp::State::STATE_EMPTY) {
			record.nAge++;

			switch (state) {
			case net::arp::State::STATE_PROBE:
				if (record.nAge > net::arp::MAX_PROBING) {
					arp_cache_clean_record(record);
				}
				break;

			case net::arp::State::STATE_REACHABLE:
				if (record.nAge > net::arp::MAX_REACHABLE) {
					record.state = net::arp::State::STATE_STALE;
					record.nAge = 0;
				}
				break;

			case net::arp::State::STATE_STALE:
				if (record.nAge > net::arp::MAX_STALE) {
					record.state = net::arp::State::STATE_PROBE;
					arp_send_request_unicast(record.nIp, record.mac_address);
				}
				break;

			default:
				break;
			}
		}
	}

	arp_cache_dump();
}

static void arp_send_reply(const struct t_arp *p_arp) {
	DEBUG_ENTRY

	// Ethernet header
	std::memcpy(s_arp_reply.ether.dst, p_arp->ether.src, ETH_ADDR_LEN);
	// ARP Header
	const auto nIpTarget = net::memcpy_ip(p_arp->arp.target_ip);
	std::memcpy(s_arp_reply.arp.target_mac, p_arp->arp.sender_mac, ETH_ADDR_LEN);
	std::memcpy(s_arp_reply.arp.target_ip, p_arp->arp.sender_ip, IPv4_ADDR_LEN);
	net::memcpy_ip(s_arp_reply.arp.sender_ip, nIpTarget);

	emac_eth_send(reinterpret_cast<void *>(&s_arp_reply), sizeof(struct t_arp));

	DEBUG_EXIT
}

// Public interface

void __attribute__((cold)) arp_init() {
	DEBUG_ENTRY

	for (auto& record : s_ArpRecords) {
		std::memset(&record, 0, sizeof(struct net::arp::Record));
	}

	// ARP Request template
	// Ethernet header
	std::memcpy(s_arp_request.ether.src, net::globals::netif_default.hwaddr, ETH_ADDR_LEN);
	std::memset(s_arp_request.ether.dst, 0xFF , ETH_ADDR_LEN);
	s_arp_request.ether.type = __builtin_bswap16(ETHER_TYPE_ARP);

	// ARP Header
	s_arp_request.arp.hardware_type = __builtin_bswap16(ARP_HWTYPE_ETHERNET);
	s_arp_request.arp.protocol_type = __builtin_bswap16(ARP_PRTYPE_IPv4);
	s_arp_request.arp.hardware_size = ARP_HARDWARE_SIZE;
	s_arp_request.arp.protocol_size = ARP_PROTOCOL_SIZE;
	s_arp_request.arp.opcode = __builtin_bswap16(ARP_OPCODE_RQST);

	std::memcpy(s_arp_request.arp.sender_mac, net::globals::netif_default.hwaddr, ETH_ADDR_LEN);
	net::memcpy_ip(s_arp_request.arp.sender_ip, net::globals::netif_default.ip.addr);
	std::memset(s_arp_request.arp.target_mac, 0x00, ETH_ADDR_LEN);

	// ARP Reply Template
	// Ethernet header
	std::memcpy(s_arp_reply.ether.src, net::globals::netif_default.hwaddr, ETH_ADDR_LEN);
	s_arp_reply.ether.type = __builtin_bswap16(ETHER_TYPE_ARP);

	// ARP Header
	s_arp_reply.arp.hardware_type = __builtin_bswap16(ARP_HWTYPE_ETHERNET);
	s_arp_reply.arp.protocol_type = __builtin_bswap16(ARP_PRTYPE_IPv4);
	s_arp_reply.arp.hardware_size = ARP_HARDWARE_SIZE;
	s_arp_reply.arp.protocol_size = ARP_PROTOCOL_SIZE;
	s_arp_reply.arp.opcode = __builtin_bswap16(ARP_OPCODE_REPLY);

	std::memcpy(s_arp_reply.arp.sender_mac, net::globals::netif_default.hwaddr, ETH_ADDR_LEN);

	Hardware::Get()->SoftwareTimerAdd(net::arp::TIMER_INTERVAL, arp_timer);

	DEBUG_EXIT
}

__attribute__((hot)) void arp_handle(struct t_arp *pArp) {
	/*
	 * RFC 826 Packet Reception:
	 */
	if (__builtin_expect(((pArp->arp.hardware_type != __builtin_bswap16(ARP_HWTYPE_ETHERNET))
					|| (pArp->arp.protocol_type != __builtin_bswap16(ARP_PRTYPE_IPv4))
					|| (pArp->arp.hardware_size != ARP_HARDWARE_SIZE)
					|| (pArp->arp.protocol_size != ARP_PROTOCOL_SIZE)), 0)) {
		DEBUG_EXIT
		return;
	}

	acd_arp_reply(pArp);

	/* ARP packet directed to us? */
	const auto nIpTarget = net::memcpy_ip(pArp->arp.target_ip);
	const auto bToUs = ((nIpTarget == net::globals::netif_default.ip.addr) || (nIpTarget == net::globals::netif_default.secondary_ip.addr));
	/* ARP packet from us? */
	const auto bFromUs = (net::memcpy_ip(pArp->arp.sender_ip) == net::globals::netif_default.ip.addr);

	DEBUG_PRINTF("bToUs:%d, bFromUs:%d", bToUs, bFromUs);

	/*
	 * ARP message directed to us?
	 *  -> add IP address in ARP cache; assume requester wants to talk to us,
	 *     can result in directly sending the queued packets for this host.
	 * ARP message not directed to us?
	 * ->  update the source IP address in the cache, if present
	 */
	arp_cache_update(pArp->arp.sender_mac, net::memcpy_ip(pArp->arp.sender_ip), bToUs ? arp::Flags::FLAG_INSERT : arp::Flags::FLAG_UPDATE);

	switch (pArp->arp.opcode) {
	case __builtin_bswap16(ARP_OPCODE_RQST):
		if (bToUs && !bFromUs) {
			arp_send_reply(pArp);
		} else {
			DEBUG_PUTS("ARP request was not for us");
		}
		break;
	case __builtin_bswap16(ARP_OPCODE_REPLY):
		/* Cache update is handled earlier */
		break;
	default:
		DEBUG_PRINTF("opcode %04x not handled", __builtin_bswap16(pArp->arp.opcode));
		break;
	}
}

template<net::arp::EthSend S>
static void arp_send_implementation(struct t_udp *pPacket, const uint32_t nSize, const uint32_t nRemoteIp) {
	DEBUG_ENTRY
	DEBUG_PRINTF(IPSTR, IP2STR(nRemoteIp));

	net::memcpy_ip(pPacket->ip4.dst, nRemoteIp);
	pPacket->ip4.chksum = 0;
#if !defined (CHECKSUM_BY_HARDWARE)
	pPacket->ip4.chksum = net_chksum(reinterpret_cast<void *>(&pPacket->ip4), sizeof(pPacket->ip4));
#endif

	auto nDestinationIp = nRemoteIp;

	if  (__builtin_expect((net::globals::nOnNetworkMask != (nRemoteIp & net::globals::nOnNetworkMask)), 0)) {
	      /* According to RFC 3297, chapter 2.6.2 (Forwarding Rules), a packet with
	         a link-local source address must always be "directly to its destination
	         on the same physical link. The host MUST NOT send the packet to any
	         router for forwarding". */
		if (!network::is_linklocal_ip(nRemoteIp)) {
			nDestinationIp = net::globals::netif_default.gw.addr;
			DEBUG_PUTS("");
		}
	}

	for (auto &record : s_ArpRecords) {
		if (record.state >= net::arp::State::STATE_REACHABLE) {
			if (record.nIp == nDestinationIp) {
				std::memcpy(pPacket->ether.dst, record.mac_address, ETH_ADDR_LEN);

				if (S == net::arp::EthSend::IS_NORMAL) {
					emac_eth_send(reinterpret_cast<void *>(pPacket), nSize);
				}
#if defined CONFIG_ENET_ENABLE_PTP
				else if (S == net::arp::EthSend::IS_TIMESTAMP) {
					emac_eth_send_timestamp(reinterpret_cast<void *>(pPacket), nSize);
				}
#endif
				DEBUG_EXIT
				return;
			}
		}
	}

	arp_query<S>(nDestinationIp, pPacket, nSize, arp::Flags::FLAG_INSERT);

	DEBUG_EXIT
	return;
}

void arp_send(struct t_udp *pPacket, const uint32_t nSize, const uint32_t nRemoteIp) {
	arp_send_implementation<net::arp::EthSend::IS_NORMAL>(pPacket, nSize, nRemoteIp);
}

#if defined CONFIG_ENET_ENABLE_PTP
void arp_send_timestamp(struct t_udp *pPacket, const uint32_t nSize, const uint32_t nRemoteIp) {
	arp_send_implementation<net::arp::EthSend::IS_TIMESTAMP>(pPacket, nSize, nRemoteIp);
}
#endif

/*
 *  The Sender IP is set to all zeros,
 *  which means it cannot map to the Sender MAC address.
 *  The Target MAC address is all zeros,
 *  which means it cannot map to the Target IP address.
 */
void arp_acd_probe(const ip4_addr_t ipaddr) {
	DEBUG_ENTRY

	memset(s_arp_request.arp.sender_ip, 0, IPv4_ADDR_LEN);
	net::memcpy_ip(s_arp_request.arp.target_ip, ipaddr.addr);

	emac_eth_send(reinterpret_cast<void *>(&s_arp_request), sizeof(struct t_arp));

	net::memcpy_ip(s_arp_request.arp.sender_ip, net::globals::netif_default.ip.addr);

	DEBUG_EXIT
}

/*
 * The packet structure is identical to the ARP Probe above,
 * with the exception that a complete mapping exists.
 * Both the Sender MAC address and the Sender IP address create a complete ARP mapping,
 * and hosts on the network can use this pair of addresses in their ARP table.
 */
void arp_acd_send_announcement(const ip4_addr_t ipaddr) {
	net::memcpy_ip(s_arp_request.arp.target_ip, ipaddr.addr);
	net::memcpy_ip(s_arp_request.arp.sender_ip, ipaddr.addr);

	emac_eth_send(reinterpret_cast<void *>(&s_arp_request), sizeof(struct t_arp));
}
}  // namespace net
