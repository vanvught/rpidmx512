/**
 * @file handleinput.cpp
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
#if (ARTNET_VERSION >= 4) && defined (E131_HAVE_DMXIN)
# include "e131bridge.h"
#endif

#include "debug.h"

/**
 * A Controller or monitoring device on the network can
 * enable or disable individual DMX512 inputs on any of the network nodes.
 * This allows the Controller to directly control network traffic and
 * ensures that unused inputs are disabled and therefore not wasting bandwidth.
 */

void ArtNetNode::HandleInput() {
	DEBUG_ENTRY

	const auto *const pArtInput = reinterpret_cast<artnet::ArtInput *>(m_pReceiveBuffer);
	const auto nPortIndex = static_cast<uint32_t>(pArtInput->BindIndex > 0 ? pArtInput->BindIndex - 1 : 0);

	if (pArtInput->NumPortsLo == 1) {
		if (m_Node.Port[nPortIndex].direction == lightset::PortDir::INPUT) {
			if (pArtInput->Input[0] & 0x01) {
				m_InputPort[nPortIndex].GoodInput |= static_cast<uint8_t>(artnet::GoodInput::DISABLED);		
			} else {
				m_InputPort[nPortIndex].GoodInput &= static_cast<uint8_t>(~static_cast<uint8_t>(artnet::GoodInput::DISABLED));
			}
#if (ARTNET_VERSION >= 4) && defined (E131_HAVE_DMXIN)
			E131Bridge::SetInputDisabled(nPortIndex, pArtInput->Input[0] & 0x01);
#endif	
		}
	}

	if (m_State.SendArtPollReplyOnChange) {
		SendPollRelply(0, m_nIpAddressFrom);
	}

	DEBUG_EXIT
}
