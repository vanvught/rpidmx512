/**
 * @file e131bridgehandledmxin.cpp
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

#include "dmx.h"

#include "network.h"
#include "hardware.h"

#include "panel_led.h"

#include "debug.h"

void E131Bridge::FillDataPacket() {
	// Root Layer (See Section 5)
	m_E131DataPacket.RootLayer.PreAmbleSize = __builtin_bswap16(0x0010);
	m_E131DataPacket.RootLayer.PostAmbleSize = __builtin_bswap16(0x0000);
	memcpy(m_E131DataPacket.RootLayer.ACNPacketIdentifier, E117Const::ACN_PACKET_IDENTIFIER, e117::PACKET_IDENTIFIER_LENGTH);
	m_E131DataPacket.RootLayer.Vector = __builtin_bswap32(e131::vector::root::DATA);
	memcpy(m_E131DataPacket.RootLayer.Cid, m_Cid, e131::CID_LENGTH);
	// E1.31 Framing Layer (See Section 6)
	m_E131DataPacket.FrameLayer.Vector = __builtin_bswap32(e131::vector::data::PACKET);
	memcpy(m_E131DataPacket.FrameLayer.SourceName, m_SourceName, e131::SOURCE_NAME_LENGTH);
	m_E131DataPacket.FrameLayer.SynchronizationAddress = __builtin_bswap16(0); // Currently not supported
	m_E131DataPacket.FrameLayer.Options = 0;
	// Data Layer
	m_E131DataPacket.DMPLayer.Vector = e131::vector::dmp::SET_PROPERTY;
	m_E131DataPacket.DMPLayer.Type = 0xa1;
	m_E131DataPacket.DMPLayer.FirstAddressProperty = __builtin_bswap16(0x0000);
	m_E131DataPacket.DMPLayer.AddressIncrement = __builtin_bswap16(0x0001);
}

static uint32_t s_ReceivingMask = 0;

void E131Bridge::HandleDmxIn() {
	for (uint32_t nPortIndex = 0 ; nPortIndex < e131bridge::MAX_PORTS; nPortIndex++) {
		if ((m_Bridge.Port[nPortIndex].direction == lightset::PortDir::INPUT) && (!m_InputPort[nPortIndex].IsDisabled)) {

			const auto *const pDmxData = reinterpret_cast<const struct Data *>(Dmx::Get()->GetDmxChanged(nPortIndex));

			if (pDmxData != nullptr) {
				// Root Layer (See Section 5)
				auto nLength = (1U + pDmxData->Statistics.nSlotsInPacket); // Add 1 for SC
				m_E131DataPacket.RootLayer.FlagsLength = __builtin_bswap16(static_cast<uint16_t>((0x07 << 12) | (DATA_ROOT_LAYER_LENGTH(nLength))));
				// E1.31 Framing Layer (See Section 6)
				m_E131DataPacket.FrameLayer.FLagsLength = __builtin_bswap16(static_cast<uint16_t>((0x07 << 12) | (DATA_FRAME_LAYER_LENGTH(nLength))));
				m_E131DataPacket.FrameLayer.Priority = m_InputPort[nPortIndex].nPriority;
				m_E131DataPacket.FrameLayer.SequenceNumber = m_InputPort[nPortIndex].nSequenceNumber++;
				m_E131DataPacket.FrameLayer.Universe = __builtin_bswap16(m_Bridge.Port[nPortIndex].nUniverse);
				// Data Layer
				m_E131DataPacket.DMPLayer.FlagsLength = __builtin_bswap16(static_cast<uint16_t>((0x07 << 12) | (DATA_LAYER_LENGTH(nLength))));
				memcpy(m_E131DataPacket.DMPLayer.PropertyValues, pDmxData, nLength);
				m_E131DataPacket.DMPLayer.PropertyValueCount = __builtin_bswap16(static_cast<uint16_t>(nLength));

				Network::Get()->SendTo(m_nHandle, &m_E131DataPacket, DATA_PACKET_SIZE(nLength), m_InputPort[nPortIndex].nMulticastIp, e131::UDP_PORT);

				if (m_Bridge.Port[nPortIndex].bLocalMerge) {
					m_pReceiveBuffer = reinterpret_cast<uint8_t *>(&m_E131DataPacket);
					m_nIpAddressFrom = Network::Get()->GetIp();
					HandleDmx();
				}

				if ((s_ReceivingMask & (1U << nPortIndex)) != (1U << nPortIndex)) {
					s_ReceivingMask |= (1U << nPortIndex);
					m_State.nReceivingDmx |= (1U << static_cast<uint8_t>(lightset::PortDir::INPUT));
					hal::panel_led_on(hal::panelled::PORT_A_RX << nPortIndex);
				}

				continue;
			} 
				
			if (Dmx::Get()->GetDmxUpdatesPerSecond(nPortIndex) == 0) {
				auto sendArtDmx = false;

				if ((s_ReceivingMask & (1U << nPortIndex)) == (1U << nPortIndex)) {
					sendArtDmx = true;

					s_ReceivingMask &= ~(1U << nPortIndex);
					hal::panel_led_off(hal::panelled::PORT_A_RX << nPortIndex);

					if (s_ReceivingMask == 0) {
						m_State.nReceivingDmx &= static_cast<uint8_t>(~(1U << static_cast<uint8_t>(lightset::PortDir::INPUT)));
					}
				} else if (m_InputPort[nPortIndex].nMillis != 0) {
					const auto nMillis = Hardware::Get()->Millis();
					if ((nMillis - m_InputPort[nPortIndex].nMillis) > 1000) {
						m_InputPort[nPortIndex].nMillis = nMillis;
						sendArtDmx = true;
					}
				}

				if (sendArtDmx) {
					const auto *const pDmxData = reinterpret_cast<const struct Data *>(Dmx::Get()->GetDmxCurrentData(nPortIndex));
					// Root Layer (See Section 5)
					auto nLength = (1U + pDmxData->Statistics.nSlotsInPacket); // Add 1 for SC
					m_E131DataPacket.RootLayer.FlagsLength = __builtin_bswap16(static_cast<uint16_t>((0x07 << 12) | (DATA_ROOT_LAYER_LENGTH(nLength))));
					// E1.31 Framing Layer (See Section 6)
					m_E131DataPacket.FrameLayer.FLagsLength = __builtin_bswap16(static_cast<uint16_t>((0x07 << 12) | (DATA_FRAME_LAYER_LENGTH(nLength))));
					m_E131DataPacket.FrameLayer.Priority = m_InputPort[nPortIndex].nPriority;
					m_E131DataPacket.FrameLayer.SequenceNumber = m_InputPort[nPortIndex].nSequenceNumber++;
					m_E131DataPacket.FrameLayer.Universe = __builtin_bswap16(m_Bridge.Port[nPortIndex].nUniverse);
					// Data Layer
					m_E131DataPacket.DMPLayer.FlagsLength = __builtin_bswap16(static_cast<uint16_t>((0x07 << 12) | (DATA_LAYER_LENGTH(nLength))));
					memcpy(m_E131DataPacket.DMPLayer.PropertyValues, pDmxData, nLength);
					m_E131DataPacket.DMPLayer.PropertyValueCount = __builtin_bswap16(static_cast<uint16_t>(nLength));

					Network::Get()->SendTo(m_nHandle, &m_E131DataPacket, DATA_PACKET_SIZE(nLength), m_InputPort[nPortIndex].nMulticastIp, e131::UDP_PORT);

					if (m_Bridge.Port[nPortIndex].bLocalMerge) {
						m_pReceiveBuffer = reinterpret_cast<uint8_t *>(&m_E131DataPacket);
						m_nIpAddressFrom = Network::Get()->GetIp();
						HandleDmx();
					}
				}

			}
		}
	}
}
