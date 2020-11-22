/**
 * @file artnetnode.cpp
 *
 */
/**
 * Art-Net Designed by and Copyright Artistic Licence Holdings Ltd.
 */
/* Copyright (C) 2016-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <algorithm>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <cassert>

#include "artnetnode.h"
#include "artnetconst.h"
#include "packets.h"

#include "lightset.h"

#include "artnetrdm.h"
#include "artnettimecode.h"
#include "artnettimesync.h"
#include "artnetipprog.h"
#include "artnetstore.h"
#include "artnetdisplay.h"
#include "artnetdmx.h"

#include "hardware.h"
#include "network.h"
#include "ledblink.h"

#include "artnetnode_internal.h"

union uip {
	uint32_t u32;
	uint8_t u8[4];
} static ip;

#define NODE_DEFAULT_SHORT_NAME		"AvV Art-Net Node"
#define NODE_DEFAULT_NET_SWITCH		0
#define NODE_DEFAULT_SUBNET_SWITCH	0
#define NODE_DEFAULT_UNIVERSE		1

static const uint8_t DEVICE_SOFTWARE_VERSION[] = { 1, 47 };

#define ARTNET_MIN_HEADER_SIZE			12
#define ARTNET_MERGE_TIMEOUT_SECONDS	10

#define NETWORK_DATA_LOSS_TIMEOUT		10	///< Seconds

#define PORT_IN_STATUS_DISABLED_MASK	0x08

ArtNetNode *ArtNetNode::s_pThis = nullptr;

ArtNetNode::ArtNetNode(uint8_t nVersion, uint8_t nPages) :
	m_nVersion(nVersion),
	m_nPages(nPages <= ArtNet::MAX_PAGES ? nPages : ArtNet::MAX_PAGES)
{
	assert(Hardware::Get() != nullptr);
	assert(Network::Get() != nullptr);
	assert(LedBlink::Get() != nullptr);

	s_pThis = this;

	memset(&m_Node, 0, sizeof (struct TArtNetNode));
	m_Node.Status1 = STATUS1_INDICATOR_NORMAL_MODE | STATUS1_PAP_FRONT_PANEL;
	m_Node.Status2 = ArtNetStatus2::PORT_ADDRESS_15BIT | (m_nVersion > 3 ? ArtNetStatus2::SACN_ABLE_TO_SWITCH : ArtNetStatus2::SACN_NO_SWITCH);

	memset(&m_State, 0, sizeof (struct TArtNetNodeState));
	m_State.reportCode = ARTNET_RCPOWEROK;
	m_State.status = ARTNET_STANDBY;
	m_State.nNetworkDataLossTimeoutMillis = NETWORK_DATA_LOSS_TIMEOUT * 1000;

	for (uint32_t i = 0; i < ARTNET_NODE_MAX_PORTS_OUTPUT; i++) {
		m_IsLightSetRunning[i] = false;
		memset(&m_OutputPorts[i], 0 , sizeof(struct TOutputPort));
	}

	for (uint32_t i = 0; i < (ARTNET_NODE_MAX_PORTS_INPUT); i++) {
		memset(&m_InputPorts[i], 0 , sizeof(struct TInputPort));
		m_InputPorts[i].nDestinationIp = Network::Get()->GetIp() | ~(Network::Get()->GetNetmask());
	}

	SetShortName(NODE_DEFAULT_SHORT_NAME);

	uint8_t nBoardNameLength;
	const char *pBoardName = Hardware::Get()->GetBoardName(nBoardNameLength);
	const char *pWebsiteUrl = Hardware::Get()->GetWebsiteUrl();
	snprintf(m_aDefaultNodeLongName, ArtNet::LONG_NAME_LENGTH, "%s %s %d %s", pBoardName, artnet::NODE_ID, m_nVersion, pWebsiteUrl);
	SetLongName(m_aDefaultNodeLongName);

	SetOemValue(ArtNetConst::OEM_ID);

	uint8_t nSysNameLenght;
	const char *pSysName = Hardware::Get()->GetSysName(nSysNameLenght);
	strncpy(m_aSysName, pSysName, (sizeof m_aSysName) - 1);
	m_aSysName[(sizeof m_aSysName) - 1] = '\0';
}

ArtNetNode::~ArtNetNode() {
	Stop();

	if (m_pTodData != nullptr) {
		delete m_pTodData;
	}

	if (m_pIpProgReply != nullptr) {
		delete m_pIpProgReply;
	}

	if (m_pTimeCodeData != nullptr) {
		delete m_pTimeCodeData;
	}
}

void ArtNetNode::Start() {
	assert(Network::Get() != nullptr);
	assert(LedBlink::Get() != nullptr);

	m_Node.IPAddressLocal = Network::Get()->GetIp();
	m_Node.IPAddressBroadcast = m_Node.IPAddressLocal | ~(Network::Get()->GetNetmask());

	Network::Get()->MacAddressCopyTo(m_Node.MACAddressLocal);

	m_Node.Status2 = (m_Node.Status2 & ~(ArtNetStatus2::IP_DHCP)) | (Network::Get()->IsDhcpUsed() ? ArtNetStatus2::IP_DHCP : ArtNetStatus2::IP_MANUALY);
	m_Node.Status2 = (m_Node.Status2 & ~(ArtNetStatus2::DHCP_CAPABLE)) | (Network::Get()->IsDhcpCapable() ? ArtNetStatus2::DHCP_CAPABLE : 0);

	FillPollReply();
#if defined ( ENABLE_SENDDIAG )
	FillDiagData();
#endif

	m_nHandle = Network::Get()->Begin(ArtNet::UDP_PORT);
	assert(m_nHandle != -1);

	m_State.status = ARTNET_ON;

	if (m_pArtNetDmx != nullptr) {
		for (uint32_t i = 0; i < ArtNet::MAX_PORTS; i++) {
			if (m_InputPorts[i].bIsEnabled) {
				if (m_InputPorts[i].nDestinationIp == 0) {
					m_InputPorts[i].nDestinationIp = m_Node.IPAddressBroadcast;
				}
				m_pArtNetDmx->Start(i);
			}
		}
	}

	LedBlink::Get()->SetMode(LEDBLINK_MODE_NORMAL);

	SendPollRelply(false);	// send a reply on startup
}

void ArtNetNode::Stop() {
	if (m_pArtNetDmx != nullptr) {
		for (uint32_t i = 0; i < ArtNet::MAX_PORTS; i++) {
			if (m_InputPorts[i].bIsEnabled) {
				m_pArtNetDmx->Stop(i);
			}
		}
	}

	if (m_pLightSet != nullptr) {
		for (uint32_t i = 0; i < ARTNET_NODE_MAX_PORTS_OUTPUT; i++) {
			if ((m_OutputPorts[i].tPortProtocol == PORT_ARTNET_ARTNET) && (m_IsLightSetRunning[i])) {
				m_pLightSet->Stop(i);
				m_IsLightSetRunning[i] = false;
			}
		}
	}

	LedBlink::Get()->SetMode(LEDBLINK_MODE_OFF_OFF);
	m_Node.Status1 = (m_Node.Status1 & ~STATUS1_INDICATOR_MASK) | STATUS1_INDICATOR_MUTE_MODE;

	m_State.status = ARTNET_OFF;
}

const uint8_t *ArtNetNode::GetSoftwareVersion() {
	return DEVICE_SOFTWARE_VERSION;
}

int ArtNetNode::SetUniverseSwitch(uint8_t nPortIndex, TArtNetPortDir dir, uint8_t nAddress) {
	assert(nPortIndex < (ArtNet::MAX_PORTS * m_nPages));
	assert(dir <= ARTNET_DISABLE_PORT);

	if (dir == ARTNET_DISABLE_PORT) {

		if (nPortIndex < ARTNET_NODE_MAX_PORTS_OUTPUT) {
			if (m_OutputPorts[nPortIndex].bIsEnabled) {
				m_OutputPorts[nPortIndex].bIsEnabled = false;
				m_State.nActiveOutputPorts = m_State.nActiveOutputPorts - 1;
			}
		}

		if (nPortIndex < ARTNET_NODE_MAX_PORTS_INPUT) {
			if (m_InputPorts[nPortIndex].bIsEnabled) {
				m_InputPorts[nPortIndex].bIsEnabled = false;
				m_State.nActiveInputPorts = m_State.nActiveInputPorts - 1;
			}
		}

		return ARTNET_EOK;
	}

	if ((dir == ARTNET_INPUT_PORT) && (nPortIndex < ARTNET_NODE_MAX_PORTS_INPUT)) {
		if (!m_InputPorts[nPortIndex].bIsEnabled) {
			m_State.nActiveInputPorts = m_State.nActiveInputPorts + 1;
			assert(m_State.nActiveInputPorts <= ArtNet::MAX_PORTS);
		}

		m_InputPorts[nPortIndex].bIsEnabled = true;
		m_InputPorts[nPortIndex].port.nDefaultAddress = nAddress & 0x0F;// Universe : Bits 3-0
		m_InputPorts[nPortIndex].port.nPortAddress = MakePortAddress(nAddress, (nPortIndex / ArtNet::MAX_PORTS));

		if (nPortIndex < ARTNET_NODE_MAX_PORTS_OUTPUT) {
			if (m_OutputPorts[nPortIndex].bIsEnabled) {
				m_OutputPorts[nPortIndex].bIsEnabled = false;
				m_State.nActiveOutputPorts = m_State.nActiveOutputPorts - 1;
			}
		}
	}

	if ((dir == ARTNET_OUTPUT_PORT) && (nPortIndex < ARTNET_NODE_MAX_PORTS_OUTPUT)) {
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

bool ArtNetNode::GetUniverseSwitch(uint8_t nPortIndex, uint8_t &nAddress, TArtNetPortDir dir) const {
	if (dir == ARTNET_INPUT_PORT) {
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

bool ArtNetNode::GetPortAddress(uint8_t nPortIndex, uint16_t &nAddress, TArtNetPortDir dir) const {
	if (dir == ARTNET_INPUT_PORT) {
		assert(nPortIndex < ARTNET_NODE_MAX_PORTS_INPUT);

		if (nPortIndex < ARTNET_NODE_MAX_PORTS_INPUT) {
			nAddress = m_InputPorts[nPortIndex].port.nPortAddress;
			return m_InputPorts[nPortIndex].bIsEnabled;
		}

		return false;
	}

	assert(nPortIndex < ARTNET_NODE_MAX_PORTS_OUTPUT);

	nAddress = m_OutputPorts[nPortIndex].port.nPortAddress;
	return m_OutputPorts[nPortIndex].bIsEnabled;
}

uint16_t ArtNetNode::MakePortAddress(uint16_t nCurrentAddress, uint8_t nPage) {
	// PortAddress Bit 15 = 0
	uint16_t newAddress = (m_Node.NetSwitch[nPage] & 0x7F) << 8;	// Net : Bits 14-8
	newAddress |= (m_Node.SubSwitch[nPage] & 0x0F) << 4;			// Sub-Net : Bits 7-4
	newAddress |= nCurrentAddress & 0x0F;							// Universe : Bits 3-0

	return newAddress;
}

void ArtNetNode::SetMergeMode(uint8_t nPortIndex, ArtNetMerge tMergeMode) {
	assert(nPortIndex < (ArtNet::MAX_PORTS * ArtNet::MAX_PAGES));

	m_OutputPorts[nPortIndex].mergeMode = tMergeMode;

	if (tMergeMode == ArtNetMerge::LTP) {
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

ArtNetMerge ArtNetNode::GetMergeMode(uint8_t nPortIndex) const {
	assert(nPortIndex < ARTNET_NODE_MAX_PORTS_OUTPUT);

	return m_OutputPorts[nPortIndex].mergeMode;
}

void ArtNetNode::SetPortProtocol(uint8_t nPortIndex, TPortProtocol tPortProtocol) {
	if (m_nVersion > 3) {
		assert(nPortIndex < ARTNET_NODE_MAX_PORTS_OUTPUT);

		m_OutputPorts[nPortIndex].tPortProtocol = tPortProtocol;

		if (tPortProtocol == PORT_ARTNET_SACN) {
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

TPortProtocol ArtNetNode::GetPortProtocol(uint8_t nPortIndex) const {
	assert(nPortIndex < (ARTNET_NODE_MAX_PORTS_OUTPUT));

	return m_OutputPorts[nPortIndex].tPortProtocol;
}

void ArtNetNode::SetShortName(const char *pShortName) {
	assert(pShortName != nullptr);

	strncpy(m_Node.ShortName, pShortName, ArtNet::SHORT_NAME_LENGTH - 1);
	m_Node.ShortName[ArtNet::SHORT_NAME_LENGTH - 1] = '\0';

	memcpy(m_PollReply.ShortName, m_Node.ShortName, ArtNet::SHORT_NAME_LENGTH);

	if (m_State.status == ARTNET_ON) {
		if (m_pArtNetStore != nullptr) {
			m_pArtNetStore->SaveShortName(m_Node.ShortName);
		}
		if (m_pArtNetDisplay != nullptr) {
			m_pArtNetDisplay->ShowShortName(m_Node.ShortName);
		}
	}
}

void ArtNetNode::SetLongName(const char *pLongName) {
	assert(pLongName != nullptr);

	strncpy(m_Node.LongName, pLongName, ArtNet::LONG_NAME_LENGTH - 1);
	m_Node.LongName[ArtNet::LONG_NAME_LENGTH - 1] = '\0';

	memcpy(m_PollReply.LongName, m_Node.LongName, ArtNet::LONG_NAME_LENGTH);

	if (m_State.status == ARTNET_ON) {
		if (m_pArtNetStore != nullptr) {
			m_pArtNetStore->SaveLongName(m_Node.LongName);
		}
		if (m_pArtNetDisplay != nullptr) {
			m_pArtNetDisplay->ShowLongName(m_Node.LongName);
		}
	}
}

void ArtNetNode::SetOemValue(const uint8_t *pOem) {
	assert(pOem != nullptr);

	m_Node.Oem[0] = pOem[0];
	m_Node.Oem[1] = pOem[1];
}

void ArtNetNode::FillPollReply() {
	memset(&m_PollReply, 0, sizeof(struct TArtPollReply));

	memcpy(m_PollReply.Id, artnet::NODE_ID, sizeof m_PollReply.Id);

	m_PollReply.OpCode = OP_POLLREPLY;

	ip.u32 = m_Node.IPAddressLocal;
	memcpy(m_PollReply.IPAddress, ip.u8, sizeof m_PollReply.IPAddress);

	m_PollReply.Port = ArtNet::UDP_PORT;

	m_PollReply.VersInfoH = DEVICE_SOFTWARE_VERSION[0];
	m_PollReply.VersInfoL = DEVICE_SOFTWARE_VERSION[1];

	m_PollReply.OemHi = m_Node.Oem[0];
	m_PollReply.Oem = m_Node.Oem[1];

	m_PollReply.Status1 = m_Node.Status1;

	m_PollReply.EstaMan[0] = ArtNetConst::ESTA_ID[1];
	m_PollReply.EstaMan[1] = ArtNetConst::ESTA_ID[0];

	memcpy(m_PollReply.ShortName, m_Node.ShortName, sizeof m_PollReply.ShortName);
	memcpy(m_PollReply.LongName, m_Node.LongName, sizeof m_PollReply.LongName);

	// Disable all input
	for (uint32_t i = 0; i < ArtNet::MAX_PORTS; i++) {
		m_PollReply.GoodInput[i] = PORT_IN_STATUS_DISABLED_MASK;
	}

	m_PollReply.Style = ARTNET_ST_NODE;

	memcpy(m_PollReply.MAC, m_Node.MACAddressLocal, sizeof m_PollReply.MAC);

	if (m_nVersion > 3) {
		memcpy(m_PollReply.BindIp, ip.u8, sizeof m_PollReply.BindIp);
	}

	m_PollReply.Status2 = m_Node.Status2;

	m_PollReply.NumPortsLo = 4; // Default
}

void ArtNetNode::SendPollRelply(bool bResponse) {
	if (!bResponse && m_State.status == ARTNET_ON) {
		m_State.ArtPollReplyCount++;
	}

	m_PollReply.Status1 = m_Node.Status1;

	for (uint32_t nPage = 0; nPage < m_nPages; nPage++) {

		m_PollReply.NetSwitch = m_Node.NetSwitch[nPage];
		m_PollReply.SubSwitch = m_Node.SubSwitch[nPage];

		m_PollReply.BindIndex = nPage + 1;

		const uint32_t nPortIndexStart = nPage * ArtNet::MAX_PORTS;

		uint32_t NumPortsLo = 0;

		for (uint32_t nPortIndex = nPortIndexStart; nPortIndex < (nPortIndexStart + ArtNet::MAX_PORTS); nPortIndex++) {
			uint8_t nStatus = m_OutputPorts[nPortIndex].port.nStatus;

			if (m_OutputPorts[nPortIndex].tPortProtocol == PORT_ARTNET_ARTNET) {
				nStatus &= (~GO_DATA_IS_BEING_TRANSMITTED);

				if (m_OutputPorts[nPortIndex].ipA != 0) {
					if ((m_nCurrentPacketMillis - m_OutputPorts[nPortIndex].nMillisA) < 1000) {
						nStatus |= GO_DATA_IS_BEING_TRANSMITTED;
					}
				}

				if (m_OutputPorts[nPortIndex].ipB != 0) {
					if ((m_nCurrentPacketMillis - m_OutputPorts[nPortIndex].nMillisB) < 1000) {
						nStatus |= GO_DATA_IS_BEING_TRANSMITTED;
					}
				}
			} else {
				if (m_pArtNet4Handler != nullptr) {
					const uint8_t nMask = GO_OUTPUT_IS_MERGING | GO_DATA_IS_BEING_TRANSMITTED | GO_OUTPUT_IS_SACN;

					nStatus &= (~nMask);
					nStatus |= (m_pArtNet4Handler->GetStatus(nPortIndex) & nMask);

					if ((nStatus & GO_OUTPUT_IS_SACN) == 0) {
						m_OutputPorts[nPortIndex].tPortProtocol = PORT_ARTNET_ARTNET;
					}
				}
			}

			m_OutputPorts[nPortIndex].port.nStatus = nStatus;

			if (m_OutputPorts[nPortIndex].bIsEnabled) {
				m_PollReply.PortTypes[nPortIndex - nPortIndexStart] = ARTNET_ENABLE_OUTPUT | ARTNET_PORT_DMX;
				NumPortsLo++;
			}

			m_PollReply.GoodOutput[nPortIndex - nPortIndexStart] = m_OutputPorts[nPortIndex].port.nStatus;
			m_PollReply.SwOut[nPortIndex - nPortIndexStart] = m_OutputPorts[nPortIndex].port.nDefaultAddress;

			if (nPortIndex < ArtNet::MAX_PORTS) {
				if (m_InputPorts[nPortIndex].bIsEnabled) {
					m_PollReply.PortTypes[nPortIndex - nPortIndexStart] |= ARTNET_ENABLE_INPUT | ARTNET_PORT_DMX;
					NumPortsLo++;
				}

				m_PollReply.GoodInput[nPortIndex - nPortIndexStart] = m_InputPorts[nPortIndex].port.nStatus;
				m_PollReply.SwIn[nPortIndex - nPortIndexStart] = m_InputPorts[nPortIndex].port.nDefaultAddress;
			}

		}

		m_PollReply.NumPortsLo = NumPortsLo;
		assert(NumPortsLo <= 4);

		snprintf(reinterpret_cast<char*>(m_PollReply.NodeReport), ArtNet::REPORT_LENGTH, "%04x [%04d] %s AvV", static_cast<int>(m_State.reportCode), static_cast<int>(m_State.ArtPollReplyCount), m_aSysName);

		Network::Get()->SendTo(m_nHandle, &m_PollReply, sizeof(struct TArtPollReply), m_Node.IPAddressBroadcast, ArtNet::UDP_PORT);
	}

	m_State.IsChanged = false;
}

bool ArtNetNode::IsDmxDataChanged(uint8_t nPortId, const uint8_t *pData, uint16_t nLength) {
	bool isChanged = false;

	const uint8_t *pSrc = pData;
	uint8_t *pDst = m_OutputPorts[nPortId].data;

	if (nLength != m_OutputPorts[nPortId].nLength) {
		m_OutputPorts[nPortId].nLength = nLength;

		for (uint32_t i = 0; i < nLength; i++) {
			*pDst++ = *pSrc++;
		}

		return true;
	}

	for (uint32_t i = 0; i < nLength; i++) {
		if (*pDst != *pSrc) {
			isChanged = true;
		}
		*pDst++ = *pSrc++;
	}

	return isChanged;
}

bool ArtNetNode::IsMergedDmxDataChanged(uint8_t nPortId, const uint8_t *pData, uint16_t nLength) {
	bool isChanged = false;

	if (!m_State.IsMergeMode) {
		m_State.IsMergeMode = true;
		m_State.IsChanged = true;
	}

	m_OutputPorts[nPortId].port.nStatus |= GO_OUTPUT_IS_MERGING;


	if (m_OutputPorts[nPortId].mergeMode == ArtNetMerge::HTP) {

		if (nLength != m_OutputPorts[nPortId].nLength) {
			m_OutputPorts[nPortId].nLength = nLength;
			for (uint32_t i = 0; i < nLength; i++) {
				uint8_t data = std::max(m_OutputPorts[nPortId].dataA[i], m_OutputPorts[nPortId].dataB[i]);
				m_OutputPorts[nPortId].data[i] = data;
			}
			return true;
		}

		for (uint32_t i = 0; i < nLength; i++) {
			uint8_t data = std::max(m_OutputPorts[nPortId].dataA[i], m_OutputPorts[nPortId].dataB[i]);
			if (data != m_OutputPorts[nPortId].data[i]) {
				m_OutputPorts[nPortId].data[i] = data;
				isChanged = true;
			}
		}

		return isChanged;
	} else {
		return IsDmxDataChanged(nPortId, pData, nLength);
	}
}

void ArtNetNode::CheckMergeTimeouts(uint8_t nPortId) {
	const uint32_t nTimeOutAMillis = m_nCurrentPacketMillis - m_OutputPorts[nPortId].nMillisA;

	if (nTimeOutAMillis > (ARTNET_MERGE_TIMEOUT_SECONDS * 1000)) {
		m_OutputPorts[nPortId].ipA = 0;
		m_OutputPorts[nPortId].port.nStatus &= (~GO_OUTPUT_IS_MERGING);
	}

	const uint32_t nTimeOutBMillis = m_nCurrentPacketMillis - m_OutputPorts[nPortId].nMillisB;

	if (nTimeOutBMillis > (ARTNET_MERGE_TIMEOUT_SECONDS * 1000)) {
		m_OutputPorts[nPortId].ipB = 0;
		m_OutputPorts[nPortId].port.nStatus &= (~GO_OUTPUT_IS_MERGING);
	}

	bool bIsMerging = false;

	for (uint32_t i = 0; i < (ArtNet::MAX_PORTS * m_nPages); i++) {
		bIsMerging |= ((m_OutputPorts[i].port.nStatus & GO_OUTPUT_IS_MERGING) != 0);
	}

	if (!bIsMerging) {
		m_State.IsChanged = true;
		m_State.IsMergeMode = false;
#if defined ( ENABLE_SENDDIAG )
		SendDiag("Leaving Merging Mode", ARTNET_DP_LOW);
#endif
	}
}

void ArtNetNode::HandlePoll() {
	const struct TArtPoll *pArtPoll = &(m_ArtNetPacket.ArtPacket.ArtPoll);

	if (pArtPoll->TalkToMe & ArtNetTalkToMe::SEND_ARTP_ON_CHANGE) {
		m_State.SendArtPollReplyOnChange = true;
	} else {
		m_State.SendArtPollReplyOnChange = false;
	}

	// If any controller requests diagnostics, the node will send diagnostics. (ArtPoll->TalkToMe->2).
	if (pArtPoll->TalkToMe & ArtNetTalkToMe::SEND_DIAG_MESSAGES) {
		m_State.SendArtDiagData = true;

		if (m_State.IPAddressArtPoll == 0) {
			m_State.IPAddressArtPoll = m_ArtNetPacket.IPAddressFrom;
		} else if (!m_State.IsMultipleControllersReqDiag && (m_State.IPAddressArtPoll != m_ArtNetPacket.IPAddressFrom)) {
			// If there are multiple controllers requesting diagnostics, diagnostics shall be broadcast.
			m_State.IPAddressDiagSend = m_Node.IPAddressBroadcast;
			m_State.IsMultipleControllersReqDiag = true;
		}

		if (m_State.IsMultipleControllersReqDiag ) {
			// The lowest minimum value of Priority shall be used. (Ignore ArtPoll->Priority).
			m_State.Priority = std::min(m_State.Priority , pArtPoll->Priority);
		} else {
			m_State.Priority = pArtPoll->Priority;
		}

		// If there are multiple controllers requesting diagnostics, diagnostics shall be broadcast. (Ignore ArtPoll->TalkToMe->3).
		if (!m_State.IsMultipleControllersReqDiag && (pArtPoll->TalkToMe & ArtNetTalkToMe::SEND_DIAG_UNICAST)) {
			m_State.IPAddressDiagSend = m_ArtNetPacket.IPAddressFrom;
		} else {
			m_State.IPAddressDiagSend = m_Node.IPAddressBroadcast;
		}
	} else {
		m_State.SendArtDiagData = false;
		m_State.IPAddressDiagSend = 0;
	}

	SendPollRelply(true);
}

void ArtNetNode::HandleDmx() {
	const struct TArtDmx *pArtDmx = &(m_ArtNetPacket.ArtPacket.ArtDmx);

	uint32_t data_length = (static_cast<uint32_t>(pArtDmx->LengthHi << 8) & 0xff00) | pArtDmx->Length;
	data_length = std::min(data_length, ArtNet::DMX_LENGTH);

	for (uint32_t i = 0; i < (ArtNet::MAX_PORTS * m_nPages); i++) {

		if (m_OutputPorts[i].bIsEnabled && (m_OutputPorts[i].tPortProtocol == PORT_ARTNET_ARTNET) && (pArtDmx->PortAddress == m_OutputPorts[i].port.nPortAddress)) {

			uint32_t ipA = m_OutputPorts[i].ipA;
			uint32_t ipB = m_OutputPorts[i].ipB;

			bool sendNewData = false;

			m_OutputPorts[i].port.nStatus = m_OutputPorts[i].port.nStatus | GO_DATA_IS_BEING_TRANSMITTED;

			if (m_State.IsMergeMode) {
				if (__builtin_expect((!m_State.bDisableMergeTimeout), 1)) {
					CheckMergeTimeouts(i);
				}
			}

			if (ipA == 0 && ipB == 0) {
#if defined ( ENABLE_SENDDIAG )
				SendDiag("1. first packet recv on this port", ARTNET_DP_LOW);
#endif
				m_OutputPorts[i].ipA = m_ArtNetPacket.IPAddressFrom;
				m_OutputPorts[i].nMillisA = m_nCurrentPacketMillis;
				memcpy(&m_OutputPorts[i].dataA, pArtDmx->Data, data_length);
				sendNewData = IsDmxDataChanged(i, pArtDmx->Data, data_length);
			} else if (ipA == m_ArtNetPacket.IPAddressFrom && ipB == 0) {
#if defined ( ENABLE_SENDDIAG )
				SendDiag("2. continued transmission from the same ip (source A)", ARTNET_DP_LOW);
#endif
				m_OutputPorts[i].nMillisA = m_nCurrentPacketMillis;
				memcpy(&m_OutputPorts[i].dataA, pArtDmx->Data, data_length);
				sendNewData = IsDmxDataChanged(i, pArtDmx->Data, data_length);
			} else if (ipA == 0 && ipB == m_ArtNetPacket.IPAddressFrom) {
#if defined ( ENABLE_SENDDIAG )
				SendDiag("3. continued transmission from the same ip (source B)", ARTNET_DP_LOW);
#endif
				m_OutputPorts[i].nMillisB = m_nCurrentPacketMillis;
				memcpy(&m_OutputPorts[i].dataB, pArtDmx->Data, data_length);
				sendNewData = IsDmxDataChanged(i, pArtDmx->Data, data_length);
			} else if (ipA != m_ArtNetPacket.IPAddressFrom && ipB == 0) {
#if defined ( ENABLE_SENDDIAG )
				SendDiag("4. new source, start the merge", ARTNET_DP_LOW);
#endif
				m_OutputPorts[i].ipB = m_ArtNetPacket.IPAddressFrom;
				m_OutputPorts[i].nMillisB = m_nCurrentPacketMillis;
				memcpy(&m_OutputPorts[i].dataB, pArtDmx->Data, data_length);
				sendNewData = IsMergedDmxDataChanged(i, m_OutputPorts[i].dataB, data_length);
			} else if (ipA == 0 && ipB != m_ArtNetPacket.IPAddressFrom) {
#if defined ( ENABLE_SENDDIAG )
				SendDiag("5. new source, start the merge", ARTNET_DP_LOW);
#endif
				m_OutputPorts[i].ipA = m_ArtNetPacket.IPAddressFrom;
				m_OutputPorts[i].nMillisA = m_nCurrentPacketMillis;
				memcpy(&m_OutputPorts[i].dataA, pArtDmx->Data, data_length);
				sendNewData = IsMergedDmxDataChanged(i, m_OutputPorts[i].dataA, data_length);
			} else if (ipA == m_ArtNetPacket.IPAddressFrom && ipB != m_ArtNetPacket.IPAddressFrom) {
#if defined ( ENABLE_SENDDIAG )
				SendDiag("6. continue merge", ARTNET_DP_LOW);
#endif
				m_OutputPorts[i].nMillisA = m_nCurrentPacketMillis;
				memcpy(&m_OutputPorts[i].dataA, pArtDmx->Data, data_length);
				sendNewData = IsMergedDmxDataChanged(i, m_OutputPorts[i].dataA, data_length);
			} else if (ipA != m_ArtNetPacket.IPAddressFrom && ipB == m_ArtNetPacket.IPAddressFrom) {
#if defined ( ENABLE_SENDDIAG )
				SendDiag("7. continue merge", ARTNET_DP_LOW);
#endif
				m_OutputPorts[i].nMillisB = m_nCurrentPacketMillis;
				memcpy(&m_OutputPorts[i].dataB, pArtDmx->Data, data_length);
				sendNewData = IsMergedDmxDataChanged(i, m_OutputPorts[i].dataB, data_length);
			} else if (ipA == m_ArtNetPacket.IPAddressFrom && ipB == m_ArtNetPacket.IPAddressFrom) {
#if defined ( ENABLE_SENDDIAG )
				SendDiag("8. Source matches both buffers, this shouldn't be happening!", ARTNET_DP_LOW);
#endif
				return;
			} else if (ipA != m_ArtNetPacket.IPAddressFrom && ipB != m_ArtNetPacket.IPAddressFrom) {
#if defined ( ENABLE_SENDDIAG )
				SendDiag("9. More than two sources, discarding data", ARTNET_DP_LOW);
#endif
				return;
			} else {
#if defined ( ENABLE_SENDDIAG )
				SendDiag("0. No cases matched, this shouldn't happen!", ARTNET_DP_LOW);
#endif
				return;
			}

			if (sendNewData || m_bDirectUpdate) {
				if (!m_State.IsSynchronousMode) {
#if defined ( ENABLE_SENDDIAG )
					SendDiag("Send new data", ARTNET_DP_LOW);
#endif
					m_pLightSet->SetData(i, m_OutputPorts[i].data, m_OutputPorts[i].nLength);

					if(!m_IsLightSetRunning[i]) {
						m_pLightSet->Start(i);
						m_State.IsChanged |= (!m_IsLightSetRunning[i]);
						m_IsLightSetRunning[i] = true;
					}
				} else {
#if defined ( ENABLE_SENDDIAG )
					SendDiag("DMX data pending", ARTNET_DP_LOW);
#endif
					m_OutputPorts[i].IsDataPending = sendNewData;
				}
			} else {
#if defined ( ENABLE_SENDDIAG )
				SendDiag("Data not changed", ARTNET_DP_LOW);
#endif
			}

			m_State.bIsReceivingDmx = true;
		}
	}
}

void ArtNetNode::HandleSync() {
	m_State.IsSynchronousMode = true;
	m_State.nArtSyncMillis = Hardware::Get()->Millis();

	for (uint32_t i = 0; i < (m_nPages * ArtNet::MAX_PORTS); i++) {
		if  ((m_OutputPorts[i].tPortProtocol == PORT_ARTNET_ARTNET) &&  ((m_OutputPorts[i].IsDataPending) || (m_OutputPorts[i].bIsEnabled && m_bDirectUpdate) )) {
#if defined ( ENABLE_SENDDIAG )
			SendDiag("Send pending data", ARTNET_DP_LOW);
#endif
			m_pLightSet->SetData(i, m_OutputPorts[i].data, 	m_OutputPorts[i].nLength);

			if(!m_IsLightSetRunning[i]) {
				m_pLightSet->Start(i);
				m_IsLightSetRunning[i] = true;
			}

			m_OutputPorts[i].IsDataPending = false;
		}
	}
}

void ArtNetNode::HandleAddress() {
	const struct TArtAddress *pArtAddress = &(m_ArtNetPacket.ArtPacket.ArtAddress);
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

	if (pArtAddress->SubSwitch == PROGRAM_DEFAULTS) {
		SetSubnetSwitch(NODE_DEFAULT_SUBNET_SWITCH);
	} else if (pArtAddress->SubSwitch & PROGRAM_CHANGE_MASK) {
		SetSubnetSwitch(pArtAddress->SubSwitch & ~PROGRAM_CHANGE_MASK);
	}

	if (pArtAddress->NetSwitch == PROGRAM_DEFAULTS) {
		SetNetSwitch(NODE_DEFAULT_NET_SWITCH);
	} else if (pArtAddress->NetSwitch & PROGRAM_CHANGE_MASK) {
		SetNetSwitch(pArtAddress->NetSwitch & ~PROGRAM_CHANGE_MASK);
	}

	for (uint32_t i = 0; i < ArtNet::MAX_PORTS; i++) {
		if (pArtAddress->SwOut[i] == PROGRAM_NO_CHANGE) {
		} else if (pArtAddress->SwOut[i] == PROGRAM_DEFAULTS) {
			SetUniverseSwitch(i, ARTNET_OUTPUT_PORT, NODE_DEFAULT_UNIVERSE);
		} else if (pArtAddress->SwOut[i] & PROGRAM_CHANGE_MASK) {
			SetUniverseSwitch(i, ARTNET_OUTPUT_PORT, pArtAddress->SwOut[i] & ~PROGRAM_CHANGE_MASK);
		}

		if (pArtAddress->SwIn[i] == PROGRAM_NO_CHANGE) {
		} else if (pArtAddress->SwIn[i] == PROGRAM_DEFAULTS) {
			SetUniverseSwitch(i, ARTNET_INPUT_PORT, NODE_DEFAULT_UNIVERSE);
		} else if (pArtAddress->SwIn[i] & PROGRAM_CHANGE_MASK) {
			SetUniverseSwitch(i, ARTNET_INPUT_PORT, pArtAddress->SwIn[i] & ~PROGRAM_CHANGE_MASK);
		}
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
		LedBlink::Get()->SetMode(LEDBLINK_MODE_NORMAL);
		m_Node.Status1 = (m_Node.Status1 & ~STATUS1_INDICATOR_MASK) | STATUS1_INDICATOR_NORMAL_MODE;
		break;
	case ARTNET_PC_LED_MUTE:
		LedBlink::Get()->SetMode(LEDBLINK_MODE_OFF_OFF);
		m_Node.Status1 = (m_Node.Status1 & ~STATUS1_INDICATOR_MASK) | STATUS1_INDICATOR_MUTE_MODE;
		break;
	case ARTNET_PC_LED_LOCATE:
		LedBlink::Get()->SetMode(LEDBLINK_MODE_FAST);
		m_Node.Status1 = (m_Node.Status1 & ~STATUS1_INDICATOR_MASK) | STATUS1_INDICATOR_LOCATE_MODE;
		break;

	case ARTNET_PC_MERGE_LTP_O:
	case ARTNET_PC_MERGE_LTP_1:
	case ARTNET_PC_MERGE_LTP_2:
	case ARTNET_PC_MERGE_LTP_3:
		SetMergeMode(pArtAddress->Command & 0x3, ArtNetMerge::LTP);
		break;

	case ARTNET_PC_MERGE_HTP_0:
	case ARTNET_PC_MERGE_HTP_1:
	case ARTNET_PC_MERGE_HTP_2:
	case ARTNET_PC_MERGE_HTP_3:
		SetMergeMode(pArtAddress->Command & 0x3, ArtNetMerge::HTP);
		break;

	case ARTNET_PC_ARTNET_SEL0:
	case ARTNET_PC_ARTNET_SEL1:
	case ARTNET_PC_ARTNET_SEL2:
	case ARTNET_PC_ARTNET_SEL3:
		SetPortProtocol(pArtAddress->Command & 0x3, PORT_ARTNET_ARTNET);
		break;

	case ARTNET_PC_ACN_SEL0:
	case ARTNET_PC_ACN_SEL1:
	case ARTNET_PC_ACN_SEL2:
	case ARTNET_PC_ACN_SEL3:
		SetPortProtocol(pArtAddress->Command & 0x3, PORT_ARTNET_SACN);
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
		if (m_OutputPorts[nPort].tPortProtocol == PORT_ARTNET_ARTNET) {
			m_pLightSet->SetData(nPort, m_OutputPorts[nPort].data, m_OutputPorts[nPort].nLength);
		}
		break;

	default:
		break;
	}

	if ((nPort < ArtNet::MAX_PORTS) && (m_OutputPorts[nPort].tPortProtocol == PORT_ARTNET_ARTNET) && !m_IsLightSetRunning[nPort]) {
		m_pLightSet->Start(nPort);
		m_IsLightSetRunning[nPort] = true;
		m_OutputPorts[nPort].port.nStatus |= GO_DATA_IS_BEING_TRANSMITTED;
	}

	if (m_pArtNet4Handler != nullptr) {
		m_pArtNet4Handler->HandleAddress(pArtAddress->Command);
	}

	SendPollRelply(true);
}

void ArtNetNode::SetNetworkDataLossCondition() {
	m_State.IsMergeMode = false;
	m_State.IsSynchronousMode = false;

	for (uint32_t i = 0; i < (ArtNet::MAX_PORTS * m_nPages); i++) {
		if  ((m_OutputPorts[i].tPortProtocol == PORT_ARTNET_ARTNET) && (m_IsLightSetRunning[i])) {
			m_pLightSet->Stop(i);
			m_IsLightSetRunning[i] = false;
		}

		m_OutputPorts[i].port.nStatus &= (~GO_DATA_IS_BEING_TRANSMITTED);
		m_OutputPorts[i].nLength = 0;
		m_OutputPorts[i].ipA = 0;
		m_OutputPorts[i].ipB = 0;
	}
}

void ArtNetNode::GetType() {
	char *data = reinterpret_cast<char*>(&(m_ArtNetPacket.ArtPacket));

	if (m_ArtNetPacket.length < ARTNET_MIN_HEADER_SIZE) {
		m_ArtNetPacket.OpCode = OP_NOT_DEFINED;
		return;
	}

	if ((data[10] != 0) || (data[11] != ArtNet::PROTOCOL_REVISION)) {
		m_ArtNetPacket.OpCode = OP_NOT_DEFINED;
		return;
	}

	if (memcmp(data, "Art-Net\0", 8) == 0) {
		m_ArtNetPacket.OpCode = static_cast<TOpCodes>(((data[9] << 8)) + data[8]);
	} else {
		m_ArtNetPacket.OpCode = OP_NOT_DEFINED;
	}
}

void ArtNetNode::Run() {
	uint16_t nForeignPort;

	const int nBytesReceived = Network::Get()->RecvFrom(m_nHandle, &(m_ArtNetPacket.ArtPacket), sizeof(m_ArtNetPacket.ArtPacket), &m_ArtNetPacket.IPAddressFrom, &nForeignPort);

	m_nCurrentPacketMillis = Hardware::Get()->Millis();

	if (__builtin_expect((nBytesReceived == 0), 1)) {
		if ((m_State.nNetworkDataLossTimeoutMillis != 0) && ((m_nCurrentPacketMillis - m_nPreviousPacketMillis) >= m_State.nNetworkDataLossTimeoutMillis)) {
			SetNetworkDataLossCondition();
		}

		if (m_State.SendArtPollReplyOnChange) {
			bool doSend = m_State.IsChanged;
			if (m_pArtNet4Handler != nullptr) {
				doSend |= m_pArtNet4Handler->IsStatusChanged();
			}
			if (doSend) {
				SendPollRelply(false);
			}
		}

		if ((m_nCurrentPacketMillis - m_nPreviousPacketMillis) >= (1 * 1000)) {
			if (((m_Node.Status1 & STATUS1_INDICATOR_MASK) == STATUS1_INDICATOR_NORMAL_MODE)) {
				LedBlink::Get()->SetMode(LEDBLINK_MODE_NORMAL);
				m_State.bIsReceivingDmx = false;
			}
		}

		if (m_pArtNetDmx != nullptr) {
			HandleDmxIn();

			if (((m_Node.Status1 & STATUS1_INDICATOR_MASK) == STATUS1_INDICATOR_NORMAL_MODE)) {
				if (m_State.bIsReceivingDmx) {
					LedBlink::Get()->SetMode(LEDBLINK_MODE_DATA);
				} else {
					LedBlink::Get()->SetMode(LEDBLINK_MODE_NORMAL);
				}
			}
		}

		return;
	}

	m_ArtNetPacket.length = nBytesReceived;
	m_nPreviousPacketMillis = m_nCurrentPacketMillis;

	GetType();

	if (m_State.IsSynchronousMode) {
		if (m_nCurrentPacketMillis - m_State.nArtSyncMillis >= (4 * 1000)) {
			m_State.IsSynchronousMode = false;
		}
	}

	switch (m_ArtNetPacket.OpCode) {
	case OP_POLL:
		HandlePoll();
		break;
	case OP_DMX:
		if (m_pLightSet != nullptr) {
			HandleDmx();
		}
		break;
	case OP_SYNC:
		if (m_pLightSet != nullptr) {
			HandleSync();
		}
		break;
	case OP_ADDRESS:
		HandleAddress();
		break;
	case OP_TIMECODE:
		if (m_pArtNetTimeCode != nullptr) {
			HandleTimeCode();
		}
		break;
	case OP_TIMESYNC:
		if (m_pArtNetTimeSync != nullptr) {
			HandleTimeSync();
		}
		break;
	case OP_TODREQUEST:
		if (m_pArtNetRdm != nullptr) {
			HandleTodRequest();
		}
		break;
	case OP_TODCONTROL:
		if (m_pArtNetRdm != nullptr) {
			HandleTodControl();
		}
		break;
	case OP_RDM:
		if (m_pArtNetRdm != nullptr) {
			HandleRdm();
		}
		break;
	case OP_IPPROG:
		if (m_pArtNetIpProg != nullptr) {
			HandleIpProg();
		}
		break;
	case OP_TRIGGER:
		if (m_pArtNetTrigger != nullptr) {
			HandleTrigger();
		}
		break;
	default:
		// ArtNet but OpCode is not implemented
		// Just skip ... no error
		break;
	}

	if (m_pArtNetDmx != nullptr) {
		HandleDmxIn();
	}

	if (((m_Node.Status1 & STATUS1_INDICATOR_MASK) == STATUS1_INDICATOR_NORMAL_MODE)) {
		if (m_State.bIsReceivingDmx) {
			LedBlink::Get()->SetMode(LEDBLINK_MODE_DATA);
		} else {
			LedBlink::Get()->SetMode(LEDBLINK_MODE_NORMAL);
		}
	}

}
