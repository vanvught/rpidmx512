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

#include <stdint.h>
#include <cassert>

#include "artnetnode.h"
#include "artnetconst.h"

#include "artnetnode_internal.h"

#include "ledblink.h"

using namespace artnet;

int ArtNetNode::SetUniverse(uint8_t nPortIndex, artnet::PortDir dir, uint16_t nAddress) {
	const uint8_t nPage = nPortIndex / ArtNet::MAX_PORTS;

	// PortAddress Bit 15 = 0
	// Net : Bits 14-8
	// Sub-Net : Bits 7-4
	// Universe : Bits 3-0

	m_Node.NetSwitch[nPage] = (nAddress >> 8) & 0x7F;
	m_Node.SubSwitch[nPage] = (nAddress >> 4) & 0x0F;

	return SetUniverseSwitch(nPortIndex, dir, nAddress & 0x0F);
}

int ArtNetNode::SetUniverseSwitch(uint8_t nPortIndex, artnet::PortDir dir, uint8_t nAddress) {
	assert(nPortIndex < (ArtNet::MAX_PORTS * m_nPages));
	assert(dir <= PortDir::DISABLE);

	if (dir == PortDir::DISABLE) {

		if (nPortIndex < ARTNET_NODE_MAX_PORTS_OUTPUT) {
			if (m_OutputPorts[nPortIndex].bIsEnabled) {
				m_OutputPorts[nPortIndex].bIsEnabled = false;
				m_State.nActiveOutputPorts = m_State.nActiveOutputPorts - 1;
			}
		}

		if (nPortIndex < ARTNET_NODE_MAX_PORTS_INPUT) {
			if (m_InputPorts[nPortIndex].bIsEnabled) {
				m_InputPorts[nPortIndex].bIsEnabled = false;
				m_InputPorts[nPortIndex].port.nStatus = GI_DISABLED;
				m_State.nActiveInputPorts = m_State.nActiveInputPorts - 1;
			}
		}

		return ARTNET_EOK;
	}

	if ((dir == PortDir::INPUT) && (nPortIndex < ARTNET_NODE_MAX_PORTS_INPUT)) {
		if (!m_InputPorts[nPortIndex].bIsEnabled) {
			m_State.nActiveInputPorts = m_State.nActiveInputPorts + 1;
			assert(m_State.nActiveInputPorts <= ArtNet::MAX_PORTS);
		}

		m_InputPorts[nPortIndex].bIsEnabled = true;
		m_InputPorts[nPortIndex].port.nStatus = 0;
		m_InputPorts[nPortIndex].port.nDefaultAddress = nAddress & 0x0F;// Universe : Bits 3-0
		m_InputPorts[nPortIndex].port.nPortAddress = MakePortAddress(nAddress, (nPortIndex / ArtNet::MAX_PORTS));

		if (nPortIndex < ARTNET_NODE_MAX_PORTS_OUTPUT) {
			if (m_OutputPorts[nPortIndex].bIsEnabled) {
				m_OutputPorts[nPortIndex].bIsEnabled = false;
				m_State.nActiveOutputPorts = m_State.nActiveOutputPorts - 1;
			}
		}
	}

	if ((dir == PortDir::OUTPUT) && (nPortIndex < ARTNET_NODE_MAX_PORTS_OUTPUT)) {
		if (!m_OutputPorts[nPortIndex].bIsEnabled) {
			m_State.nActiveOutputPorts = m_State.nActiveOutputPorts + 1;
			assert(m_State.nActiveOutputPorts <= (ArtNet::MAX_PORTS * m_nPages));
		}

		m_OutputPorts[nPortIndex].bIsEnabled = true;
		m_OutputPorts[nPortIndex].port.nDefaultAddress = nAddress & 0x0F;// Universe : Bits 3-0
		m_OutputPorts[nPortIndex].port.nPortAddress = MakePortAddress(nAddress, (nPortIndex / ArtNet::MAX_PORTS));

		if (nPortIndex < ARTNET_NODE_MAX_PORTS_INPUT) {
			if (m_InputPorts[nPortIndex].bIsEnabled) {
				m_InputPorts[nPortIndex].bIsEnabled = false;
				m_InputPorts[nPortIndex].port.nStatus = GI_DISABLED;
				m_State.nActiveInputPorts = m_State.nActiveInputPorts - 1;
			}
		}
	}

	if ((m_pArtNet4Handler != nullptr) && (m_State.status != ARTNET_ON)) {
		m_pArtNet4Handler->SetPort(nPortIndex, dir);
	}

	if (m_State.status == ARTNET_ON) {
		if (m_pArtNetStore != nullptr) {
			if (nPortIndex < ArtNet::MAX_PORTS) {
				m_pArtNetStore->SaveUniverseSwitch(nPortIndex, nAddress);
			}
		}
		if (m_pArtNetDisplay != nullptr) {
			m_pArtNetDisplay->ShowUniverseSwitch(nPortIndex, nAddress);
		}
	}

	return ARTNET_EOK;
}

bool ArtNetNode::GetUniverseSwitch(uint8_t nPortIndex, uint8_t &nAddress, artnet::PortDir dir) const {
	if (dir == PortDir::INPUT) {
		assert(nPortIndex < ARTNET_NODE_MAX_PORTS_INPUT);

		if (nPortIndex < ARTNET_NODE_MAX_PORTS_INPUT) {
			nAddress = m_InputPorts[nPortIndex].port.nDefaultAddress;
			return m_InputPorts[nPortIndex].bIsEnabled;
		}

		return false;
	}

	assert(nPortIndex < ARTNET_NODE_MAX_PORTS_OUTPUT);

	nAddress = m_OutputPorts[nPortIndex].port.nDefaultAddress;
	return m_OutputPorts[nPortIndex].bIsEnabled;
}

void ArtNetNode::SetSubnetSwitch(uint8_t nAddress, uint8_t nPage) {
	assert(nPage < ArtNet::MAX_PAGES);

	m_Node.SubSwitch[nPage] = nAddress;

	const uint32_t nPortIndexStart = nPage * ArtNet::MAX_PORTS;

	for (uint32_t i = nPortIndexStart; i < (nPortIndexStart + ArtNet::MAX_PORTS); i++) {
		m_OutputPorts[i].port.nPortAddress = MakePortAddress(m_OutputPorts[i].port.nPortAddress, (i / ArtNet::MAX_PORTS));
	}

	if ((m_pArtNetStore != nullptr) && (m_State.status == ARTNET_ON)) {
		if (nPage == 0) {
			m_pArtNetStore->SaveSubnetSwitch(nAddress);
		}
	}
}

uint8_t ArtNetNode::GetSubnetSwitch(uint8_t nPage) const {
	assert(nPage < ArtNet::MAX_PAGES);

	return m_Node.SubSwitch[nPage];
}

void ArtNetNode::SetNetSwitch(uint8_t nAddress, uint8_t nPage) {
	assert(nPage < ArtNet::MAX_PAGES);

	m_Node.NetSwitch[nPage] = nAddress;

	const uint32_t nPortIndexStart = nPage * ArtNet::MAX_PORTS;

	for (uint32_t i = nPortIndexStart; i < (nPortIndexStart + ArtNet::MAX_PORTS); i++) {
		m_OutputPorts[i].port.nPortAddress = MakePortAddress(m_OutputPorts[i].port.nPortAddress, (i / ArtNet::MAX_PORTS));
	}

	if ((m_pArtNetStore != nullptr) && (m_State.status == ARTNET_ON)) {
		if (nPage == 0) {
			m_pArtNetStore->SaveNetSwitch(nAddress);
		}
	}
}

uint8_t ArtNetNode::GetNetSwitch(uint8_t nPage) const {
	assert(nPage < ArtNet::MAX_PAGES);

	return m_Node.NetSwitch[nPage];
}

uint16_t ArtNetNode::MakePortAddress(uint16_t nCurrentAddress, uint8_t nPage) {
	// PortAddress Bit 15 = 0
	uint16_t newAddress = (m_Node.NetSwitch[nPage] & 0x7F) << 8;	// Net : Bits 14-8
	newAddress |= (m_Node.SubSwitch[nPage] & 0x0F) << 4;			// Sub-Net : Bits 7-4
	newAddress |= nCurrentAddress & 0x0F;							// Universe : Bits 3-0

	return newAddress;
}

void ArtNetNode::SetPortProtocol(uint8_t nPortIndex, PortProtocol tPortProtocol) {
	if (m_nVersion > 3) {
		assert(nPortIndex < ARTNET_NODE_MAX_PORTS_OUTPUT);

		m_OutputPorts[nPortIndex].tPortProtocol = tPortProtocol;

		if (tPortProtocol == PortProtocol::SACN) {
			m_OutputPorts[nPortIndex].port.nStatus |= GO_OUTPUT_IS_SACN;
		} else {
			m_OutputPorts[nPortIndex].port.nStatus &= (~GO_OUTPUT_IS_SACN);
		}

		if (m_State.status == ARTNET_ON) {
			if (nPortIndex < ArtNet::MAX_PORTS) {
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

PortProtocol ArtNetNode::GetPortProtocol(uint8_t nPortIndex) const {
	assert(nPortIndex < (ARTNET_NODE_MAX_PORTS_OUTPUT));

	return m_OutputPorts[nPortIndex].tPortProtocol;
}

void ArtNetNode::SetMergeMode(uint8_t nPortIndex, Merge tMergeMode) {
	assert(nPortIndex < (ArtNet::MAX_PORTS * ArtNet::MAX_PAGES));

	m_OutputPorts[nPortIndex].mergeMode = tMergeMode;

	if (tMergeMode == Merge::LTP) {
		m_OutputPorts[nPortIndex].port.nStatus |= GO_MERGE_MODE_LTP;
	} else {
		m_OutputPorts[nPortIndex].port.nStatus &= (~GO_MERGE_MODE_LTP);
	}

	if (m_State.status == ARTNET_ON) {
		if (nPortIndex < ArtNet::MAX_PORTS) {
			if (m_pArtNetStore != nullptr) {
				m_pArtNetStore->SaveMergeMode(nPortIndex, tMergeMode);
			}
			if (m_pArtNetDisplay != nullptr) {
				m_pArtNetDisplay->ShowMergeMode(nPortIndex, tMergeMode);
			}
		}
	}
}

Merge ArtNetNode::GetMergeMode(uint8_t nPortIndex) const {
	assert(nPortIndex < ARTNET_NODE_MAX_PORTS_OUTPUT);

	return m_OutputPorts[nPortIndex].mergeMode;
}

void ArtNetNode::HandleAddress() {
	const auto *pArtAddress = &(m_ArtNetPacket.ArtPacket.ArtAddress);
	uint8_t nPort = 0xFF;

	m_State.reportCode = ARTNET_RCPOWEROK;

	if (pArtAddress->ShortName[0] != 0)  {
		SetShortName(reinterpret_cast<const char*>(pArtAddress->ShortName));
		m_State.reportCode = ARTNET_RCSHNAMEOK;
	}

	if (pArtAddress->LongName[0] != 0) {
		SetLongName(reinterpret_cast<const char*>(pArtAddress->LongName));
		m_State.reportCode = ARTNET_RCLONAMEOK;
	}

	const uint8_t nPage = pArtAddress->BindIndex > 0 ? pArtAddress->BindIndex - 1 : 0;

	if (pArtAddress->SubSwitch == PROGRAM_DEFAULTS) {
		SetSubnetSwitch(defaults::SUBNET_SWITCH, nPage);
	} else if (pArtAddress->SubSwitch & PROGRAM_CHANGE_MASK) {
		SetSubnetSwitch(pArtAddress->SubSwitch & ~PROGRAM_CHANGE_MASK, nPage);
	}

	if (pArtAddress->NetSwitch == PROGRAM_DEFAULTS) {
		SetNetSwitch(defaults::NET_SWITCH, nPage);
	} else if (pArtAddress->NetSwitch & PROGRAM_CHANGE_MASK) {
		SetNetSwitch(pArtAddress->NetSwitch & ~PROGRAM_CHANGE_MASK, nPage);
	}

	uint8_t nPortIndex = nPage * ArtNet::MAX_PORTS;

	for (uint32_t i = 0; i < ArtNet::MAX_PORTS; i++) {
		if (pArtAddress->SwOut[i] == PROGRAM_NO_CHANGE) {
		} else if (pArtAddress->SwOut[i] == PROGRAM_DEFAULTS) {
			SetUniverseSwitch(nPortIndex, PortDir::OUTPUT, defaults::UNIVERSE);
		} else if (pArtAddress->SwOut[i] & PROGRAM_CHANGE_MASK) {
			SetUniverseSwitch(nPortIndex, PortDir::OUTPUT, pArtAddress->SwOut[i] & ~PROGRAM_CHANGE_MASK);
		}

		if (pArtAddress->SwIn[i] == PROGRAM_NO_CHANGE) {
		} else if (pArtAddress->SwIn[i] == PROGRAM_DEFAULTS) {
			SetUniverseSwitch(nPortIndex, PortDir::INPUT, defaults::UNIVERSE);
		} else if (pArtAddress->SwIn[i] & PROGRAM_CHANGE_MASK) {
			SetUniverseSwitch(nPortIndex, PortDir::INPUT, pArtAddress->SwIn[i] & ~PROGRAM_CHANGE_MASK);
		}

		nPortIndex++;
	}

	switch (pArtAddress->Command) {
	case ARTNET_PC_CANCEL:
		// If Node is currently in merge mode, cancel merge mode upon receipt of next ArtDmx packet.
		m_State.IsMergeMode = false;
		for (uint32_t i = 0; i < (ArtNet::MAX_PORTS * m_nPages); i++) {
			m_OutputPorts[i].port.nStatus &= (~GO_OUTPUT_IS_MERGING);
		}
		break;

	case ARTNET_PC_LED_NORMAL:
		LedBlink::Get()->SetMode(ledblink::Mode::NORMAL);
		m_Node.Status1 = (m_Node.Status1 & ~STATUS1_INDICATOR_MASK) | STATUS1_INDICATOR_NORMAL_MODE;
		break;
	case ARTNET_PC_LED_MUTE:
		LedBlink::Get()->SetMode(ledblink::Mode::OFF_OFF);
		m_Node.Status1 = (m_Node.Status1 & ~STATUS1_INDICATOR_MASK) | STATUS1_INDICATOR_MUTE_MODE;
		break;
	case ARTNET_PC_LED_LOCATE:
		LedBlink::Get()->SetMode(ledblink::Mode::FAST);
		m_Node.Status1 = (m_Node.Status1 & ~STATUS1_INDICATOR_MASK) | STATUS1_INDICATOR_LOCATE_MODE;
		break;

	case ARTNET_PC_MERGE_LTP_O:
	case ARTNET_PC_MERGE_LTP_1:
	case ARTNET_PC_MERGE_LTP_2:
	case ARTNET_PC_MERGE_LTP_3:
		SetMergeMode(pArtAddress->Command & 0x3, Merge::LTP);
		break;

	case ARTNET_PC_MERGE_HTP_0:
	case ARTNET_PC_MERGE_HTP_1:
	case ARTNET_PC_MERGE_HTP_2:
	case ARTNET_PC_MERGE_HTP_3:
		SetMergeMode(pArtAddress->Command & 0x3, Merge::HTP);
		break;

	case ARTNET_PC_ARTNET_SEL0:
	case ARTNET_PC_ARTNET_SEL1:
	case ARTNET_PC_ARTNET_SEL2:
	case ARTNET_PC_ARTNET_SEL3:
		SetPortProtocol(pArtAddress->Command & 0x3, PortProtocol::ARTNET);
		break;

	case ARTNET_PC_ACN_SEL0:
	case ARTNET_PC_ACN_SEL1:
	case ARTNET_PC_ACN_SEL2:
	case ARTNET_PC_ACN_SEL3:
		SetPortProtocol(pArtAddress->Command & 0x3, PortProtocol::SACN);
		break;

	case ARTNET_PC_CLR_0:
	case ARTNET_PC_CLR_1:
	case ARTNET_PC_CLR_2:
	case ARTNET_PC_CLR_3:
		nPort = pArtAddress->Command & 0x3;
		for (uint32_t i = 0; i < ArtNet::DMX_LENGTH; i++) {
			m_OutputPorts[nPort].data[i] = 0;
		}
		m_OutputPorts[nPort].nLength = ArtNet::DMX_LENGTH;
		if (m_OutputPorts[nPort].tPortProtocol == PortProtocol::ARTNET) {
			m_pLightSet->SetData(nPort, m_OutputPorts[nPort].data, m_OutputPorts[nPort].nLength);
		}
		break;

	default:
		break;
	}

	if ((nPort < ArtNet::MAX_PORTS) && (m_OutputPorts[nPort].tPortProtocol == PortProtocol::ARTNET) && !m_IsLightSetRunning[nPort]) {
		m_pLightSet->Start(nPort);
		m_IsLightSetRunning[nPort] = true;
		m_OutputPorts[nPort].port.nStatus |= GO_DATA_IS_BEING_TRANSMITTED;
	}

	if (m_pArtNet4Handler != nullptr) {
		m_pArtNet4Handler->HandleAddress(pArtAddress->Command);
	}

	SendPollRelply(true);
}
