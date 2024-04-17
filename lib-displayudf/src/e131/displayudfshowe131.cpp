/**
 * @file displayudfshowe131.cpp
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
#include <algorithm>

#include "displayudf.h"
#include "e131bridge.h"
#include "e131.h"

#include "debug.h"

namespace e131bridge {
namespace configstore {
extern uint32_t DMXPORT_OFFSET;
}  // namespace configstore
}  // namespace e131bridge

void DisplayUdf::ShowE131Bridge() {
	DEBUG_ENTRY
	DEBUG_PRINTF("e131bridge::configstore::DMXPORT_OFFSET=%u", e131bridge::configstore::DMXPORT_OFFSET);

	auto *pE131Bridge = E131Bridge::Get();

	Printf(m_aLabels[static_cast<uint32_t>(displayudf::Labels::AP)], "AP: %d", pE131Bridge->GetActiveOutputPorts() + pE131Bridge->GetActiveInputPorts());

	for (uint32_t nBridgePortIndex = 0; nBridgePortIndex < std::min(static_cast<uint32_t>(4), e131bridge::MAX_PORTS); nBridgePortIndex++) {
		const auto nPortIndex = nBridgePortIndex + e131bridge::configstore::DMXPORT_OFFSET;

		if (nPortIndex >= std::min(static_cast<uint32_t>(4), e131bridge::MAX_PORTS)) {
			break;
		}

		const auto nLabelIndex = static_cast<uint32_t>(displayudf::Labels::UNIVERSE_PORT_A) + nBridgePortIndex;

		if (nLabelIndex != 0xFF) {
			uint16_t nUniverse;
			if (pE131Bridge->GetUniverse(nPortIndex, nUniverse, lightset::PortDir::OUTPUT)) {
				Printf(m_aLabels[nLabelIndex], "Port %c: %d %s", ('A' + nBridgePortIndex), nUniverse, lightset::get_merge_mode(pE131Bridge->GetMergeMode(nPortIndex), true));
			}
		}
	}

	ShowDmxInfo();

	DEBUG_EXIT
}
