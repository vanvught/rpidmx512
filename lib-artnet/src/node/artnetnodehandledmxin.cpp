/**
 * @file artnetnodehandledmxin.cpp
 *
 */
/**
 * Art-Net Designed by and Copyright Artistic Licence Holdings Ltd.
 */
/* Copyright (C) 2019-2022 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "artnetnode.h"
#include "artnet.h"
#include "artnetdmx.h"

#include "network.h"
#include "hardware.h"

#include "panel_led.h"

#include "debug.h"

static uint32_t s_ReceivingMask = 0;

using namespace artnet;

void ArtNetNode::SetDestinationIp(uint32_t nPortIndex, uint32_t nDestinationIp) {
	if (nPortIndex < artnetnode::MAX_PORTS) {
		if (Network::Get()->IsValidIp(nDestinationIp)) {
			m_InputPort[nPortIndex].nDestinationIp = nDestinationIp;
		} else {
			m_InputPort[nPortIndex].nDestinationIp = m_Node.IPAddressBroadcast;
		}

		DEBUG_PRINTF("m_nDestinationIp=" IPSTR, IP2STR(m_InputPort[nPortIndex].nDestinationIp));
	}
}

void ArtNetNode::HandleDmxIn() {
	for (uint32_t i = 0; i < artnetnode::MAX_PORTS; i++) {
		if (!m_InputPort[i].genericPort.bIsEnabled) {
			continue;
		}

		uint32_t nLength;
		uint32_t nUpdatesPerSecond;
		const auto *pDmxData = m_pArtNetDmx->Handler(i, nLength, nUpdatesPerSecond);

		if (pDmxData != nullptr) {
			m_ArtDmx.Sequence = static_cast<uint8_t>(1U + m_InputPort[i].nSequenceNumber++);
			m_ArtDmx.Physical = static_cast<uint8_t>(i);
			m_ArtDmx.PortAddress = m_InputPort[i].genericPort.nPortAddress;

			memcpy(m_ArtDmx.Data, pDmxData, nLength);

			if ((nLength & 0x1) == 0x1) {
				m_ArtDmx.Data[nLength] = 0x00;
				nLength++;
			}

			m_ArtDmx.LengthHi = static_cast<uint8_t>((nLength & 0xFF00) >> 8);
			m_ArtDmx.Length = static_cast<uint8_t>(nLength & 0xFF);

			m_InputPort[i].genericPort.nStatus = GoodInput::DATA_RECIEVED;

			Network::Get()->SendTo(m_nHandle, &m_ArtDmx, sizeof(struct TArtDmx), m_InputPort[i].nDestinationIp, artnet::UDP_PORT);

			if ((s_ReceivingMask & (1U << i)) != (1U << i)) {
				s_ReceivingMask |= (1U << i);
				m_State.nReceivingDmx |= (1U << static_cast<uint8_t>(lightset::PortDir::INPUT));
				hal::panel_led_on(hal::panelled::PORT_A_RX << i);
			}
		} else {
			if ((m_InputPort[i].genericPort.nStatus & GoodInput::DATA_RECIEVED) == GoodInput::DATA_RECIEVED) {
				if (nUpdatesPerSecond == 0) {
					m_InputPort[i].genericPort.nStatus = static_cast<uint8_t>(m_InputPort[i].genericPort.nStatus & ~GoodInput::DATA_RECIEVED);
					s_ReceivingMask &= ~(1U << i);
					hal::panel_led_off(hal::panelled::PORT_A_RX << i);
					if (s_ReceivingMask == 0) {
						m_State.nReceivingDmx &= static_cast<uint8_t>(~(1U << static_cast<uint8_t>(lightset::PortDir::INPUT)));
					}
				}
			}
		}
	}
}
