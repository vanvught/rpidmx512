/**
 * @file artnetnodeprint.cpp
 *
 */
/**
 * Art-Net Designed by and Copyright Artistic Licence Holdings Ltd.
 */
/* Copyright (C) 2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include <stdio.h>

#include "artnetnode.h"

#define MERGEMODE2STRING(m)		(m == ARTNET_MERGE_HTP) ? "HTP" : "LTP"
#define PROTOCOL2STRING(p)		(p == PORT_ARTNET_ARTNET) ? "Art-Net" : "sACN"

void ArtNetNode::Print(void) {
	const uint8_t *firmware_version = GetSoftwareVersion();

	printf("Node %d configuration\n", m_nVersion);
	printf(" Firmware   : %d.%d\n", firmware_version[0], firmware_version[1]);
	printf(" Short name : %s\n", m_Node.ShortName);
	printf(" Long name  : %s\n", m_Node.LongName);

	for (uint32_t i = 0; i < (m_nPages * ARTNET_MAX_PORTS); i++) {
		uint8_t nAddress;
		if (GetUniverseSwitch(i, nAddress)) {
			const uint8_t nNet = m_Node.NetSwitch[i/ ARTNET_MAX_PORTS];
			const uint8_t nSubSwitch = m_Node.SubSwitch[i/ ARTNET_MAX_PORTS];

			printf("  Port %c %d:%d:%d [%s]", (char) ('A' + i), nNet, nSubSwitch, nAddress, MERGEMODE2STRING(m_OutputPorts[i].mergeMode));
			if (m_nVersion == 4) {
				printf(" {%s}\n", PROTOCOL2STRING(m_OutputPorts[i].tPortProtocol));
			} else {
				printf("\n");
			}
		}
	}
}
