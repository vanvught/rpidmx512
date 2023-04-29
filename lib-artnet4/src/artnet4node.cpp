/**
 * @file artnet4node.cpp
 *
 */
/**
 * Art-Net Designed by and Copyright Artistic Licence Holdings Ltd.
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
#include <cstring>
#include <cstdio>
#include <cassert>

#include "artnet4node.h"
#include "artnet.h"

#include "e131bridge.h"

#include "debug.h"

using namespace artnet;

ArtNet4Node::ArtNet4Node() {
	DEBUG_ENTRY
	assert((artnetnode::MAX_PORTS) <= e131bridge::MAX_PORTS);

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

		DEBUG_PRINTF("\tProtocol %s", artnet::get_protocol_mode(tPortProtocol));

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
	DEBUG_PRINTF("artnetnode::PAGES=%u", artnetnode::PAGES);

	for (uint32_t nPortIndex = 0; nPortIndex < artnetnode::MAX_PORTS; nPortIndex++) {
		uint16_t nUniverse;
		const bool isActive = GetPortAddress(nPortIndex, nUniverse, lightset::PortDir::OUTPUT);
		
		DEBUG_PRINTF("Port %d, Active %c, Universe %d", nPortIndex, isActive ? 'Y' : 'N', nUniverse);
		
		if (isActive) {
			const auto tPortProtocol = GetPortProtocol(nPortIndex);
			
			DEBUG_PRINTF("\tProtocol %s", artnet::get_protocol_mode(tPortProtocol));
			
			if (tPortProtocol == PortProtocol::SACN) {
				const auto mergeMode = ArtNetNode::GetMergeMode(nPortIndex);
				m_Bridge.SetMergeMode(nPortIndex, mergeMode);
				DEBUG_PRINTF("\tMerge mode %s", lightset::get_merge_mode(mergeMode));
			}
		}
	}

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
	DEBUG_PRINTF("artnetnode::PAGES=%u", artnetnode::PAGES);

	for (uint32_t i = 0; i < artnetnode::MAX_PORTS; i++) {
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

	case PortCommand::LED_NORMAL:
		m_Bridge.SetEnableDataIndicator(true);
		break;
	case PortCommand::LED_MUTE:
		m_Bridge.SetEnableDataIndicator(false);
		break;
	case PortCommand::LED_LOCATE:
		m_Bridge.SetEnableDataIndicator(false);
		break;

	case PortCommand::MERGE_LTP_O:
	case PortCommand::MERGE_LTP_1:
	case PortCommand::MERGE_LTP_2:
	case PortCommand::MERGE_LTP_3:
		m_Bridge.SetMergeMode(nPort, lightset::MergeMode::LTP);
		break;

	case PortCommand::MERGE_HTP_0:
	case PortCommand::MERGE_HTP_1:
	case PortCommand::MERGE_HTP_2:
	case PortCommand::MERGE_HTP_3:
		m_Bridge.SetMergeMode(nPort, lightset::MergeMode::HTP);
		break;

	case PortCommand::CLR_0:
	case PortCommand::CLR_1:
	case PortCommand::CLR_2:
	case PortCommand::CLR_3:
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
	assert(nPortIndex < e131bridge::MAX_PORTS);

	uint16_t nUniverse;
	const auto isActive = m_Bridge.GetUniverse(nPortIndex, nUniverse, lightset::PortDir::OUTPUT);

	DEBUG_PRINTF("Port %u, Active %c, Universe %d", nPortIndex, isActive ? 'Y' : 'N', nUniverse);

	if (isActive) {
		uint8_t nStatus = GoodOutput::OUTPUT_IS_SACN;
		nStatus = nStatus | (m_Bridge.IsTransmitting(nPortIndex) ? GoodOutput::DATA_IS_BEING_TRANSMITTED : GoodOutput::OUTPUT_NONE);
		nStatus = nStatus | (m_Bridge.IsMerging(nPortIndex) ? GoodOutput::OUTPUT_IS_MERGING : GoodOutput::OUTPUT_NONE);
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
