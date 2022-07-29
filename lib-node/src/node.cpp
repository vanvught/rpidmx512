/**
 * @file node.cpp
 *
 */
/* Copyright (C) 2022 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <cstdio>
#include <cassert>

#include "node.h"
#include "nodemsgconst.h"

#include "display.h"

#include "debug.h"

Node *Node::s_pThis;

Node::Node(node::Personality personality, NodeStore *pNodeStore) :
	m_Personality(personality == node::Personality::NODE ? node::defaults::PERSONALITY : personality)
{
	DEBUG_ENTRY
	DEBUG_PRINTF("Personality=%s", node::get_personality_full(m_Personality));

	assert(s_pThis == nullptr);
	s_pThis = this;

	if (m_Personality == node::Personality::ARTNET) {
		ArtNetNode::SetArtNetStore(pNodeStore);
		ArtNetNode::SetArtNet4Handler(static_cast<ArtNet4Handler*>(this));
	}

	constexpr auto nArtNetPorts = artnetnode::MAX_PORTS;
	constexpr auto nE131Ports __attribute__((unused)) = e131bridge::MAX_PORTS;
	assert(nArtNetPorts == nE131Ports);

	m_nPorts = nArtNetPorts;

	DEBUG_PRINTF("m_nPorts=%u", m_nPorts);
	DEBUG_EXIT
}

Node::~Node() {
	DEBUG_ENTRY

	if (m_pArtNetRdmController != nullptr) {
		delete m_pArtNetRdmController;
	}

	if (m_pDmxConfigUdp != nullptr) {
		delete m_pDmxConfigUdp;
	}

	DEBUG_EXIT
}

void Node::SetFailSafe(const lightset::FailSafe failsafe) {
	DEBUG_ENTRY
	DEBUG_PRINTF("failsafe=%u", static_cast<uint32_t>(failsafe));

	if (m_Personality == node::Personality::ARTNET) {
		switch (failsafe) {
		case lightset::FailSafe::HOLD:
			ArtNetNode::SetFailSafe(artnetnode::FailSafe::LAST);
			break;
		case lightset::FailSafe::OFF:
			ArtNetNode::SetFailSafe(artnetnode::FailSafe::OFF);
			break;
		case lightset::FailSafe::ON:
			ArtNetNode::SetFailSafe(artnetnode::FailSafe::ON);
			break;
		case lightset::FailSafe::PLAYBACK:
			ArtNetNode::SetFailSafe(artnetnode::FailSafe::PLAYBACK);
			break;
		default:
			assert(0);
			__builtin_unreachable();
			break;
		}

		DEBUG_EXIT
		return;
	}

	if (m_Personality == node::Personality::E131) {
		E131Bridge::SetFailSafe(failsafe);

		DEBUG_EXIT
		return;
	}

	assert(0);
	__builtin_unreachable();
	DEBUG_EXIT
}

lightset::FailSafe Node::GetFailSafe() {
	DEBUG_ENTRY

	if (m_Personality == node::Personality::ARTNET) {
		const auto failsafe = ArtNetNode::GetFailSafe();
		switch (failsafe) {
		case artnetnode::FailSafe::LAST:
			return lightset::FailSafe::HOLD;
			break;
		case artnetnode::FailSafe::OFF:
			return lightset::FailSafe::OFF;
			break;
		case artnetnode::FailSafe::ON:
			return lightset::FailSafe::ON;
			break;
		case artnetnode::FailSafe::PLAYBACK:
			return lightset::FailSafe::PLAYBACK;
			break;
		default:
			assert(0);
			__builtin_unreachable();
			break;
		}
	}

	if (m_Personality == node::Personality::E131) {
		DEBUG_EXIT
		return E131Bridge::GetFailSafe();
	}

	assert(0);
	__builtin_unreachable();
	DEBUG_EXIT
	return lightset::FailSafe::HOLD;
}

void Node::Start() {
	DEBUG_ENTRY

	if (m_Personality == node::Personality::ARTNET) {
		constexpr auto nArtNetPorts = artnetnode::MAX_PORTS;
		constexpr auto nE131Ports __attribute__((unused)) = e131bridge::MAX_PORTS;

#if defined (RDM_CONTROLLER)
		if (m_pArtNetRdmController != nullptr) {
			m_pArtNetRdmController->Init();
			m_pArtNetRdmController->Print();

			Display::Get()->TextStatus(NodeMsgConst::RDM_RUN, Display7SegmentMessage::INFO_RDM_RUN, CONSOLE_YELLOW);

			for (uint32_t nPortIndex = 0; nPortIndex < nArtNetPorts; nPortIndex++) {
				uint16_t nUniverse;
				const bool isActive = ArtNetNode::GetRdm(nPortIndex) && ArtNetNode::GetPortAddress(nPortIndex, nUniverse, lightset::PortDir::OUTPUT);

				if (isActive) {
					m_pArtNetRdmController->Full(nPortIndex);
				}
			}

			ArtNetNode::SetRdmHandler(m_pArtNetRdmController);
		}
#endif

		// Art-Net 4

		DEBUG_PRINTF("nArtNetPages=%u->%u, nE131Ports=%u", artnetnode::PAGES, nArtNetPorts, nE131Ports);

		for (uint32_t nPortIndex = 0; nPortIndex < nArtNetPorts; nPortIndex++) {
			uint16_t nUniverse;
			const bool isActive = ArtNetNode::GetPortAddress(nPortIndex, nUniverse, lightset::PortDir::OUTPUT);

			DEBUG_PRINTF("Port %c, Active %c, Universe %d", 'A' + nPortIndex, isActive ? 'Y' : 'N', nUniverse);

			if (isActive) {
				const auto tPortProtocol = ArtNetNode::GetPortProtocol(nPortIndex);

				DEBUG_PRINTF("\tProtocol %s", artnet::get_protocol_mode(tPortProtocol));

				if (tPortProtocol == artnet::PortProtocol::SACN) {
					const auto mergeMode = ArtNetNode::GetMergeMode(nPortIndex);
					E131Bridge::SetMergeMode(nPortIndex, mergeMode);
					DEBUG_PRINTF("\tMerge mode %s", lightset::get_merge_mode(mergeMode));
				}
			}
		}

		E131Bridge::SetFailSafe(static_cast<lightset::FailSafe>(ArtNetNode::GetFailSafe()));
		E131Bridge::SetDisableMergeTimeout(ArtNetNode::GetDisableMergeTimeout());
		E131Bridge::SetOutput(ArtNetNode::GetOutput());
	}

	EventSetUniverse(true);

	Display::Get()->TextStatus(NodeMsgConst::START, Display7SegmentMessage::INFO_NODE_START, CONSOLE_YELLOW);

	if (m_Personality == node::Personality::ARTNET) {
		ArtNetNode::Start();
	}

	if ((m_Personality == node::Personality::E131) || (m_Personality == node::Personality::ARTNET)) {
		E131Bridge::Start();
	}

	Display::Get()->TextStatus(NodeMsgConst::STARTED, Display7SegmentMessage::INFO_NODE_STARTED, CONSOLE_GREEN);

	m_IsStarted = true;

	DEBUG_EXIT
}

void Node::Stop() {
	DEBUG_ENTRY

	if ((m_Personality == node::Personality::E131) || (m_Personality == node::Personality::ARTNET)) {
		E131Bridge::Stop();
	}

	if (m_Personality == node::Personality::ARTNET) {
		ArtNetNode::Stop();
	}

	m_IsStarted = false;

	DEBUG_EXIT
}

void Node::SetLightSet(LightSet *pLightSet) {
	DEBUG_ENTRY

	switch (m_Personality) {
		case node::Personality::ARTNET:
			ArtNetNode::SetOutput(pLightSet);
			break;
		case node::Personality::E131:
			E131Bridge::SetOutput(pLightSet);
			break;
		default:
			assert(0);
			__builtin_unreachable();
			break;
	}

	DEBUG_EXIT
}

void Node::EventSetUniverse(bool bOverwriteIsStarted) {
	DEBUG_ENTRY

	if (m_IsStarted || bOverwriteIsStarted) {
		if (m_Personality == node::Personality::ARTNET) {
			if (ArtNetNode::GetActiveInputPorts() != 0) {
				ArtNetNode::SetArtNetDmx(&m_ArtNetDmxInput);
			} else {
				ArtNetNode::SetArtNetDmx(nullptr);
			}
		}

		if (m_Personality == node::Personality::E131) {
			if (E131Bridge::GetActiveInputPorts() != 0) {
				E131Bridge::SetE131Dmx(&m_E131DmxInput);
			} else {
				E131Bridge::SetE131Dmx(nullptr);
			}
		}

		const auto bArtNet = ((m_Personality == node::Personality::ARTNET) && (ArtNetNode::GetActiveOutputPorts() != 0));
		const auto bE131 = ((m_Personality == node::Personality::E131) && (E131Bridge::GetActiveOutputPorts() != 0));

		if (bArtNet || bE131) {
			if (m_pDmxConfigUdp == nullptr) {
				m_pDmxConfigUdp = new DmxConfigUdp;
				assert(m_pDmxConfigUdp != nullptr);
			} else {
				delete m_pDmxConfigUdp;
				m_pDmxConfigUdp = nullptr;
			}
		}
	}

	DEBUG_EXIT
}

void Node::SetUniverse(uint32_t nPortIndex, lightset::PortDir portDir, uint16_t nUniverse) {
	DEBUG_ENTRY
	DEBUG_PRINTF("nPortIndex=%u, dir=%s, nUniverse=%u", nPortIndex, lightset::get_direction(portDir), nUniverse);

	switch (m_Personality) {
		case node::Personality::ARTNET:
			ArtNetNode::SetUniverse(nPortIndex, portDir, nUniverse);
			break;
		case node::Personality::E131:
			E131Bridge::SetUniverse(nPortIndex, portDir, nUniverse);
			break;
		default:
			assert(0);
			__builtin_unreachable();
			break;
	}

	EventSetUniverse();
	DEBUG_EXIT
}

bool Node::GetUniverse(uint32_t nPortIndex, uint16_t &nUniverse, lightset::PortDir portDir) {
	DEBUG_ENTRY
	DEBUG_PRINTF("nPortIndex=%u", nPortIndex);

	switch (m_Personality) {
		case node::Personality::ARTNET:
			return ArtNetNode::GetPortAddress(nPortIndex, nUniverse, portDir);
			break;
		case node::Personality::E131:
			return E131Bridge::GetUniverse(nPortIndex, nUniverse, portDir);
			break;
		default:
			assert(0);
			__builtin_unreachable();
			break;
	}

	__builtin_unreachable();
	DEBUG_EXIT
	return false;
}

void Node::SetMergeMode(uint32_t nPortIndex, lightset::MergeMode mergeMode) {
	DEBUG_ENTRY
	DEBUG_PRINTF("nPortIndex=%u, mergeMode=%s", nPortIndex, lightset::get_merge_mode(mergeMode));

	switch (m_Personality) {
		case node::Personality::ARTNET:
			ArtNetNode::SetMergeMode(nPortIndex, mergeMode);
			break;
		case node::Personality::E131:
			E131Bridge::SetMergeMode(nPortIndex, mergeMode);
			break;
		default:
			assert(0);
			__builtin_unreachable();
			break;
	}

	__builtin_unreachable();
	DEBUG_EXIT
}

lightset::MergeMode Node::GetMergeMode(uint32_t nPortIndex) {
	DEBUG_ENTRY
	DEBUG_PRINTF("nPortIndex=%u", nPortIndex);

	switch (m_Personality) {
		case node::Personality::ARTNET:
			return ArtNetNode::GetMergeMode(nPortIndex);
			break;
		case node::Personality::E131:
			return E131Bridge::GetMergeMode(nPortIndex);
			break;
		default:
			assert(0);
			__builtin_unreachable();
			break;
	}

	__builtin_unreachable();
	DEBUG_EXIT
	return lightset::MergeMode::HTP;
}

/**
 * Extra's
 */

void Node::SetDisableMergeTimeout(bool bDisable) {
	DEBUG_ENTRY
	DEBUG_PRINTF("bDisable=%u", static_cast<uint32_t>(bDisable));

	switch (m_Personality) {
		case node::Personality::ARTNET:
			ArtNetNode::SetDisableMergeTimeout(bDisable);
			break;
		case node::Personality::E131:
			E131Bridge::SetDisableMergeTimeout(bDisable);
			break;
		default:
			assert(0);
			__builtin_unreachable();
			break;
	}

	DEBUG_EXIT
}

bool Node::GetDisableMergeTimeout() {
	DEBUG_ENTRY

	switch (m_Personality) {
		case node::Personality::ARTNET:
			return ArtNetNode::GetDisableMergeTimeout();
			break;
		case node::Personality::E131:
			return E131Bridge::GetDisableMergeTimeout();
			break;
		default:
			assert(0);
			__builtin_unreachable();
			break;
	}

	assert(0);
	__builtin_unreachable();
	DEBUG_EXIT
	return false;
}

/**
 * Art-Net
 */

void Node::EventSetRdm(bool bOverwriteIsStarted) {
	DEBUG_ENTRY
	DEBUG_PRINTF("RDM %c", IsOptionSet(node::Option::ENABLE_RDM) ?  'Y' : 'N');

	if (m_IsStarted || bOverwriteIsStarted) {
#if defined (RDM_CONTROLLER)
		if (IsOptionSet(node::Option::ENABLE_RDM)) {
			if (m_pArtNetRdmController != nullptr) {
				ArtNetNode::SetRdmHandler(m_pArtNetRdmController);
				DEBUG_EXIT
				return;
			}

			m_pArtNetRdmController = new ArtNetRdmController(artnetnode::MAX_PORTS);
			assert(m_pArtNetRdmController != nullptr);
			DEBUG_EXIT
			return;
		}

		ArtNetNode::SetRdmHandler(nullptr);
#endif
	}
	DEBUG_EXIT
}

/**
 * Art-Net 4
 */

void Node::SetPort(uint32_t nPortIndex, lightset::PortDir portDir) {
	DEBUG_ENTRY
	DEBUG_PRINTF("nPortIndex=%u", nPortIndex);

	uint16_t nUniverse;
	const bool isActive = ArtNetNode::GetPortAddress(nPortIndex, nUniverse, portDir);

	DEBUG_PRINTF("Port %u, Active %c, Universe %d, [%s]", nPortIndex, isActive ? 'Y' : 'N', nUniverse, portDir == lightset::PortDir::OUTPUT ? "Output" : "Input");

	if (portDir == lightset::PortDir::INPUT) {
		DEBUG_PUTS("Input is not supported");
		return;
	}

	if (isActive) {
		const auto ArtNetPortProtocol = ArtNetNode::GetPortProtocol(nPortIndex);

		DEBUG_PRINTF("\tProtocol %s", artnet::get_protocol_mode(ArtNetPortProtocol));

		if (ArtNetPortProtocol == artnet::PortProtocol::SACN) {

			if (ArtNetNode::IsMapUniverse0()) {
				nUniverse++;
			}

			if (nUniverse == 0) {
				SetUniverseSwitch(nPortIndex, lightset::PortDir::DISABLE, 0);
				return;
			}

			E131Bridge::SetUniverse(nPortIndex, portDir == lightset::PortDir::OUTPUT ? lightset::PortDir::OUTPUT : lightset::PortDir::INPUT, nUniverse);
		}
	}

	DEBUG_EXIT
}

void Node::HandleAddress(uint8_t nCommand) {
	DEBUG_ENTRY
	DEBUG_PRINTF("nCommand=%u", nCommand);

	for (uint32_t i = 0; i < artnetnode::MAX_PORTS; i++) {
		uint16_t nUniverse;
		const bool isActive = ArtNetNode::GetPortAddress(i, nUniverse, lightset::PortDir::OUTPUT);

		if (isActive) {
			if (ArtNetNode::IsMapUniverse0()) {
				nUniverse++;
			}

			if (nUniverse == 0) {
				continue;
			}

			if (ArtNetNode::GetPortProtocol(i) == artnet::PortProtocol::SACN) {
				E131Bridge::SetUniverse(i, lightset::PortDir::OUTPUT, nUniverse);
			} else {
				E131Bridge::SetUniverse(i, lightset::PortDir::DISABLE, nUniverse);
			}
		}
	}

	const uint8_t nPort = nCommand & 0x3;
	DEBUG_PRINTF("nPort=%d", nPort);

	switch (nCommand) {

	case artnet::PortCommand::PC_LED_NORMAL:
		E131Bridge::SetEnableDataIndicator(true);
		break;
	case artnet::PortCommand::PC_LED_MUTE:
		E131Bridge::SetEnableDataIndicator(false);
		break;
	case artnet::PortCommand::PC_LED_LOCATE:
		E131Bridge::SetEnableDataIndicator(false);
		break;

	case artnet::PortCommand::PC_MERGE_LTP_O:
	case artnet::PortCommand::PC_MERGE_LTP_1:
	case artnet::PortCommand::PC_MERGE_LTP_2:
	case artnet::PortCommand::PC_MERGE_LTP_3:
		E131Bridge::SetMergeMode(nPort, lightset::MergeMode::LTP);
		break;

	case artnet::PortCommand::PC_MERGE_HTP_0:
	case artnet::PortCommand::PC_MERGE_HTP_1:
	case artnet::PortCommand::PC_MERGE_HTP_2:
	case artnet::PortCommand::PC_MERGE_HTP_3:
		E131Bridge::SetMergeMode(nPort, lightset::MergeMode::HTP);
		break;

	case artnet::PortCommand::PC_CLR_0:
	case artnet::PortCommand::PC_CLR_1:
	case artnet::PortCommand::PC_CLR_2:
	case artnet::PortCommand::PC_CLR_3:
		if (ArtNetNode::GetPortProtocol(nPort) == artnet::PortProtocol::SACN) {
			E131Bridge::Clear(nPort);
		}
		break;

	default:
		break;
	}

	DEBUG_EXIT
}

uint8_t Node::GetStatus(uint32_t nPortIndex) {
	DEBUG_ENTRY
	DEBUG_PRINTF("nPortIndex=%u", nPortIndex);

	uint16_t nUniverse;
	const auto isActive = E131Bridge::GetUniverse(nPortIndex, nUniverse, lightset::PortDir::OUTPUT);

	DEBUG_PRINTF("Port %u, Active %c, Universe %d", nPortIndex, isActive ? 'Y' : 'N', nUniverse);

	if (isActive) {
		auto nStatus = artnet::GoodOutput::OUTPUT_IS_SACN;
		nStatus = nStatus | (E131Bridge::IsTransmitting(nPortIndex) ? artnet::GoodOutput::DATA_IS_BEING_TRANSMITTED : artnet::GoodOutput::OUTPUT_NONE);
		nStatus = nStatus | (E131Bridge::IsMerging(nPortIndex) ? artnet::GoodOutput::OUTPUT_IS_MERGING : artnet::GoodOutput::OUTPUT_NONE);
		return nStatus;
	}

	return 0;

	DEBUG_EXIT
}

void Node::Print() {
	DEBUG_ENTRY

	if (m_Personality == node::Personality::ARTNET) {
		ArtNetNode::Print();

		if (ArtNetNode::IsMapUniverse0()) {
			printf("  Universes are mappped +1\n");
		}
	}

	if ((m_Personality == node::Personality::E131) || (m_Personality == node::Personality::ARTNET)) {
		E131Bridge::Print();
	}

	DEBUG_EXIT
}
