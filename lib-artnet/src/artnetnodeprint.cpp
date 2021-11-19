/**
 * @file artnetnodeprint.cpp
 *
 */
/**
 * Art-Net Designed by and Copyright Artistic Licence Holdings Ltd.
 */
/* Copyright (C) 2018-2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#if !defined(__clang__)	// Needed for compiling on MacOS
# pragma GCC push_options
# pragma GCC optimize ("Os")
#endif

#include <cstdint>
#include <cstdio>

#include "artnetnode.h"
#include "artnetconst.h"
#include "artnet.h"

#include "network.h"

using namespace artnet;

void ArtNetNode::Print() {
	printf("Node %d [%u]\n", ArtNet::VERSION, m_nPages);
	printf(" Firmware   : %d.%d\n", ArtNetConst::VERSION[0], ArtNetConst::VERSION[1]);
	printf(" Short name : %s\n", m_Node.ShortName);
	printf(" Long name  : %s\n", m_Node.LongName);

	if (m_State.nActiveOutputPorts != 0) {
		printf(" Output\n");

		for (uint32_t nPortIndex = 0; nPortIndex < (m_nPages * ArtNet::PORTS); nPortIndex++) {
			uint8_t nAddress;
			if (GetUniverseSwitch(nPortIndex, nAddress, lightset::PortDir::OUTPUT)) {
				const auto nPage = nPortIndex / ArtNet::PORTS;
				const auto nNet = m_Node.NetSwitch[nPage];
				const auto nSubSwitch = m_Node.SubSwitch[nPage];

				uint16_t nUniverse;
				GetPortAddress(nPortIndex, nUniverse, lightset::PortDir::OUTPUT);

				printf("  Port %2d %d:[%-2d:%-2d:%-2d]:%4u [%s]", nPortIndex, nPage, nNet, nSubSwitch, nAddress, nUniverse, lightset::get_merge_mode(m_OutputPort[nPortIndex].mergeMode, true));
				if (ArtNet::VERSION == 4) {
					printf(" {%s}\n", ArtNet::GetProtocolMode(m_OutputPort[nPortIndex].protocol, true));
				} else {
					printf("\n");
				}
			}
		}
	}

	if (m_State.nActiveInputPorts != 0) {
		printf(" Input\n");

		for (uint32_t nPortIndex = 0; nPortIndex < (m_nPages * ArtNet::PORTS); nPortIndex++) {
			uint8_t nAddress;
			if (GetUniverseSwitch(nPortIndex, nAddress, lightset::PortDir::INPUT)) {
				const auto nPage = nPortIndex / ArtNet::PORTS;
				const auto nNet = m_Node.NetSwitch[nPage];
				const auto nSubSwitch = m_Node.SubSwitch[nPage];
				const auto nDestinationIp = (m_InputPort[nPortIndex].nDestinationIp == 0 ? Network::Get()->GetBroadcastIp() : m_InputPort[nPortIndex].nDestinationIp);

				uint16_t nUniverse;
				GetPortAddress(nPortIndex, nUniverse, lightset::PortDir::INPUT);

				printf("  Port %2d %d:[%-2d:%-2d:%-2d]:%4u -> " IPSTR "\n", nPortIndex, nPage, nNet, nSubSwitch, nAddress, nUniverse, IP2STR(nDestinationIp));
			}
		}
	}
}
