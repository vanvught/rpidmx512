/**
 * @file handlerdmin.cpp
 *
 */
/**
 * Art-Net Designed by and Copyright Artistic Licence Holdings Ltd.
 */
/* Copyright (C) 2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include "artnetrdm.h"

#include "network.h"
#include "hardware.h"

#include "panel_led.h"

#include "debug.h"

void ArtNetNode::HandleRdmIn() {
	auto *const pArtRdm = &m_ArtTodPacket.ArtRdm;

	for (uint32_t nPortIndex = 0; nPortIndex < artnetnode::MAX_PORTS; nPortIndex++) {
		if ((m_Node.Port[nPortIndex].direction != lightset::PortDir::INPUT)) {
			continue;
		}

		const auto nPage = nPortIndex;

		if (m_pArtNetRdm->RdmReceive(nPortIndex, pArtRdm->RdmPacket)) {
			memcpy(pArtRdm->Id, artnet::NODE_ID, sizeof(pArtRdm->Id));
			pArtRdm->OpCode = static_cast<uint16_t>(artnet::OpCodes::OP_RDM);
			pArtRdm->ProtVerHi = 0;
			pArtRdm->ProtVerLo = artnet::PROTOCOL_REVISION;
			pArtRdm->RdmVer = 0x01;
			pArtRdm->Net = m_Node.Port[nPage].NetSwitch;
			pArtRdm->Command = 0;
			pArtRdm->Address = m_Node.Port[nPortIndex].DefaultAddress;

			Network::Get()->SendTo(m_nHandle, pArtRdm, sizeof(struct artnet::ArtRdm), m_InputPort[nPortIndex].nDestinationIp, artnet::UDP_PORT);
		}
	}
}
