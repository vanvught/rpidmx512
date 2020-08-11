/**
 * @file displayudfshowartnet.cpp
 *
 */
/* Copyright (C) 2019-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <stdint.h>

#include "displayudf.h"

#include "artnetnode.h"
#include "artnet.h"

#include "network.h"

#include "debug.h"

void DisplayUdf::Show(ArtNetNode *pArtNetNode) {
	DEBUG_ENTRY

	Show();

	ShowNodeName(pArtNetNode);
	ShowUniverse(pArtNetNode);
	ShowDestinationIp(pArtNetNode);

	Printf(m_aLabels[DISPLAY_UDF_LABEL_AP], "AP: %d", pArtNetNode->GetActiveOutputPorts() + pArtNetNode->GetActiveInputPorts());

	DEBUG_EXIT
}

void DisplayUdf::ShowNodeName(ArtNetNode *pArtNetNode) {
	ClearLine(m_aLabels[DISPLAY_UDF_LABEL_NODE_NAME]);
	Write(m_aLabels[DISPLAY_UDF_LABEL_NODE_NAME], pArtNetNode->GetShortName());
}

void DisplayUdf::ShowUniverse(ArtNetNode *pArtNetNode) {
	uint8_t nAddress;
	if (pArtNetNode->GetUniverseSwitch(0, nAddress)) {
		Printf(m_aLabels[DISPLAY_UDF_LABEL_UNIVERSE], "O: %.2d:%d:%d %c %s", pArtNetNode->GetNetSwitch(), pArtNetNode->GetSubnetSwitch(), nAddress, ArtNet::GetMergeMode(pArtNetNode->GetMergeMode(), true), pArtNetNode->GetPortProtocol() == PORT_ARTNET_ARTNET ? "    " : "sACN");
	}

	for (uint32_t i = 0; i < ArtNet::MAX_PORTS; i++) {
		if (pArtNetNode->GetUniverseSwitch(i, nAddress)) {
			Printf(m_aLabels[DISPLAY_UDF_LABEL_UNIVERSE_PORT_A + i], "O%d: %.2d:%d:%d %c %s", (i+1), pArtNetNode->GetNetSwitch(i), pArtNetNode->GetSubnetSwitch(i), nAddress, ArtNet::GetMergeMode(pArtNetNode->GetMergeMode(i), true), pArtNetNode->GetPortProtocol(i) == PORT_ARTNET_ARTNET ? "    " : "sACN");
		}
	}
}

void DisplayUdf::ShowDestinationIp(ArtNetNode *pArtNetNode) {
	for (uint32_t i = 0; i < ARTNET_NODE_MAX_PORTS_INPUT; i++) {
		Printf(m_aLabels[DISPLAY_UDF_LABEL_DESTINATION_IP_PORT_A + i], "%c: " IPSTR, 'A' + i, IP2STR(pArtNetNode->GetDestinationIp(i)));
	}
}
