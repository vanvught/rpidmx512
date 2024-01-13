/**
 * @file handlerdm.cpp
 *
 */
/* Copyright (C) 2023 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include "artnetnode.h"

#include "debug.h"

/**
 * An Output Gateway must not interpret receipt of an ArtTodRequest
 * as an instruction to perform full RDM Discovery on the DMX512 physical layer;
 * it is just a request to send the ToD back to the controller.
 */
void ArtNetNode::HandleTodRequest() {
	DEBUG_ENTRY

	const auto *const pArtTodRequest = reinterpret_cast<artnet::ArtTodRequest *>(m_pReceiveBuffer);
	const auto nAddCount = pArtTodRequest->AddCount & 0x1f;

	for (auto nCount = 0; nCount < nAddCount; nCount++) {
		const auto portAddress = static_cast<uint16_t>((pArtTodRequest->Net << 8)) | static_cast<uint16_t>((pArtTodRequest->Address[nCount]));

		for (uint32_t nPortIndex = 0; nPortIndex < artnetnode::MAX_PORTS; nPortIndex++) {
			if ((m_OutputPort[nPortIndex].GoodOutputB & artnet::GoodOutputB::RDM_DISABLED) == artnet::GoodOutputB::RDM_DISABLED) {
				continue;
			}

			if ((portAddress == m_Node.Port[nPortIndex].PortAddress) && (m_Node.Port[nPortIndex].direction == lightset::PortDir::OUTPUT)) {
				SendTod(nPortIndex);
			}
		}
	}

	DEBUG_EXIT
}
