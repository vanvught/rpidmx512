/**
 * @file nodeparams.cpp
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
#include <cstring>
#include <cassert>
#include <algorithm>

#include "nodeparams.h"
#include "node.h"
#include "nodeparamsconst.h"

#include "lightset.h"
#include "network.h"

#include "readconfigfile.h"
#include "sscan.h"

#include "propertiesbuilder.h"

#include "debug.h"

static uint32_t s_nPortsMax;

nodeparams::Params NodeParams::m_Params;

namespace nodeparams {
static constexpr uint16_t shift_right(const uint32_t nValue, const uint32_t i) {
	return static_cast<uint16_t>((nValue >> i) & 0x1);
}

static constexpr bool is_set(const uint16_t nValue, const uint32_t i) {
	return (nValue & static_cast<uint16_t>(1U << (i + 8))) == static_cast<uint16_t>(1U << (i + 8));
}

static constexpr uint16_t portdir_shift_left(const lightset::PortDir portDir, const uint32_t i) {
	return static_cast<uint16_t>((static_cast<uint32_t>(portDir) & 0x3) << (i * 2));
}
static constexpr uint16_t portdir_shif_right(const uint32_t nValue, const uint32_t i) {
	return static_cast<uint16_t>((nValue >> (i * 2)) & 0x3);
}
static constexpr uint16_t portdir_clear(const uint32_t i) {
	return static_cast<uint16_t>(~(0x3 << (i * 2)));
}
}  // namespace nodeparams

void NodeParams::ResetSet(node::Personality personality) {
	DEBUG_PRINTF("personality=%u", static_cast<uint32_t>(personality));

	switch (personality) {
	case node::Personality::NODE:
		m_Params.nSetList &= ~nodeparams::MASK_NODE;
		m_Params.nPersonality = static_cast<uint8_t>(node::defaults::PERSONALITY);
		m_Params.nFailSafe = static_cast<uint8_t>(node::defaults::FAILSAFE);

		for (uint32_t i = 0; i < nodeparams::MAX_PORTS; i++) {
			m_Params.nUniverse[i] = static_cast<uint16_t>(i + 1);
			m_Params.nMergeMode &= nodeparams::clear_mask(i);
			constexpr auto n = static_cast<uint32_t>(lightset::PortDir::OUTPUT) & 0x3;
			m_Params.nDirection |= static_cast<uint16_t>(n << (i * 2));
		}
		break;
	case node::Personality::ARTNET:
		m_Params.nSetList &= ~nodeparams::MASK_ARTNET;
		memset(m_Params.aLongName, 0, sizeof(m_Params.aLongName));
		memset(m_Params.aShortName, 0, sizeof(m_Params.aShortName));
		for (uint32_t i = 0; i < nodeparams::MAX_PORTS; i++) {
			m_Params.nProtocol &= nodeparams::clear_mask(i);
			m_Params.nRdm &= nodeparams::clear_mask(i);
			m_Params.nDestinationIp[i] = 0;
		}
		break;
	case node::Personality::E131:
		m_Params.nSetList &= ~nodeparams::MASK_E131;
		for (uint32_t i = 0; i < nodeparams::MAX_PORTS; i++) {
			m_Params.nPriority[i] = node::priority::DEFAULT;
		}
		break;
	case node::Personality::UNKNOWN:
		memset(&m_Params, 0, sizeof(struct nodeparams::Params));
		break;
	default:
		assert(0);
		__builtin_unreachable();
		break;
	}
}

NodeParams::NodeParams(NodeParamsStore *pNodeParamsStore, node::Personality personality):
	m_pNodeParamsStore(pNodeParamsStore),
	m_Personality(personality)
{
	DEBUG_ENTRY
	DEBUG_PRINTF("%s", node::get_personality_full(personality));

	debug_dump(&m_Params, sizeof(m_Params));

	if (m_Personality == node::Personality::UNKNOWN) {
		for (uint32_t i = 0; i < static_cast<uint32_t>(node::Personality::UNKNOWN); i++) {
			ResetSet(static_cast<node::Personality>(i));
		}
	} else {
		ResetSet(m_Personality);
	}

	if (s_nPortsMax == 0) {
		s_nPortsMax = std::min(nodeparams::MAX_PORTS, artnetnode::MAX_PORTS);
	}

	DEBUG_PRINTF("s_nPortsMax=%u", s_nPortsMax);
	DEBUG_EXIT
}

bool NodeParams::Load() {
	DEBUG_ENTRY

	ResetSet(m_Personality);

#if !defined(DISABLE_FS)
	ReadConfigFile configfile(NodeParams::staticCallbackFunction, this);

	auto isRead = false;

	if (m_Personality != node::Personality::UNKNOWN) {
		isRead = configfile.Read(NodeParamsConst::FILE_NAME[static_cast<uint32_t>(m_Personality)]);
	} else {
		for (uint32_t i = 0; i < static_cast<uint32_t>(node::Personality::UNKNOWN); i++) {
			ResetSet(m_Personality);
			m_Personality = static_cast<node::Personality>(i);
			isRead |= configfile.Read(NodeParamsConst::FILE_NAME[i]);
		}
		m_Personality = node::Personality::UNKNOWN;
	}

	DEBUG_PRINTF("isRead %c", isRead ? 'Y' : 'N');

	if (isRead) {
		if (m_pNodeParamsStore != nullptr) {
			m_pNodeParamsStore->Update(&m_Params);
		}
	} else
#endif
	if (m_pNodeParamsStore != nullptr) {
		m_pNodeParamsStore->Copy(&m_Params);
	} else {
		DEBUG_EXIT
		return false;
	}

	DEBUG_EXIT
	return true;
}

void NodeParams::Load(const char *pBuffer, uint32_t nLength) {
	DEBUG_ENTRY

	assert(pBuffer != nullptr);
	assert(nLength != 0);

	assert(m_pNodeParamsStore != nullptr);

	if (m_pNodeParamsStore == nullptr) {
		return;
	}

	ResetSet(m_Personality);

	ReadConfigFile config(NodeParams::staticCallbackFunction, this);

	config.Read(pBuffer, nLength);

	m_pNodeParamsStore->Update(&m_Params);

	DEBUG_EXIT
}

void NodeParams::callbackFunction(const char *pLine) {
	assert(pLine != nullptr);

	uint8_t nValue8;
	char value[16];
	uint32_t nLength = 4;

	/**
	 * Node
	 */

	if (Sscan::Char(pLine, NodeParamsConst::PERSONALITY, value, nLength) == Sscan::OK) {
		const auto personality = node::get_personality(value);

		if (personality == node::defaults::PERSONALITY) {
			m_Params.nSetList &= ~nodeparams::Mask::PERSONALITY;
		} else {
			m_Params.nSetList |= nodeparams::Mask::PERSONALITY;
		}

		m_Params.nPersonality = static_cast<uint8_t>(personality);
		return;
	}

	nLength = 8;

	if (Sscan::Char(pLine, NodeParamsConst::FAILSAFE, value, nLength) == Sscan::OK) {
		const auto failsafe = lightset::get_failsafe(value);

		if (failsafe == node::defaults::FAILSAFE) {
			m_Params.nSetList &= ~nodeparams::Mask::FAILSAFE;
		} else {
			m_Params.nSetList |= nodeparams::Mask::FAILSAFE;
		}

		m_Params.nFailSafe = static_cast<uint8_t>(failsafe);
		return;
	}

	if (Sscan::Uint8(pLine, NodeParamsConst::DISABLE_MERGE_TIMEOUT, nValue8) == Sscan::OK) {
		if (nValue8 != 0) {
			m_Params.nSetList |= nodeparams::Mask::DISABLE_MERGE_TIMEOUT;
		} else {
			m_Params.nSetList &= ~nodeparams::Mask::DISABLE_MERGE_TIMEOUT;
		}
		return;
	}

	/**
	 * Art-Net 4
	 */

	nLength = node::SHORT_NAME_LENGTH - 1;

	if (Sscan::Char(pLine, NodeParamsConst::NODE_SHORT_NAME, reinterpret_cast<char*>(m_Params.aShortName), nLength) == Sscan::OK) {
		if (nLength > 0) {
			m_Params.aShortName[nLength] = '\0';
			m_Params.nSetList |= nodeparams::Mask::SHORT_NAME;
		} else {
			m_Params.nSetList &= ~nodeparams::Mask::SHORT_NAME;
		}
		return;
	}

	nLength = node::LONG_NAME_LENGTH - 1;

	if (Sscan::Char(pLine, NodeParamsConst::NODE_LONG_NAME, reinterpret_cast<char*>(m_Params.aLongName), nLength) == Sscan::OK) {
		if (nLength > 0) {
			m_Params.aLongName[nLength] = '\0';
			m_Params.nSetList |= nodeparams::Mask::LONG_NAME;
		} else {
			m_Params.nSetList &= ~nodeparams::Mask::LONG_NAME;
		}
		return;
	}

	if (Sscan::Uint8(pLine, NodeParamsConst::ENABLE_RDM, nValue8) == Sscan::OK) {
		if (nValue8 != 0) {
			m_Params.nSetList |= nodeparams::Mask::ENABLE_RDM;
		} else {
			m_Params.nSetList &= ~nodeparams::Mask::ENABLE_RDM;
		}
		return;
	}

	if (Sscan::Uint8(pLine, NodeParamsConst::MAP_UNIVERSE0, nValue8) == Sscan::OK) {
		if (nValue8 != 0) {
			m_Params.nSetList |= nodeparams::Mask::MAP_UNIVERSE0;
		} else {
			m_Params.nSetList &= ~nodeparams::Mask::MAP_UNIVERSE0;
		}
		return;
	}

	for (uint32_t i = 0; i < nodeparams::MAX_PORTS; i++) {
		uint16_t nValue16;

		/**
		 * Node
		 */

		if (Sscan::Uint16(pLine, NodeParamsConst::UNIVERSE_PORT[i], nValue16) == Sscan::OK) {
			if ((nValue16 > node::universe::MAX)) {
				m_Params.nUniverse[i] = static_cast<uint16_t>(i + 1);
				m_Params.nSetList &= ~(nodeparams::Mask::UNIVERSE_A << i);
			} else {
				m_Params.nUniverse[i] = nValue16;
				m_Params.nSetList |= (nodeparams::Mask::UNIVERSE_A << i);
			}
			return;
		}

		nLength = 3;

		if (Sscan::Char(pLine, NodeParamsConst::MERGE_MODE_PORT[i], value, nLength) == Sscan::OK) {
			m_Params.nMergeMode &= nodeparams::clear_mask(i);

			if (lightset::get_merge_mode(value) == lightset::MergeMode::LTP) {
				m_Params.nMergeMode |= nodeparams::shift_left(static_cast<uint32_t>(lightset::MergeMode::LTP), i);
				m_Params.nMergeMode |= static_cast<uint16_t>(1U << (i + 8));
			} else {
				m_Params.nMergeMode |= nodeparams::shift_left(static_cast<uint32_t>(lightset::MergeMode::HTP), i);
			}
			return;
		}

		nLength = 7;

		if (Sscan::Char(pLine, NodeParamsConst::DIRECTION[i], value, nLength) == Sscan::OK) {
			const auto portDir = lightset::get_direction(value);
			m_Params.nDirection &= nodeparams::portdir_clear(i);

			if (portDir == lightset::PortDir::INPUT) {
				m_Params.nDirection |= nodeparams::portdir_shift_left(lightset::PortDir::INPUT, i);
			} else if (portDir == lightset::PortDir::DISABLE) {
				m_Params.nDirection |= nodeparams::portdir_shift_left(lightset::PortDir::DISABLE, i);
			} else {
				m_Params.nDirection |= nodeparams::portdir_shift_left(lightset::PortDir::OUTPUT, i);
			}
			return;
		}

		/**
		 * Art-Net 4
		 */

		nLength = 4;

		if (Sscan::Char(pLine, NodeParamsConst::PROTOCOL_PORT[i], value, nLength) == Sscan::OK) {
			m_Params.nProtocol &= nodeparams::clear_mask(i);

			if (memcmp(value, "sacn", 4) == 0) {
				m_Params.nProtocol |= nodeparams::shift_left(static_cast<uint32_t>(node::PortProtocol::SACN), i);
				m_Params.nProtocol |= static_cast<uint16_t>(1U << (i + 8));
			} else {
				m_Params.nProtocol |= nodeparams::shift_left(static_cast<uint32_t>(node::PortProtocol::ARTNET), i);
			}
			return;
		}

		uint32_t nValue32;

		if (Sscan::IpAddress(pLine, NodeParamsConst::DESTINATION_IP_PORT[i], nValue32) == Sscan::OK) {
			m_Params.nDestinationIp[i] = nValue32;
		}

		if (Sscan::Uint8(pLine, NodeParamsConst::RDM_ENABLE_PORT[i], nValue8) == Sscan::OK) {
			m_Params.nRdm &= nodeparams::clear_mask(i);

			if (nValue8 != 0) {
				m_Params.nRdm |= nodeparams::shift_left(1, i);
				m_Params.nRdm |= static_cast<uint16_t>(1U << (i + 8));
			}
			return;
		}

		/**
		 * sACN E1.31
		 */

		if (Sscan::Uint8(pLine, NodeParamsConst::PRIORITY[i], nValue8) == Sscan::OK) {
			if ((nValue8 >= node::priority::LOWEST) && (nValue8 <= node::priority::HIGHEST) && (nValue8 != node::priority::DEFAULT)) {
				m_Params.nPriority[i] = nValue8;
				m_Params.nSetList |= (nodeparams::Mask::PRIORITY_A << i);
			} else {
				m_Params.nPriority[i] = node::priority::DEFAULT;
				m_Params.nSetList &= ~(nodeparams::Mask::PRIORITY_A << i);
			}
			return;
		}
	}
}

void NodeParams::Builder(const struct nodeparams::Params *pParams, char *pBuffer, uint32_t nLength, uint32_t &nSize) {
	DEBUG_ENTRY

	if (pParams != nullptr) {
		memcpy(&m_Params, pParams, sizeof(struct nodeparams::Params));
	} else {
		m_pNodeParamsStore->Copy(&m_Params);
	}

	PropertiesBuilder builder(NodeParamsConst::FILE_NAME[static_cast<uint32_t>(m_Personality)], pBuffer, nLength);

	/**
	 * Node
	 */

	if (m_Personality == node::Personality::NODE) {
		builder.Add(NodeParamsConst::PERSONALITY, node::get_personality(static_cast<node::Personality>(m_Params.nPersonality)), isMaskSet(nodeparams::Mask::PERSONALITY));
		builder.AddComment(node::get_personality_full(static_cast<node::Personality>(m_Params.nPersonality)));

		builder.Add(NodeParamsConst::FAILSAFE, lightset::get_failsafe(static_cast<lightset::FailSafe>(m_Params.nFailSafe)), isMaskSet(nodeparams::Mask::FAILSAFE));

		for (uint32_t i = 0; i < s_nPortsMax; i++) {
			builder.Add(NodeParamsConst::UNIVERSE_PORT[i], m_Params.nUniverse[i], isMaskSet(nodeparams::Mask::UNIVERSE_A << i));
			const auto portDir = static_cast<lightset::PortDir>(nodeparams::portdir_shif_right(m_Params.nDirection, i));
			const auto isDefault = (portDir == lightset::PortDir::OUTPUT);
			builder.Add(NodeParamsConst::DIRECTION[i], lightset::get_direction(portDir), !isDefault);
			builder.Add(NodeParamsConst::MERGE_MODE_PORT[i], lightset::get_merge_mode(nodeparams::shift_right(m_Params.nMergeMode, i)), nodeparams::is_set(m_Params.nMergeMode, i));
		}

		builder.Add(NodeParamsConst::DISABLE_MERGE_TIMEOUT, isMaskSet(nodeparams::Mask::DISABLE_MERGE_TIMEOUT));
	}

	/**
	 * Art-Net
	 */

	if (m_Personality == node::Personality::ARTNET) {
		if (!isMaskSet(nodeparams::Mask::SHORT_NAME)) {
			strncpy(reinterpret_cast<char *>(m_Params.aShortName), ArtNetNode::Get()->GetShortName(), sizeof(m_Params.aShortName) - 1);
			m_Params.aShortName[sizeof(m_Params.aShortName) - 1] = '\0';
		}

		builder.Add(NodeParamsConst::NODE_SHORT_NAME, reinterpret_cast<const char*>(m_Params.aShortName), isMaskSet(nodeparams::Mask::SHORT_NAME));

		if (!isMaskSet(nodeparams::Mask::LONG_NAME)) {
			strncpy(reinterpret_cast<char *>(m_Params.aLongName), ArtNetNode::Get()->GetLongName(), sizeof(m_Params.aLongName) - 1);
			m_Params.aLongName[sizeof(m_Params.aLongName) - 1] = '\0';
		}

		builder.Add(NodeParamsConst::NODE_LONG_NAME, reinterpret_cast<const char*>(m_Params.aLongName), isMaskSet(nodeparams::Mask::LONG_NAME));

		builder.AddComment("DMX Output");

		builder.Add(NodeParamsConst::ENABLE_RDM, isMaskSet(nodeparams::Mask::ENABLE_RDM));

		for (uint32_t i = 0; i < s_nPortsMax; i++) {
			builder.Add(NodeParamsConst::RDM_ENABLE_PORT[i], nodeparams::is_set(m_Params.nRdm, i));
		}

		builder.AddComment("DMX Input");

		for (uint32_t i = 0; i < s_nPortsMax; i++) {
			const auto isSet = (m_Params.nDestinationIp[i] != 0);
			if (!isSet) {
				m_Params.nDestinationIp[i] = ArtNetNode::Get()->GetDestinationIp(i);
			}
			builder.AddIpAddress(NodeParamsConst::DESTINATION_IP_PORT[i], m_Params.nDestinationIp[i], isSet);
		}

		builder.AddComment("Art-Net 4");

		for (uint32_t i = 0; i < s_nPortsMax; i++) {
			builder.Add(NodeParamsConst::PROTOCOL_PORT[i], artnet::get_protocol_mode(nodeparams::shift_right(m_Params.nProtocol, i)), nodeparams::is_set(m_Params.nProtocol, i));
		}

		builder.Add(NodeParamsConst::MAP_UNIVERSE0, isMaskSet(nodeparams::Mask::MAP_UNIVERSE0));
	}

	/**
	 * sACN E.131
	 */

	if (m_Personality == node::Personality::E131) {
		builder.AddComment("DMX Input");

		for (uint32_t i = 0; i < s_nPortsMax; i++) {
			builder.Add(NodeParamsConst::PRIORITY[i], m_Params.nPriority[i], isMaskSet(nodeparams::Mask::PRIORITY_A << i));
		}
	}

	nSize = builder.GetSize();

	DEBUG_PRINTF("nSize=%d", nSize);
	DEBUG_EXIT
}

void NodeParams::Save(char *pBuffer, uint32_t nLength, uint32_t &nSize) {
	DEBUG_ENTRY

	if (m_pNodeParamsStore == nullptr) {
		nSize = 0;
		DEBUG_EXIT
		return;
	}

	Builder(nullptr, pBuffer, nLength, nSize);
}

void NodeParams::Set(uint32_t nPortIndexOffset) {
	DEBUG_ENTRY

	if ((nPortIndexOffset != 0) && (nPortIndexOffset < Node::Get()->GetPorts())) {
		s_nPortsMax = std::min(s_nPortsMax, (Node::Get()->GetPorts() - nPortIndexOffset));
	}

	DEBUG_PRINTF("s_nPortsMax=%u", s_nPortsMax);

	for (uint32_t nPortIndex = 0; nPortIndex < s_nPortsMax; nPortIndex++) {
		const auto nOffset = nPortIndex + nPortIndexOffset;

		/**
		 * Art-Net
		 */
		///< This must be first!
		if (nodeparams::is_set(m_Params.nProtocol, nPortIndex)) {
			const auto portProtocol = static_cast<node::PortProtocol>(nodeparams::shift_right(m_Params.nProtocol, nPortIndex));
			Node::Get()->SetProtocol(nOffset, portProtocol);
		}

		if (m_Params.nDestinationIp[nPortIndex] != 0) {
			Node::Get()->SetDestinationIp(nOffset, m_Params.nDestinationIp[nPortIndex]);
		}

		if (nodeparams::is_set(m_Params.nRdm, nPortIndex)) {
			Node::Get()->SetRmd(nOffset, true);
		}

		/**
		 * sACN E.131
		 */

		if (isMaskSet(nodeparams::Mask::PRIORITY_A << nPortIndex)) {
			Node::Get()->SetPriority(nOffset, m_Params.nPriority[nPortIndex]);
		}

		/**
		 * Node
		 */

		const auto dir = static_cast<lightset::PortDir>(nodeparams::portdir_shif_right(m_Params.nDirection, nPortIndex));
		const auto nUniverse =  m_Params.nUniverse[nPortIndex];
		Node::Get()->SetUniverse(nOffset, dir, nUniverse);

		if (nodeparams::is_set(m_Params.nMergeMode, nPortIndex)) {
			Node::Get()->SetMergeMode(nOffset, static_cast<lightset::MergeMode>(nodeparams::shift_right(m_Params.nMergeMode, nPortIndex)));
		}
	}

	/**
	 * Node
	 */

	Node::Get()->SetFailSafe(static_cast<lightset::FailSafe>(m_Params.nFailSafe));

	/**
	 * Art-Net
	 */

	if (isMaskSet(nodeparams::Mask::SHORT_NAME)) {
		Node::Get()->SetShortName(reinterpret_cast<const char*>(m_Params.aShortName));
	}

	if (isMaskSet(nodeparams::Mask::LONG_NAME)) {
		Node::Get()->SetLongName(reinterpret_cast<const char*>(m_Params.aLongName));
	}

	Node::Get()->SetRdm(isMaskSet(nodeparams::Mask::ENABLE_RDM));
	Node::Get()->SetMapUniverse0(isMaskSet(nodeparams::Mask::MAP_UNIVERSE0));

	DEBUG_EXIT
}

void NodeParams::Dump() {
#ifndef NDEBUG
	printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, NodeParamsConst::FILE_NAME[static_cast<uint32_t>(node::Personality::NODE)]);

	if (isMaskSet(nodeparams::Mask::PERSONALITY)) {
		printf(" %s=%d [%s]\n", NodeParamsConst::PERSONALITY, m_Params.nPersonality, node::get_personality(static_cast<node::Personality>(m_Params.nPersonality)));
	}

	if (isMaskSet(nodeparams::Mask::FAILSAFE)) {
		printf(" %s=%d [%s]\n", NodeParamsConst::FAILSAFE, m_Params.nFailSafe, lightset::get_failsafe(static_cast<lightset::FailSafe>(m_Params.nFailSafe)));
	}

	for (uint32_t i = 0; i < nodeparams::MAX_PORTS; i++) {
		if (isMaskSet(nodeparams::Mask::UNIVERSE_A << i)) {
			printf(" %s=%d\n", NodeParamsConst::UNIVERSE_PORT[i], m_Params.nUniverse[i]);
		}

		printf(" %s=%d [%s]\n", NodeParamsConst::DIRECTION[i], nodeparams::portdir_shif_right(m_Params.nDirection, i), lightset::get_direction(static_cast<lightset::PortDir>(nodeparams::portdir_shif_right(m_Params.nDirection, i))));

		if (nodeparams::is_set(m_Params.nMergeMode, i)) {
			printf(" %s=%s\n", NodeParamsConst::MERGE_MODE_PORT[i], lightset::get_merge_mode(nodeparams::shift_right(m_Params.nMergeMode, i)));
		}
	}

	if (isMaskSet(nodeparams::Mask::DISABLE_MERGE_TIMEOUT)) {
		printf(" %s=1 [Yes]\n", NodeParamsConst::DISABLE_MERGE_TIMEOUT);
	}

	/**
	 * Art-Net
	 */

	printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, NodeParamsConst::FILE_NAME[static_cast<uint32_t>(node::Personality::ARTNET)]);

	if (isMaskSet(nodeparams::Mask::SHORT_NAME)) {
		printf(" %s=%s\n", NodeParamsConst::NODE_SHORT_NAME, m_Params.aShortName);
	}

	if (isMaskSet(nodeparams::Mask::LONG_NAME)) {
		printf(" %s=%s\n", NodeParamsConst::NODE_LONG_NAME, m_Params.aLongName);
	}

	if (isMaskSet(nodeparams::Mask::ENABLE_RDM)) {
		printf(" %s=1 [Yes]\n", NodeParamsConst::ENABLE_RDM);
	}

	for (uint32_t i = 0; i < nodeparams::MAX_PORTS; i++) {
		if (nodeparams::is_set(m_Params.nProtocol, i)) {
			printf(" %s=%s\n", NodeParamsConst::PROTOCOL_PORT[i], artnet::get_protocol_mode(nodeparams::shift_right(m_Params.nProtocol, i), true));
		}
	}

	for (uint32_t i = 0; i < nodeparams::MAX_PORTS; i++) {
		if (m_Params.nDestinationIp[i] != 0) {
			printf(" %s=" IPSTR "\n", NodeParamsConst::DESTINATION_IP_PORT[i], IP2STR(m_Params.nDestinationIp[i]));
		}
	}

	for (uint32_t i = 0; i < nodeparams::MAX_PORTS; i++) {
		if (nodeparams::is_set(m_Params.nRdm, i)) {
			printf(" %s=1 [Yes]\n", NodeParamsConst::RDM_ENABLE_PORT[i]);
		}
	}

	if (isMaskSet(nodeparams::Mask::MAP_UNIVERSE0)) {
		printf(" %s=1 [Yes]\n", NodeParamsConst::MAP_UNIVERSE0);
	}

	/**
	 * sACN E.131
	 */

	printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, NodeParamsConst::FILE_NAME[static_cast<uint32_t>(node::Personality::E131)]);

	for (uint32_t i = 0; i < nodeparams::MAX_PORTS; i++) {
		if (isMaskSet(nodeparams::Mask::PRIORITY_A << i)) {
			printf(" %s=%d\n", NodeParamsConst::PRIORITY[i], m_Params.nPriority[i]);
		}
	}
#endif
}

void NodeParams::staticCallbackFunction(void *p, const char *s) {
	assert(p != nullptr);
	assert(s != nullptr);

	(static_cast<NodeParams*>(p))->callbackFunction(s);
}
