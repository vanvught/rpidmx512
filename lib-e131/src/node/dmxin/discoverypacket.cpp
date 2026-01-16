/**
 * @file discoverypacket.cpp
 *
 */
/* Copyright (C) 2019-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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
#include <cstring>

#include "e131bridge.h"
#include "e131.h"
#include "e117.h"
#include "network.h"

void E131Bridge::FillDiscoveryPacket() {
	state_.discovery_packet_length = static_cast<uint16_t>(e131::DiscoveryPacketSize(state_.enabled_input_ports));

	memset(&e131_discovery_packet_, 0, sizeof(struct e131::DiscoveryPacket));

	// Root Layer (See Section 5)
	e131_discovery_packet_.root_layer.pre_amble_size = __builtin_bswap16(0x10);
	memcpy(e131_discovery_packet_.root_layer.acn_packet_identifier, e117::kAcnPacketIdentifier, e117::kAcnPacketIdentifierLength);
	e131_discovery_packet_.root_layer.flags_length = __builtin_bswap16(static_cast<uint16_t>((0x07 << 12) | (e131::DiscoveryRootLayerLength(state_.enabled_input_ports))));
	e131_discovery_packet_.root_layer.vector = __builtin_bswap32(e131::vector::root::kExtended);
	memcpy(e131_discovery_packet_.root_layer.cid, cid_, e117::kCidLength);

	// E1.31 Framing Layer (See Section 6)
	e131_discovery_packet_.frame_layer.flags_length = __builtin_bswap16(static_cast<uint16_t>((0x07 << 12) | (e131::DiscoveryFrameLayerLength(state_.enabled_input_ports))));
	e131_discovery_packet_.frame_layer.vector = __builtin_bswap32(e131::vector::extended::kDiscovery);
	memcpy(e131_discovery_packet_.frame_layer.source_name, source_name_, e131::kSourceNameLength);

	// Universe Discovery Layer (See Section 8)
	e131_discovery_packet_.universe_discovery_layer.flags_length = __builtin_bswap16(static_cast<uint16_t>((0x07 << 12) | e131::DiscoveryLayerLength(state_.enabled_input_ports)));
	e131_discovery_packet_.universe_discovery_layer.vector = __builtin_bswap32(e131::vector::universe::kDiscoveryUniverseList);
}

void E131Bridge::SendDiscoveryPacket() {
	uint32_t list_of_universes = 0;

	if (state_.enabled_input_ports != 0) {
		for (uint32_t i = 0; i < dmxnode::kMaxPorts; i++) {
			uint16_t universe;
			if (GetUniverse(i, universe, dmxnode::PortDirection::kInput)) {
				e131_discovery_packet_.universe_discovery_layer.list_of_universes[list_of_universes++] = __builtin_bswap16(universe);
			}
		}

		network::udp::Send(handle_, reinterpret_cast<const uint8_t*>(&e131_discovery_packet_), state_.discovery_packet_length, discovery_ip_address_, e131::kUdpPort);
	}
}
