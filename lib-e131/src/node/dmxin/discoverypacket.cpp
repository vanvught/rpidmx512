/**
 * @file e131bridgediscoverypacket
 *
 */
/* Copyright (C) 2019-2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <cassert>

#include "e131bridge.h"
#include "e131packets.h"
#include "e117const.h"

#include "network.h"

void E131Bridge::FillDiscoveryPacket() {
	m_State.DiscoveryPacketLength = static_cast<uint16_t>(DISCOVERY_PACKET_SIZE(m_State.nEnabledInputPorts));

	memset(&m_E131DiscoveryPacket, 0, sizeof(struct TE131DiscoveryPacket));

	// Root Layer (See Section 5)
	m_E131DiscoveryPacket.RootLayer.PreAmbleSize = __builtin_bswap16(0x10);
	memcpy(m_E131DiscoveryPacket.RootLayer.ACNPacketIdentifier, E117Const::ACN_PACKET_IDENTIFIER, e117::PACKET_IDENTIFIER_LENGTH);
	m_E131DiscoveryPacket.RootLayer.FlagsLength = __builtin_bswap16(static_cast<uint16_t>((0x07 << 12) | (DISCOVERY_ROOT_LAYER_LENGTH(m_State.nEnabledInputPorts))));
	m_E131DiscoveryPacket.RootLayer.Vector = __builtin_bswap32(e131::vector::root::EXTENDED);
	memcpy(m_E131DiscoveryPacket.RootLayer.Cid, m_Cid, e131::CID_LENGTH);

	// E1.31 Framing Layer (See Section 6)
	m_E131DiscoveryPacket.FrameLayer.FLagsLength = __builtin_bswap16(static_cast<uint16_t>((0x07 << 12) | (DISCOVERY_FRAME_LAYER_LENGTH(m_State.nEnabledInputPorts))));
	m_E131DiscoveryPacket.FrameLayer.Vector = __builtin_bswap32(e131::vector::extended::DISCOVERY);
	memcpy(m_E131DiscoveryPacket.FrameLayer.SourceName, m_SourceName, e131::SOURCE_NAME_LENGTH);

	// Universe Discovery Layer (See Section 8)
	m_E131DiscoveryPacket.UniverseDiscoveryLayer.FlagsLength = __builtin_bswap16(static_cast<uint16_t>((0x07 << 12) | DISCOVERY_LAYER_LENGTH(m_State.nEnabledInputPorts)));
	m_E131DiscoveryPacket.UniverseDiscoveryLayer.Vector = __builtin_bswap32(e131::vector::universe::DISCOVERY_UNIVERSE_LIST);
}

void E131Bridge::SendDiscoveryPacket() {
	assert(m_DiscoveryIpAddress != 0);

	if (m_nCurrentPacketMillis - m_State.DiscoveryTime >= (e131::UNIVERSE_DISCOVERY_INTERVAL_SECONDS * 1000)) {
		m_State.DiscoveryTime = m_nCurrentPacketMillis;

		uint32_t nListOfUniverses = 0;

		if (m_State.nEnabledInputPorts != 0) {
			for (uint32_t i = 0; i < e131bridge::MAX_PORTS; i++) {
				uint16_t nUniverse;
				if (GetUniverse(i, nUniverse, lightset::PortDir::INPUT)) {
					m_E131DiscoveryPacket.UniverseDiscoveryLayer.ListOfUniverses[nListOfUniverses++] = __builtin_bswap16(nUniverse);
				}
			}

			Network::Get()->SendTo(m_nHandle, &m_E131DiscoveryPacket, m_State.DiscoveryPacketLength, m_DiscoveryIpAddress, e131::UDP_PORT);
		}
	}
}
