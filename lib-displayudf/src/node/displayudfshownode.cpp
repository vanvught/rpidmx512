/**
 * @file displayudfshownode.cpp
 *
 */
/* Copyright (C) 2022-2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "node.h"

#include "network.h"

#include "debug.h"

static constexpr auto MAX_PORTS = LIGHTSET_PORTS > 4U ? 4U : LIGHTSET_PORTS;

void DisplayUdf::ShowNode() {
	DEBUG_ENTRY

	ShowNodeNameNode();
	ShowUniverseNode();
	ShowDestinationIpNode();

	DEBUG_EXIT
}

void DisplayUdf::ShowNodeNameNode() {
	ClearEndOfLine();
	Write(m_aLabels[static_cast<uint32_t>(displayudf::Labels::NODE_NAME)], Node::Get()->GetShortName());
}

void DisplayUdf::ShowUniverseNode() {
	for (uint32_t nPortIndex = 0; nPortIndex < MAX_PORTS; nPortIndex++) {

	}
}

void DisplayUdf::ShowDestinationIpNode() {
	for (uint32_t i = 0; i < MAX_PORTS; i++) {
		Printf(m_aLabels[static_cast<uint32_t>(displayudf::Labels::DESTINATION_IP_PORT_A) + i], "%c: " IPSTR, 'A' + i, IP2STR(Node::Get()->GetDestinationIp(i)));
	}
}
