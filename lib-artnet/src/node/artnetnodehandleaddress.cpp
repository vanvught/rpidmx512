/**
 * @file artnetnodehandleaddress.cpp
 *
 */
/**
 * Art-Net Designed by and Copyright Artistic Licence Holdings Ltd.
 */
/* Copyright (C) 2021-2022 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <cassert>

#include "artnetnode.h"
#include "artnetconst.h"
#include "artnetnode_internal.h"

#include "lightsetdata.h"
#include "ledblink.h"

#include "debug.h"

using namespace artnet;
using namespace artnetnode;

uint16_t ArtNetNode::MakePortAddress(uint16_t nUniverse, uint32_t nPage) {
	return artnet::make_port_address(m_Node.NetSwitch[nPage], m_Node.SubSwitch[nPage], nUniverse);
}

int ArtNetNode::SetUniverse(uint32_t nPortIndex, lightset::PortDir dir, uint16_t nUniverse) {
	const auto nPage = nPortIndex / artnetnode::PAGE_SIZE;
	assert(nPage < artnetnode::PAGES);

	// PortAddress Bit 15 = 0
	// Net : Bits 14-8
	// Sub-Net : Bits 7-4
	// Universe : Bits 3-0

	m_Node.NetSwitch[nPage] = (nUniverse >> 8) & 0x7F;
	m_Node.SubSwitch[nPage] = (nUniverse >> 4) & 0x0F;

	return SetUniverseSwitch(nPortIndex, dir, nUniverse & 0x0F);
}

int ArtNetNode::SetUniverseSwitch(uint32_t nPortIndex, lightset::PortDir dir, uint8_t nAddress) {
	if (nPortIndex >= artnetnode::MAX_PORTS) {
		return ARTNET_EACTION;
	}

	const auto nPage = nPortIndex / artnetnode::PAGE_SIZE;

	if (dir == lightset::PortDir::DISABLE) {
		if (m_OutputPort[nPortIndex].genericPort.bIsEnabled) {
			m_OutputPort[nPortIndex].genericPort.bIsEnabled = false;
			m_State.nActiveOutputPorts = static_cast<uint8_t>(m_State.nActiveOutputPorts - 1);
		}
		if (m_InputPort[nPortIndex].genericPort.bIsEnabled) {
			m_InputPort[nPortIndex].genericPort.bIsEnabled = false;
			m_InputPort[nPortIndex].genericPort.nStatus = GoodInput::DISABLED;
			m_State.nActiveInputPorts = static_cast<uint8_t>(m_State.nActiveInputPorts - 1);
		}
		return ARTNET_EOK;
	}

	if (dir == lightset::PortDir::INPUT) {
		if (!m_InputPort[nPortIndex].genericPort.bIsEnabled) {
			m_State.nActiveInputPorts = static_cast<uint8_t>(m_State.nActiveInputPorts + 1);
			assert(m_State.nActiveInputPorts <= artnetnode::MAX_PORTS);
		}

		m_InputPort[nPortIndex].genericPort.bIsEnabled = true;
		m_InputPort[nPortIndex].genericPort.nStatus = 0;
		m_InputPort[nPortIndex].genericPort.nDefaultAddress = nAddress & 0x0F;// Universe : Bits 3-0
		m_InputPort[nPortIndex].genericPort.nPortAddress = MakePortAddress(nAddress, nPage);

		if (m_OutputPort[nPortIndex].genericPort.bIsEnabled) {
			m_OutputPort[nPortIndex].genericPort.bIsEnabled = false;
			m_State.nActiveOutputPorts = static_cast<uint8_t>(m_State.nActiveOutputPorts - 1);
		}
	}

	if (dir == lightset::PortDir::OUTPUT) {
		if (!m_OutputPort[nPortIndex].genericPort.bIsEnabled) {
			m_State.nActiveOutputPorts = static_cast<uint8_t>(m_State.nActiveOutputPorts + 1);
			assert(m_State.nActiveOutputPorts <= artnetnode::MAX_PORTS);
		}

		m_OutputPort[nPortIndex].genericPort.bIsEnabled = true;
		m_OutputPort[nPortIndex].genericPort.nDefaultAddress = nAddress & 0x0F;// Universe : Bits 3-0
		m_OutputPort[nPortIndex].genericPort.nPortAddress = MakePortAddress(nAddress, nPage);

		if (m_InputPort[nPortIndex].genericPort.bIsEnabled) {
			m_InputPort[nPortIndex].genericPort.bIsEnabled = false;
			m_InputPort[nPortIndex].genericPort.nStatus = GoodInput::DISABLED;
			m_State.nActiveInputPorts = static_cast<uint8_t>(m_State.nActiveInputPorts - 1);
		}
	}

	if ((m_pArtNet4Handler != nullptr) && (m_State.status != Status::ON)) {
		m_pArtNet4Handler->SetPort(nPortIndex, dir);
	}

	if (m_State.status == Status::ON) {
		if (m_pArtNetStore != nullptr) {
			m_pArtNetStore->SaveUniverseSwitch(nPortIndex, nAddress);
		}

		artnet::display_universe_switch(nPortIndex, nAddress);
	}

	return ARTNET_EOK;
}

bool ArtNetNode::GetUniverseSwitch(uint32_t nPortIndex, uint8_t& nAddress, lightset::PortDir dir) const {
	if (nPortIndex >= artnetnode::MAX_PORTS) {
		return false;
	}

	if (dir == lightset::PortDir::INPUT) {
		nAddress = m_InputPort[nPortIndex].genericPort.nDefaultAddress;
		return m_InputPort[nPortIndex].genericPort.bIsEnabled;
	}

	if (dir == lightset::PortDir::DISABLE) {
		return false;
	}

	nAddress = m_OutputPort[nPortIndex].genericPort.nDefaultAddress;
	return m_OutputPort[nPortIndex].genericPort.bIsEnabled;
}

void ArtNetNode::SetSubnetSwitch(uint8_t nAddress, uint32_t nPage) {
	DEBUG_PRINTF("nPage=%u, artnetnode::PAGES)=%u", nPage, artnetnode::PAGES);
	assert(nPage < artnetnode::PAGES);

	m_Node.SubSwitch[nPage] = nAddress;

	const auto nPortIndexStart = nPage * artnetnode::PAGE_SIZE;
	const auto nPortIndexEnd = std::min(artnetnode::MAX_PORTS, nPortIndexStart + artnetnode::PAGE_SIZE);

	DEBUG_PRINTF("nPortIndexStart=%u, nPortIndexEnd=%u", nPortIndexStart, nPortIndexEnd);

	for (uint32_t nPortIndex = nPortIndexStart; nPortIndex < nPortIndexEnd; nPortIndex++) {
		m_OutputPort[nPortIndex].genericPort.nPortAddress = MakePortAddress(m_OutputPort[nPortIndex].genericPort.nPortAddress, nPage);
	}

	if ((m_pArtNetStore != nullptr) && (m_State.status == Status::ON)) {
		m_pArtNetStore->SaveSubnetSwitch(nPage, nAddress);
	}
}

void ArtNetNode::SetNetSwitch(uint8_t nAddress, uint32_t nPage) {
	assert(nPage < artnetnode::PAGES);

	m_Node.NetSwitch[nPage] = nAddress;

	const auto nPortIndexStart = nPage * artnetnode::PAGE_SIZE;
	const auto nPortIndexEnd = std::min(artnetnode::MAX_PORTS, nPortIndexStart + artnetnode::PAGE_SIZE);

	DEBUG_PRINTF("nPortIndexStart=%u, nPortIndexEnd=%u", nPortIndexStart, nPortIndexEnd);

	for (uint32_t nPortIndex = nPortIndexStart; nPortIndex < nPortIndexEnd; nPortIndex++) {
		m_OutputPort[nPortIndex].genericPort.nPortAddress = MakePortAddress(m_OutputPort[nPortIndex].genericPort.nPortAddress, nPage);
	}

	if ((m_pArtNetStore != nullptr) && (m_State.status == Status::ON)) {
		m_pArtNetStore->SaveNetSwitch(nPage, nAddress);
	}
}

void ArtNetNode::SetPortProtocol(uint32_t nPortIndex, PortProtocol portProtocol) {
	DEBUG_ENTRY
	DEBUG_PRINTF("nPortIndex=%u, PortProtocol=%s", nPortIndex, artnet::get_protocol_mode(portProtocol, false));

	if (artnet::VERSION > 3) {
		if (nPortIndex >= artnetnode::MAX_PORTS) {
			DEBUG_EXIT
			return;
		}

		m_OutputPort[nPortIndex].protocol = portProtocol;

		if (portProtocol == PortProtocol::SACN) {
			m_OutputPort[nPortIndex].genericPort.nStatus |= GoodOutput::OUTPUT_IS_SACN;
		} else {
			m_OutputPort[nPortIndex].genericPort.nStatus &= static_cast<uint8_t>(~GoodOutput::OUTPUT_IS_SACN);
		}

		if (m_State.status == Status::ON) {
			if (m_pArtNetStore != nullptr) {
				m_pArtNetStore->SavePortProtocol(nPortIndex, portProtocol);
			}

			artnet::display_port_protocol(nPortIndex, portProtocol);
		}
	}

	DEBUG_EXIT
}

void ArtNetNode::SetMergeMode(uint32_t nPortIndex, lightset::MergeMode mergeMode) {
	if (nPortIndex >= artnetnode::MAX_PORTS) {
		DEBUG_EXIT
		return;
	}

	m_OutputPort[nPortIndex].mergeMode = mergeMode;

	if (mergeMode == lightset::MergeMode::LTP) {
		m_OutputPort[nPortIndex].genericPort.nStatus |= GoodOutput::MERGE_MODE_LTP;
	} else {
		m_OutputPort[nPortIndex].genericPort.nStatus &= static_cast<uint8_t>(~GoodOutput::MERGE_MODE_LTP);
	}

	if (m_State.status == Status::ON) {
		if (m_pArtNetStore != nullptr) {
			m_pArtNetStore->SaveMergeMode(nPortIndex, mergeMode);
		}

		artnet::display_merge_mode(nPortIndex, mergeMode);
	}
}

void ArtNetNode::SetFailSafe(const artnetnode::FailSafe failsafe) {
	DEBUG_ENTRY
	DEBUG_PRINTF("failsafe=%u", static_cast<uint32_t>(failsafe));

#if defined(ARTNET_HAVE_FAILSAFE_RECORD)
	if ((m_State.status == Status::ON) && (failsafe == artnetnode::FailSafe::RECORD)) {
		FailSafeRecord();
		return;
	}
#endif

	m_Node.Status3 &= static_cast<uint8_t>(~Status3::NETWORKLOSS_MASK);

	switch (failsafe) {
	case FailSafe::LAST:
		m_Node.Status3 |= Status3::NETWORKLOSS_LAST_STATE;
		break;

	case FailSafe::OFF:
		m_Node.Status3 |= Status3::NETWORKLOSS_OFF_STATE;
		break;

	case FailSafe::ON:
		m_Node.Status3 |= Status3::NETWORKLOSS_ON_STATE;
		break;

	case FailSafe::PLAYBACK:
#if defined(ARTNET_HAVE_FAILSAFE_RECORD)
		m_Node.Status3 |= Status3::NETWORKLOSS_PLAYBACK;
		break;
#else
		return;
#endif
		break;

	case FailSafe::RECORD:
#if defined(ARTNET_HAVE_FAILSAFE_RECORD)
		assert(0);
		__builtin_unreachable();
		break;
#else
		return;
#endif
		break;

	default:
		assert(0);
		__builtin_unreachable();
		break;
	}

	if (m_State.status == Status::ON) {
		const auto nFailSafe = static_cast<uint8_t>(static_cast<uint8_t>(failsafe) & 0x3);

		if (m_pArtNetStore != nullptr) {
			m_pArtNetStore->SaveFailSafe(nFailSafe);
		}

		artnet::display_failsafe(nFailSafe);
	}

	DEBUG_EXIT
}

void ArtNetNode::HandleAddress() {
	const auto *pArtAddress = &(m_ArtNetPacket.ArtPacket.ArtAddress);
	uint8_t nPort = 0xFF;

	m_State.reportCode = ReportCode::RCPOWEROK;

	if (pArtAddress->ShortName[0] != 0)  {
		SetShortName(reinterpret_cast<const char*>(pArtAddress->ShortName));
		m_State.reportCode = ReportCode::RCSHNAMEOK;
	}

	if (pArtAddress->LongName[0] != 0) {
		SetLongName(reinterpret_cast<const char*>(pArtAddress->LongName));
		m_State.reportCode = ReportCode::RCLONAMEOK;
	}

	const auto nPage = static_cast<uint32_t>(pArtAddress->BindIndex > 0 ? pArtAddress->BindIndex - 1 : 0);

	DEBUG_PRINTF("nPage=%u", nPage);

	if (pArtAddress->SubSwitch == Program::DEFAULTS) {
		SetSubnetSwitch(defaults::SUBNET_SWITCH, nPage);
	} else if (pArtAddress->SubSwitch & Program::CHANGE_MASK) {
		SetSubnetSwitch(static_cast<uint8_t>(pArtAddress->SubSwitch & ~Program::CHANGE_MASK), nPage);
	}

	if (pArtAddress->NetSwitch == Program::DEFAULTS) {
		SetNetSwitch(defaults::NET_SWITCH, nPage);
	} else if (pArtAddress->NetSwitch & Program::CHANGE_MASK) {
		SetNetSwitch(static_cast<uint8_t>(pArtAddress->NetSwitch & ~Program::CHANGE_MASK), nPage);
	}

	auto nPortIndex = nPage * artnetnode::PAGE_SIZE;

	DEBUG_PRINTF("nPortIndex=%u", nPortIndex);

	for (uint32_t i = 0; i < std::min(artnet::PORTS, artnetnode::PAGE_SIZE); i++) {
		DEBUG_PRINTF("pArtAddress->SwOut[%u]=%u", i, pArtAddress->SwOut[i]);

		if (pArtAddress->SwOut[i] == Program::NO_CHANGE) {
			// Nothing here
		} else if (pArtAddress->SwOut[i] == Program::DEFAULTS) {
			SetUniverseSwitch(nPortIndex, lightset::PortDir::OUTPUT, defaults::UNIVERSE);
		} else if (pArtAddress->SwOut[i] & Program::CHANGE_MASK) {
			SetUniverseSwitch(nPortIndex, lightset::PortDir::OUTPUT, static_cast<uint8_t>(pArtAddress->SwOut[i] & ~Program::CHANGE_MASK));
		}

		DEBUG_PRINTF("pArtAddress->SwIn[%u]=%u", i, pArtAddress->SwIn[i]);

		if (pArtAddress->SwIn[i] == Program::NO_CHANGE) {
			// Nothing here
		} else if (pArtAddress->SwIn[i] == Program::DEFAULTS) {
			SetUniverseSwitch(nPortIndex, lightset::PortDir::INPUT, defaults::UNIVERSE);
		} else if (pArtAddress->SwIn[i] & Program::CHANGE_MASK) {
			SetUniverseSwitch(nPortIndex, lightset::PortDir::INPUT, static_cast<uint8_t>(pArtAddress->SwIn[i] & ~Program::CHANGE_MASK));
		}

		nPortIndex++;
	}

	nPortIndex = (nPage * artnetnode::PAGE_SIZE) + (pArtAddress->Command & 0x3);

	switch (pArtAddress->Command) {
	case PortCommand::PC_CANCEL:
		// If Node is currently in merge mode, cancel merge mode upon receipt of next ArtDmx packet.
		m_State.IsMergeMode = false;
		for (uint32_t i = 0; i < artnetnode::MAX_PORTS; i++) {
			m_OutputPort[i].genericPort.nStatus &= static_cast<uint8_t>(~GoodOutput::OUTPUT_IS_MERGING);
		}
		break;

	case PortCommand::PC_LED_NORMAL:
		LedBlink::Get()->SetMode(ledblink::Mode::NORMAL);
		m_Node.Status1 = (m_Node.Status1 & ~Status1::INDICATOR_MASK) | Status1::INDICATOR_NORMAL_MODE;
		break;

	case PortCommand::PC_LED_MUTE:
		LedBlink::Get()->SetMode(ledblink::Mode::OFF_OFF);
		m_Node.Status1 = static_cast<uint8_t>((m_Node.Status1 & ~Status1::INDICATOR_MASK) | Status1::INDICATOR_MUTE_MODE);
		break;

	case PortCommand::PC_LED_LOCATE:
		LedBlink::Get()->SetMode(ledblink::Mode::FAST);
		m_Node.Status1 = static_cast<uint8_t>((m_Node.Status1 & ~Status1::INDICATOR_MASK) | Status1::INDICATOR_LOCATE_MODE);
		break;

	case PortCommand::FAIL_HOLD:
	case PortCommand::FAIL_ZERO:
	case PortCommand::FAIL_FULL:
	case PortCommand::FAIL_SCENE:
	case PortCommand::FAIL_RECORD:
		SetFailSafe(static_cast<artnetnode::FailSafe>(pArtAddress->Command & 0x0f));
		break;

	case PortCommand::PC_MERGE_LTP_O:
	case PortCommand::PC_MERGE_LTP_1:
	case PortCommand::PC_MERGE_LTP_2:
	case PortCommand::PC_MERGE_LTP_3:
		SetMergeMode(nPortIndex, lightset::MergeMode::LTP);
		break;

	case PortCommand::PC_MERGE_HTP_0:
	case PortCommand::PC_MERGE_HTP_1:
	case PortCommand::PC_MERGE_HTP_2:
	case PortCommand::PC_MERGE_HTP_3:
		SetMergeMode(nPortIndex, lightset::MergeMode::HTP);
		break;

	case PortCommand::PC_ARTNET_SEL0:
	case PortCommand::PC_ARTNET_SEL1:
	case PortCommand::PC_ARTNET_SEL2:
	case PortCommand::PC_ARTNET_SEL3:
		SetPortProtocol(nPortIndex, PortProtocol::ARTNET);
		break;

	case PortCommand::PC_ACN_SEL0:
	case PortCommand::PC_ACN_SEL1:
	case PortCommand::PC_ACN_SEL2:
	case PortCommand::PC_ACN_SEL3:
		SetPortProtocol(nPortIndex, PortProtocol::SACN);
		break;

	case PortCommand::PC_CLR_0:
	case PortCommand::PC_CLR_1:
	case PortCommand::PC_CLR_2:
	case PortCommand::PC_CLR_3:
		nPort = static_cast<uint8_t>((nPage * artnetnode::PAGE_SIZE) + (pArtAddress->Command & 0x3));
		assert(nPort < artnetnode::MAX_PORTS);
		lightset::Data::OutputClear(m_pLightSet, nPort);
		break;

	case PortCommand::RDM_ENABLE0:
	case PortCommand::RDM_ENABLE1:
	case PortCommand::RDM_ENABLE2:
	case PortCommand::RDM_ENABLE3:
		SetRmd(nPortIndex, true);
		break;

	case PortCommand::RDM_DISABLE0:
	case PortCommand::RDM_DISABLE1:
	case PortCommand::RDM_DISABLE2:
	case PortCommand::RDM_DISABLE3:
		SetRmd(nPortIndex, false);
		break;

	default:
		DEBUG_PUTS("> Not implemented <");
		break;
	}

	if ((nPort < artnetnode::MAX_PORTS) && (m_OutputPort[nPort].protocol == PortProtocol::ARTNET) && !m_OutputPort[nPort].IsTransmitting) {
		m_pLightSet->Start(nPort);
		m_OutputPort[nPort].IsTransmitting = true;
		m_OutputPort[nPort].genericPort.nStatus |= GoodOutput::DATA_IS_BEING_TRANSMITTED;
	}

	if (m_pArtNet4Handler != nullptr) {
		m_pArtNet4Handler->HandleAddress(pArtAddress->Command);
	}

	SendPollRelply(true);
}
