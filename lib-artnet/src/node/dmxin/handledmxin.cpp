/**
 * @file handledmxin.cpp
 *
 */
/**
 * Art-Net Designed by and Copyright Artistic Licence Holdings Ltd.
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

#if !defined(__clang__)
# pragma GCC push_options
# pragma GCC optimize ("O2")
# pragma GCC optimize ("no-tree-loop-distribute-patterns")
#endif

#include <cstdint>
#include <cstring>
#include <cassert>

#include "artnetnode.h"
#include "artnet.h"

#include "dmx.h"

#include "network.h"
#include "hardware.h"

#include "panel_led.h"

#include "debug.h"

static uint32_t s_ReceivingMask = 0;

void ArtNetNode::HandleDmxIn() {
	for (uint32_t nPortIndex = 0; nPortIndex < artnetnode::MAX_PORTS; nPortIndex++) {
		if  ((m_Node.Port[nPortIndex].direction == lightset::PortDir::INPUT)
		 &&  (m_Node.Port[nPortIndex].protocol == artnet::PortProtocol::ARTNET)
		 && ((m_InputPort[nPortIndex].GoodInput & artnet::GoodInput::DISABLED) != artnet::GoodInput::DISABLED)) {

			const auto *const pDmxData = reinterpret_cast<const struct Data *>(Dmx::Get()->GetDmxChanged(nPortIndex));

			if (pDmxData != nullptr) {
				m_ArtDmx.Sequence = static_cast<uint8_t>(1U + m_InputPort[nPortIndex].nSequenceNumber++);
				m_ArtDmx.Physical = static_cast<uint8_t>(nPortIndex);
				m_ArtDmx.PortAddress = m_Node.Port[nPortIndex].PortAddress;

				auto nLength = pDmxData->Statistics.nSlotsInPacket;

				memcpy(m_ArtDmx.Data, &pDmxData->Data[1], nLength);

				if ((nLength & 0x1) == 0x1) {
					m_ArtDmx.Data[nLength] = 0x00;
					nLength++;
				}

				m_ArtDmx.LengthHi = static_cast<uint8_t>((nLength & 0xFF00) >> 8);
				m_ArtDmx.Length = static_cast<uint8_t>(nLength & 0xFF);

				m_InputPort[nPortIndex].GoodInput = artnet::GoodInput::DATA_RECIEVED;

				Network::Get()->SendTo(m_nHandle, &m_ArtDmx, sizeof(struct artnet::ArtDmx), m_InputPort[nPortIndex].nDestinationIp, artnet::UDP_PORT);

				SendDiag(artnet::PriorityCodes::DIAG_LOW, "%u: Input DMX sent", nPortIndex);

				if (m_Node.Port[nPortIndex].bLocalMerge) {
					m_pReceiveBuffer = reinterpret_cast<uint8_t *>(&m_ArtDmx);
					m_nIpAddressFrom = Network::Get()->GetIp();
					HandleDmx();

					SendDiag(artnet::PriorityCodes::DIAG_LOW, "%u: Input DMX local merge", nPortIndex);
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

				if ((m_InputPort[nPortIndex].GoodInput & artnet::GoodInput::DATA_RECIEVED) == artnet::GoodInput::DATA_RECIEVED) {
					m_InputPort[nPortIndex].GoodInput = static_cast<uint8_t>(m_InputPort[nPortIndex].GoodInput & ~artnet::GoodInput::DATA_RECIEVED);
					m_InputPort[nPortIndex].nMillis = Hardware::Get()->Millis();
					sendArtDmx = true;

					s_ReceivingMask &= ~(1U << nPortIndex);
					hal::panel_led_off(hal::panelled::PORT_A_RX << nPortIndex);

					if (s_ReceivingMask == 0) {
						m_State.nReceivingDmx &= static_cast<uint8_t>(~(1U << static_cast<uint8_t>(lightset::PortDir::INPUT)));
					}

					SendDiag(artnet::PriorityCodes::DIAG_LOW, "%u: Input DMX updates per second is 0", nPortIndex);
				} else if (m_InputPort[nPortIndex].nMillis != 0) {
					const auto nMillis = Hardware::Get()->Millis();
					if ((nMillis - m_InputPort[nPortIndex].nMillis) > 1000) {
						m_InputPort[nPortIndex].nMillis = nMillis;
						sendArtDmx = true;

						SendDiag(artnet::PriorityCodes::DIAG_LOW, "%u: Input DMX timeout 1 second", nPortIndex);
					}
				}

				if (sendArtDmx) {
					const auto *const pDmxData = reinterpret_cast<const struct Data *>(Dmx::Get()->GetDmxCurrentData(nPortIndex));

					m_ArtDmx.Sequence = static_cast<uint8_t>(1U + m_InputPort[nPortIndex].nSequenceNumber++);
					m_ArtDmx.Physical = static_cast<uint8_t>(nPortIndex);
					m_ArtDmx.PortAddress = m_Node.Port[nPortIndex].PortAddress;

					auto nLength = pDmxData->Statistics.nSlotsInPacket;

					memcpy(m_ArtDmx.Data, &pDmxData->Data[1], nLength);

					if ((nLength & 0x1) == 0x1) {
						m_ArtDmx.Data[nLength] = 0x00;
						nLength++;
					}

					m_ArtDmx.LengthHi = static_cast<uint8_t>((nLength & 0xFF00) >> 8);
					m_ArtDmx.Length = static_cast<uint8_t>(nLength & 0xFF);

					Network::Get()->SendTo(m_nHandle, &m_ArtDmx, sizeof(struct artnet::ArtDmx), m_InputPort[nPortIndex].nDestinationIp, artnet::UDP_PORT);

					SendDiag(artnet::PriorityCodes::DIAG_LOW, "%u: Input DMX sent (timeout)", nPortIndex);

					if (m_Node.Port[nPortIndex].bLocalMerge) {
						m_pReceiveBuffer = reinterpret_cast<uint8_t *>(&m_ArtDmx);
						m_nIpAddressFrom = Network::Get()->GetIp();
						HandleDmx();

						SendDiag(artnet::PriorityCodes::DIAG_LOW, "%u: Input DMX local merge", nPortIndex);
					}
				}
			}
		}
	}
}
