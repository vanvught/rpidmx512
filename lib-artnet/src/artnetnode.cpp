/**
 * @file artnetnode.cpp
 *
 */
/**
 * Art-Net Designed by and Copyright Artistic Licence Holdings Ltd.
 */
/* Copyright (C) 2016-2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#include "artnetnode.h"
#include "packets.h"

#include "lightset.h"
#include "ledblink.h"

#include "artnetrdm.h"
#include "artnettimecode.h"
#include "artnettimesync.h"
#include "artnetipprog.h"
#include "artnetstore.h"

#include "hardware.h"
#include "network.h"

#include "artnetnode_internal.h"

union uip {
	uint32_t u32;
	uint8_t u8[4];
} static ip;

#ifndef MIN
 #define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifndef MAX
 #define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

#define NODE_DEFAULT_SHORT_NAME		"AvV Art-Net Node"
#define NODE_DEFAULT_NET_SWITCH		0
#define NODE_DEFAULT_SUBNET_SWITCH	0
#define NODE_DEFAULT_UNIVERSE		0

static const uint8_t DEVICE_MANUFACTURER_ID[] = { 0x7F, 0xF0 };
static const uint8_t DEVICE_SOFTWARE_VERSION[] = { 1, 35 };
static const uint8_t DEVICE_OEM_VALUE[] = { 0x20, 0xE0 };

#define ARTNET_MIN_HEADER_SIZE			12
#define ARTNET_MERGE_TIMEOUT_SECONDS	10

#define NETWORK_DATA_LOSS_TIMEOUT		10	///< Seconds

#define PORT_IN_STATUS_DISABLED_MASK	0x08

ArtNetNode::ArtNetNode(uint8_t nVersion, uint8_t nPages) :
	m_nVersion(nVersion),
	m_nPages(nPages <= ARTNET_MAX_PAGES ? nPages : ARTNET_MAX_PAGES),
	m_nHandle(-1),
	m_pLightSet(0),
	m_pArtNetTimeCode(0),
	m_pArtNetTimeSync(0),
	m_pArtNetRdm(0),
	m_pArtNetIpProg(0),
	m_pArtNetStore(0),
	m_pArtNet4Handler(0),
	m_pTimeCodeData(0),
	m_pTodData(0),
	m_pIpProgReply(0),
	m_bDirectUpdate(false),
	m_nCurrentPacketTime(0),
	m_nPreviousPacketTime(0),
	m_IsRdmResponder(false)
{
	assert(Hardware::Get() != 0);
	assert(Network::Get() != 0);

	memset(&m_Node, 0, sizeof (struct TArtNetNode));
	m_Node.Status1 = STATUS1_INDICATOR_NORMAL_MODE | STATUS1_PAP_FRONT_PANEL;
	m_Node.Status2 = STATUS2_PORT_ADDRESS_15BIT | (m_nVersion > 3 ? STATUS2_SACN_ABLE_TO_SWITCH : STATUS2_SACN_NO_SWITCH);

	memset(&m_State, 0, sizeof (struct TArtNetNodeState));
	m_State.reportCode = ARTNET_RCPOWEROK;
	m_State.status = ARTNET_STANDBY;
	m_State.nNetworkDataLossTimeout = NETWORK_DATA_LOSS_TIMEOUT;

	for (uint32_t i = 0; i < (ARTNET_MAX_PORTS * ARTNET_MAX_PAGES); i++) {
		m_IsLightSetRunning[i] = false;
		memset(&m_OutputPorts[i], 0 , sizeof(struct TOutputPort));
	}

	SetShortName((const char *) NODE_DEFAULT_SHORT_NAME);

	uint8_t nBoardNameLength;
	const char *pBoardName = Hardware::Get()->GetBoardName(nBoardNameLength);
	const char *pWebsiteUrl = Hardware::Get()->GetWebsiteUrl();
	snprintf((char *)m_aDefaultNodeLongName, ARTNET_LONG_NAME_LENGTH, "%s %s %d %s", pBoardName, NODE_ID, m_nVersion, pWebsiteUrl);
	SetLongName((const char *) m_aDefaultNodeLongName);

	SetManufacturerId(DEVICE_MANUFACTURER_ID);
	SetOemValue(DEVICE_OEM_VALUE);

	uint8_t nSysNameLenght;
	const char *pSysName = Hardware::Get()->GetSysName(nSysNameLenght);
	strncpy(m_aSysName, pSysName, sizeof m_aSysName);
	m_aSysName[(sizeof m_aSysName) - 1] = '\0';
}

ArtNetNode::~ArtNetNode(void) {
	Stop();

	if (m_pTodData != 0) {
		delete m_pTodData;
	}

	if (m_pIpProgReply != 0) {
		delete m_pIpProgReply;
	}

	if (m_pTimeCodeData != 0) {
		delete m_pTimeCodeData;
	}
}

void ArtNetNode::SetArtNetStore(ArtNetStore *pArtNetStore) {
	assert(pArtNetStore != 0);

	m_pArtNetStore = pArtNetStore;
}

void ArtNetNode::Start(void) {
	assert(Network::Get() != 0);
	assert(LedBlink::Get() != 0);

	m_Node.IPAddressLocal = Network::Get()->GetIp();
	m_Node.IPAddressBroadcast = m_Node.IPAddressLocal | ~(Network::Get()->GetNetmask());
	Network::Get()->MacAddressCopyTo(m_Node.MACAddressLocal);

	m_Node.Status2 = (m_Node.Status2 & ~(STATUS2_IP_DHCP)) | (Network::Get()->IsDhcpUsed() ? STATUS2_IP_DHCP : STATUS2_IP_MANUALY);
	m_Node.Status2 = (m_Node.Status2 & ~(STATUS2_DHCP_CAPABLE)) | (Network::Get()->IsDhcpCapable() ? STATUS2_DHCP_CAPABLE : 0);

	FillPollReply();
#if defined ( ENABLE_SENDDIAG )
	FillDiagData();
#endif

	m_nHandle = Network::Get()->Begin(ARTNET_UDP_PORT);
	assert(m_nHandle != -1);

	m_State.status = ARTNET_ON;

	LedBlink::Get()->SetMode(LEDBLINK_MODE_NORMAL);

	SendPollRelply(false);	// send a reply on startup
}

void ArtNetNode::Stop(void) {
	if (m_pLightSet != 0) {
		for (uint32_t i = 0; i < (ARTNET_MAX_PORTS * ARTNET_MAX_PAGES); i++) {
			if ((m_OutputPorts[i].tPortProtocol == PORT_ARTNET_ARTNET) && (m_IsLightSetRunning[i])) {
				m_pLightSet->Stop(i);
				m_IsLightSetRunning[i] = false;
			}
		}
	}

	LedBlink::Get()->SetMode(LEDBLINK_MODE_OFF);
	m_Node.Status1 = (m_Node.Status1 & ~STATUS1_INDICATOR_MASK) | STATUS1_INDICATOR_MUTE_MODE;

	m_State.status = ARTNET_OFF;
}

void ArtNetNode::SetOutput(LightSet *pLightSet) {
	assert(pLightSet != 0);

	m_pLightSet = pLightSet;
}

const uint8_t *ArtNetNode::GetSoftwareVersion(void) {
	return DEVICE_SOFTWARE_VERSION;
}

int ArtNetNode::SetUniverseSwitch(uint8_t nPortIndex, TArtNetPortDir dir, uint8_t nAddress) {
	assert(nPortIndex < (ARTNET_MAX_PORTS * m_nPages));
	assert(dir <= ARTNET_DISABLE_PORT);

	if (dir == ARTNET_INPUT_PORT) {
		// Not supported. We have output ports only.
		return ARTNET_EACTION;
	}

	if (dir == ARTNET_DISABLE_PORT) {
		if (m_OutputPorts[nPortIndex].bIsEnabled) {
			m_OutputPorts[nPortIndex].bIsEnabled = false;
			m_State.nActivePorts = m_State.nActivePorts - 1;
		}
		return ARTNET_EOK;
	}

	if (dir == ARTNET_OUTPUT_PORT) {
		if (!m_OutputPorts[nPortIndex].bIsEnabled) {
			m_State.nActivePorts = m_State.nActivePorts + 1;
			assert(m_State.nActivePorts <= (ARTNET_MAX_PORTS * m_nPages));
		}
		m_OutputPorts[nPortIndex].bIsEnabled = true;
	} else {
		return ARTNET_EARG;
	}

	m_OutputPorts[nPortIndex].port.nDefaultAddress = nAddress & (uint16_t) 0x0F;// Universe : Bits 3-0
	m_OutputPorts[nPortIndex].port.nPortAddress = MakePortAddress((uint16_t) nAddress, (nPortIndex / ARTNET_MAX_PORTS));

	if ((m_pArtNet4Handler != 0) && (m_State.status != ARTNET_ON)) {
		m_pArtNet4Handler->SetPort(nPortIndex);
	}

	if ((m_pArtNetStore != 0) && (m_State.status == ARTNET_ON)) {
		if (nPortIndex < ARTNET_MAX_PORTS) {
			m_pArtNetStore->SaveUniverseSwitch(nPortIndex, nAddress);
		}
	}

	return ARTNET_EOK;
}

bool ArtNetNode::GetUniverseSwitch(uint8_t nPortIndex, uint8_t &nAddress) const {
	assert(nPortIndex < ARTNET_MAX_PORTS * ARTNET_MAX_PAGES);

	nAddress = m_OutputPorts[nPortIndex].port.nDefaultAddress;

	return m_OutputPorts[nPortIndex].bIsEnabled;
}

void ArtNetNode::SetSubnetSwitch(uint8_t nAddress, uint8_t nPage) {
	assert(nPage < ARTNET_MAX_PAGES);

	m_Node.SubSwitch[nPage] = nAddress;

	const uint32_t nPortIndexStart = nPage * ARTNET_MAX_PORTS;

	for (uint32_t i = nPortIndexStart; i < (nPortIndexStart + ARTNET_MAX_PORTS); i++) {
		m_OutputPorts[i].port.nPortAddress = MakePortAddress(m_OutputPorts[i].port.nPortAddress, (i / ARTNET_MAX_PORTS));
	}

	if ((m_pArtNetStore != 0) && (m_State.status == ARTNET_ON)) {
		if (nPage == 0) {
			m_pArtNetStore->SaveSubnetSwitch(nAddress);
		}
	}
}

uint8_t ArtNetNode::GetSubnetSwitch(uint8_t nPage) const {
	assert(nPage < ARTNET_MAX_PAGES);

	return m_Node.SubSwitch[nPage];
}

void ArtNetNode::SetNetSwitch(uint8_t nAddress, uint8_t nPage) {
	assert(nPage < ARTNET_MAX_PAGES);

	m_Node.NetSwitch[nPage] = nAddress;

	const uint32_t nPortIndexStart = nPage * ARTNET_MAX_PORTS;

	for (uint32_t i = nPortIndexStart; i < (nPortIndexStart + ARTNET_MAX_PORTS); i++) {
		m_OutputPorts[i].port.nPortAddress = MakePortAddress(m_OutputPorts[i].port.nPortAddress, (i / ARTNET_MAX_PORTS));
	}

	if ((m_pArtNetStore != 0) && (m_State.status == ARTNET_ON)) {
		if (nPage == 0) {
			m_pArtNetStore->SaveNetSwitch(nAddress);
		}
	}
}

uint8_t ArtNetNode::GetNetSwitch(uint8_t nPage) const {
	assert(nPage < ARTNET_MAX_PAGES);

	return m_Node.NetSwitch[nPage];
}

bool ArtNetNode::GetPortAddress(uint8_t nPortIndex, uint16_t &nAddress) const {
	assert(nPortIndex < (ARTNET_MAX_PORTS * ARTNET_MAX_PAGES));

	nAddress = m_OutputPorts[nPortIndex].port.nPortAddress;

	return m_OutputPorts[nPortIndex].bIsEnabled;
}

uint16_t ArtNetNode::MakePortAddress(uint16_t nCurrentAddress, uint8_t nPage) {
	// PortAddress Bit 15 = 0
	uint16_t newAddress = (m_Node.NetSwitch[nPage] & 0x7F) << 8;	// Net : Bits 14-8
	newAddress |= (m_Node.SubSwitch[nPage] & (uint8_t) 0x0F) << 4;	// Sub-Net : Bits 7-4
	newAddress |= nCurrentAddress & (uint16_t) 0x0F;				// Universe : Bits 3-0

	return newAddress;
}

void ArtNetNode::SetMergeMode(uint8_t nPortIndex, TMerge tMergeMode) {
	assert(nPortIndex < (ARTNET_MAX_PORTS * ARTNET_MAX_PAGES));

	m_OutputPorts[nPortIndex].mergeMode = tMergeMode;

	if (tMergeMode == ARTNET_MERGE_LTP) {
		m_OutputPorts[nPortIndex].port.nStatus |= GO_MERGE_MODE_LTP;
	} else {
		m_OutputPorts[nPortIndex].port.nStatus &= (~GO_MERGE_MODE_LTP);
	}

	if ((m_pArtNetStore != 0) && (m_State.status == ARTNET_ON)) {
		if (nPortIndex < ARTNET_MAX_PORTS) {
			m_pArtNetStore->SaveMergeMode(nPortIndex, tMergeMode);
		}
	}
}

TMerge ArtNetNode::GetMergeMode(uint8_t nPortIndex) const {
	assert(nPortIndex < (ARTNET_MAX_PORTS * ARTNET_MAX_PAGES));

	return m_OutputPorts[nPortIndex].mergeMode;
}

void ArtNetNode::SetPortProtocol(uint8_t nPortIndex, TPortProtocol tPortProtocol) {
	if (m_nVersion > 3) {
		assert(nPortIndex < (ARTNET_MAX_PORTS * ARTNET_MAX_PAGES));

		m_OutputPorts[nPortIndex].tPortProtocol = tPortProtocol;

		if (tPortProtocol == PORT_ARTNET_SACN) {
			m_OutputPorts[nPortIndex].port.nStatus |= GO_OUTPUT_IS_SACN;
		} else {
			m_OutputPorts[nPortIndex].port.nStatus &= (~GO_OUTPUT_IS_SACN);
		}

		if ((m_pArtNetStore != 0) && (m_State.status == ARTNET_ON)) {
			if (nPortIndex < ARTNET_MAX_PORTS) {
				m_pArtNetStore->SavePortProtocol(nPortIndex, tPortProtocol);
			}
		}
	}
}

TPortProtocol ArtNetNode::GetPortProtocol(uint8_t nPortIndex) const {
	assert(nPortIndex < (ARTNET_MAX_PORTS * ARTNET_MAX_PAGES));

	return m_OutputPorts[nPortIndex].tPortProtocol;
}

void ArtNetNode::SetShortName(const char *pName) {
	assert(pName != 0);

	strncpy((char *) m_Node.ShortName, pName, ARTNET_SHORT_NAME_LENGTH);
	m_Node.ShortName[ARTNET_SHORT_NAME_LENGTH - 1] = '\0';

	memcpy(m_PollReply.ShortName, m_Node.ShortName, ARTNET_SHORT_NAME_LENGTH);

	if ((m_pArtNetStore != 0) && (m_State.status == ARTNET_ON)) {
		m_pArtNetStore->SaveShortName((const char *) m_Node.ShortName);
	}
}

void ArtNetNode::SetLongName(const char *pName) {
	assert(pName != 0);

	strncpy((char *) m_Node.LongName, pName, ARTNET_LONG_NAME_LENGTH);
	m_Node.LongName[ARTNET_LONG_NAME_LENGTH - 1] = '\0';

	memcpy(m_PollReply.LongName, m_Node.LongName, ARTNET_LONG_NAME_LENGTH);

	if ((m_pArtNetStore != 0) && (m_State.status == ARTNET_ON)) {
		m_pArtNetStore->SaveLongName((const char *) m_Node.LongName);
	}
}

void ArtNetNode::SetManufacturerId(const uint8_t *pEsta) {
	assert(pEsta != 0);

	m_Node.Esta[0] = pEsta[1];
	m_Node.Esta[1] = pEsta[0];
}

void ArtNetNode::SetOemValue(const uint8_t *pOem) {
	assert(pOem != 0);

	m_Node.Oem[0] = pOem[0];
	m_Node.Oem[1] = pOem[1];
}

void ArtNetNode::SetNetworkTimeout(time_t nNetworkDataLossTimeout) {
	m_State.nNetworkDataLossTimeout = nNetworkDataLossTimeout;
}

void ArtNetNode::SetDisableMergeTimeout(bool bDisable) {
	m_State.bDisableMergeTimeout = bDisable;
}

void ArtNetNode::FillPollReply(void) {
	memset(&m_PollReply, 0, sizeof(struct TArtPollReply));

	memcpy(m_PollReply.Id, (const char *) NODE_ID, sizeof m_PollReply.Id);

	m_PollReply.OpCode = OP_POLLREPLY;

	ip.u32 = m_Node.IPAddressLocal;
	memcpy(m_PollReply.IPAddress, ip.u8, sizeof m_PollReply.IPAddress);

	m_PollReply.Port = ARTNET_UDP_PORT;

	m_PollReply.VersInfoH = DEVICE_SOFTWARE_VERSION[0];
	m_PollReply.VersInfoL = DEVICE_SOFTWARE_VERSION[1];

	m_PollReply.OemHi = m_Node.Oem[0];
	m_PollReply.Oem = m_Node.Oem[1];

	m_PollReply.Status1 = m_Node.Status1;

	m_PollReply.EstaMan[0] = m_Node.Esta[0];
	m_PollReply.EstaMan[1] = m_Node.Esta[1];

	memcpy(m_PollReply.ShortName, m_Node.ShortName, sizeof m_PollReply.ShortName);
	memcpy(m_PollReply.LongName, m_Node.LongName, sizeof m_PollReply.LongName);

	// Disable all input
	for (uint32_t i = 0; i < ARTNET_MAX_PORTS; i++) {
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

		const uint32_t nPortIndexStart = nPage * ARTNET_MAX_PORTS;

		uint8_t NumPortsLo = 0;

		for (uint32_t nPortIndex = nPortIndexStart; nPortIndex < (nPortIndexStart + ARTNET_MAX_PORTS); nPortIndex++) {

			if (m_OutputPorts[nPortIndex].tPortProtocol == PORT_ARTNET_SACN) {

				if (m_pArtNet4Handler != 0) {
					const uint8_t nMask = GO_OUTPUT_IS_MERGING | GO_DATA_IS_BEING_TRANSMITTED | GO_OUTPUT_IS_SACN;
					uint8_t nStatus = m_OutputPorts[nPortIndex].port.nStatus;

					nStatus &= (~nMask);
					nStatus |= (m_pArtNet4Handler->GetStatus(nPortIndex) & nMask);

					if ((nStatus & GO_OUTPUT_IS_SACN) == 0) {
						m_OutputPorts[nPortIndex].tPortProtocol = PORT_ARTNET_ARTNET;
					}

					m_OutputPorts[nPortIndex].port.nStatus = nStatus;
				}
			}

			if (m_OutputPorts[nPortIndex].bIsEnabled) {
				m_PollReply.PortTypes[nPortIndex - nPortIndexStart] = ARTNET_ENABLE_OUTPUT | ARTNET_PORT_DMX;
				NumPortsLo++;
			}

			m_PollReply.GoodOutput[nPortIndex - nPortIndexStart] = m_OutputPorts[nPortIndex].port.nStatus;
			m_PollReply.SwOut[nPortIndex - nPortIndexStart] = m_OutputPorts[nPortIndex].port.nDefaultAddress;
		}

		m_PollReply.NumPortsLo = NumPortsLo;

		snprintf((char *) m_PollReply.NodeReport, ARTNET_REPORT_LENGTH, "%04x [%04d] %s AvV", (int) m_State.reportCode, (int) m_State.ArtPollReplyCount, m_aSysName);

		Network::Get()->SendTo(m_nHandle, (const uint8_t *) &(m_PollReply), (uint16_t) sizeof(struct TArtPollReply), m_Node.IPAddressBroadcast, (uint16_t) ARTNET_UDP_PORT);
	}

	m_State.IsChanged = false;
}

bool ArtNetNode::IsDmxDataChanged(uint8_t nPortId, const uint8_t *pData, uint16_t nLength) {
	bool isChanged = false;

	const uint8_t *src = (uint8_t *) pData;
	uint8_t *dst = m_OutputPorts[nPortId].data;

	if (nLength != m_OutputPorts[nPortId].nLength) {
		m_OutputPorts[nPortId].nLength = nLength;

		for (uint32_t i = 0; i < nLength; i++) {
			*dst++ = *src++;
		}

		return true;
	}

	for (uint32_t i = 0; i < nLength; i++) {
		if (*dst != *src) {
			isChanged = true;
		}
		*dst++ = *src++;
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


	if (m_OutputPorts[nPortId].mergeMode == ARTNET_MERGE_HTP) {

		if (nLength != m_OutputPorts[nPortId].nLength) {
			m_OutputPorts[nPortId].nLength = nLength;
			for (uint32_t i = 0; i < nLength; i++) {
				uint8_t data = MAX(m_OutputPorts[nPortId].dataA[i], m_OutputPorts[nPortId].dataB[i]);
				m_OutputPorts[nPortId].data[i] = data;
			}
			return true;
		}

		for (uint32_t i = 0; i < nLength; i++) {
			uint8_t data = MAX(m_OutputPorts[nPortId].dataA[i], m_OutputPorts[nPortId].dataB[i]);
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
	const time_t timeOutA = m_nCurrentPacketTime - m_OutputPorts[nPortId].timeA;
	const time_t timeOutB = m_nCurrentPacketTime - m_OutputPorts[nPortId].timeB;

	if (timeOutA > (time_t) ARTNET_MERGE_TIMEOUT_SECONDS) {
		m_OutputPorts[nPortId].ipA = 0;
		m_OutputPorts[nPortId].port.nStatus &= (~GO_OUTPUT_IS_MERGING);
	}

	if (timeOutB > (time_t) ARTNET_MERGE_TIMEOUT_SECONDS) {
		m_OutputPorts[nPortId].ipB = 0;
		m_OutputPorts[nPortId].port.nStatus &= (~GO_OUTPUT_IS_MERGING);
	}

	bool bIsMerging = false;

	for (uint32_t i = 0; i < (ARTNET_MAX_PORTS * m_nPages); i++) {
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

void ArtNetNode::HandlePoll(void) {
	const struct TArtPoll *packet = (struct TArtPoll *)&(m_ArtNetPacket.ArtPacket.ArtPoll);

	if (packet->TalkToMe & TTM_SEND_ARTP_ON_CHANGE) {
		m_State.SendArtPollReplyOnChange = true;
	} else {
		m_State.SendArtPollReplyOnChange = false;
	}

	// If any controller requests diagnostics, the node will send diagnostics. (ArtPoll->TalkToMe->2).
	if (packet->TalkToMe & TTM_SEND_DIAG_MESSAGES) {
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
			m_State.Priority = MIN(m_State.Priority , packet->Priority);
		} else {
			m_State.Priority = packet->Priority;
		}

		// If there are multiple controllers requesting diagnostics, diagnostics shall be broadcast. (Ignore ArtPoll->TalkToMe->3).
		if (!m_State.IsMultipleControllersReqDiag && (packet->TalkToMe & TTM_SEND_DIAG_UNICAST)) {
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

void ArtNetNode::HandleDmx(void) {
	const struct TArtDmx *packet = (struct TArtDmx *)&(m_ArtNetPacket.ArtPacket.ArtDmx);

	uint32_t data_length = (uint32_t) ((packet->LengthHi << 8) & 0xff00) | (packet->Length);
	data_length = MIN(data_length, ARTNET_DMX_LENGTH);

	for (uint32_t i = 0; i < (ARTNET_MAX_PORTS * m_nPages); i++) {

		if (m_OutputPorts[i].bIsEnabled && (m_OutputPorts[i].tPortProtocol == PORT_ARTNET_ARTNET) && (packet->PortAddress == m_OutputPorts[i].port.nPortAddress)) {

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
				m_OutputPorts[i].timeA = m_nCurrentPacketTime;
				memcpy(&m_OutputPorts[i].dataA, packet->Data, data_length);
				sendNewData = IsDmxDataChanged(i, packet->Data, data_length);
			} else if (ipA == m_ArtNetPacket.IPAddressFrom && ipB == 0) {
#if defined ( ENABLE_SENDDIAG )
				SendDiag("2. continued transmission from the same ip (source A)", ARTNET_DP_LOW);
#endif
				m_OutputPorts[i].timeA = m_nCurrentPacketTime;
				memcpy(&m_OutputPorts[i].dataA, packet->Data, data_length);
				sendNewData = IsDmxDataChanged(i, packet->Data, data_length);
			} else if (ipA == 0 && ipB == m_ArtNetPacket.IPAddressFrom) {
#if defined ( ENABLE_SENDDIAG )
				SendDiag("3. continued transmission from the same ip (source B)", ARTNET_DP_LOW);
#endif
				m_OutputPorts[i].timeB = m_nCurrentPacketTime;
				memcpy(&m_OutputPorts[i].dataB, packet->Data, data_length);
				sendNewData = IsDmxDataChanged(i, packet->Data, data_length);
			} else if (ipA != m_ArtNetPacket.IPAddressFrom && ipB == 0) {
#if defined ( ENABLE_SENDDIAG )
				SendDiag("4. new source, start the merge", ARTNET_DP_LOW);
#endif
				m_OutputPorts[i].ipB = m_ArtNetPacket.IPAddressFrom;
				m_OutputPorts[i].timeB = m_nCurrentPacketTime;
				memcpy(&m_OutputPorts[i].dataB, packet->Data, data_length);
				sendNewData = IsMergedDmxDataChanged(i, m_OutputPorts[i].dataB, data_length);
			} else if (ipA == 0 && ipB != m_ArtNetPacket.IPAddressFrom) {
#if defined ( ENABLE_SENDDIAG )
				SendDiag("5. new source, start the merge", ARTNET_DP_LOW);
#endif
				m_OutputPorts[i].ipA = m_ArtNetPacket.IPAddressFrom;
				m_OutputPorts[i].timeA = m_nCurrentPacketTime;
				memcpy(&m_OutputPorts[i].dataA, packet->Data, data_length);
				sendNewData = IsMergedDmxDataChanged(i, m_OutputPorts[i].dataA, data_length);
			} else if (ipA == m_ArtNetPacket.IPAddressFrom && ipB != m_ArtNetPacket.IPAddressFrom) {
#if defined ( ENABLE_SENDDIAG )
				SendDiag("6. continue merge", ARTNET_DP_LOW);
#endif
				m_OutputPorts[i].timeA = m_nCurrentPacketTime;
				memcpy(&m_OutputPorts[i].dataA, packet->Data, data_length);
				sendNewData = IsMergedDmxDataChanged(i, m_OutputPorts[i].dataA, data_length);
			} else if (ipA != m_ArtNetPacket.IPAddressFrom && ipB == m_ArtNetPacket.IPAddressFrom) {
#if defined ( ENABLE_SENDDIAG )
				SendDiag("7. continue merge", ARTNET_DP_LOW);
#endif
				m_OutputPorts[i].timeB = m_nCurrentPacketTime;
				memcpy(&m_OutputPorts[i].dataB, packet->Data, data_length);
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

void ArtNetNode::HandleSync(void) {
	m_State.IsSynchronousMode = true;
	m_State.ArtSyncTime = Hardware::Get()->GetTime();

	for (uint32_t i = 0; i < (m_nPages * ARTNET_MAX_PORTS); i++) {
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

void ArtNetNode::HandleAddress(void) {
	const struct TArtAddress *packet = (struct TArtAddress *) &(m_ArtNetPacket.ArtPacket.ArtAddress);
	uint8_t nPort = 0xFF;

	m_State.reportCode = ARTNET_RCPOWEROK;

	if (packet->ShortName[0] != 0)  {
		SetShortName((const char *) packet->ShortName);
		m_State.reportCode = ARTNET_RCSHNAMEOK;
	}

	if (packet->LongName[0] != 0) {
		SetLongName((const char *) packet->LongName);
		m_State.reportCode = ARTNET_RCLONAMEOK;
	}

	if (packet->SubSwitch == PROGRAM_DEFAULTS) {
		SetSubnetSwitch(NODE_DEFAULT_SUBNET_SWITCH);
	} else if (packet->SubSwitch & PROGRAM_CHANGE_MASK) {
		SetSubnetSwitch(packet->SubSwitch & ~PROGRAM_CHANGE_MASK);
	}

	if (packet->NetSwitch == PROGRAM_DEFAULTS) {
		SetNetSwitch(NODE_DEFAULT_NET_SWITCH);
	} else if (packet->NetSwitch & PROGRAM_CHANGE_MASK) {
		SetNetSwitch(packet->NetSwitch & ~PROGRAM_CHANGE_MASK);
	}

	for (uint32_t i = 0; i < ARTNET_MAX_PORTS; i++) {
		if (packet->SwOut[i] == PROGRAM_NO_CHANGE) {
			continue;
		} else if (packet->SwOut[i] == PROGRAM_DEFAULTS) {
			SetUniverseSwitch(i, ARTNET_OUTPUT_PORT, NODE_DEFAULT_UNIVERSE);
		} else if (packet->SwOut[i] & PROGRAM_CHANGE_MASK) {
			SetUniverseSwitch(i, ARTNET_OUTPUT_PORT, packet->SwOut[i] & ~PROGRAM_CHANGE_MASK);
		}
	}

	switch (packet->Command) {
	case ARTNET_PC_CANCEL:
		// If Node is currently in merge mode, cancel merge mode upon receipt of next ArtDmx packet.
		m_State.IsMergeMode = false;
		for (uint32_t i = 0; i < (ARTNET_MAX_PORTS * m_nPages); i++) {
			m_OutputPorts[i].port.nStatus &= (~GO_OUTPUT_IS_MERGING);
		}
		break;

	case ARTNET_PC_LED_NORMAL:
		LedBlink::Get()->SetMode(LEDBLINK_MODE_NORMAL);
		m_Node.Status1 = (m_Node.Status1 & ~STATUS1_INDICATOR_MASK) | STATUS1_INDICATOR_NORMAL_MODE;
		break;
	case ARTNET_PC_LED_MUTE:
		LedBlink::Get()->SetMode(LEDBLINK_MODE_OFF);
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
		SetMergeMode(packet->Command & 0x3, ARTNET_MERGE_LTP);
		break;

	case ARTNET_PC_MERGE_HTP_0:
	case ARTNET_PC_MERGE_HTP_1:
	case ARTNET_PC_MERGE_HTP_2:
	case ARTNET_PC_MERGE_HTP_3:
		SetMergeMode(packet->Command & 0x3, ARTNET_MERGE_HTP);
		break;

	case ARTNET_PC_ARTNET_SEL0:
	case ARTNET_PC_ARTNET_SEL1:
	case ARTNET_PC_ARTNET_SEL2:
	case ARTNET_PC_ARTNET_SEL3:
		SetPortProtocol(packet->Command & 0x3, PORT_ARTNET_ARTNET);
		break;

	case ARTNET_PC_ACN_SEL0:
	case ARTNET_PC_ACN_SEL1:
	case ARTNET_PC_ACN_SEL2:
	case ARTNET_PC_ACN_SEL3:
		SetPortProtocol(packet->Command & 0x3, PORT_ARTNET_SACN);
		break;

	case ARTNET_PC_CLR_0:
	case ARTNET_PC_CLR_1:
	case ARTNET_PC_CLR_2:
	case ARTNET_PC_CLR_3:
		nPort = packet->Command & 0x3;
		for (uint32_t i = 0; i < ARTNET_DMX_LENGTH; i++) {
			m_OutputPorts[nPort].data[i] = 0;
		}
		m_OutputPorts[nPort].nLength = ARTNET_DMX_LENGTH;
		if (m_OutputPorts[nPort].tPortProtocol == PORT_ARTNET_ARTNET) {
			m_pLightSet->SetData(nPort, m_OutputPorts[nPort].data, m_OutputPorts[nPort].nLength);
		}
		break;

	default:
		break;
	}

	if ((nPort < ARTNET_MAX_PORTS) && (m_OutputPorts[nPort].tPortProtocol == PORT_ARTNET_ARTNET) && !m_IsLightSetRunning[nPort]) {
		m_pLightSet->Start(nPort);
		m_IsLightSetRunning[nPort] = true;
		m_OutputPorts[nPort].port.nStatus |= GO_DATA_IS_BEING_TRANSMITTED;
	}

	if (m_pArtNet4Handler != 0) {
		m_pArtNet4Handler->HandleAddress(packet->Command);
	}

	SendPollRelply(true);
}

void ArtNetNode::SetNetworkDataLossCondition(void) {
	m_State.IsMergeMode = false;
	m_State.IsSynchronousMode = false;

	for (uint32_t i = 0; i < (ARTNET_MAX_PORTS * m_nPages); i++) {
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

void ArtNetNode::GetType(void) {
	char *data = (char *) &(m_ArtNetPacket.ArtPacket);

	if (m_ArtNetPacket.length < ARTNET_MIN_HEADER_SIZE) {
		m_ArtNetPacket.OpCode = OP_NOT_DEFINED;
		return;
	}

	if ((data[10] != 0) || (data[11] != (char) ARTNET_PROTOCOL_REVISION)) {
		m_ArtNetPacket.OpCode = OP_NOT_DEFINED;
		return;
	}

	if (memcmp(data, "Art-Net\0", 8) == 0) {
		m_ArtNetPacket.OpCode = (TOpCodes) ((uint16_t) (data[9] << 8) + data[8]);
	} else {
		m_ArtNetPacket.OpCode = OP_NOT_DEFINED;
	}
}

int ArtNetNode::Run(void) {
	const char *packet = (char *) &(m_ArtNetPacket.ArtPacket);
	uint16_t nForeignPort;

	const int nBytesReceived = Network::Get()->RecvFrom(m_nHandle, (uint8_t *) packet, (uint16_t) sizeof(m_ArtNetPacket.ArtPacket), &m_ArtNetPacket.IPAddressFrom, &nForeignPort);

	m_nCurrentPacketTime = Hardware::Get()->GetTime();

	if (nBytesReceived == 0) {
		if ((m_State.nNetworkDataLossTimeout != 0) && ((m_nCurrentPacketTime - m_nPreviousPacketTime) >= m_State.nNetworkDataLossTimeout)) {
			SetNetworkDataLossCondition();
		}

		if (m_State.SendArtPollReplyOnChange) {
			bool doSend = m_State.IsChanged;
			if (m_pArtNet4Handler != 0) {
				doSend |= m_pArtNet4Handler->IsStatusChanged();
			}
			if (doSend) {
				SendPollRelply(false);
			}
		}

		if ((m_nCurrentPacketTime - m_nPreviousPacketTime) >= 1) {
			if (((m_Node.Status1 & STATUS1_INDICATOR_MASK) == STATUS1_INDICATOR_NORMAL_MODE)) {
				LedBlink::Get()->SetMode(LEDBLINK_MODE_NORMAL);
			}
		}

		return 0;
	}

	m_ArtNetPacket.length = nBytesReceived;
	m_nPreviousPacketTime = m_nCurrentPacketTime;

	GetType();

	if (m_State.IsSynchronousMode) {
		if (m_nCurrentPacketTime - m_State.ArtSyncTime >= 4) {
			m_State.IsSynchronousMode = false;
		}
	}

	switch (m_ArtNetPacket.OpCode) {
	case OP_POLL:
		HandlePoll();
		break;
	case OP_DMX:
		if (m_pLightSet != 0) {
			HandleDmx();
		}
		break;
	case OP_SYNC:
		if (m_pLightSet != 0) {
			HandleSync();
		}
		break;
	case OP_ADDRESS:
		HandleAddress();
		break;
	case OP_TIMECODE:
		if (m_pArtNetTimeCode != 0) {
			HandleTimeCode();
		}
		break;
	case OP_TIMESYNC:
		if (m_pArtNetTimeSync != 0) {
			HandleTimeSync();
		}
		break;
	case OP_TODREQUEST:
		if (m_pArtNetRdm != 0) {
			HandleTodRequest();
		}
		break;
	case OP_TODCONTROL:
		if (m_pArtNetRdm != 0) {
			HandleTodControl();
		}
		break;
	case OP_RDM:
		if (m_pArtNetRdm != 0) {
			HandleRdm();
		}
		break;
	case OP_IPPROG:
		if (m_pArtNetIpProg != 0) {
			HandleIpProg();
		}
		break;
	default:
		// ArtNet but OpCode is not implemented
		// Just skip ... no error
		return 0;
		__builtin_unreachable ();
		break;
	}

	if (((m_Node.Status1 & STATUS1_INDICATOR_MASK) == STATUS1_INDICATOR_NORMAL_MODE)) {
		if (m_State.bIsReceivingDmx) {
			LedBlink::Get()->SetMode(LEDBLINK_MODE_DATA);
			m_State.bIsReceivingDmx = false;
		} else {
			LedBlink::Get()->SetMode(LEDBLINK_MODE_NORMAL);
		}
	}

	return m_ArtNetPacket.length;
}

#if defined ( ENABLE_SENDDIAG )
void ArtNetNode::FillDiagData(void) {
	memset(&m_DiagData, 0, sizeof (struct TArtDiagData));

	strncpy((char *)m_DiagData.Id, (const char *)NODE_ID, sizeof m_DiagData.Id);
	m_DiagData.OpCode = OP_DIAGDATA;
	m_DiagData.ProtVerHi = 0;
	m_DiagData.ProtVerLo = ARTNET_PROTOCOL_REVISION;
}

void ArtNetNode::SendDiag(const char *text, TPriorityCodes nPriority) {
	if (!m_State.SendArtDiagData) {
		return;
	}

	if (nPriority < m_State.Priority) {
		return;
	}

	m_DiagData.Priority = nPriority;

	strncpy((char *) m_DiagData.Data, text, sizeof m_DiagData.Data);
	m_DiagData.Data[sizeof(m_DiagData.Data) - 1] = '\0';// Just be sure we have a last '\0'
	m_DiagData.LengthLo = strlen((const char *) m_DiagData.Data) + 1;// Text length including the '\0'

	const uint16_t nSize = sizeof(struct TArtDiagData) - sizeof(m_DiagData.Data) + m_DiagData.LengthLo;

	Network::Get()->SendTo(m_nHandle, (const uint8_t *) &(m_DiagData), nSize, m_State.IPAddressDiagSend, (uint16_t) ARTNET_UDP_PORT);
}
#endif
