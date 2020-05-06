/**
 * @file artnetnodeprint.cpp
 *
 */
/**
 * Art-Net Designed by and Copyright Artistic Licence Holdings Ltd.
 */
/* Copyright (C) 2018-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <stdio.h>

#include "artnetnode.h"

#include "network.h"

#define MERGEMODE2STRING(m)		(m == ARTNET_MERGE_HTP) ? "HTP" : "LTP"
#define PROTOCOL2STRING(p)		(p == PORT_ARTNET_ARTNET) ? "Art-Net" : "sACN"

void ArtNetNode::Print(void) {
	const uint8_t *pSoftwareVersion = GetSoftwareVersion();

	printf("Node %d\n", m_nVersion);
	printf(" Firmware   : %d.%d\n", pSoftwareVersion[0], pSoftwareVersion[1]);
	printf(" Short name : %s\n", m_Node.ShortName);
	printf(" Long name  : %s\n", m_Node.LongName);

	if (m_State.nActiveOutputPorts != 0) {
		printf(" Output\n");

		for (uint32_t nPortIndex = 0; nPortIndex < (m_nPages * ARTNET_MAX_PORTS); nPortIndex++) {
			uint8_t nAddress;
			if (GetUniverseSwitch(nPortIndex, nAddress)) {
				const uint8_t nNet = m_Node.NetSwitch[nPortIndex / ARTNET_MAX_PORTS];
				const uint8_t nSubSwitch = m_Node.SubSwitch[nPortIndex / ARTNET_MAX_PORTS];

				printf("  Port %2d %d:%-3d[%2x] [%s]", nPortIndex, nNet, nSubSwitch * 16 + nAddress, nSubSwitch * 16 + nAddress, MERGEMODE2STRING(m_OutputPorts[nPortIndex].mergeMode));
				if (m_nVersion == 4) {
					printf(" {%s}\n", PROTOCOL2STRING(m_OutputPorts[nPortIndex].tPortProtocol));
				} else {
					printf("\n");
				}
			}
		}

		if (m_bDirectUpdate) {
			printf(" Direct update : Yes\n");
		}
	}

	if (m_State.nActiveInputPorts != 0) {
		printf(" Input\n");

		for (uint32_t nPortIndex = 0; nPortIndex < (ARTNET_NODE_MAX_PORTS_INPUT); nPortIndex++) {
			uint8_t nAddress;
			if (GetUniverseSwitch(nPortIndex, nAddress, ARTNET_INPUT_PORT)) {
				const uint32_t nNet = m_Node.NetSwitch[nPortIndex];
				const uint32_t nSubSwitch = m_Node.SubSwitch[nPortIndex];
				const uint32_t nDestinationIp = (m_InputPorts[nPortIndex].nDestinationIp == 0 ? Network::Get()->GetBroadcastIp() : m_InputPorts[nPortIndex].nDestinationIp);

				printf("  Port %2d %d:%-3d[%2x] -> " IPSTR "\n", nPortIndex, nNet, nSubSwitch * 16 + nAddress, nSubSwitch * 16 + nAddress, IP2STR(nDestinationIp));
			}
		}
	}
}
