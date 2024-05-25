/**
 * @file handlerdmin.cpp
 *
 */
/**
 * Art-Net Designed by and Copyright Artistic Licence Holdings Ltd.
 */
/* Copyright (C) 2023-2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include "artnetrdmcontroller.h"

#include "rdm.h"
#include "network.h"
#include "hardware.h"
#include "panel_led.h"

#include "debug.h"

void ArtNetNode::HandleRdmIn() {
	for (uint32_t nPortIndex = 0; nPortIndex < artnetnode::MAX_PORTS; nPortIndex++) {
		auto *const pArtRdm = &m_ArtTodPacket.ArtRdm;

		if (m_Node.Port[nPortIndex].direction == lightset::PortDir::INPUT) {
			const auto *pRdmData = Rdm::Receive(nPortIndex);
			if (pRdmData != nullptr) {
				if (m_pArtNetRdmController->RdmReceive(nPortIndex, pRdmData)) {
					pArtRdm->OpCode = static_cast<uint16_t>(artnet::OpCodes::OP_RDM);
					pArtRdm->RdmVer = 0x01;
					pArtRdm->Net = m_Node.Port[nPortIndex].NetSwitch;
					pArtRdm->Command = 0;
					pArtRdm->Address = m_Node.Port[nPortIndex].DefaultAddress;

					auto *pMessage = reinterpret_cast<const struct TRdmMessage *>(pRdmData);
					memcpy(pArtRdm->RdmPacket, &pRdmData[1], pMessage->message_length + 1U);

					const auto *pRdmMessage = reinterpret_cast<const struct TRdmMessageNoSc *>(pArtRdm->RdmPacket);

					Network::Get()->SendTo(m_nHandle, pArtRdm, ((sizeof(struct artnet::ArtRdm)) - 256) + pRdmMessage->message_length + 1 , m_InputPort[nPortIndex].nDestinationIp, artnet::UDP_PORT);

#if defined(CONFIG_PANELLED_RDM_PORT)
					hal::panel_led_on(hal::panelled::PORT_A_RDM << nPortIndex);
#elif defined(CONFIG_PANELLED_RDM_NO_PORT)
					hal::panel_led_on(hal::panelled::RDM << nPortIndex);
#endif
				}
			}
		} else if (m_Node.Port[nPortIndex].direction == lightset::PortDir::OUTPUT) {
			if (m_OutputPort[nPortIndex].nIpRdm != 0) {
				const auto *pRdmData = Rdm::Receive(nPortIndex);
				if (pRdmData != nullptr) {
					pArtRdm->OpCode = static_cast<uint16_t>(artnet::OpCodes::OP_RDM);
					pArtRdm->RdmVer = 0x01;
					pArtRdm->Net = m_Node.Port[nPortIndex].NetSwitch;
					pArtRdm->Command = 0;
					pArtRdm->Address = m_Node.Port[nPortIndex].DefaultAddress;

					auto *pMessage = reinterpret_cast<const struct TRdmMessage *>(pRdmData);
					memcpy(pArtRdm->RdmPacket, &pRdmData[1], pMessage->message_length + 1U);

					const auto *pRdmMessage = reinterpret_cast<const struct TRdmMessageNoSc *>(pArtRdm->RdmPacket);

					Network::Get()->SendTo(m_nHandle, pArtRdm, ((sizeof(struct artnet::ArtRdm)) - 256) + pRdmMessage->message_length + 1 , m_OutputPort[nPortIndex].nIpRdm, artnet::UDP_PORT);

					m_OutputPort[nPortIndex].nIpRdm = 0;

#if defined(CONFIG_PANELLED_RDM_PORT)
					hal::panel_led_on(hal::panelled::PORT_A_RDM << nPortIndex);
#elif defined(CONFIG_PANELLED_RDM_NO_PORT)
					hal::panel_led_on(hal::panelled::RDM << nPortIndex);
#endif
				}
			}
		}
	}
}
