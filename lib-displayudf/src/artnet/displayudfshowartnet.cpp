/**
 * @file displayudfshowartnet.cpp
 *
 */
/* Copyright (C) 2019-2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

using namespace displayudf;
using namespace artnet;

void DisplayUdf::Show(ArtNetNode *pArtNetNode) {
	DEBUG_ENTRY

	Show();

	ShowNodeName(pArtNetNode);
	ShowUniverse(pArtNetNode);
	ShowDestinationIp(pArtNetNode);

	Printf(m_aLabels[static_cast<uint32_t>(Labels::AP)], "AP: %d", pArtNetNode->GetActiveOutputPorts() + pArtNetNode->GetActiveInputPorts());

	ShowDmxInfo();

	DEBUG_EXIT
}

void DisplayUdf::ShowNodeName(ArtNetNode *pArtNetNode) {
	ClearLine(m_aLabels[static_cast<uint32_t>(Labels::NODE_NAME)]);
	Write(m_aLabels[static_cast<uint32_t>(Labels::NODE_NAME)], pArtNetNode->GetShortName());
}

void DisplayUdf::ShowUniverse(ArtNetNode *pArtNetNode) {
	uint8_t nAddress;

	if (pArtNetNode->GetUniverseSwitch(0, nAddress, lightset::PortDir::OUTPUT)) {
		Printf(m_aLabels[static_cast<uint32_t>(Labels::UNIVERSE)],
				"O: %.2d:%d:%d %c %s",
				pArtNetNode->GetNetSwitch(0),
				pArtNetNode->GetSubnetSwitch(0),
				nAddress,
				lightset::get_merge_mode(pArtNetNode->GetMergeMode(0), true),
				pArtNetNode->GetPortProtocol(0) == PortProtocol::ARTNET ? "    " : "sACN");
	}

	for (uint32_t nPortIndex = 0; nPortIndex < ArtNet::PORTS; nPortIndex++) {
		if (pArtNetNode->GetUniverseSwitch(nPortIndex, nAddress, lightset::PortDir::OUTPUT)) {
			const auto nPage = nPortIndex / ArtNet::PORTS;
			Printf(m_aLabels[static_cast<uint32_t>(Labels::UNIVERSE_PORT_A) + nPortIndex],
					"O%d: %.2d:%d:%d %c %s", (nPortIndex + 1),
					pArtNetNode->GetNetSwitch(nPage),
					pArtNetNode->GetSubnetSwitch(nPage),
					nAddress,
					lightset::get_merge_mode(pArtNetNode->GetMergeMode(nPortIndex), true),
					pArtNetNode->GetPortProtocol(nPortIndex) == PortProtocol::ARTNET ? "    " : "sACN");
		}
	}
}

void DisplayUdf::ShowDestinationIp(ArtNetNode *pArtNetNode) {
	for (uint32_t i = 0; i < ArtNet::PORTS; i++) {
		Printf(m_aLabels[static_cast<uint32_t>(Labels::DESTINATION_IP_PORT_A) + i], "%c: " IPSTR, 'A' + i, IP2STR(pArtNetNode->GetDestinationIp(i)));
	}
}
