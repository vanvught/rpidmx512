/**
 * @file artnetnodehandleaddress.cpp
 *
 */
/**
 * Art-Net Designed by and Copyright Artistic Licence Holdings Ltd.
 */
/* Copyright (C) 2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

int ArtNetNode::SetUniverse(uint32_t nPortIndex, lightset::PortDir dir, uint16_t nUniverse) {
	const auto nPage = nPortIndex / ArtNet::PORTS;

	// PortAddress Bit 15 = 0
	// Net : Bits 14-8
	// Sub-Net : Bits 7-4
	// Universe : Bits 3-0

	m_Node.NetSwitch[nPage] = (nUniverse >> 8) & 0x7F;
	m_Node.SubSwitch[nPage] = (nUniverse >> 4) & 0x0F;

	return SetUniverseSwitch(nPortIndex, dir, nUniverse & 0x0F);
}

int ArtNetNode::SetUniverseSwitch(uint32_t nPortIndex, lightset::PortDir dir, uint8_t nAddress) {
	assert(nPortIndex < (ArtNet::PORTS * m_nPages));
	assert(dir <= lightset::PortDir::DISABLE);

	if (nPortIndex >= artnetnode::MAX_PORTS) {
		return ARTNET_EACTION;
	}

	if (dir == lightset::PortDir::DISABLE) {
		if (m_OutputPort[nPortIndex].genericPort.bIsEnabled) {
			m_OutputPort[nPortIndex].genericPort.bIsEnabled = false;
			m_State.nActiveOutputPorts = static_cast<uint8_t>(m_State.nActiveOutputPorts - 1);
		}
		if (m_InputPort[nPortIndex].genericPort.bIsEnabled) {
			m_InputPort[nPortIndex].genericPort.bIsEnabled = false;
			m_InputPort[nPortIndex].genericPort.nStatus = GoodInput::GI_DISABLED;
			m_State.nActiveInputPorts = static_cast<uint8_t>(m_State.nActiveInputPorts - 1);
		}
		return ARTNET_EOK;
	}

	if (dir == lightset::PortDir::INPUT) {
		if (!m_InputPort[nPortIndex].genericPort.bIsEnabled) {
			m_State.nActiveInputPorts = static_cast<uint8_t>(m_State.nActiveInputPorts + 1);
			assert(m_State.nActiveInputPorts <= ArtNet::PORTS);
		}

		m_InputPort[nPortIndex].genericPort.bIsEnabled = true;
		m_InputPort[nPortIndex].genericPort.nStatus = 0;
		m_InputPort[nPortIndex].genericPort.nDefaultAddress = nAddress & 0x0F;// Universe : Bits 3-0
		m_InputPort[nPortIndex].genericPort.nPortAddress = MakePortAddress(nAddress, (nPortIndex / ArtNet::PORTS));

		if (nPortIndex < artnetnode::MAX_PORTS) {
			if (m_OutputPort[nPortIndex].genericPort.bIsEnabled) {
				m_OutputPort[nPortIndex].genericPort.bIsEnabled = false;
				m_State.nActiveOutputPorts = static_cast<uint8_t>(m_State.nActiveOutputPorts - 1);
			}
		}
	}

	if (dir == lightset::PortDir::OUTPUT) {
		if (!m_OutputPort[nPortIndex].genericPort.bIsEnabled) {
			m_State.nActiveOutputPorts = static_cast<uint8_t>(m_State.nActiveOutputPorts + 1);
			assert(m_State.nActiveOutputPorts <= (ArtNet::PORTS * m_nPages));
		}

		m_OutputPort[nPortIndex].genericPort.bIsEnabled = true;
		m_OutputPort[nPortIndex].genericPort.nDefaultAddress = nAddress & 0x0F;// Universe : Bits 3-0
		m_OutputPort[nPortIndex].genericPort.nPortAddress = MakePortAddress(nAddress, (nPortIndex / ArtNet::PORTS));

		if (nPortIndex < artnetnode::MAX_PORTS) {
			if (m_InputPort[nPortIndex].genericPort.bIsEnabled) {
				m_InputPort[nPortIndex].genericPort.bIsEnabled = false;
				m_InputPort[nPortIndex].genericPort.nStatus = GoodInput::GI_DISABLED;
				m_State.nActiveInputPorts = static_cast<uint8_t>(m_State.nActiveInputPorts - 1);
			}
		}
	}

	if ((m_pArtNet4Handler != nullptr) && (m_State.status != Status::ON)) {
		m_pArtNet4Handler->SetPort(nPortIndex, dir);
	}

	if (m_State.status == Status::ON) {
		if (m_pArtNetStore != nullptr) {
			if (nPortIndex < ArtNet::PORTS) {
				m_pArtNetStore->SaveUniverseSwitch(nPortIndex, nAddress);
			}
		}
		if (m_pArtNetDisplay != nullptr) {
			m_pArtNetDisplay->ShowUniverseSwitch(nPortIndex, nAddress);
		}
	}

	return ARTNET_EOK;
}

bool ArtNetNode::GetUniverseSwitch(uint32_t nPortIndex, uint8_t &nAddress, lightset::PortDir dir) const {
	if (dir == lightset::PortDir::INPUT) {
		assert(nPortIndex < artnetnode::MAX_PORTS);

		if (nPortIndex < artnetnode::MAX_PORTS) {
			nAddress = m_InputPort[nPortIndex].genericPort.nDefaultAddress;
			return m_InputPort[nPortIndex].genericPort.bIsEnabled;
		}

		return false;
	}

	assert(nPortIndex < artnetnode::MAX_PORTS);

	nAddress = m_OutputPort[nPortIndex].genericPort.nDefaultAddress;
	return m_OutputPort[nPortIndex].genericPort.bIsEnabled;
}

void ArtNetNode::SetSubnetSwitch(uint8_t nAddress, uint32_t nPage) {
	assert(nPage < ArtNet::PAGES);

	m_Node.SubSwitch[nPage] = nAddress;

	const auto nPortIndexStart = static_cast<uint8_t>(nPage * ArtNet::PORTS);

	for (uint32_t i = nPortIndexStart; i < static_cast<uint32_t>(nPortIndexStart + ArtNet::PORTS); i++) {
		m_OutputPort[i].genericPort.nPortAddress = MakePortAddress(m_OutputPort[i].genericPort.nPortAddress, (i / ArtNet::PORTS));
	}

	if ((m_pArtNetStore != nullptr) && (m_State.status == Status::ON)) {
		if (nPage == 0) {
			m_pArtNetStore->SaveSubnetSwitch(nAddress);
		}
	}
}

uint8_t ArtNetNode::GetSubnetSwitch(uint32_t nPage) const {
	assert(nPage < ArtNet::PAGES);

	return m_Node.SubSwitch[nPage];
}

void ArtNetNode::SetNetSwitch(uint8_t nAddress, uint32_t nPage) {
	assert(nPage < ArtNet::PAGES);

	m_Node.NetSwitch[nPage] = nAddress;

	const auto nPortIndexStart = static_cast<uint8_t>(nPage * ArtNet::PORTS);

	for (uint32_t i = nPortIndexStart; i < static_cast<uint32_t>(nPortIndexStart + ArtNet::PORTS); i++) {
		m_OutputPort[i].genericPort.nPortAddress = MakePortAddress(m_OutputPort[i].genericPort.nPortAddress, (i / ArtNet::PORTS));
	}

	if ((m_pArtNetStore != nullptr) && (m_State.status == Status::ON)) {
		if (nPage == 0) {
			m_pArtNetStore->SaveNetSwitch(nAddress);
		}
	}
}

uint8_t ArtNetNode::GetNetSwitch(uint32_t nPage) const {
	DEBUG_PRINTF("nPage=%u", nPage);
	assert(nPage < ArtNet::PAGES);

	return m_Node.NetSwitch[nPage];
}

uint16_t ArtNetNode::MakePortAddress(uint16_t nCurrentAddress, uint32_t nPage) {
	// PortAddress Bit 15 = 0
	uint16_t newAddress = (m_Node.NetSwitch[nPage] & 0x7F) << 8;				// Net : Bits 14-8
	newAddress |= static_cast<uint16_t>((m_Node.SubSwitch[nPage] & 0x0F) << 4);	// Sub-Net : Bits 7-4
	newAddress |= nCurrentAddress & 0x0F;										// Universe : Bits 3-0

	return newAddress;
}

void ArtNetNode::SetPortProtocol(uint32_t nPortIndex, PortProtocol tPortProtocol) {
	if (ArtNet::VERSION > 3) {
		assert(nPortIndex < artnetnode::MAX_PORTS);

		m_OutputPort[nPortIndex].protocol = tPortProtocol;

		if (tPortProtocol == PortProtocol::SACN) {
			m_OutputPort[nPortIndex].genericPort.nStatus |= GoodOutput::GO_OUTPUT_IS_SACN;
		} else {
			m_OutputPort[nPortIndex].genericPort.nStatus &= static_cast<uint8_t>(~GoodOutput::GO_OUTPUT_IS_SACN);
		}

		if (m_State.status == Status::ON) {
			if (nPortIndex < ArtNet::PORTS) {
				if (m_pArtNetStore != nullptr) {
					m_pArtNetStore->SavePortProtocol(nPortIndex, tPortProtocol);
				}
				if (m_pArtNetDisplay != nullptr) {
					m_pArtNetDisplay->ShowPortProtocol(nPortIndex, tPortProtocol);
				}
			}
		}
	}
}

PortProtocol ArtNetNode::GetPortProtocol(uint32_t nPortIndex) const {
	assert(nPortIndex < (artnetnode::MAX_PORTS));

	return m_OutputPort[nPortIndex].protocol;
}

void ArtNetNode::SetMergeMode(uint32_t nPortIndex, lightset::MergeMode tMergeMode) {
	assert(nPortIndex < (ArtNet::PORTS * ArtNet::PAGES));

	m_OutputPort[nPortIndex].mergeMode = tMergeMode;

	if (tMergeMode == lightset::MergeMode::LTP) {
		m_OutputPort[nPortIndex].genericPort.nStatus |= GoodOutput::GO_MERGE_MODE_LTP;
	} else {
		m_OutputPort[nPortIndex].genericPort.nStatus &= static_cast<uint8_t>(~GoodOutput::GO_MERGE_MODE_LTP);
	}

	if (m_State.status == Status::ON) {
		if (nPortIndex < ArtNet::PORTS) {
			if (m_pArtNetStore != nullptr) {
				m_pArtNetStore->SaveMergeMode(nPortIndex, tMergeMode);
			}
			if (m_pArtNetDisplay != nullptr) {
				m_pArtNetDisplay->ShowMergeMode(nPortIndex, tMergeMode);
			}
		}
	}
}

lightset::MergeMode ArtNetNode::GetMergeMode(uint32_t nPortIndex) const {
	assert(nPortIndex < artnetnode::MAX_PORTS);

	return m_OutputPort[nPortIndex].mergeMode;
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

	auto nPortIndex = static_cast<uint8_t>(nPage * ArtNet::PORTS);

	for (uint32_t i = 0; i < ArtNet::PORTS; i++) {
		if (pArtAddress->SwOut[i] == Program::NO_CHANGE) {
		} else if (pArtAddress->SwOut[i] == Program::DEFAULTS) {
			SetUniverseSwitch(nPortIndex, lightset::PortDir::OUTPUT, defaults::UNIVERSE);
		} else if (pArtAddress->SwOut[i] & Program::CHANGE_MASK) {
			SetUniverseSwitch(nPortIndex, lightset::PortDir::OUTPUT, static_cast<uint8_t>(pArtAddress->SwOut[i] & ~Program::CHANGE_MASK));
		}

		if (pArtAddress->SwIn[i] == Program::NO_CHANGE) {
		} else if (pArtAddress->SwIn[i] == Program::DEFAULTS) {
			SetUniverseSwitch(nPortIndex, lightset::PortDir::INPUT, defaults::UNIVERSE);
		} else if (pArtAddress->SwIn[i] & Program::CHANGE_MASK) {
			SetUniverseSwitch(nPortIndex, lightset::PortDir::INPUT, static_cast<uint8_t>(pArtAddress->SwIn[i] & ~Program::CHANGE_MASK));
		}

		nPortIndex++;
	}

	switch (pArtAddress->Command) {
	case PortCommand::PC_CANCEL:
		// If Node is currently in merge mode, cancel merge mode upon receipt of next ArtDmx packet.
		m_State.IsMergeMode = false;
		for (uint32_t i = 0; i < (ArtNet::PORTS * m_nPages); i++) {
			m_OutputPort[i].genericPort.nStatus &= static_cast<uint8_t>(~GoodOutput::GO_OUTPUT_IS_MERGING);
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

	case PortCommand::PC_MERGE_LTP_O:
	case PortCommand::PC_MERGE_LTP_1:
	case PortCommand::PC_MERGE_LTP_2:
	case PortCommand::PC_MERGE_LTP_3:
		SetMergeMode(pArtAddress->Command & 0x3, lightset::MergeMode::LTP);
		break;

	case PortCommand::PC_MERGE_HTP_0:
	case PortCommand::PC_MERGE_HTP_1:
	case PortCommand::PC_MERGE_HTP_2:
	case PortCommand::PC_MERGE_HTP_3:
		SetMergeMode(pArtAddress->Command & 0x3, lightset::MergeMode::HTP);
		break;

	case PortCommand::PC_ARTNET_SEL0:
	case PortCommand::PC_ARTNET_SEL1:
	case PortCommand::PC_ARTNET_SEL2:
	case PortCommand::PC_ARTNET_SEL3:
		SetPortProtocol(pArtAddress->Command & 0x3, PortProtocol::ARTNET);
		break;

	case PortCommand::PC_ACN_SEL0:
	case PortCommand::PC_ACN_SEL1:
	case PortCommand::PC_ACN_SEL2:
	case PortCommand::PC_ACN_SEL3:
		SetPortProtocol(pArtAddress->Command & 0x3, PortProtocol::SACN);
		break;

	case PortCommand::PC_CLR_0:
	case PortCommand::PC_CLR_1:
	case PortCommand::PC_CLR_2:
	case PortCommand::PC_CLR_3:
		nPort = static_cast<uint8_t>((nPage * ArtNet::PORTS) + (pArtAddress->Command & 0x3));
		assert(nPort < artnetnode::MAX_PORTS);
		lightset::Data::OutputClear(m_pLightSet, nPort);
		break;

	default:
		break;
	}

	if ((nPort < artnetnode::MAX_PORTS) && (m_OutputPort[nPort].protocol == PortProtocol::ARTNET) && !m_OutputPort[nPort].IsTransmitting) {
		m_pLightSet->Start(nPort);
		m_OutputPort[nPort].IsTransmitting = true;
		m_OutputPort[nPort].genericPort.nStatus |= GoodOutput::GO_DATA_IS_BEING_TRANSMITTED;
	}

	if (m_pArtNet4Handler != nullptr) {
		m_pArtNet4Handler->HandleAddress(pArtAddress->Command);
	}

	SendPollRelply(true);
}
