/**
 * @file displayudfshowartnet.cpp
 *
 */
/* Copyright (C) 2019-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "displayudf.h"

#include "artnetnode.h"
#include "artnet.h"

#include "network.h"

#include "debug.h"

void DisplayUdf::Show(ArtNetNode *pArtNetNode, uint32_t nDmxPortIndexOffset) {
	DEBUG_ENTRY

	m_nPortIndexOffset = nDmxPortIndexOffset;

	DEBUG_PRINTF("m_nPortIndexOffset=%u", m_nPortIndexOffset);

	Show();

	ShowNodeName(pArtNetNode);
	ShowUniverse(pArtNetNode);
#if defined (ARTNET_HAVE_DMXIN)
	ShowDestinationIp(pArtNetNode);
#endif
	Printf(m_aLabels[static_cast<uint32_t>(displayudf::Labels::AP)], "AP: %d", pArtNetNode->GetActiveOutputPorts() + pArtNetNode->GetActiveInputPorts());

	ShowDmxInfo();

	DEBUG_EXIT
}

void DisplayUdf::ShowNodeName(ArtNetNode *pArtNetNode) {
	ClearLine(m_aLabels[static_cast<uint32_t>(displayudf::Labels::NODE_NAME)]);
	Write(m_aLabels[static_cast<uint32_t>(displayudf::Labels::NODE_NAME)], pArtNetNode->GetShortName());
}

void DisplayUdf::ShowUniverse(ArtNetNode *pArtNetNode) {
	uint16_t nUniverse;

	for (uint32_t nArtNetPortIndex = 0; nArtNetPortIndex < artnet::PORTS; nArtNetPortIndex++) {
		const auto nPortIndex = nArtNetPortIndex + m_nPortIndexOffset;
		const auto nLabelIndex = static_cast<uint32_t>(displayudf::Labels::UNIVERSE_PORT_A) + nArtNetPortIndex;

		if (nLabelIndex != 0xFF) {
			if (pArtNetNode->GetPortAddress(nPortIndex, nUniverse, lightset::PortDir::OUTPUT)) {
				ClearLine(m_aLabels[nLabelIndex]);
				Printf(m_aLabels[nLabelIndex],
						"%c: %d %s %s %s",
						'A' + nArtNetPortIndex,
						nUniverse,
						lightset::get_merge_mode(pArtNetNode->GetMergeMode(nPortIndex), true),
						artnet::get_protocol_mode(pArtNetNode->GetPortProtocol(nPortIndex), true),
						pArtNetNode->GetRdm(nPortIndex) ? "RDM" : "");
			}
		}
	}
}

void DisplayUdf::ShowDestinationIp(ArtNetNode *pArtNetNode) {
	for (uint32_t nPortIndex = 0; nPortIndex < artnet::PORTS; nPortIndex++) {
		Printf(m_aLabels[static_cast<uint32_t>(displayudf::Labels::DESTINATION_IP_PORT_A) + nPortIndex], "%c: " IPSTR, 'A' + nPortIndex, IP2STR(pArtNetNode->GetDestinationIp(nPortIndex)));
	}
}
