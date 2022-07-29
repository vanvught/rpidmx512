/**
 * @file artnet4node.cpp
 *
 */
/**
 * Art-Net Designed by and Copyright Artistic Licence Holdings Ltd.
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
#include <cstring>
#include <cstdio>
#include <cassert>

#include "artnet4node.h"
#include "artnet.h"

#include "e131bridge.h"

#include "debug.h"

using namespace artnet;

ArtNet4Node::ArtNet4Node(uint8_t nPages) : ArtNetNode(nPages) {
	DEBUG_ENTRY
	assert((ArtNet::PORTS * nPages) <= E131::PORTS);

	ArtNetNode::SetArtNet4Handler(static_cast<ArtNet4Handler*>(this));

	DEBUG_EXIT
}

void ArtNet4Node::SetPort(uint32_t nPortIndex, lightset::PortDir dir) {
	DEBUG_ENTRY

	uint16_t nUniverse;
	const bool isActive = GetPortAddress(nPortIndex, nUniverse, dir);

	DEBUG_PRINTF("Port %u, Active %c, Universe %d, [%s]", nPortIndex, isActive ? 'Y' : 'N', nUniverse, dir == lightset::PortDir::OUTPUT ? "Output" : "Input");

	if (dir == lightset::PortDir::INPUT) {
		DEBUG_PUTS("Input is not supported");
		return;
	}

	if (isActive) {
		const auto tPortProtocol = GetPortProtocol(nPortIndex);

		DEBUG_PRINTF("\tProtocol %s", ArtNet::GetProtocolMode(tPortProtocol));

		if (tPortProtocol == PortProtocol::SACN) {

			if (IsMapUniverse0()) {
				nUniverse++;
			}

			if (nUniverse == 0) {
				SetUniverseSwitch(nPortIndex, lightset::PortDir::DISABLE, 0);
				return;
			}

			m_Bridge.SetUniverse(nPortIndex, dir == lightset::PortDir::OUTPUT ? lightset::PortDir::OUTPUT : lightset::PortDir::INPUT, nUniverse);
		}
	}

	DEBUG_EXIT
}

void ArtNet4Node::Start() {
	DEBUG_ENTRY
	DEBUG_PRINTF("m_nPages=%d", GetPages());

	for (uint32_t nPortIndex = 0; nPortIndex < (ArtNet::PORTS * GetPages()); nPortIndex++) {
		uint16_t nUniverse;
		const bool isActive = GetPortAddress(nPortIndex, nUniverse, lightset::PortDir::OUTPUT);
		
		DEBUG_PRINTF("Port %d, Active %c, Universe %d", nPortIndex, isActive ? 'Y' : 'N', nUniverse);
		
		if (isActive) {
			const auto tPortProtocol = GetPortProtocol(nPortIndex);
			
			DEBUG_PRINTF("\tProtocol %s", ArtNet::GetProtocolMode(tPortProtocol));
			
			if (tPortProtocol == PortProtocol::SACN) {
				const auto mergeMode = ArtNetNode::GetMergeMode(nPortIndex);
				m_Bridge.SetMergeMode(nPortIndex, mergeMode);
				DEBUG_PRINTF("\tMerge mode %s", lightset::get_merge_mode(mergeMode));
			}
		}
	}

	m_Bridge.SetDisableNetworkDataLossTimeout(ArtNetNode::GetNetworkTimeout() == 0);
	m_Bridge.SetDisableMergeTimeout(ArtNet4Node::GetDisableMergeTimeout());
	m_Bridge.SetOutput(ArtNetNode::GetOutput());

	ArtNetNode::Start();
	m_Bridge.Start();

	DEBUG_EXIT
}

void ArtNet4Node::Stop() {
	DEBUG_ENTRY

	m_Bridge.Stop();
	ArtNetNode::Stop();

	DEBUG_EXIT
}

void ArtNet4Node::Run() {
	ArtNetNode::Run();

	if (m_Bridge.GetActiveOutputPorts() != 0) {
		m_Bridge.Run();
	}
}

void ArtNet4Node::HandleAddress(uint8_t nCommand) {
	DEBUG_ENTRY
	DEBUG_PRINTF("m_nPages=%d", GetPages());

	for (uint32_t i = 0; i < (ArtNet::PORTS * GetPages()); i++) {
		uint16_t nUniverse;
		const bool isActive = GetPortAddress(i, nUniverse, lightset::PortDir::OUTPUT);

		if (isActive) {
			if (IsMapUniverse0()) {
				nUniverse++;
			}

			if (nUniverse == 0) {
				continue;
			}

			if (GetPortProtocol(i) == PortProtocol::SACN) {
				m_Bridge.SetUniverse(i, lightset::PortDir::OUTPUT, nUniverse);
			} else {
				m_Bridge.SetUniverse(i, lightset::PortDir::DISABLE, nUniverse);
			}
		}
	}

	const uint8_t nPort = nCommand & 0x3;
	DEBUG_PRINTF("nPort=%d", nPort);

	switch (nCommand) {

	case PortCommand::PC_LED_NORMAL:
		m_Bridge.SetEnableDataIndicator(true);
		break;
	case PortCommand::PC_LED_MUTE:
		m_Bridge.SetEnableDataIndicator(false);
		break;
	case PortCommand::PC_LED_LOCATE:
		m_Bridge.SetEnableDataIndicator(false);
		break;

	case PortCommand::PC_MERGE_LTP_O:
	case PortCommand::PC_MERGE_LTP_1:
	case PortCommand::PC_MERGE_LTP_2:
	case PortCommand::PC_MERGE_LTP_3:
		m_Bridge.SetMergeMode(nPort, lightset::MergeMode::LTP);
		break;

	case PortCommand::PC_MERGE_HTP_0:
	case PortCommand::PC_MERGE_HTP_1:
	case PortCommand::PC_MERGE_HTP_2:
	case PortCommand::PC_MERGE_HTP_3:
		m_Bridge.SetMergeMode(nPort, lightset::MergeMode::HTP);
		break;

	case PortCommand::PC_CLR_0:
	case PortCommand::PC_CLR_1:
	case PortCommand::PC_CLR_2:
	case PortCommand::PC_CLR_3:
		if (GetPortProtocol(nPort) == PortProtocol::SACN) {
			m_Bridge.Clear(nPort);
		}
		break;

	default:
		break;
	}

	DEBUG_EXIT
}

uint8_t ArtNet4Node::GetStatus(uint32_t nPortIndex) {
	assert(nPortIndex < E131::PORTS);

	uint16_t nUniverse;
	const auto isActive = m_Bridge.GetUniverse(nPortIndex, nUniverse, lightset::PortDir::OUTPUT);

	DEBUG_PRINTF("Port %u, Active %c, Universe %d", nPortIndex, isActive ? 'Y' : 'N', nUniverse);

	if (isActive) {
		uint8_t nStatus = GoodOutput::GO_OUTPUT_IS_SACN;
		nStatus = nStatus | (m_Bridge.IsTransmitting(nPortIndex) ? GoodOutput::GO_DATA_IS_BEING_TRANSMITTED : GoodOutput::GO_OUTPUT_NONE);
		nStatus = nStatus | (m_Bridge.IsMerging(nPortIndex) ? GoodOutput::GO_OUTPUT_IS_MERGING : GoodOutput::GO_OUTPUT_NONE);
		return nStatus;
	}

	return 0;
}

void ArtNet4Node::Print() {
	ArtNetNode::Print();

	if (ArtNetNode::GetActiveOutputPorts() != 0) {
		if (IsMapUniverse0()) {
			printf("  Universes are mappped +1\n");
		}

		m_Bridge.Print();
	}
}
