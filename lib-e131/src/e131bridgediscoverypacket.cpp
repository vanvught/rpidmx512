/**
 * @file e131bridgediscoverypacket
 *
 */
/* Copyright (C) 2019-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <stdint.h>
#include <string.h>
#include <cassert>

#include "e131bridge.h"
#include "e131packets.h"

#include "e117const.h"

#include "network.h"

void E131Bridge::FillDiscoveryPacket() {
	m_State.DiscoveryPacketLength = DISCOVERY_PACKET_SIZE(m_State.nActiveInputPorts);

	memset(m_pE131DiscoveryPacket, 0, sizeof(struct TE131DiscoveryPacket));

	// Root Layer (See Section 5)
	m_pE131DiscoveryPacket->RootLayer.PreAmbleSize = __builtin_bswap16(0x10);
	memcpy(m_pE131DiscoveryPacket->RootLayer.ACNPacketIdentifier, E117Const::ACN_PACKET_IDENTIFIER, E117_PACKET_IDENTIFIER_LENGTH);
	m_pE131DiscoveryPacket->RootLayer.FlagsLength = __builtin_bswap16((0x07 << 12) | (DISCOVERY_ROOT_LAYER_LENGTH(m_State.nActiveInputPorts)));
	m_pE131DiscoveryPacket->RootLayer.Vector = __builtin_bswap32(E131_VECTOR_ROOT_EXTENDED);
	memcpy(m_pE131DiscoveryPacket->RootLayer.Cid, m_Cid, E131_CID_LENGTH);

	// E1.31 Framing Layer (See Section 6)
	m_pE131DiscoveryPacket->FrameLayer.FLagsLength = __builtin_bswap16((0x07 << 12) | (DISCOVERY_FRAME_LAYER_LENGTH(m_State.nActiveInputPorts)) );
	m_pE131DiscoveryPacket->FrameLayer.Vector = __builtin_bswap32(E131_VECTOR_EXTENDED_DISCOVERY);
	memcpy(m_pE131DiscoveryPacket->FrameLayer.SourceName, m_SourceName, E131_SOURCE_NAME_LENGTH);

	// Universe Discovery Layer (See Section 8)
	m_pE131DiscoveryPacket->UniverseDiscoveryLayer.FlagsLength = __builtin_bswap16((0x07 << 12) | DISCOVERY_LAYER_LENGTH(m_State.nActiveInputPorts));
	m_pE131DiscoveryPacket->UniverseDiscoveryLayer.Vector = __builtin_bswap32(VECTOR_UNIVERSE_DISCOVERY_UNIVERSE_LIST);
}

void E131Bridge::SendDiscoveryPacket() {
	assert(m_DiscoveryIpAddress != 0);

	if (m_nCurrentPacketMillis - m_State.DiscoveryTime >= (E131_UNIVERSE_DISCOVERY_INTERVAL_SECONDS * 1000)) {
		m_State.DiscoveryTime = m_nCurrentPacketMillis;

		uint32_t nListOfUniverses = 0;

		if (m_State.nActiveInputPorts != 0) {
			for (uint32_t i = 0; i < E131_MAX_UARTS; i++) {
				uint16_t nUniverse;
				if (GetUniverse(i, nUniverse, E131_INPUT_PORT)) {
					m_pE131DiscoveryPacket->UniverseDiscoveryLayer.ListOfUniverses[nListOfUniverses++] = __builtin_bswap16(nUniverse);
				}
			}
		}

		Network::Get()->SendTo(m_nHandle, m_pE131DiscoveryPacket, m_State.DiscoveryPacketLength, m_DiscoveryIpAddress, E131_DEFAULT_PORT);
	}
}
