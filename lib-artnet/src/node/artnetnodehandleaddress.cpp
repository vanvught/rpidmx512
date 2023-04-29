/**
 * @file artnetnodehandleaddress.cpp
 *
 */
/**
 * Art-Net Designed by and Copyright Artistic Licence Holdings Ltd.
 */
/* Copyright (C) 2021-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <algorithm>
#include <cassert>

#include "artnetnode.h"
#include "artnetconst.h"
#include "artnetnode_internal.h"

#include "lightsetdata.h"
#include "hardware.h"

#include "debug.h"

uint16_t ArtNetNode::MakePortAddress(const uint16_t nUniverse, const uint32_t nPage) {
	return artnet::make_port_address(m_Node.NetSwitch[nPage], m_Node.SubSwitch[nPage], nUniverse);
}

int ArtNetNode::SetUniverse(const uint32_t nPortIndex, const lightset::PortDir dir, const uint16_t nUniverse) {
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

int ArtNetNode::SetUniverseSwitch(const uint32_t nPortIndex, const lightset::PortDir dir, const uint8_t nAddress) {
	DEBUG_PRINTF("nPortIndex=%u, dir=%s, nAddress=%u", nPortIndex, lightset::get_direction(dir), nAddress);

	if (nPortIndex >= artnetnode::MAX_PORTS) {
		return ARTNET_EACTION;
	}

	const auto nPage = nPortIndex / artnetnode::PAGE_SIZE;

	if (dir == lightset::PortDir::DISABLE) {
		if (m_OutputPort[nPortIndex].genericPort.bIsEnabled) {
			m_OutputPort[nPortIndex].genericPort.bIsEnabled = false;
			m_State.nEnabledOutputPorts = static_cast<uint8_t>(m_State.nEnabledOutputPorts - 1);
		}
#if defined (ARTNET_HAVE_DMXIN)
		if (m_InputPort[nPortIndex].genericPort.bIsEnabled) {
			m_InputPort[nPortIndex].genericPort.bIsEnabled = false;
			m_State.nEnabledInputPorts = static_cast<uint8_t>(m_State.nEnabledInputPorts - 1);
		}
#endif
		DEBUG_EXIT
		return ARTNET_EOK;
	}

	if (dir == lightset::PortDir::INPUT) {
#if defined (ARTNET_HAVE_DMXIN)
		if (!m_InputPort[nPortIndex].genericPort.bIsEnabled) {
			m_State.nEnabledInputPorts = static_cast<uint8_t>(m_State.nEnabledInputPorts + 1);
			assert(m_State.nEnabledInputPorts <= artnetnode::MAX_PORTS);
		}

		m_InputPort[nPortIndex].GoodInput = 0;
		m_InputPort[nPortIndex].genericPort.bIsEnabled = true;
		m_InputPort[nPortIndex].genericPort.nDefaultAddress = nAddress & 0x0F;// Universe : Bits 3-0
		m_InputPort[nPortIndex].genericPort.nPortAddress = MakePortAddress(nAddress, nPage);

		if (m_OutputPort[nPortIndex].genericPort.bIsEnabled) {
			m_OutputPort[nPortIndex].genericPort.bIsEnabled = false;
			m_State.nEnabledOutputPorts = static_cast<uint8_t>(m_State.nEnabledOutputPorts - 1);
		}

		if 	(artnetnode::PAGE_SIZE != 1) {
			for (uint32_t nPage = 0; nPage < artnetnode::PAGES; nPage++) {
				const auto nPortIndexStart = nPage * artnetnode::PAGE_SIZE;
				uint8_t nPollReplyIndex = 0;

				for (auto nPortIndex = nPortIndexStart; nPortIndex < std::min((nPortIndexStart + artnetnode::PAGE_SIZE), artnetnode::MAX_PORTS); nPortIndex++) {
					if (m_InputPort[nPortIndex].genericPort.bIsEnabled) {
						m_InputPort[nPortIndex].genericPort.nPollReplyIndex = nPollReplyIndex++;
					} else {
						m_InputPort[nPortIndex].genericPort.nPollReplyIndex = static_cast<uint8_t>(~0);
					}
				}
			}
		}
#else
		return ARTNET_EARG;
#endif
	}

	if (dir == lightset::PortDir::OUTPUT) {
		if (!m_OutputPort[nPortIndex].genericPort.bIsEnabled) {
			m_State.nEnabledOutputPorts = static_cast<uint8_t>(m_State.nEnabledOutputPorts + 1);
			assert(m_State.nEnabledOutputPorts <= artnetnode::MAX_PORTS);
		}

		m_OutputPort[nPortIndex].genericPort.bIsEnabled = true;
		m_OutputPort[nPortIndex].genericPort.nDefaultAddress = nAddress & 0x0F;// Universe : Bits 3-0
		m_OutputPort[nPortIndex].genericPort.nPortAddress = MakePortAddress(nAddress, nPage);

		if (m_InputPort[nPortIndex].genericPort.bIsEnabled) {
			m_InputPort[nPortIndex].genericPort.bIsEnabled = false;
			m_State.nEnabledInputPorts = static_cast<uint8_t>(m_State.nEnabledInputPorts - 1);
		}

		if 	(artnetnode::PAGE_SIZE != 1) {
			for (uint32_t nPage = 0; nPage < artnetnode::PAGES; nPage++) {
				const auto nPortIndexStart = nPage * artnetnode::PAGE_SIZE;
				uint8_t nPollReplyIndex = 0;

				for (auto nPortIndex = nPortIndexStart; nPortIndex < std::min((nPortIndexStart + artnetnode::PAGE_SIZE), artnetnode::MAX_PORTS); nPortIndex++) {
					if (m_OutputPort[nPortIndex].genericPort.bIsEnabled) {
						m_OutputPort[nPortIndex].genericPort.nPollReplyIndex = nPollReplyIndex++;
					} else {
						m_OutputPort[nPortIndex].genericPort.nPollReplyIndex = static_cast<uint8_t>(~0);
					}
				}
			}
		}
	}

	if ((m_pArtNet4Handler != nullptr) && (m_State.status != artnetnode::Status::ON)) {
		m_pArtNet4Handler->SetPort(nPortIndex, dir);
	}

	if (m_State.status == artnetnode::Status::ON) {
		if (m_pArtNetStore != nullptr) {
			m_pArtNetStore->SaveUniverseSwitch(nPortIndex, nAddress);
		}

		artnet::display_universe_switch(nPortIndex, nAddress);
	}

	DEBUG_EXIT
	return ARTNET_EOK;
}

bool ArtNetNode::GetUniverseSwitch(const uint32_t nPortIndex, uint8_t& nAddress, const lightset::PortDir dir) const {
	if (nPortIndex >= artnetnode::MAX_PORTS) {
		return false;
	}

	if (dir == lightset::PortDir::INPUT) {
#if defined (ARTNET_HAVE_DMXIN)
		nAddress = m_InputPort[nPortIndex].genericPort.nDefaultAddress;
		return m_InputPort[nPortIndex].genericPort.bIsEnabled;
#else
		return false;
#endif
	}

	if (dir == lightset::PortDir::DISABLE) {
		return false;
	}

	nAddress = m_OutputPort[nPortIndex].genericPort.nDefaultAddress;
	return m_OutputPort[nPortIndex].genericPort.bIsEnabled;
}

void ArtNetNode::SetSubnetSwitch(uint8_t nAddress, uint32_t nPage) {
	DEBUG_PRINTF("nPage=%u, artnetnode::PAGES=%u", nPage, artnetnode::PAGES);
	assert(nPage < artnetnode::PAGES);

	m_Node.SubSwitch[nPage] = nAddress;

	const auto nPortIndexStart = nPage * artnetnode::PAGE_SIZE;
	const auto nPortIndexEnd = std::min(artnetnode::MAX_PORTS, nPortIndexStart + artnetnode::PAGE_SIZE);

	DEBUG_PRINTF("nPortIndexStart=%u, nPortIndexEnd=%u", nPortIndexStart, nPortIndexEnd);

	for (uint32_t nPortIndex = nPortIndexStart; nPortIndex < nPortIndexEnd; nPortIndex++) {
		m_OutputPort[nPortIndex].genericPort.nPortAddress = MakePortAddress(m_OutputPort[nPortIndex].genericPort.nPortAddress, nPage);
	}

	if ((m_pArtNetStore != nullptr) && (m_State.status == artnetnode::Status::ON)) {
		m_pArtNetStore->SaveSubnetSwitch(nPage, nAddress);
	}

	DEBUG_EXIT
}

void ArtNetNode::SetNetSwitch(uint8_t nAddress, uint32_t nPage) {
	DEBUG_ENTRY
	assert(nPage < artnetnode::PAGES);

	m_Node.NetSwitch[nPage] = nAddress;

	const auto nPortIndexStart = nPage * artnetnode::PAGE_SIZE;
	const auto nPortIndexEnd = std::min(artnetnode::MAX_PORTS, nPortIndexStart + artnetnode::PAGE_SIZE);

	DEBUG_PRINTF("nPortIndexStart=%u, nPortIndexEnd=%u", nPortIndexStart, nPortIndexEnd);

	for (uint32_t nPortIndex = nPortIndexStart; nPortIndex < nPortIndexEnd; nPortIndex++) {
		m_OutputPort[nPortIndex].genericPort.nPortAddress = MakePortAddress(m_OutputPort[nPortIndex].genericPort.nPortAddress, nPage);
	}

	if ((m_pArtNetStore != nullptr) && (m_State.status == artnetnode::Status::ON)) {
		m_pArtNetStore->SaveNetSwitch(nPage, nAddress);
	}

	DEBUG_EXIT
}

void ArtNetNode::SetPortProtocol(uint32_t nPortIndex, artnet::PortProtocol portProtocol) {
	DEBUG_PRINTF("nPortIndex=%u, PortProtocol=%s", nPortIndex, artnet::get_protocol_mode(portProtocol, false));

	if (artnet::VERSION > 3) {
		if (nPortIndex >= artnetnode::MAX_PORTS) {
			DEBUG_EXIT
			return;
		}

		m_OutputPort[nPortIndex].protocol = portProtocol;

		if (portProtocol == artnet::PortProtocol::SACN) {
			m_OutputPort[nPortIndex].GoodOutput |= artnet::GoodOutput::OUTPUT_IS_SACN;
		} else {
			m_OutputPort[nPortIndex].GoodOutput &= static_cast<uint8_t>(~artnet::GoodOutput::OUTPUT_IS_SACN);
		}

		if (m_State.status == artnetnode::Status::ON) {
			if (m_pArtNetStore != nullptr) {
				m_pArtNetStore->SavePortProtocol(nPortIndex, portProtocol);
			}

			artnet::display_port_protocol(nPortIndex, portProtocol);
		}
	}

	DEBUG_EXIT
}

void ArtNetNode::SetMergeMode(const uint32_t nPortIndex, const lightset::MergeMode mergeMode) {
	if (nPortIndex >= artnetnode::MAX_PORTS) {
		DEBUG_EXIT
		return;
	}

	if (mergeMode == lightset::MergeMode::LTP) {
		m_OutputPort[nPortIndex].GoodOutput |= artnet::GoodOutput::MERGE_MODE_LTP;
	} else {
		m_OutputPort[nPortIndex].GoodOutput &= static_cast<uint8_t>(~artnet::GoodOutput::MERGE_MODE_LTP);
	}

	if (m_State.status == artnetnode::Status::ON) {
		if (m_pArtNetStore != nullptr) {
			m_pArtNetStore->SaveMergeMode(nPortIndex, mergeMode);
		}

		artnet::display_merge_mode(nPortIndex, mergeMode);
	}
}

void ArtNetNode::SetFailSafe(const artnetnode::FailSafe failsafe) {
	DEBUG_PRINTF("failsafe=%u", static_cast<uint32_t>(failsafe));

#if defined(ARTNET_HAVE_FAILSAFE_RECORD)
	if ((m_State.status == artnetnode::Status::ON) && (failsafe == artnetnode::FailSafe::RECORD)) {
		FailSafeRecord();
		return;
	}
#endif

	m_Node.Status3 &= static_cast<uint8_t>(~artnet::Status3::NETWORKLOSS_MASK);

	switch (failsafe) {
	case artnetnode::FailSafe::LAST:
		m_Node.Status3 |= artnet::Status3::NETWORKLOSS_LAST_STATE;
		break;

	case artnetnode::FailSafe::OFF:
		m_Node.Status3 |= artnet::Status3::NETWORKLOSS_OFF_STATE;
		break;

	case artnetnode::FailSafe::ON:
		m_Node.Status3 |= artnet::Status3::NETWORKLOSS_ON_STATE;
		break;

	case artnetnode::FailSafe::PLAYBACK:
#if defined(ARTNET_HAVE_FAILSAFE_RECORD)
		m_Node.Status3 |= artnet::Status3::NETWORKLOSS_PLAYBACK;
		break;
#else
		return;
#endif
		break;

	case artnetnode::FailSafe::RECORD:
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

	if (m_State.status == artnetnode::Status::ON) {
		const auto nFailSafe = static_cast<uint8_t>(static_cast<uint8_t>(failsafe) & 0x3);

		if (m_pArtNetStore != nullptr) {
			m_pArtNetStore->SaveFailSafe(nFailSafe);
		}

		artnet::display_failsafe(nFailSafe);
	}

	DEBUG_EXIT
}

bool ArtNetNode::GetPortIndexInput(const uint32_t nPage, const uint32_t nPollReplyIndex, uint32_t& nPortIndex) {
	if (artnetnode::PAGE_SIZE != 1) {
		const auto nPortIndexStart = nPage * artnetnode::PAGE_SIZE;

		for (nPortIndex = nPortIndexStart; nPortIndex < nPortIndexStart + artnetnode::PAGE_SIZE; nPortIndex++) {
			if (m_InputPort[nPortIndex].genericPort.nPollReplyIndex == nPollReplyIndex) {
				return true;
			}
		}

		return false;
	}

	nPortIndex = 0;
	return true;
}

bool ArtNetNode::GetPortIndexOutput(const uint32_t nPage, const uint32_t nPollReplyIndex, uint32_t& nPortIndex) {
	if (artnetnode::PAGE_SIZE != 1) {
		const auto nPortIndexStart = nPage * artnetnode::PAGE_SIZE;

		for (nPortIndex = nPortIndexStart; nPortIndex < nPortIndexStart + artnetnode::PAGE_SIZE; nPortIndex++) {
			if (m_OutputPort[nPortIndex].genericPort.nPollReplyIndex == nPollReplyIndex) {
				return true;
			}
		}

		return false;
	}

	nPortIndex = 0;
	return true;
}

void ArtNetNode::HandleAddress() {
	const auto *const pArtAddress = reinterpret_cast<TArtAddress *>(m_pReceiveBuffer);
	uint8_t nPort = 0xFF;

	m_State.reportCode = artnetnode::ReportCode::RCPOWEROK;

	if (pArtAddress->ShortName[0] != 0)  {
		SetShortName(reinterpret_cast<const char*>(pArtAddress->ShortName));
		m_State.reportCode = artnetnode::ReportCode::RCSHNAMEOK;
	}

	if (pArtAddress->LongName[0] != 0) {
		SetLongName(reinterpret_cast<const char*>(pArtAddress->LongName));
		m_State.reportCode = artnetnode::ReportCode::RCLONAMEOK;
	}

	const auto nPage = static_cast<uint32_t>(pArtAddress->BindIndex > 0 ? pArtAddress->BindIndex - 1 : 0);

	DEBUG_PRINTF("nPage=%u", nPage);

	if (pArtAddress->SubSwitch == artnet::Program::DEFAULTS) {
		SetSubnetSwitch(defaults::SUBNET_SWITCH, nPage);
	} else if (pArtAddress->SubSwitch & artnet::Program::CHANGE_MASK) {
		SetSubnetSwitch(static_cast<uint8_t>(pArtAddress->SubSwitch & ~artnet::Program::CHANGE_MASK), nPage);
	}

	if (pArtAddress->NetSwitch == artnet::Program::DEFAULTS) {
		SetNetSwitch(defaults::NET_SWITCH, nPage);
	} else if (pArtAddress->NetSwitch & artnet::Program::CHANGE_MASK) {
		SetNetSwitch(static_cast<uint8_t>(pArtAddress->NetSwitch & ~artnet::Program::CHANGE_MASK), nPage);
	}

	for (uint32_t i = 0; i < std::min(artnet::PORTS, artnetnode::PAGE_SIZE); i++) {
		if (pArtAddress->SwOut[i] == artnet::Program::NO_CHANGE) {
			// Nothing here
		} else {
			DEBUG_PRINTF("pArtAddress->SwOut[%u]=%u", i, pArtAddress->SwOut[i]);
			uint32_t nPortIndex;

			if (GetPortIndexOutput(nPage, i, nPortIndex)) {
				DEBUG_PRINTF("nPortIndex=%u", nPortIndex);

				if (pArtAddress->SwOut[i] == artnet::Program::DEFAULTS) {
					SetUniverseSwitch(nPortIndex, lightset::PortDir::OUTPUT, defaults::UNIVERSE);
				} else if (pArtAddress->SwOut[i] & artnet::Program::CHANGE_MASK) {
					SetUniverseSwitch(nPortIndex, lightset::PortDir::OUTPUT, static_cast<uint8_t>(pArtAddress->SwOut[i] & ~artnet::Program::CHANGE_MASK));
				}
			}
		}

		if (pArtAddress->SwIn[i] == artnet::Program::NO_CHANGE) {
			// Nothing here
		} else {
			DEBUG_PRINTF("pArtAddress->SwIn[%u]=%u", i, pArtAddress->SwIn[i]);
			uint32_t nPortIndex;

			if (GetPortIndexInput(nPage, i, nPortIndex)) {
				DEBUG_PRINTF("nPortIndex=%u", nPortIndex);

				if (pArtAddress->SwIn[i] == artnet::Program::DEFAULTS) {
					SetUniverseSwitch(nPortIndex, lightset::PortDir::INPUT, defaults::UNIVERSE);
				} else if (pArtAddress->SwIn[i] & artnet::Program::CHANGE_MASK) {
					SetUniverseSwitch(nPortIndex, lightset::PortDir::INPUT, static_cast<uint8_t>(pArtAddress->SwIn[i] & ~artnet::Program::CHANGE_MASK));
				}
			}
		}
	}

	uint32_t nPortIndex;
	const auto isPortIndexOutput __attribute__((unused)) = GetPortIndexOutput(nPage, (pArtAddress->Command & 0x3), nPortIndex);
	DEBUG_PRINTF("isPortIndexOutput=%c, nPortIndex=%u", isPortIndexOutput ? 'Y' : 'N', nPortIndex);

	switch (pArtAddress->Command) {
	case artnet::PortCommand::NONE:
		DEBUG_PUTS("No action.");
		break;
	case artnet::PortCommand::CANCEL:
		m_State.IsMergeMode = false;
		for (uint32_t nPortIndex = 0; nPortIndex < artnetnode::MAX_PORTS; nPortIndex++) {
			m_OutputPort[nPortIndex].sourceA.nIp = 0;
			m_OutputPort[nPortIndex].sourceB.nIp = 0;
			m_OutputPort[nPortIndex].GoodOutput &= static_cast<uint8_t>(~artnet::GoodOutput::OUTPUT_IS_MERGING);
		}
		break;

	case artnet::PortCommand::LED_NORMAL:
		Hardware::Get()->SetModeWithLock(hardware::ledblink::Mode::NORMAL, false);
		m_Node.Status1 = static_cast<uint8_t>((m_Node.Status1 & ~artnet::Status1::INDICATOR_MASK) | artnet::Status1::INDICATOR_NORMAL_MODE);
		break;

	case artnet::PortCommand::LED_MUTE:
		Hardware::Get()->SetModeWithLock(hardware::ledblink::Mode::OFF_OFF, true);
		m_Node.Status1 = static_cast<uint8_t>((m_Node.Status1 & ~artnet::Status1::INDICATOR_MASK) | artnet::Status1::INDICATOR_MUTE_MODE);
		break;

	case artnet::PortCommand::LED_LOCATE:
		Hardware::Get()->SetModeWithLock(hardware::ledblink::Mode::FAST, true);
		m_Node.Status1 = static_cast<uint8_t>((m_Node.Status1 & ~artnet::Status1::INDICATOR_MASK) | artnet::Status1::INDICATOR_LOCATE_MODE);
		break;
#if defined (ARTNET_HAVE_DMXIN)
	case artnet::PortCommand::RESET:
		for (uint32_t nPortIndex = 0; nPortIndex < artnetnode::MAX_PORTS; nPortIndex++) {
			const auto MASK = artnet::GoodInput::INCLUDES_TEST_PACKETS | artnet::GoodInput::INCLUDES_SIP | artnet::GoodInput::INCLUDES_TEXT | artnet::GoodInput::ERRORS;
			m_InputPort[nPortIndex].GoodInput &= static_cast<uint8_t>(~MASK);
		}
		break;
#endif
	case artnet::PortCommand::FAIL_HOLD:
	case artnet::PortCommand::FAIL_ZERO:
	case artnet::PortCommand::FAIL_FULL:
	case artnet::PortCommand::FAIL_SCENE:
	case artnet::PortCommand::FAIL_RECORD:
		SetFailSafe(static_cast<artnetnode::FailSafe>(pArtAddress->Command & 0x0f));
		break;

	case artnet::PortCommand::MERGE_LTP_O:
	case artnet::PortCommand::MERGE_LTP_1:
	case artnet::PortCommand::MERGE_LTP_2:
	case artnet::PortCommand::MERGE_LTP_3:
		assert(isPortIndexOutput);
		SetMergeMode(nPortIndex, lightset::MergeMode::LTP);
		break;
#if defined (ARTNET_HAVE_DMXIN)
	case artnet::PortCommand::DIRECTION_TX_O:
	case artnet::PortCommand::DIRECTION_TX_1:
	case artnet::PortCommand::DIRECTION_TX_2:
	case artnet::PortCommand::DIRECTION_TX_3:
		DEBUG_PUTS("ToDo: PortCommand::DIRECTION_TX");
		break;

	case artnet::PortCommand::DIRECTION_RX_O:
	case artnet::PortCommand::DIRECTION_RX_1:
	case artnet::PortCommand::DIRECTION_RX_2:
	case artnet::PortCommand::DIRECTION_RX_3:
		DEBUG_PUTS("ToDo: PortCommand::DIRECTION_RX");
		break;
#endif
	case artnet::PortCommand::MERGE_HTP_0:
	case artnet::PortCommand::MERGE_HTP_1:
	case artnet::PortCommand::MERGE_HTP_2:
	case artnet::PortCommand::MERGE_HTP_3:
		assert(isPortIndexOutput);
		SetMergeMode(nPortIndex, lightset::MergeMode::HTP);
		break;

	case artnet::PortCommand::ARTNET_SEL0:
	case artnet::PortCommand::ARTNET_SEL1:
	case artnet::PortCommand::ARTNET_SEL2:
	case artnet::PortCommand::ARTNET_SEL3:
		assert(isPortIndexOutput);
		SetPortProtocol(nPortIndex, artnet::PortProtocol::ARTNET);
		break;

	case artnet::PortCommand::ACN_SEL0:
	case artnet::PortCommand::ACN_SEL1:
	case artnet::PortCommand::ACN_SEL2:
	case artnet::PortCommand::ACN_SEL3:
		assert(isPortIndexOutput);
		SetPortProtocol(nPortIndex, artnet::PortProtocol::SACN);
		break;

	case artnet::PortCommand::CLR_0:
	case artnet::PortCommand::CLR_1:
	case artnet::PortCommand::CLR_2:
	case artnet::PortCommand::CLR_3:
		assert(isPortIndexOutput);
		nPort = static_cast<uint8_t>(nPortIndex);
		lightset::Data::OutputClear(m_pLightSet, nPort);
		break;
#if defined (RDM_CONTROLLER) || defined (RDM_RESPONDER)
	case artnet::PortCommand::RDM_ENABLE0:
	case artnet::PortCommand::RDM_ENABLE1:
	case artnet::PortCommand::RDM_ENABLE2:
	case artnet::PortCommand::RDM_ENABLE3:
		assert(isPortIndexOutput);
		SetRmd(nPortIndex, true);
		break;

	case artnet::PortCommand::RDM_DISABLE0:
	case artnet::PortCommand::RDM_DISABLE1:
	case artnet::PortCommand::RDM_DISABLE2:
	case artnet::PortCommand::RDM_DISABLE3:
		assert(isPortIndexOutput);
		SetRmd(nPortIndex, false);
		break;
#endif
	default:
		DEBUG_PRINTF("> Not implemented: %u [%x]", pArtAddress->Command, pArtAddress->Command);
		break;
	}

	if ((nPort < artnetnode::MAX_PORTS) && (m_OutputPort[nPort].protocol == artnet::PortProtocol::ARTNET) && !m_OutputPort[nPort].IsTransmitting) {
		m_pLightSet->Start(nPort);
		m_OutputPort[nPort].IsTransmitting = true;
		m_OutputPort[nPort].GoodOutput |= artnet::GoodOutput::DATA_IS_BEING_TRANSMITTED;
	}

	if (m_pArtNet4Handler != nullptr) {
		m_pArtNet4Handler->HandleAddress(pArtAddress->Command);
	}

	SendPollRelply(true);
}
