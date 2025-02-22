/**
 * @file displayudfshowartnet.cpp
 *
 */
/* Copyright (C) 2019-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#if defined (DEBUG_DISPLAYUDF)
# undef NDEBUG
#endif

#include <cstdint>
#include <algorithm>

#include "displayudf.h"

#include "artnetnode.h"
#include "artnet.h"

#include "dmxnode.h"

#include "network.h"

#include "debug.h"

void DisplayUdf::ShowArtNetNode() {
	DEBUG_ENTRY
	DEBUG_PRINTF("dmxnode::DMXPORT_OFFSET=%u", dmxnode::DMXPORT_OFFSET);

	auto *pArtNetNode = ArtNetNode::Get();

	ShowUniverseArtNetNode();
#if defined (ARTNET_HAVE_DMXIN)
	ShowDestinationIpArtNetNode();
#endif
	Printf(m_aLabels[static_cast<uint32_t>(displayudf::Labels::AP)], "AP: %d", pArtNetNode->GetActiveOutputPorts() + pArtNetNode->GetActiveInputPorts());

	ShowDmxInfo();

	DEBUG_EXIT
}

void DisplayUdf::ShowUniverseArtNetNode() {
	DEBUG_ENTRY
	if constexpr (dmxnode::CONFIG_PORT_COUNT != 0) {
		auto *pArtNetNode = ArtNetNode::Get();
		uint16_t nUniverse;

		for (uint32_t nArtNetPortIndex = 0; nArtNetPortIndex < std::min(artnetnode::MAX_PORTS, dmxnode::PARAM_PORTS); nArtNetPortIndex++) {
			const auto nPortIndex = nArtNetPortIndex + dmxnode::DMXPORT_OFFSET;

			if (nPortIndex >= artnetnode::MAX_PORTS) {
				break;
			}

			const auto nLabelIndex = static_cast<uint32_t>(displayudf::Labels::UNIVERSE_PORT_A) + nArtNetPortIndex;

			if (nLabelIndex != 0xFF) {
				if (pArtNetNode->GetPortAddress(nPortIndex, nUniverse, dmxnode::PortDirection::OUTPUT)) {
					ClearEndOfLine();
					Printf(m_aLabels[nLabelIndex],
#if defined (OUTPUT_HAVE_STYLESWITCH)
							"%c %d %s %s %c %s",
#else
							"%c %d %s %s %s",
#endif
							'A' + nArtNetPortIndex,
							nUniverse,
							dmxnode::get_merge_mode(pArtNetNode->GetMergeMode(nPortIndex), true),
#if (ARTNET_VERSION >= 4)
							artnet::get_protocol_mode(pArtNetNode->GetPortProtocol4(nPortIndex), true),
#else
							"Art-Net",
#endif
#if defined (OUTPUT_HAVE_STYLESWITCH)
							pArtNetNode->GetOutputStyle(nPortIndex) == dmxnode::OutputStyle::CONSTANT ? 'C' : 'D',
#endif
									pArtNetNode->GetRdm(nPortIndex) ? "RDM" : "");
				}
			}

		}
	}
	DEBUG_EXIT
}

void DisplayUdf::ShowDestinationIpArtNetNode() {
	DEBUG_ENTRY
#if defined (ARTNET_HAVE_DMXIN)
	if constexpr (dmxnode::CONFIG_PORT_COUNT != 0) {
		auto *pArtNetNode = ArtNetNode::Get();

		for (uint32_t nArtNetPortIndex = 0; nArtNetPortIndex < std::min(artnetnode::MAX_PORTS, dmxnode::PARAM_PORTS); nArtNetPortIndex++) {
			const auto nPortIndex = nArtNetPortIndex + dmxnode::DMXPORT_OFFSET;

			if (nPortIndex >= artnetnode::MAX_PORTS) {
				break;
			}

			Printf(m_aLabels[static_cast<uint32_t>(displayudf::Labels::DESTINATION_IP_PORT_A) + nArtNetPortIndex], "%c: " IPSTR, 'A' + nPortIndex, IP2STR(pArtNetNode->GetDestinationIp(nPortIndex)));
		}
	}
#endif
	DEBUG_EXIT
}
