/**
 * @file artnetnode.cpp
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

#ifndef NDEBUG
# define NDEBUG
#endif

#include <cstdint>

#include "artnetnode.h"
#include "artnet.h"
#include "artnetstore.h"

#include "e131bridge.h"

#if (ARTNET_VERSION < 4)
# error ARTNET_VERSION is not 4
#endif

void ArtNetNode::SetPortProtocol4(const uint32_t nPortIndex, const artnet::PortProtocol portProtocol) {
	DEBUG_PRINTF("nPortIndex=%u, PortProtocol=%s", nPortIndex, artnet::get_protocol_mode(portProtocol, false));

	if (nPortIndex >= artnetnode::MAX_PORTS) {
		DEBUG_EXIT
		return;
	}

	m_Node.Port[nPortIndex].protocol = portProtocol;

	if (portProtocol == artnet::PortProtocol::SACN) {
		m_OutputPort[nPortIndex].GoodOutput |= artnet::GoodOutput::OUTPUT_IS_SACN;
	} else {
		m_OutputPort[nPortIndex].GoodOutput &= static_cast<uint8_t>(~artnet::GoodOutput::OUTPUT_IS_SACN);
	}

	if (m_State.status == artnet::Status::ON) {
		ArtNetStore::SavePortProtocol(nPortIndex, portProtocol);
		artnet::display_port_protocol(nPortIndex, portProtocol);
	}

	DEBUG_EXIT
}

void ArtNetNode::SetUniverse4(const uint32_t nPortIndex, const lightset::PortDir portDirection) {
	DEBUG_ENTRY

	uint16_t nUniverse = 0;
	const bool isActive = ArtNetNode::GetPortAddress(nPortIndex, nUniverse, portDirection);
	const auto portProtocol = m_Node.Port[nPortIndex].protocol;

	DEBUG_PRINTF("Port %u, Active %c, Universe %d, Protocol %s [%s]", nPortIndex, isActive ? 'Y' : 'N', nUniverse, artnet::get_protocol_mode(portProtocol), portDirection == lightset::PortDir::OUTPUT ? "Output" : "Input");

	if (portProtocol == artnet::PortProtocol::SACN) {
		if (!isActive) {
			if (E131Bridge::GetUniverse(nPortIndex, nUniverse, lightset::PortDir::INPUT) || E131Bridge::GetUniverse(nPortIndex, nUniverse, lightset::PortDir::OUTPUT)) {
				E131Bridge::SetUniverse(nPortIndex, lightset::PortDir::DISABLE, nUniverse);

				DEBUG_EXIT
				return;
			}

			DEBUG_EXIT
			return;
		}

		if (IsMapUniverse0()) {
			nUniverse++;
		}

		if (nUniverse == 0) {
			DEBUG_EXIT
			return;
		}

		E131Bridge::SetUniverse(nPortIndex, portDirection, nUniverse);

		DEBUG_EXIT
		return;
	}

	DEBUG_EXIT
}

static hardware::ledblink::Mode s_mode;

void ArtNetNode::SetLedBlinkMode4(hardware::ledblink::Mode mode) {
	if (s_mode != mode) {
		s_mode = mode;
		DEBUG_PRINTF("mode=%u", static_cast<uint32_t>(mode));
	}

	E131Bridge::SetEnableDataIndicator(mode == hardware::ledblink::Mode::NORMAL);

	for (uint32_t nPortIndex = 0; nPortIndex < e131bridge::MAX_PORTS; nPortIndex++) {
		if (E131Bridge::IsTransmitting(nPortIndex)) {
			return;
		}
	}

	Hardware::Get()->SetMode(mode);
}

uint8_t ArtNetNode::GetGoodOutput4(const uint32_t nPortIndex) {
	assert(nPortIndex < e131bridge::MAX_PORTS);

	uint16_t nUniverse;
	const auto isActive = E131Bridge::GetUniverse(nPortIndex, nUniverse, lightset::PortDir::OUTPUT);

	DEBUG_PRINTF("Port %u, Active %c, Universe %d, %s", nPortIndex, isActive ? 'Y' : 'N', nUniverse, lightset::get_merge_mode(E131Bridge::GetMergeMode(nPortIndex), true));

	if (isActive) {
		uint8_t nStatus = artnet::GoodOutput::OUTPUT_IS_SACN;
		nStatus = nStatus | (E131Bridge::IsTransmitting(nPortIndex) ? artnet::GoodOutput::DATA_IS_BEING_TRANSMITTED : artnet::GoodOutput::OUTPUT_NONE);
		nStatus = nStatus | (E131Bridge::IsMerging(nPortIndex) ? artnet::GoodOutput::OUTPUT_IS_MERGING : artnet::GoodOutput::OUTPUT_NONE);
		return nStatus;
	}

	return 0;
}
