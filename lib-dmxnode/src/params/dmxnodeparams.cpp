/**
 * @file dmxnodeparams.cpp
 */
/* Copyright (C) 2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#if defined (DEBUG_DMXNODEPARAMS)
# undef NDEBUG
#endif

#include <cstdint>
#include <cstring>
#include <cstddef>
#include <algorithm>
#include <cassert>

#include "dmxnodeparams.h"
#include "dmxnodeparamsconst.h"
#include "dmxnode.h"
#include "dmxnode_nodetype.h"

#include "configstore.h"

#include "propertiesbuilder.h"
#include "readconfigfile.h"
#include "sscan.h"

#include "network.h"

#include "debug.h"

DmxNodeParams::DmxNodeParams(dmxnode::Personality personality) : m_Personality(personality) {
	DEBUG_ENTRY
	DEBUG_PUTS(dmxnode::get_personality(m_Personality));

	ConfigStore::Get()->Copy(configstore::Store::NODE, &m_Params, sizeof(struct dmxnodeparams::Params));
	SetFactoryDefaults();

	debug_dump(&m_Params, static_cast<uint32_t>(sizeof(struct dmxnodeparams::Params)));

	DEBUG_EXIT
}

void DmxNodeParams::Load() {
	DEBUG_ENTRY

#if !defined(DISABLE_FS)
	ReadConfigFile configfile(DmxNodeParams::StaticCallbackFunction, this);

	if (configfile.Read(DmxNodeParamsConst::FILE_NAME[static_cast<uint32_t>(m_Personality)])) {
		DEBUG_PUTS("Update");
		ConfigStore::Get()->Update(configstore::Store::NODE, &m_Params, sizeof(struct dmxnodeparams::Params));
	} else
#endif
		ConfigStore::Get()->Copy(configstore::Store::NODE, &m_Params, sizeof(struct dmxnodeparams::Params));

#ifndef NDEBUG
	Dump();
#endif

	DEBUG_EXIT
}

void DmxNodeParams::Load(const char *pBuffer, const uint32_t nLength) {
	DEBUG_ENTRY

	assert(pBuffer != nullptr);
	assert(nLength != 0);

	ReadConfigFile configfile(DmxNodeParams::StaticCallbackFunction, this);

	configfile.Read(pBuffer, nLength);

	ConfigStore::Get()->Update(configstore::Store::NODE, &m_Params, sizeof(struct dmxnodeparams::Params));

#ifndef NDEBUG
	Dump();
#endif

	DEBUG_EXIT
}

void DmxNodeParams::SetFactoryDefaults() {
	DEBUG_ENTRY
	DEBUG_PRINTF("dmxnode::MAX_PORTS=%u, dmxnode::DMXPORT_OFFSET=%u, dmxnode::CONFIG_PORT_COUNT=%u", dmxnode::MAX_PORTS, dmxnode::DMXPORT_OFFSET, dmxnode::CONFIG_PORT_COUNT);
	DEBUG_PUTS(dmxnode::get_personality(m_Personality));

	switch (m_Personality) {
	case dmxnode::Personality::NODE: {
		m_Params.nSetList &= ~dmxnodeparams::Mask::MASK_NODE;
		constexpr auto nStart = offsetof(struct dmxnodeparams::Params, nPersonality);
		constexpr auto nEnd = offsetof(struct dmxnodeparams::Params, nProtocol);
		constexpr auto nSize = nEnd - nStart;
		static_assert(nSize > 0);
		auto *pBlock = reinterpret_cast<uint8_t *>(&m_Params) + nStart;
		memset(pBlock, 0, nSize);

		if constexpr (dmxnode::CONFIG_PORT_COUNT != 0) {
			for (uint32_t nConfigPortIndex = 0; nConfigPortIndex < dmxnode::CONFIG_PORT_COUNT; nConfigPortIndex++) {
				m_Params.nUniverse[nConfigPortIndex] = nConfigPortIndex + 1;
				auto nDirection = m_Params.nDirection;
				dmxnodeparams::port_set<dmxnode::PortDirection>(nConfigPortIndex, dmxnode::PortDirection::OUTPUT, nDirection);
				m_Params.nDirection = nDirection;
			}
		}
	}
	break;
#if defined (DMXNODE_TYPE_ARTNETNODE)
	case dmxnode::Personality::ARTNET: {
		m_Params.nSetList &= ~dmxnodeparams::Mask::MASK_ARTNET;
		constexpr auto nStart = offsetof(struct dmxnodeparams::Params, nProtocol);
		constexpr auto nEnd = offsetof(struct dmxnodeparams::Params, nPriority);
		constexpr auto nSize = nEnd - nStart;
		static_assert(nSize > 0);
		auto *pBlock = reinterpret_cast<uint8_t *>(&m_Params) + nStart;
		memset(pBlock, 0, nSize);
	}
	break;
#endif
#if defined (DMXNODE_TYPE_E131BRIDGE) || (ARTNET_VERSION >= 4)
	case dmxnode::Personality::SACN: {
		m_Params.nSetList &= ~dmxnodeparams::Mask::MASK_SACN;
		constexpr auto nStart = offsetof(struct dmxnodeparams::Params, nPriority);
		constexpr auto nEnd = offsetof(struct dmxnodeparams::Params, Filler2);
		constexpr auto nSize = nEnd - nStart;
		static_assert(nSize > 0);
		auto *pBlock = reinterpret_cast<uint8_t *>(&m_Params) + nStart;
		memset(pBlock, 0, nSize);
	}
	break;
#endif
	default:
		assert(0);
		break;
	}

	DEBUG_EXIT
}

void DmxNodeParams::CallbackFunction(const char *pLine) {
	DEBUG_ENTRY
	assert(pLine != nullptr);
	DEBUG_PUTS(pLine);

	char value[dmxnode::NODE_NAME_LENGTH];
	uint32_t nLength = 6;

	if (Sscan::Char(pLine, DmxNodeParamsConst::PERSONALITY, value, nLength) == Sscan::OK) {
		m_Params.nPersonality = static_cast<uint8_t>(dmxnode::get_personality(value));
		DEBUG_EXIT
		return;
	}

	nLength = dmxnode::NODE_NAME_LENGTH - 1;

	if (Sscan::Char(pLine, DmxNodeParamsConst::NODE_NAME, reinterpret_cast<char*>(m_Params.aLongName), nLength) == Sscan::OK) {
		m_Params.aLongName[nLength] = '\0';

		DEBUG_EXIT
		return;
	}

	nLength = 8;

	if (Sscan::Char(pLine, DmxNodeParamsConst::FAILSAFE, value, nLength) == Sscan::OK) {
		m_Params.nFailSafe = static_cast<uint8_t>(dmxnode::get_failsafe(value));

		DEBUG_EXIT
		return;
	}

	uint8_t nValue8;

	if (Sscan::Uint8(pLine, DmxNodeParamsConst::DISABLE_MERGE_TIMEOUT, nValue8) == Sscan::OK) {
		if (nValue8 != 0) {
			m_Params.nSetList |= dmxnodeparams::Mask::DISABLE_MERGE_TIMEOUT;
		} else {
			m_Params.nSetList &= ~dmxnodeparams::Mask::DISABLE_MERGE_TIMEOUT;
		}

		DEBUG_EXIT
		return;
	}

	if constexpr (dmxnode::CONFIG_PORT_COUNT != 0) {
		for (uint32_t nConfigPortIndex = 0; nConfigPortIndex < dmxnode::CONFIG_PORT_COUNT; nConfigPortIndex++) {
			nLength = dmxnode::LABEL_NAME_LENGTH - 1;

			if (Sscan::Char(pLine, DmxNodeParamsConst::LABEL_PORT[nConfigPortIndex], reinterpret_cast<char *>(m_Params.aLabel[nConfigPortIndex]), nLength) == Sscan::OK) {
				m_Params.aLabel[nConfigPortIndex][nLength] = '\0';
				return;
			}

			uint16_t nValue16;

			if (Sscan::Uint16(pLine, DmxNodeParamsConst::UNIVERSE_PORT[nConfigPortIndex], nValue16) == Sscan::OK) {
				if (nValue16 != 0) {
					m_Params.nUniverse[nConfigPortIndex] = nValue16;
				}
				return;
			}

			nLength = 7; // input, output, disable

			if (Sscan::Char(pLine, DmxNodeParamsConst::DIRECTION_PORT[nConfigPortIndex], value, nLength) == Sscan::OK) {
				auto nDirection = m_Params.nDirection;
				dmxnodeparams::port_set<dmxnode::PortDirection>(nConfigPortIndex, dmxnode::get_port_direction(value), nDirection);
				m_Params.nDirection = nDirection;
				return;
			}

			nLength = 3; // htp, ltp

			if (Sscan::Char(pLine, DmxNodeParamsConst::MERGE_MODE_PORT[nConfigPortIndex], value, nLength) == Sscan::OK) {
				auto nMergeMode = m_Params.nMergeMode;
				dmxnodeparams::port_set<dmxnode::MergeMode>(nConfigPortIndex, dmxnode::get_merge_mode(value), nMergeMode);
				m_Params.nMergeMode = nMergeMode;
				return;
			}

			nLength = 5; // const, delta

			if (Sscan::Char(pLine, DmxNodeParamsConst::OUTPUT_STYLE_PORT[nConfigPortIndex], value, nLength) == Sscan::OK) {
				const auto nOutputStyle = dmxnode::get_output_style(value);

				if (nOutputStyle != dmxnode::OutputStyle::DELTA) {
					m_Params.nOutputStyle |= static_cast<uint8_t>(1U << nConfigPortIndex);
				} else {
					m_Params.nOutputStyle &= static_cast<uint8_t>(~(1U << nConfigPortIndex));
				}
				return;
			}

#if defined (DMXNODE_TYPE_ARTNETNODE)
			uint32_t nValue32;

			if (Sscan::IpAddress(pLine, DmxNodeParamsConst::DESTINATION_IP_PORT[nConfigPortIndex], nValue32) == Sscan::OK) {
				m_Params.nDestinationIp[nConfigPortIndex] = nValue32;
				return;
			}

# if (ARTNET_VERSION >= 4)
			nLength = 6;	// artnet, sacn

			if (Sscan::Char(pLine, DmxNodeParamsConst::PROTOCOL_PORT[nConfigPortIndex], value, nLength) == Sscan::OK) {
				auto nProtocol = m_Params.nProtocol;
				dmxnodeparams::port_set<artnet::PortProtocol>(nConfigPortIndex, artnet::get_protocol_mode(value), nProtocol);
				m_Params.nProtocol = nProtocol;
				return;
			}

			if (Sscan::Uint8(pLine, DmxNodeParamsConst::RDM_ENABLE_PORT[nConfigPortIndex], nValue8) == Sscan::OK) {
				auto nRdm =  m_Params.nRdm;
				dmxnodeparams::port_set<dmxnode::Rdm>(nConfigPortIndex, static_cast<dmxnode::Rdm>(nValue8 != 0), nRdm);
				m_Params.nRdm = nRdm;
				return;
			}
# endif
#endif

#if defined (DMXNODE_TYPE_E131BRIDGE) || (ARTNET_VERSION >= 4)
			if (Sscan::Uint8(pLine, DmxNodeParamsConst::PRIORITY_PORT[nConfigPortIndex], nValue8) == Sscan::OK) {
				if ((nValue8 >= dmxnode::PRIORITY_LOWEST) && (nValue8 <= dmxnode::PRIORITY_HIGHEST) && (nValue8 != dmxnode::PRIORITY_DEFAULT)) {
					m_Params.nPriority[nConfigPortIndex] = nValue8;
				} else {
					m_Params.nPriority[nConfigPortIndex] = dmxnode::PRIORITY_DEFAULT;
				}
				return;
			}
#endif
		}
	}

#if (ARTNET_VERSION >= 4)
	if (Sscan::Uint8(pLine, DmxNodeParamsConst::ENABLE_RDM, nValue8) == Sscan::OK) {
		if (nValue8 != 0) {
			m_Params.nSetList |= dmxnodeparams::Mask::ENABLE_RDM;
		} else {
			m_Params.nSetList &= ~dmxnodeparams::Mask::ENABLE_RDM;
		}
		return;
	}

	if (Sscan::Uint8(pLine, DmxNodeParamsConst::MAP_UNIVERSE0, nValue8) == Sscan::OK) {
		if (nValue8 != 0) {
			m_Params.nSetList |= dmxnodeparams::Mask::MAP_UNIVERSE0;
		} else {
			m_Params.nSetList &= ~dmxnodeparams::Mask::MAP_UNIVERSE0;
		}
		return;
	}
#endif
	DEBUG_EXIT
}

void DmxNodeParams::Builder(const struct dmxnodeparams::Params *pParams, char *pBuffer, uint32_t nLength, uint32_t &nSize) {
	DEBUG_ENTRY

	if (pParams != nullptr) {
		memcpy(&m_Params, pParams, sizeof(struct dmxnodeparams::Params));
	} else {
		ConfigStore::Get()->Copy(configstore::Store::NODE, &m_Params, sizeof(struct dmxnodeparams::Params));
	}

	PropertiesBuilder builder(DmxNodeParamsConst::FILE_NAME[static_cast<uint32_t>(m_Personality)], pBuffer, nLength);

	if (m_Personality == dmxnode::Personality::NODE) {
		if (m_Params.aLongName[0] == '\0') {
			const auto *pLongName = DmxNodeNodeType::Get()->GetLongName();
			if (pLongName != nullptr) {
				memcpy(m_Params.aLongName, pLongName, dmxnode::NODE_NAME_LENGTH);
			}
		}

		builder.Add(DmxNodeParamsConst::PERSONALITY, dmxnode::get_personality(static_cast<dmxnode::Personality>(m_Params.nPersonality)));
		builder.Add(DmxNodeParamsConst::NODE_NAME, reinterpret_cast<char *>(m_Params.aLongName));
		builder.Add(DmxNodeParamsConst::FAILSAFE, dmxnode::get_failsafe(static_cast<dmxnode::FailSafe>(m_Params.nFailSafe)));
		builder.Add(DmxNodeParamsConst::DISABLE_MERGE_TIMEOUT, IsMaskSet(dmxnodeparams::Mask::DISABLE_MERGE_TIMEOUT));

		if constexpr (dmxnode::CONFIG_PORT_COUNT != 0) {
			for (uint32_t nConfigPortIndex = 0; nConfigPortIndex < dmxnode::CONFIG_PORT_COUNT; nConfigPortIndex++) {
				const auto nPortIndex = nConfigPortIndex + dmxnode::DMXPORT_OFFSET;

				if (nPortIndex >= dmxnode::MAX_PORTS) {
					DEBUG_PUTS("break");
					break;
				}


				if (m_Params.aLabel[nConfigPortIndex][0] == '\0') {
					const auto *pLabelPort = DmxNodeNodeType::Get()->GetShortName(nPortIndex);
					if (pLabelPort != nullptr) {
						memcpy(m_Params.aLabel[nConfigPortIndex], pLabelPort, dmxnode::LABEL_NAME_LENGTH);
					}
				}

				const auto portDirection = dmxnodeparams::port_get<dmxnode::PortDirection>(nConfigPortIndex, m_Params.nDirection);
				const auto portMergeMode = dmxnodeparams::port_get<dmxnode::MergeMode>(nConfigPortIndex, m_Params.nMergeMode);
#if defined (OUTPUT_HAVE_STYLESWITCH)
				const auto nOutputStyle = static_cast<uint32_t>(IsOutputStyleSet(1U << nConfigPortIndex));
#endif
				builder.Add(DmxNodeParamsConst::LABEL_PORT[nConfigPortIndex], reinterpret_cast<char *>(m_Params.aLabel[nConfigPortIndex]));
				builder.Add(DmxNodeParamsConst::UNIVERSE_PORT[nConfigPortIndex], m_Params.nUniverse[nConfigPortIndex]);
				builder.Add(DmxNodeParamsConst::DIRECTION_PORT[nConfigPortIndex], dmxnode::get_port_direction(portDirection));
				builder.Add(DmxNodeParamsConst::MERGE_MODE_PORT[nConfigPortIndex], dmxnode::get_merge_mode(portMergeMode));
#if defined (OUTPUT_HAVE_STYLESWITCH)
				builder.Add(DmxNodeParamsConst::OUTPUT_STYLE_PORT[nConfigPortIndex], dmxnode::get_output_style(static_cast<dmxnode::OutputStyle>(nOutputStyle)));
#endif

			}
		}
	}

#if defined (DMXNODE_TYPE_ARTNETNODE)
	if (m_Personality == dmxnode::Personality::ARTNET) {
		if constexpr (dmxnode::CONFIG_PORT_COUNT != 0) {
			for (uint32_t nConfigPortIndex = 0; nConfigPortIndex < dmxnode::CONFIG_PORT_COUNT; nConfigPortIndex++) {
				const auto nPortIndex = nConfigPortIndex + dmxnode::DMXPORT_OFFSET;

				if (nPortIndex >= dmxnode::MAX_PORTS) {
					DEBUG_PUTS("break");
					break;
				}
# if (ARTNET_VERSION >= 4)
				const auto protocol = dmxnodeparams::port_get<artnet::PortProtocol>(nConfigPortIndex, m_Params.nProtocol);
				builder.Add( DmxNodeParamsConst::PROTOCOL_PORT[nConfigPortIndex], artnet::get_protocol_mode(protocol), static_cast<uint32_t>(protocol));
#  if defined (RDM_CONTROLLER) || defined (RDM_RESPONDER)
				const auto rdm = dmxnodeparams::port_get<dmxnode::Rdm>(nConfigPortIndex, m_Params.nRdm);
				builder.Add(DmxNodeParamsConst::RDM_ENABLE_PORT[nConfigPortIndex], static_cast<uint32_t>(rdm));
#  endif
# endif
				builder.AddIpAddress(DmxNodeParamsConst::DESTINATION_IP_PORT[nConfigPortIndex], m_Params.nDestinationIp[nConfigPortIndex]);
			}

		}
# if (ARTNET_VERSION >= 4)
		builder.AddComment("Art-Net 4");
#  if defined (RDM_CONTROLLER) || defined (RDM_RESPONDER)
		builder.Add(DmxNodeParamsConst::ENABLE_RDM, IsMaskSet(dmxnodeparams::Mask::ENABLE_RDM));
#  endif
		builder.Add(DmxNodeParamsConst::MAP_UNIVERSE0, IsMaskSet(dmxnodeparams::Mask::MAP_UNIVERSE0));
# endif
	}
#endif

#if defined (DMXNODE_TYPE_E131BRIDGE) || (ARTNET_VERSION >= 4)
	if (m_Personality == dmxnode::Personality::SACN) {
		if constexpr (dmxnode::CONFIG_PORT_COUNT != 0) {
			for (uint32_t nConfigPortIndex = 0; nConfigPortIndex < dmxnode::CONFIG_PORT_COUNT; nConfigPortIndex++) {
				const auto nPortIndex = nConfigPortIndex + dmxnode::DMXPORT_OFFSET;

				if (nPortIndex >= dmxnode::MAX_PORTS) {
					DEBUG_PUTS("break");
					break;
				}

				if (m_Params.nPriority[nConfigPortIndex] < dmxnode::PRIORITY_LOWEST) {
					m_Params.nPriority[nConfigPortIndex] = DmxNodeNodeType::Get()->GetPriority(nPortIndex);
				}

				builder.Add(DmxNodeParamsConst::PRIORITY_PORT[nConfigPortIndex], m_Params.nPriority[nConfigPortIndex]);
			}
		}
	}
#endif

	nSize = builder.GetSize();

	DEBUG_PRINTF("nSize=%d", nSize);
	DEBUG_EXIT
}

void DmxNodeParams::Set() {
	DEBUG_ENTRY
	DEBUG_PRINTF("dmxnode::MAX_PORTS=%u, dmxnode::DMXPORT_OFFSET=%u, dmxnode::CONFIG_PORT_COUNT=%u", dmxnode::MAX_PORTS, dmxnode::DMXPORT_OFFSET, dmxnode::CONFIG_PORT_COUNT);
	DEBUG_PUTS(dmxnode::get_personality(m_Personality));

	auto *pDmxNode = DmxNodeNodeType::Get();
	assert(pDmxNode != nullptr);

	if constexpr (dmxnode::CONFIG_PORT_COUNT != 0) {
		for (uint32_t nConfigPortIndex = 0; nConfigPortIndex < dmxnode::CONFIG_PORT_COUNT; nConfigPortIndex++) {
			const auto nPortIndex = nConfigPortIndex + dmxnode::DMXPORT_OFFSET;

			if (nPortIndex >= dmxnode::MAX_PORTS) {
				DEBUG_PUTS("break");
				break;
			}

			if (m_Personality == dmxnode::Personality::NODE) {
				if (m_Params.aLabel[nConfigPortIndex][0] != '\0') {
					pDmxNode->SetShortName(nPortIndex, reinterpret_cast<char *>(m_Params.aLabel[nConfigPortIndex]));
				} else {
					pDmxNode->SetShortName(nPortIndex, nullptr);
				}

				const auto portDirection = dmxnodeparams::port_get<dmxnode::PortDirection>(nConfigPortIndex, m_Params.nDirection);
				pDmxNode->SetUniverse(nPortIndex, portDirection, m_Params.nUniverse[nConfigPortIndex]);

				const auto portMergeMode = dmxnodeparams::port_get<dmxnode::MergeMode>(nConfigPortIndex, m_Params.nMergeMode);
				pDmxNode->SetMergeMode(nPortIndex, portMergeMode);

#if defined (OUTPUT_HAVE_STYLESWITCH)
				const auto nOutputStyle = static_cast<dmxnode::OutputStyle>(IsOutputStyleSet(1U << nConfigPortIndex));
				pDmxNode->SetOutputStyle(nPortIndex, nOutputStyle);
#endif
			}

#if defined (DMXNODE_TYPE_ARTNETNODE)
			if (m_Personality == dmxnode::Personality::ARTNET) {
				pDmxNode->SetDestinationIp(nPortIndex, m_Params.nDestinationIp[nConfigPortIndex]);
#if  (ARTNET_VERSION >= 4)
				pDmxNode->SetPortProtocol4(nPortIndex, dmxnodeparams::port_get<artnet::PortProtocol>(nConfigPortIndex, m_Params.nProtocol));
#  if defined (RDM_CONTROLLER) || defined (RDM_RESPONDER)
				pDmxNode->SetRdm(nPortIndex, (dmxnodeparams::port_get<dmxnode::Rdm>(nConfigPortIndex, m_Params.nRdm) == dmxnode::Rdm::ENABLE));
#  endif
# endif
			}
#endif

#if defined (DMXNODE_TYPE_E131BRIDGE) || (ARTNET_VERSION >= 4)
			pDmxNode->SetPriority(nPortIndex, m_Params.nPriority[nConfigPortIndex]);
#endif
		}
	}

	if (m_Personality == dmxnode::Personality::NODE) {
		pDmxNode->SetFailSafe(static_cast<dmxnode::FailSafe>(m_Params.nFailSafe));
	}

#if defined (DMXNODE_TYPE_ARTNETNODE)
	if (m_Personality == dmxnode::Personality::NODE) {
		if (m_Params.aLongName[0] != 0) {
			pDmxNode->SetLongName(reinterpret_cast<char *>(m_Params.aLongName));
		} else {
			pDmxNode->SetLongName(nullptr);
		}
	}

# if (ARTNET_VERSION >= 4)
	if (m_Personality == dmxnode::Personality::ARTNET) {
		pDmxNode->SetMapUniverse0(IsMaskSet(dmxnodeparams::Mask::MAP_UNIVERSE0));
# if defined (RDM_CONTROLLER) || defined (RDM_RESPONDER)
		pDmxNode->SetRdm(IsMaskSet(dmxnodeparams::Mask::ENABLE_RDM));
# endif
	}
# endif
#endif
}

void DmxNodeParams::Dump() {
	printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, DmxNodeParamsConst::FILE_NAME[static_cast<uint32_t>(m_Personality)]);

	if (m_Personality == dmxnode::Personality::NODE) {
		puts("Node");
		printf(" %s=%s [%u]\n", DmxNodeParamsConst::PERSONALITY, dmxnode::get_personality(static_cast<dmxnode::Personality>(m_Params.nPersonality)), m_Params.nPersonality);
		printf(" %s=%s\n", DmxNodeParamsConst::NODE_NAME, m_Params.aLongName);
		printf(" %s=%s [%u]\n", DmxNodeParamsConst::FAILSAFE, dmxnode::get_failsafe(static_cast<dmxnode::FailSafe>(m_Params.nFailSafe)), m_Params.nFailSafe);
		printf(" %s=%u\n", DmxNodeParamsConst::DISABLE_MERGE_TIMEOUT, IsMaskSet(dmxnodeparams::Mask::DISABLE_MERGE_TIMEOUT));

		if constexpr (dmxnode::CONFIG_PORT_COUNT != 0) {
			for (uint32_t nPortIndex = 0; nPortIndex < dmxnode::CONFIG_PORT_COUNT; nPortIndex++) {
				printf(" %s=%s\n", DmxNodeParamsConst::LABEL_PORT[nPortIndex], reinterpret_cast<char *>(m_Params.aLabel[nPortIndex]));
				printf(" %s=%u\n", DmxNodeParamsConst::UNIVERSE_PORT[nPortIndex], m_Params.nUniverse[nPortIndex]);
				const auto portDirection = dmxnodeparams::port_get<dmxnode::PortDirection>(nPortIndex, m_Params.nDirection);
				printf(" %s=%s [%u]\n", DmxNodeParamsConst::DIRECTION_PORT[nPortIndex], dmxnode::get_port_direction(portDirection), static_cast<uint32_t>(portDirection));
				const auto portMergeMode = dmxnodeparams::port_get<dmxnode::MergeMode>(nPortIndex, m_Params.nMergeMode);
				printf(" %s=%s [%u]\n", DmxNodeParamsConst::MERGE_MODE_PORT[nPortIndex], dmxnode::get_merge_mode(portMergeMode), static_cast<uint32_t>(portMergeMode));
#if defined (OUTPUT_HAVE_STYLESWITCH)
				const auto nOutputStyle = static_cast<uint32_t>(IsOutputStyleSet(1U << nPortIndex));
				printf(" %s=%s [%u]\n", DmxNodeParamsConst::OUTPUT_STYLE_PORT[nPortIndex], dmxnode::get_output_style(static_cast<dmxnode::OutputStyle>(nOutputStyle)), nOutputStyle);
#endif
			}
		}
	}

#if defined (DMXNODE_TYPE_ARTNETNODE)
	if (m_Personality == dmxnode::Personality::ARTNET) {
		puts("Art-Net");
# if (ARTNET_VERSION >= 4)
		printf(" %s=%u\n", DmxNodeParamsConst::ENABLE_RDM, IsMaskSet(dmxnodeparams::Mask::ENABLE_RDM));
		printf(" %s=%u\n", DmxNodeParamsConst::MAP_UNIVERSE0, IsMaskSet(dmxnodeparams::Mask::MAP_UNIVERSE0));
# endif
		if constexpr (dmxnode::CONFIG_PORT_COUNT != 0) {
			for (uint32_t nPortIndex = 0; nPortIndex < dmxnode::CONFIG_PORT_COUNT; nPortIndex++) {
# if (ARTNET_VERSION >= 4)
				const auto protocol = dmxnodeparams::port_get<artnet::PortProtocol>(nPortIndex, m_Params.nProtocol);
				printf(" %s=%s [%u]\n", DmxNodeParamsConst::PROTOCOL_PORT[nPortIndex], artnet::get_protocol_mode(protocol), static_cast<uint32_t>(protocol));
				const auto rdm = dmxnodeparams::port_get<dmxnode::Rdm>(nPortIndex, m_Params.nRdm);
				printf(" %s=%u\n", DmxNodeParamsConst::RDM_ENABLE_PORT[nPortIndex], static_cast<uint32_t>(rdm));
# endif
				printf(" %s=" IPSTR "\n", DmxNodeParamsConst::DESTINATION_IP_PORT[nPortIndex], IP2STR(m_Params.nDestinationIp[nPortIndex]));
			}
		}
	}
#endif

#if defined (DMXNODE_TYPE_E131BRIDGE) || (ARTNET_VERSION >= 4)
	if ((m_Personality == dmxnode::Personality::SACN) || (m_Personality == dmxnode::Personality::ARTNET)) {
		puts("sACN");

		if constexpr (dmxnode::CONFIG_PORT_COUNT != 0) {
			for (uint32_t nPortIndex = 0; nPortIndex < dmxnode::CONFIG_PORT_COUNT; nPortIndex++) {
				printf(" %s=%d\n", DmxNodeParamsConst::PRIORITY_PORT[nPortIndex], m_Params.nPriority[nPortIndex]);
			}
		}
	}
#endif
}
