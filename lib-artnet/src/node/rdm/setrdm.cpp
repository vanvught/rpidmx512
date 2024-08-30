/**
 * @file setrdm.cpp
 *
 */
/* Copyright (C) 2023-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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
#include "artnetstore.h"

#include "debug.h"

void ArtNetNode::SetRdm(const uint32_t nPortIndex, const bool bEnable) {
	DEBUG_ENTRY
	assert(nPortIndex < artnetnode::MAX_PORTS);

	const auto isEnabled = !((m_OutputPort[nPortIndex].GoodOutputB & artnet::GoodOutputB::RDM_DISABLED) == artnet::GoodOutputB::RDM_DISABLED);

	if (isEnabled == bEnable) {
		DEBUG_EXIT
		return;
	}

	if (!bEnable) {
		m_OutputPort[nPortIndex].GoodOutputB |= artnet::GoodOutputB::RDM_DISABLED;
	} else {
		m_OutputPort[nPortIndex].GoodOutputB &= static_cast<uint8_t>(~artnet::GoodOutputB::RDM_DISABLED);
	}

	if (m_State.status == artnet::Status::ON) {
		ArtNetStore::SaveRdmEnabled(nPortIndex, bEnable);
		artnet::display_rdm_enabled(nPortIndex, bEnable);
	}

	DEBUG_EXIT
}

void ArtNetNode::SetRdmDiscovery(const uint32_t nPortIndex, const bool bEnable) {
	DEBUG_ENTRY
	assert(nPortIndex < artnetnode::MAX_PORTS);

	const auto isEnabled = !((m_OutputPort[nPortIndex].GoodOutputB & artnet::GoodOutputB::DISCOVERY_DISABLED) == artnet::GoodOutputB::DISCOVERY_DISABLED);

	if (isEnabled == bEnable) {
		DEBUG_EXIT
		return;
	}

	if (!bEnable) {
		m_OutputPort[nPortIndex].GoodOutputB |= artnet::GoodOutputB::DISCOVERY_DISABLED;
	} else {
		m_OutputPort[nPortIndex].GoodOutputB &= static_cast<uint8_t>(~artnet::GoodOutputB::DISCOVERY_DISABLED);
	}

	DEBUG_EXIT
}
