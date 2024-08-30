/**
 * @file artnetnodehandleaddress.cpp
 *
 */
/**
 * Art-Net Designed by and Copyright Artistic Licence Holdings Ltd.
 */
/* Copyright (C) 2021-2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include "artnetstore.h"

#include "lightsetdata.h"
#include "hardware.h"

#include "debug.h"

void ArtNetNode::SetLocalMerging() {
	DEBUG_ENTRY

	for (uint32_t nInputPortIndex = 0; nInputPortIndex < artnetnode::MAX_PORTS; nInputPortIndex++) {
		if (m_Node.Port[nInputPortIndex].direction == lightset::PortDir::OUTPUT) {
			continue;
		}

		m_Node.Port[nInputPortIndex].bLocalMerge = false;

		for (uint32_t nOutputPortIndex = 0; nOutputPortIndex < artnetnode::MAX_PORTS; nOutputPortIndex++) {
			if (m_Node.Port[nOutputPortIndex].direction == lightset::PortDir::INPUT) {
				continue;
			}

			DEBUG_PRINTF("nInputPortIndex=%u %s %u, nOutputPortIndex=%u %s %u",
					nInputPortIndex,
					artnet::get_protocol_mode(m_Node.Port[nInputPortIndex].protocol),
					m_Node.Port[nInputPortIndex].PortAddress,
					nOutputPortIndex,
					artnet::get_protocol_mode(m_Node.Port[nOutputPortIndex].protocol),
					m_Node.Port[nOutputPortIndex].PortAddress);

			if ((m_Node.Port[nInputPortIndex].protocol == m_Node.Port[nOutputPortIndex].protocol) &&
				(m_Node.Port[nInputPortIndex].PortAddress == m_Node.Port[nOutputPortIndex].PortAddress)) {

				if (!m_Node.Port[nOutputPortIndex].bLocalMerge) {
					m_OutputPort[nOutputPortIndex].SourceA.nIp = Network::Get()->GetIp();
					DEBUG_PUTS("Local merge Source A");
				} else {
					m_OutputPort[nOutputPortIndex].SourceB.nIp = Network::Get()->GetIp();
					DEBUG_PUTS("Local merge Source B");
				}

				m_Node.Port[nInputPortIndex].bLocalMerge = true;
				m_Node.Port[nOutputPortIndex].bLocalMerge = true;
			}
		}
	}

	for (uint32_t nPortIndex = 0; nPortIndex < artnetnode::MAX_PORTS; nPortIndex++) {
		DEBUG_PRINTF("nPortIndex=%u, bLocalMerge=%c", nPortIndex, m_Node.Port[nPortIndex].bLocalMerge ? 'Y' : 'N');
	}

	DEBUG_EXIT
}

void ArtNetNode::SetUniverse(const uint32_t nPortIndex, const lightset::PortDir dir, const uint16_t nUniverse) {
	assert(nPortIndex < artnetnode::MAX_PORTS);

	// PortAddress Bit 15 = 0
	// Net : Bits 14-8
	// Sub-Net : Bits 7-4
	// Universe : Bits 3-0

	m_Node.Port[nPortIndex].NetSwitch = (nUniverse >> 8) & 0x7F;
	m_Node.Port[nPortIndex].SubSwitch = (nUniverse >> 4) & 0x0F;

	SetUniverseSwitch(nPortIndex, dir, nUniverse & 0x0F);
}

void ArtNetNode::SetUniverseSwitch(const uint32_t nPortIndex, const lightset::PortDir dir, const uint8_t nAddress) {
	DEBUG_PRINTF("nPortIndex=%u, dir=%s, nAddress=%u", nPortIndex, lightset::get_direction(dir), nAddress);
	assert(nPortIndex < artnetnode::MAX_PORTS);

	if (dir == lightset::PortDir::DISABLE) {
		if (m_Node.Port[nPortIndex].direction == lightset::PortDir::OUTPUT) {
			assert(m_State.nEnabledOutputPorts > 1);
			m_State.nEnabledOutputPorts = static_cast<uint8_t>(m_State.nEnabledOutputPorts - 1);
		}
#if defined (ARTNET_HAVE_DMXIN)
		if (m_Node.Port[nPortIndex].direction == lightset::PortDir::INPUT) {
			assert(m_State.nEnabledInputPorts > 1);
			m_State.nEnabledInputPorts = static_cast<uint8_t>(m_State.nEnabledInputPorts - 1);
		}
#endif
		m_Node.Port[nPortIndex].direction = lightset::PortDir::DISABLE;
	}

#if defined (ARTNET_HAVE_DMXIN)
	if (dir == lightset::PortDir::INPUT) {
		if (m_Node.Port[nPortIndex].direction != lightset::PortDir::INPUT) {
			m_State.nEnabledInputPorts = static_cast<uint8_t>(m_State.nEnabledInputPorts + 1);
			assert(m_State.nEnabledInputPorts <= artnetnode::MAX_PORTS);
		}

		if (m_Node.Port[nPortIndex].direction == lightset::PortDir::OUTPUT) {
			assert(m_State.nEnabledOutputPorts > 1);
			m_State.nEnabledOutputPorts = static_cast<uint8_t>(m_State.nEnabledOutputPorts - 1);
		}

		m_InputPort[nPortIndex].GoodInput = 0;

		m_Node.Port[nPortIndex].DefaultAddress = nAddress & 0x0F;
		m_Node.Port[nPortIndex].PortAddress = MakePortAddress(nAddress, nPortIndex);
		m_Node.Port[nPortIndex].direction = lightset::PortDir::INPUT;
	}
#endif

	if (dir == lightset::PortDir::OUTPUT) {
		if (m_Node.Port[nPortIndex].direction != lightset::PortDir::OUTPUT) {
			m_State.nEnabledOutputPorts = static_cast<uint8_t>(m_State.nEnabledOutputPorts + 1);
			assert(m_State.nEnabledOutputPorts <= artnetnode::MAX_PORTS);
		}

		if (m_Node.Port[nPortIndex].direction == lightset::PortDir::INPUT) {
			assert(m_State.nEnabledInputPorts > 1);
			m_State.nEnabledInputPorts = static_cast<uint8_t>(m_State.nEnabledInputPorts - 1);
		}

		m_Node.Port[nPortIndex].DefaultAddress = nAddress & 0x0F;
		m_Node.Port[nPortIndex].PortAddress = MakePortAddress(nAddress, nPortIndex);
		m_Node.Port[nPortIndex].direction = lightset::PortDir::OUTPUT;
	}

#if (ARTNET_VERSION >= 4)
	SetUniverse4(nPortIndex, dir);
#endif

	if (m_State.status == artnet::Status::ON) {
		ArtNetStore::SaveUniverseSwitch(nPortIndex, nAddress);
		artnet::display_universe_switch(nPortIndex, nAddress);

#if defined (ARTNET_HAVE_DMXIN)
		SetLocalMerging();
#endif
	}

	DEBUG_EXIT
	return;
}

void ArtNetNode::SetSubnetSwitch(const uint32_t nPortIndex, const uint8_t nSubnetSwitch) {
	assert(nPortIndex < artnetnode::MAX_PORTS);

	m_Node.Port[nPortIndex].SubSwitch = nSubnetSwitch;
	m_Node.Port[nPortIndex].PortAddress = MakePortAddress(m_Node.Port[nPortIndex].PortAddress, nPortIndex);

	if (m_State.status == artnet::Status::ON) {
		ArtNetStore::SaveSubnetSwitch(nPortIndex, nSubnetSwitch);
	}

	DEBUG_EXIT
}

void ArtNetNode::SetNetSwitch(const uint32_t nPortIndex, const uint8_t nNetSwitch) {
	DEBUG_ENTRY
	assert(nPortIndex < artnetnode::MAX_PORTS);

	m_Node.Port[nPortIndex].NetSwitch = nNetSwitch;
	m_Node.Port[nPortIndex].PortAddress = MakePortAddress(m_Node.Port[nPortIndex].PortAddress, nPortIndex);

	if (m_State.status == artnet::Status::ON) {
		ArtNetStore::SaveNetSwitch(nPortIndex, nNetSwitch);
	}

	DEBUG_EXIT
}

void ArtNetNode::SetMergeMode(const uint32_t nPortIndex, const lightset::MergeMode mergeMode) {
	assert(nPortIndex < artnetnode::MAX_PORTS);

	if (mergeMode == lightset::MergeMode::LTP) {
		m_OutputPort[nPortIndex].GoodOutput |= artnet::GoodOutput::MERGE_MODE_LTP;
	} else {
		m_OutputPort[nPortIndex].GoodOutput &= static_cast<uint8_t>(~artnet::GoodOutput::MERGE_MODE_LTP);
	}

#if (ARTNET_VERSION >= 4)
	E131Bridge::SetMergeMode(nPortIndex, mergeMode);
#endif

	if (m_State.status == artnet::Status::ON) {
		ArtNetStore::SaveMergeMode(nPortIndex, mergeMode);
		artnet::display_merge_mode(nPortIndex, mergeMode);
	}
}

void ArtNetNode::SetFailSafe(const artnetnode::FailSafe failsafe) {
	DEBUG_PRINTF("failsafe=%u", static_cast<uint32_t>(failsafe));

#if defined(ARTNET_HAVE_FAILSAFE_RECORD)
	if ((m_State.status == artnet::Status::ON) && (failsafe == artnetnode::FailSafe::RECORD)) {
		FailSafeRecord();
		return;
	}
#endif

	m_ArtPollReply.Status3 &= static_cast<uint8_t>(~artnet::Status3::NETWORKLOSS_MASK);

	switch (failsafe) {
	case artnetnode::FailSafe::LAST:
		m_ArtPollReply.Status3 |= artnet::Status3::NETWORKLOSS_LAST_STATE;
		break;

	case artnetnode::FailSafe::OFF:
		m_ArtPollReply.Status3 |= artnet::Status3::NETWORKLOSS_OFF_STATE;
		break;

	case artnetnode::FailSafe::ON:
		m_ArtPollReply.Status3 |= artnet::Status3::NETWORKLOSS_ON_STATE;
		break;

	case artnetnode::FailSafe::PLAYBACK:
#if defined(ARTNET_HAVE_FAILSAFE_RECORD)
		m_ArtPollReply.Status3 |= artnet::Status3::NETWORKLOSS_PLAYBACK;
#endif
		break;

	case artnetnode::FailSafe::RECORD:
#if defined(ARTNET_HAVE_FAILSAFE_RECORD)
		assert(0);
		__builtin_unreachable();
#endif
		break;

	default:
		assert(0);
		__builtin_unreachable();
		break;
	}

#if (ARTNET_VERSION >= 4)
	E131Bridge::SetFailSafe(static_cast<lightset::FailSafe>(static_cast<uint8_t>(failsafe) & 0x3));
#endif

	if (m_State.status == artnet::Status::ON) {
		const auto nFailSafe = static_cast<uint8_t>(static_cast<uint8_t>(failsafe) & 0x3);
		ArtNetStore::SaveFailSafe(nFailSafe);
		artnet::display_failsafe(nFailSafe);
	}

	DEBUG_EXIT
}

void ArtNetNode::HandleAddress() {
	const auto *const pArtAddress = reinterpret_cast<artnet::ArtAddress *>(m_pReceiveBuffer);
	m_State.reportCode = artnet::ReportCode::RCPOWEROK;

	const auto nPage = static_cast<uint32_t>(pArtAddress->BindIndex > 0 ? pArtAddress->BindIndex - 1 : 0);

	DEBUG_PRINTF("nPage=%u", nPage);

	if (pArtAddress->ShortName[0] != 0)  {
		SetShortName(nPage, reinterpret_cast<const char*>(pArtAddress->ShortName));
		m_State.reportCode = artnet::ReportCode::RCSHNAMEOK;
	}

	if (pArtAddress->LongName[0] != 0) {
		SetLongName(reinterpret_cast<const char*>(pArtAddress->LongName));
		m_State.reportCode = artnet::ReportCode::RCLONAMEOK;
	}

	if (pArtAddress->SubSwitch == artnet::Program::DEFAULTS) {
		SetSubnetSwitch(nPage, defaults::SUBNET_SWITCH);
	} else if (pArtAddress->SubSwitch & artnet::Program::CHANGE_MASK) {
		SetSubnetSwitch(nPage, static_cast<uint8_t>(pArtAddress->SubSwitch & ~artnet::Program::CHANGE_MASK));
	}

	if (pArtAddress->NetSwitch == artnet::Program::DEFAULTS) {
		SetNetSwitch(nPage, defaults::NET_SWITCH);
	} else if (pArtAddress->NetSwitch & artnet::Program::CHANGE_MASK) {
		SetNetSwitch(nPage, static_cast<uint8_t>(pArtAddress->NetSwitch & ~artnet::Program::CHANGE_MASK));
	}

	if (pArtAddress->SwOut[0] == artnet::Program::NO_CHANGE) {
		// Nothing here
	} else {
		if (m_Node.Port[nPage].direction == lightset::PortDir::OUTPUT) {
			if (pArtAddress->SwOut[0] == artnet::Program::DEFAULTS) {
				SetUniverseSwitch(nPage, lightset::PortDir::OUTPUT, defaults::UNIVERSE);
			} else if (pArtAddress->SwOut[0] & artnet::Program::CHANGE_MASK) {
				SetUniverseSwitch(nPage, lightset::PortDir::OUTPUT, static_cast<uint8_t>(pArtAddress->SwOut[0] & ~artnet::Program::CHANGE_MASK));
			}
		}
	}

	if (pArtAddress->SwIn[0] == artnet::Program::NO_CHANGE) {
		// Nothing here
	} else {
		if (m_Node.Port[nPage].direction == lightset::PortDir::INPUT) {
			if (pArtAddress->SwIn[0] == artnet::Program::DEFAULTS) {
				SetUniverseSwitch(nPage, lightset::PortDir::INPUT, defaults::UNIVERSE);
			} else if (pArtAddress->SwIn[0] & artnet::Program::CHANGE_MASK) {
				SetUniverseSwitch(nPage, lightset::PortDir::INPUT, static_cast<uint8_t>(pArtAddress->SwIn[0] & ~artnet::Program::CHANGE_MASK));
			}
		}
	}

	switch (pArtAddress->Command) {
	case artnet::PortCommand::NONE:
		DEBUG_PUTS("No action.");
		break;
	case artnet::PortCommand::CANCEL:
		m_State.IsMergeMode = false;
		for (uint32_t nPortIndex = 0; nPortIndex < artnetnode::MAX_PORTS; nPortIndex++) {
			m_OutputPort[nPortIndex].SourceA.nIp = 0;
			m_OutputPort[nPortIndex].SourceB.nIp = 0;
			m_OutputPort[nPortIndex].GoodOutput &= static_cast<uint8_t>(~artnet::GoodOutput::OUTPUT_IS_MERGING);
		}
		break;

	case artnet::PortCommand::LED_NORMAL:
		Hardware::Get()->SetModeWithLock(hardware::ledblink::Mode::NORMAL, false);
		m_ArtPollReply.Status1 = static_cast<uint8_t>((m_ArtPollReply.Status1 & ~artnet::Status1::INDICATOR_MASK) | artnet::Status1::INDICATOR_NORMAL_MODE);
#if (ARTNET_VERSION >= 4)
		E131Bridge::SetEnableDataIndicator(true);
#endif
		break;

	case artnet::PortCommand::LED_MUTE:
		Hardware::Get()->SetModeWithLock(hardware::ledblink::Mode::OFF_OFF, true);
		m_ArtPollReply.Status1 = static_cast<uint8_t>((m_ArtPollReply.Status1 & ~artnet::Status1::INDICATOR_MASK) | artnet::Status1::INDICATOR_MUTE_MODE);
#if (ARTNET_VERSION >= 4)
		E131Bridge::SetEnableDataIndicator(false);
#endif
		break;

	case artnet::PortCommand::LED_LOCATE:
		Hardware::Get()->SetModeWithLock(hardware::ledblink::Mode::FAST, true);
		m_ArtPollReply.Status1 = static_cast<uint8_t>((m_ArtPollReply.Status1 & ~artnet::Status1::INDICATOR_MASK) | artnet::Status1::INDICATOR_LOCATE_MODE);
#if (ARTNET_VERSION >= 4)
		E131Bridge::SetEnableDataIndicator(false);
#endif
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
		SetMergeMode(nPage, lightset::MergeMode::LTP);
#if (ARTNET_VERSION >= 4)
		E131Bridge::SetMergeMode(nPage, lightset::MergeMode::LTP);
#endif
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
		SetMergeMode(nPage, lightset::MergeMode::HTP);
#if (ARTNET_VERSION >= 4)
		E131Bridge::SetMergeMode(nPage, lightset::MergeMode::HTP);
#endif
		break;

#if (ARTNET_VERSION >= 4)
	case artnet::PortCommand::ARTNET_SEL0:
	case artnet::PortCommand::ARTNET_SEL1:
	case artnet::PortCommand::ARTNET_SEL2:
	case artnet::PortCommand::ARTNET_SEL3:
		SetPortProtocol4(nPage, artnet::PortProtocol::ARTNET);
		SetUniverse4(nPage, lightset::PortDir::DISABLE);
		break;

	case artnet::PortCommand::ACN_SEL0:
	case artnet::PortCommand::ACN_SEL1:
	case artnet::PortCommand::ACN_SEL2:
	case artnet::PortCommand::ACN_SEL3:
		SetPortProtocol4(nPage, artnet::PortProtocol::SACN);
		SetUniverse4(nPage, lightset::PortDir::OUTPUT);
		break;
#endif

	case artnet::PortCommand::CLR_0:
	case artnet::PortCommand::CLR_1:
	case artnet::PortCommand::CLR_2:
	case artnet::PortCommand::CLR_3:
		if (m_Node.Port[nPage].protocol == artnet::PortProtocol::ARTNET) {
			lightset::Data::OutputClear(m_pLightSet, nPage);
		}
#if (ARTNET_VERSION >= 4)
		if (m_Node.Port[nPage].protocol == artnet::PortProtocol::SACN) {
			E131Bridge::Clear(nPage);
		}
#endif
		break;

#if defined (OUTPUT_HAVE_STYLESWITCH)
	case artnet::PortCommand::STYLE_DELTA0:
	case artnet::PortCommand::STYLE_DELTA1:
	case artnet::PortCommand::STYLE_DELTA2:
	case artnet::PortCommand::STYLE_DELTA3:
		SetOutputStyle(nPage, lightset::OutputStyle::DELTA);
		break;

	case artnet::PortCommand::STYLE_CONSTANT0:
	case artnet::PortCommand::STYLE_CONSTANT1:
	case artnet::PortCommand::STYLE_CONSTANT2:
	case artnet::PortCommand::STYLE_CONSTANT3:
		SetOutputStyle(nPage, lightset::OutputStyle::CONSTANT);
		break;
#endif

#if defined (RDM_CONTROLLER) || defined (RDM_RESPONDER)
	case artnet::PortCommand::RDM_ENABLE0:
	case artnet::PortCommand::RDM_ENABLE1:
	case artnet::PortCommand::RDM_ENABLE2:
	case artnet::PortCommand::RDM_ENABLE3:
		SetRdm(nPage, true);
		break;

	case artnet::PortCommand::RDM_DISABLE0:
	case artnet::PortCommand::RDM_DISABLE1:
	case artnet::PortCommand::RDM_DISABLE2:
	case artnet::PortCommand::RDM_DISABLE3:
		SetRdm(nPage, false);
		break;
#endif
	default:
		DEBUG_PRINTF("> Not implemented: %u [%x]", pArtAddress->Command, pArtAddress->Command);
		break;
	}

	SendPollRelply(pArtAddress->BindIndex, m_nIpAddressFrom);
}
