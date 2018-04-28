/**
 * @file artnetnode.cpp
 *
 */
/**
 * Art-Net Designed by and Copyright Artistic Licence Holdings Ltd.
 *
 * Art-Net 3 Protocol Release V1.4 Document Revision 1.4bk 23/1/2016
 *
 */
/* Copyright (C) 2016-2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#include <assert.h>

#if defined (BARE_METAL)
 #include <stdbool.h>
 #include "util.h"
#elif defined (__circle__)
 #include <circle/util.h>
#else
 #include <stdbool.h>
 #include <string.h>
#endif

#include "artnetnode.h"
#include "packets.h"

#include "lightset.h"
#include "ledblink.h"

#include "artnetrdm.h"
#include "artnettimecode.h"
#include "artnettimesync.h"
#include "artnetipprog.h"

#include "hardware.h"
#include "network.h"

/**
 * Defines output status of the node.
 */
enum TGoodOutput {
	GO_DATA_IS_BEING_TRANSMITTED = (1 << 7),	///< Bit 7 Set – Data is being transmitted.
	GO_INCLUDES_DMX_TEST_PACKETS = (1 << 6),	///< Bit 6 Set – Channel includes DMX512 test packets.
	GO_INCLUDES_DMX_SIP = (1 << 5),				///< Bit 5 Set – Channel includes DMX512 SIP’s.
	GO_INCLUDES_DMX_TEXT_PACKETS = (1 << 4),	///< Bit 4 Channel includes DMX512 text packets.
	GO_OUTPUT_IS_MERGING = (1 << 3),			///< Bit 3 Set – Output is merging ArtNet data.
	GO_DMX_SHORT_DETECTED = (1 << 2),			///< Bit 2 Set – DMX output short detected on power up
	GO_MERGE_MODE_LTP = (1 << 1)				///< Bit 1 Set – Merge Mode is LTP.
};

/**
 *
 */
enum TProgram {
	PROGRAM_NO_CHANGE = 0x7F,					///<
	PROGRAM_DEFAULTS = 0x00,					///<
	PROGRAM_CHANGE_MASK = 0x80					///<
};

union uip {
	uint32_t u32;
	uint8_t u8[4];
} ip;

#ifndef min
 #define min(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifndef max
 #define max(a, b) ((a) > (b) ? (a) : (b))
#endif

#define ARTNET_PROTOCOL_REVISION	14							///< Art-Net 3 Protocol Release V1.4 Document Revision 1.4bk 23/1/2016

#define NODE_ID						"Art-Net"					///< Array of 8 characters, the final character is a null termination. Value = A r t - N e t 0x00
#define NODE_DEFAULT_SHORT_NAME		"AvV Art-Net Node"
#define NODE_DEFAULT_LONG_NAME		"Raspberry Pi Art-Net 3 Node http://www.raspberrypi-dmx.org"
#define NODE_DEFAULT_NET_SWITCH		0
#define NODE_DEFAULT_SUBNET_SWITCH	0
#define NODE_DEFAULT_UNIVERSE		0

static const uint8_t DEVICE_MANUFACTURER_ID[] = { 0x7F, 0xF0 };	///< 0x7F, 0xF0 : RESERVED FOR PROTOTYPING/EXPERIMENTAL USE ONLY
static const uint8_t DEVICE_SOFTWARE_VERSION[] = {0x01, 0x14 };	///<
static const uint8_t DEVICE_OEM_VALUE[] = { 0x20, 0xE0 };		///< OemArtRelay , 0x00FF = developer code

#define ARTNET_MIN_HEADER_SIZE			12						///< \ref TArtPoll \ref TArtSync
#define ARTNET_MERGE_TIMEOUT_SECONDS	10						///<

#define NETWORK_DATA_LOSS_TIMEOUT		10						///< Seconds

#define PORT_IN_STATUS_DISABLED_MASK	0x08

ArtNetNode::ArtNetNode(void) :
		m_pLightSet(0),
		m_pArtNetTimeCode(0),
		m_pArtNetTimeSync(0),
		m_pArtNetRdm(0),
		m_pArtNetIpProg(0),
		m_pTodData(0),
		m_pIpProgReply(0),
		m_bDirectUpdate(false),
		m_nCurrentPacketTime(0),
		m_nPreviousPacketTime(0),
		m_IsLightSetRunning(false),
		m_IsRdmResponder(false)

 {
	memset(&m_Node, 0, sizeof (struct TArtNetNode));

	for (unsigned i = 0; i < ARTNET_MAX_PORTS; i++) {
		m_OutputPorts[i].port.nStatus = (uint8_t) 0;
		m_OutputPorts[i].port.nPortAddress = (uint16_t) 0;
		m_OutputPorts[i].port.nDefaultAddress = (uint8_t) 0;
		m_OutputPorts[i].mergeMode = ARTNET_MERGE_HTP;
		m_OutputPorts[i].IsDataPending = false;
		m_OutputPorts[i].bIsEnabled = false;
		m_OutputPorts[i].nLength = (uint16_t) 0;
		m_OutputPorts[i].ipA = (uint32_t) 0;
		m_OutputPorts[i].ipB = (uint32_t) 0;
	}

	m_Node.Status1 = STATUS1_INDICATOR_NORMAL_MODE | STATUS1_PAP_FRONT_PANEL;
	m_Node.Status2 = STATUS2_DHCP_CAPABLE | STATUS2_PORT_ADDRESS_15BIT;

	m_State.IsSynchronousMode = false;
	m_State.SendArtDiagData = false;
	m_State.IsMergeMode = false;
	m_State.IsChanged = false;
	m_State.SendArtPollReplyOnChange = false;
	m_State.ArtPollReplyCount = (uint32_t)0;
	m_State.IPAddressArtPoll = (uint32_t)0;
	m_State.IsMultipleControllersReqDiag = false;
	m_State.reportCode = ARTNET_RCPOWEROK;
	m_State.nActivePorts = 0;
	m_State.status = ARTNET_STANDBY;
	m_State.nNetworkDataLossTimeout = NETWORK_DATA_LOSS_TIMEOUT;

	m_tOpCodePrevious = OP_NOT_DEFINED;

	SetShortName((const char *)NODE_DEFAULT_SHORT_NAME);
	SetLongName((const char *)NODE_DEFAULT_LONG_NAME);
	SetManufacturerId(DEVICE_MANUFACTURER_ID);
	SetOemValue(DEVICE_OEM_VALUE);

	uint8_t nSysNameLenght;
	const char *pSysName = Hardware::Get()->GetSysName(nSysNameLenght);
	strncpy(m_aSysName, pSysName, sizeof m_aSysName);

	if (nSysNameLenght >= sizeof m_aSysName) {
		m_aSysName[(sizeof m_aSysName) -1] = '\0';
	}

}

ArtNetNode::~ArtNetNode(void) {
	if (m_pLightSet != 0) {
		m_pLightSet->Stop();
		m_pLightSet = 0;
		m_IsLightSetRunning = false;
	}

	LedBlink::Get()->SetMode(LEDBLINK_MODE_OFF);

	if (m_pTodData != 0) {
		delete m_pTodData;
	}

	if (m_pIpProgReply != 0) {
		delete m_pIpProgReply;
	}

	memset(&m_Node, 0, sizeof (struct TArtNetNode));
	memset(&m_PollReply, 0, sizeof (struct TArtPollReply));
	memset(&m_DiagData, 0, sizeof (struct TArtDiagData));
	memset(&m_TimeCodeData, 0, sizeof (struct TArtTimeCode));
}

void ArtNetNode::Start(void) {
	m_Node.IPAddressLocal = Network::Get()->GetIp();
	m_Node.IPAddressBroadcast = m_Node.IPAddressLocal | ~(Network::Get()->GetNetmask());
	Network::Get()->MacAddressCopyTo(m_Node.MACAddressLocal);
	m_Node.Status2 = (m_Node.Status2 & ~(1 << 1)) | (Network::Get()->IsDhcpUsed() ? STATUS2_IP_DHCP : STATUS2_IP_MANUALY);

	FillPollReply();
	FillDiagData();
	FillTimeCodeData();

	Network::Get()->Begin(ARTNET_UDP_PORT);

	m_PollReply.NumPortsLo = m_State.nActivePorts;

	for (unsigned i = 0 ; i < ARTNET_MAX_PORTS; i++) {
		if (m_OutputPorts[i].bIsEnabled) {
			m_PollReply.PortTypes[i] = ARTNET_ENABLE_OUTPUT | ARTNET_PORT_DMX;
		}
	}
	m_State.status = ARTNET_ON;

	LedBlink::Get()->SetMode(LEDBLINK_MODE_NORMAL);

	SendPollRelply(false);	// send a reply on startup
}

void ArtNetNode::Stop(void) {
	if (m_pLightSet != 0) {
		m_pLightSet->Stop();
		m_IsLightSetRunning = false;
	}

	LedBlink::Get()->SetMode(LEDBLINK_MODE_OFF);

	m_State.status = ARTNET_OFF;
}

void ArtNetNode::SetOutput(LightSet *pLightSet) {
	m_pLightSet = pLightSet;
}

bool ArtNetNode::GetDirectUpdate() const {
	return m_bDirectUpdate;
}

void ArtNetNode::SetDirectUpdate(bool bDirectUpdate) {
	m_bDirectUpdate = bDirectUpdate;
}

const uint8_t *ArtNetNode::GetSoftwareVersion(void) {
	return DEVICE_SOFTWARE_VERSION;
}

uint8_t ArtNetNode::GetActiveOutputPorts(void) const{
	return m_State.nActivePorts;
}

uint8_t ArtNetNode::GetActiveInputPorts(void) const {
	return 0;
}

uint8_t ArtNetNode::GetUniverseSwitch(uint8_t nPortId) const {
	if (nPortId >= ARTNET_MAX_PORTS) {
		return ARTNET_EARG;
	}
	return m_OutputPorts[nPortId].port.nDefaultAddress;
}

int ArtNetNode::SetUniverseSwitch(const uint8_t nPortIndex, const TArtNetPortDir dir, const uint8_t nAddress) {
	if (nPortIndex >= ARTNET_MAX_PORTS) {
		return ARTNET_EARG;
	}

	if (dir == ARTNET_INPUT_PORT) {
		// Not supported. We have output ports only.
		return ARTNET_EACTION;
	} else if (dir == ARTNET_OUTPUT_PORT) {
		if (!m_OutputPorts[nPortIndex].bIsEnabled) {
			m_State.nActivePorts = m_State.nActivePorts + 1;
			assert(m_State.nActivePorts <= ARTNET_MAX_PORTS);
		}
		m_OutputPorts[nPortIndex].bIsEnabled = true;
	} else {
		return ARTNET_EARG;
	}

	m_OutputPorts[nPortIndex].port.nDefaultAddress = nAddress & (uint16_t)0x0F;		// Universe : Bits 3-0
	m_OutputPorts[nPortIndex].port.nPortAddress = MakePortAddress((uint16_t)nAddress);

	return ARTNET_EOK;
}

uint8_t ArtNetNode::GetSubnetSwitch(void) const {
	return m_Node.SubSwitch;
}

void ArtNetNode::SetSubnetSwitch(const uint8_t nAddress) {
	m_Node.SubSwitch = nAddress;

	for (unsigned i = 0; i < ARTNET_MAX_PORTS; i++) {
		m_OutputPorts[i].port.nPortAddress = MakePortAddress(m_OutputPorts[i].port.nPortAddress);
	}
}

uint8_t ArtNetNode::GetNetSwitch(void) const{
	return m_Node.NetSwitch;
}

void ArtNetNode::SetNetSwitch(uint8_t nAddress) {
	m_Node.NetSwitch = nAddress;

	for (unsigned i = 0; i < ARTNET_MAX_PORTS; i++) {
		m_OutputPorts[i].port.nPortAddress = MakePortAddress(m_OutputPorts[i].port.nPortAddress);
	}
}

const char *ArtNetNode::GetShortName(void) {
	return (const char *)m_Node.ShortName;
}

void ArtNetNode::SetShortName(const char *pName) {
	if (pName == 0) {
		return;
	}

	memset((void *)m_Node.ShortName, 0, ARTNET_SHORT_NAME_LENGTH);
	strncpy((char *) m_Node.ShortName, pName, ARTNET_SHORT_NAME_LENGTH);
	m_Node.ShortName[ARTNET_SHORT_NAME_LENGTH-1] = '\0';

	memcpy (m_PollReply.ShortName, m_Node.ShortName, ARTNET_SHORT_NAME_LENGTH);
}

const char *ArtNetNode::GetLongName(void) {
	return (const char *)m_Node.LongName;
}

void ArtNetNode::SetLongName(const char *pName) {
	if (pName == 0) {
		return;
	}

	memset((void *)m_Node.LongName, 0, ARTNET_LONG_NAME_LENGTH);
	strncpy((char *) m_Node.LongName, pName, ARTNET_LONG_NAME_LENGTH);
	m_Node.LongName[ARTNET_LONG_NAME_LENGTH-1] = '\0';

	memcpy (m_PollReply.LongName, m_Node.LongName, ARTNET_LONG_NAME_LENGTH);
}

void ArtNetNode::SetManufacturerId(const uint8_t *pEsta) {
	m_Node.Esta[0] = pEsta[1];
	m_Node.Esta[1] = pEsta[0];
}

const uint8_t* ArtNetNode::GetManufacturerId(void) {
	return m_Node.Esta;
}

void ArtNetNode::SetOemValue(const uint8_t *pOem) {
	m_Node.Oem[0] = pOem[0];
	m_Node.Oem[1] = pOem[1];
}

const uint8_t* ArtNetNode::GetOemValue(void) {
	return m_Node.Oem;
}

time_t ArtNetNode::GetNetworkTimeout(void) const {
	return m_State.nNetworkDataLossTimeout;
}

void ArtNetNode::SetNetworkTimeout(time_t nNetworkDataLossTimeout) {
	if (nNetworkDataLossTimeout != 0) {
		m_State.nNetworkDataLossTimeout = nNetworkDataLossTimeout;
	}
}

uint16_t ArtNetNode::MakePortAddress(const uint16_t nCurrentAddress) {
	// PortAddress Bit 15 = 0
	uint16_t newAddress = (m_Node.NetSwitch & 0x7F) << 8;	// Net : Bits 14-8
	newAddress |= (m_Node.SubSwitch & (uint8_t)0x0F) << 4;	// Sub-Net : Bits 7-4
	newAddress |= nCurrentAddress & (uint16_t)0x0F;			// Universe : Bits 3-0

	return newAddress;
}

void ArtNetNode::FillPollReply(void) {
	memset(&m_PollReply, 0, sizeof (struct TArtPollReply));

	memcpy (m_PollReply.Id, (const char *)NODE_ID, sizeof m_PollReply.Id);
	m_PollReply.OpCode = OP_POLLREPLY;

	ip.u32 = m_Node.IPAddressLocal;
	memcpy(m_PollReply.IPAddress, ip.u8, sizeof m_PollReply.IPAddress);
	m_PollReply.Port = (uint16_t) ARTNET_UDP_PORT;
	m_PollReply.VersInfoH = DEVICE_SOFTWARE_VERSION[0];
	m_PollReply.VersInfoL = DEVICE_SOFTWARE_VERSION[1];
	m_PollReply.NetSwitch = m_Node.NetSwitch;
	m_PollReply.SubSwitch = m_Node.SubSwitch;
	m_PollReply.OemHi = m_Node.Oem[0];
	m_PollReply.Oem = m_Node.Oem[1];
	m_PollReply.Status1 = m_Node.Status1;
	m_PollReply.EstaMan[0] = m_Node.Esta[0];
	m_PollReply.EstaMan[1] = m_Node.Esta[1];
	memcpy(m_PollReply.ShortName, m_Node.ShortName, sizeof m_PollReply.ShortName);
	memcpy(m_PollReply.LongName, m_Node.LongName, sizeof m_PollReply.LongName);

	// Disable all input
	for (unsigned i = 0 ; i < ARTNET_MAX_PORTS; i++) {
		m_PollReply.GoodInput[i] = PORT_IN_STATUS_DISABLED_MASK;
	}

	m_PollReply.Style = ARTNET_ST_NODE;
	memcpy (m_PollReply.MAC, m_Node.MACAddressLocal, sizeof m_PollReply.MAC);
	m_PollReply.Status2 = m_Node.Status2;
}

void ArtNetNode::FillDiagData(void) {
	memset(&m_DiagData, 0, sizeof (struct TArtDiagData));

	strncpy((char *)m_DiagData.Id, (const char *)NODE_ID, sizeof m_DiagData.Id);
	m_DiagData.OpCode = OP_DIAGDATA;
	m_DiagData.ProtVerHi = (uint8_t) 0;							// high byte of the Art-Net protocol revision number.
	m_DiagData.ProtVerLo = (uint8_t) ARTNET_PROTOCOL_REVISION;	// low byte of the Art-Net protocol revision number.
}

void ArtNetNode::FillTimeCodeData(void) {
	memset(&m_TimeCodeData, 0, sizeof (struct TArtTimeCode));

	memcpy (m_TimeCodeData.Id, (const char *)NODE_ID, sizeof m_TimeCodeData.Id);

	m_TimeCodeData.OpCode = OP_TIMECODE;
	m_TimeCodeData.ProtVerHi = (uint8_t) 0;							// high byte of the Art-Net protocol revision number.
	m_TimeCodeData.ProtVerLo = (uint8_t) ARTNET_PROTOCOL_REVISION;	// low byte of the Art-Net protocol revision number.
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
		m_ArtNetPacket.OpCode = (TOpCodes) ((uint16_t)(data[9] << 8) + data[8]);
	} else {
		m_ArtNetPacket.OpCode = OP_NOT_DEFINED;
	}
}

void ArtNetNode::SendPollRelply(const bool bResponse) {
	if (!bResponse && m_State.status == ARTNET_ON) {
		m_State.ArtPollReplyCount++;
	}

	for (unsigned i = 0 ; i < ARTNET_MAX_PORTS; i++) {
		m_PollReply.GoodOutput[i] = m_OutputPorts[i].port.nStatus;
		m_PollReply.SwOut[i] = m_OutputPorts[i].port.nDefaultAddress;
	}

	char report[ARTNET_REPORT_LENGTH];

	snprintf(report, ARTNET_REPORT_LENGTH, "%04x [%04d] %s AvV", (int)m_State.reportCode, (int)m_State.ArtPollReplyCount, m_aSysName);
	strcpy((char *)m_PollReply.NodeReport, report);

	Network::Get()->SendTo((const uint8_t *)&(m_PollReply), (const uint16_t)sizeof (struct TArtPollReply), m_Node.IPAddressBroadcast, (uint16_t)ARTNET_UDP_PORT);
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
	m_DiagData.Data[sizeof(m_DiagData.Data) - 1] = '\0';				// Just be sure we have a last '\0'
	m_DiagData.LengthLo = strlen((const char *) m_DiagData.Data) + 1;	// Text length including the '\0'

	unsigned size = sizeof(struct TArtDiagData) - sizeof(m_DiagData.Data) + m_DiagData.LengthLo;

	Network::Get()->SendTo((const uint8_t *)&(m_DiagData), (const uint16_t)size, m_State.IPAddressDiagSend, (uint16_t)ARTNET_UDP_PORT);
}

bool ArtNetNode::IsDmxDataChanged(const uint8_t nPortId, const uint8_t *pData, const uint16_t nLength) {
	bool isChanged = false;

	uint8_t *src = (uint8_t *) pData;
	uint8_t *dst = m_OutputPorts[nPortId].data;

	if (nLength != m_OutputPorts[nPortId].nLength) {
		m_OutputPorts[nPortId].nLength = nLength;

		for (unsigned i = 0 ; i < ARTNET_DMX_LENGTH; i++) {
			*dst++ = *src++;
		}
		return true;
	}

	for (unsigned i = 0; i < ARTNET_DMX_LENGTH; i++) {
		if (*dst != *src) {
			*dst = *src;
			isChanged = true;
		}
		dst++;
		src++;
	}

	return isChanged;
}

bool ArtNetNode::IsMergedDmxDataChanged(const uint8_t nPortId, const uint8_t *pData, const uint16_t nLength) {
	bool isChanged = false;

	if (!m_State.IsMergeMode) {
		m_State.IsMergeMode = true;
		m_State.IsChanged = true;
		uint8_t nStatus = m_OutputPorts[nPortId].port.nStatus;
		m_OutputPorts[nPortId].port.nStatus = nStatus | (1 << 3);	// Bit 3 : Set – Output is merging ArtNet data.
	}


	if (m_OutputPorts[nPortId].mergeMode == ARTNET_MERGE_HTP) {

		if (nLength != m_OutputPorts[nPortId].nLength) {
			m_OutputPorts[nPortId].nLength = nLength;
			for (unsigned i = 0; i < nLength; i++) {
				uint8_t data = max(m_OutputPorts[nPortId].dataA[i], m_OutputPorts[nPortId].dataB[i]);
				m_OutputPorts[nPortId].data[i] = data;
			}
			return true;
		}

		for (unsigned i = 0; i < nLength; i++) {
			uint8_t data = max(m_OutputPorts[nPortId].dataA[i], m_OutputPorts[nPortId].dataB[i]);
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

void ArtNetNode::CheckMergeTimeouts(const uint8_t nPortId) {
	const time_t timeOutA = m_nCurrentPacketTime - m_OutputPorts[nPortId].timeA;
	const time_t timeOutB = m_nCurrentPacketTime - m_OutputPorts[nPortId].timeB;

	if (timeOutA > (time_t)ARTNET_MERGE_TIMEOUT_SECONDS) {
		m_OutputPorts[nPortId].ipA = 0;
		m_State.IsMergeMode = false;
	}

	if (timeOutB > (time_t)ARTNET_MERGE_TIMEOUT_SECONDS) {
		m_OutputPorts[nPortId].ipB = 0;
		m_State.IsMergeMode = false;
	}

	if (!m_State.IsMergeMode) {
		m_State.IsChanged = true;
		const uint8_t nStatus = m_OutputPorts[nPortId].port.nStatus;
		m_OutputPorts[nPortId].port.nStatus = nStatus & ~GO_OUTPUT_IS_MERGING;
#ifdef SENDDIAG
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
			m_State.Priority = min(m_State.Priority , packet->Priority);
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
		m_State.IPAddressDiagSend = (uint32_t) 0;
	}

	SendPollRelply(true);
}

void ArtNetNode::HandleDmx(void) {
	const struct TArtDmx *packet = (struct TArtDmx *)&(m_ArtNetPacket.ArtPacket.ArtDmx);

	unsigned data_length = (unsigned) ((packet->LengthHi << 8) & 0xff00) | (packet->Length);
	data_length = min(data_length, ARTNET_DMX_LENGTH);

	for (unsigned i = 0; i < ARTNET_MAX_PORTS; i++) {
		if ((packet->PortAddress == m_OutputPorts[i].port.nPortAddress) &&  m_OutputPorts[i].bIsEnabled) {

			uint32_t ipA = m_OutputPorts[i].ipA;
			uint32_t ipB = m_OutputPorts[i].ipB;

			bool sendNewData = false;

			m_OutputPorts[i].port.nStatus = m_OutputPorts[i].port.nStatus |GO_DATA_IS_BEING_TRANSMITTED;

			if (m_State.IsMergeMode) {
				CheckMergeTimeouts(i);
			}

			if (ipA == 0 && ipB == 0) {
#ifdef SENDDIAG
				SendDiag("1. first packet recv on this port", ARTNET_DP_LOW);
#endif
				m_OutputPorts[i].ipA = m_ArtNetPacket.IPAddressFrom;
				m_OutputPorts[i].timeA = m_nCurrentPacketTime;
				memcpy(&m_OutputPorts[i].dataA, packet->Data, data_length);
				sendNewData = IsDmxDataChanged(i, packet->Data, data_length);
			} else if (ipA == m_ArtNetPacket.IPAddressFrom && ipB == 0) {
#ifdef SENDDIAG
				SendDiag("2. continued transmission from the same ip (source A)", ARTNET_DP_LOW);
#endif
				m_OutputPorts[i].timeA = m_nCurrentPacketTime;
				memcpy(&m_OutputPorts[i].dataA, packet->Data, data_length);
				sendNewData = IsDmxDataChanged(i, packet->Data, data_length);
			} else if (ipA == 0 && ipB == m_ArtNetPacket.IPAddressFrom) {
#ifdef SENDDIAG
				SendDiag("3. continued transmission from the same ip (source B)", ARTNET_DP_LOW);
#endif
				m_OutputPorts[i].timeB = m_nCurrentPacketTime;
				memcpy(&m_OutputPorts[i].dataB, packet->Data, data_length);
				sendNewData = IsDmxDataChanged(i, packet->Data, data_length);
			} else if (ipA != m_ArtNetPacket.IPAddressFrom && ipB == 0) {
#ifdef SENDDIAG
				SendDiag("4. new source, start the merge", ARTNET_DP_LOW);
#endif
				m_OutputPorts[i].ipB = m_ArtNetPacket.IPAddressFrom;
				m_OutputPorts[i].timeB = m_nCurrentPacketTime;
				memcpy(&m_OutputPorts[i].dataB, packet->Data, data_length);
				sendNewData = IsMergedDmxDataChanged(i, m_OutputPorts[i].dataB, data_length);
			} else if (ipA == 0 && ipB != m_ArtNetPacket.IPAddressFrom) {
#ifdef SENDDIAG
				SendDiag("5. new source, start the merge", ARTNET_DP_LOW);
#endif
				m_OutputPorts[i].ipA = m_ArtNetPacket.IPAddressFrom;
				m_OutputPorts[i].timeA = m_nCurrentPacketTime;
				memcpy(&m_OutputPorts[i].dataA, packet->Data, data_length);
				sendNewData = IsMergedDmxDataChanged(i, m_OutputPorts[i].dataA, data_length);
			} else if (ipA == m_ArtNetPacket.IPAddressFrom && ipB != m_ArtNetPacket.IPAddressFrom) {
#ifdef SENDDIAG
				SendDiag("6. continue merge", ARTNET_DP_LOW);
#endif
				m_OutputPorts[i].timeA = m_nCurrentPacketTime;
				memcpy(&m_OutputPorts[i].dataA, packet->Data, data_length);
				sendNewData = IsMergedDmxDataChanged(i, m_OutputPorts[i].dataA, data_length);
			} else if (ipA != m_ArtNetPacket.IPAddressFrom && ipB == m_ArtNetPacket.IPAddressFrom) {
#ifdef SENDDIAG
				SendDiag("7. continue merge", ARTNET_DP_LOW);
#endif
				m_OutputPorts[i].timeB = m_nCurrentPacketTime;
				memcpy(&m_OutputPorts[i].dataB, packet->Data, data_length);
				sendNewData = IsMergedDmxDataChanged(i, m_OutputPorts[i].dataB, data_length);
			} else if (ipA == m_ArtNetPacket.IPAddressFrom && ipB == m_ArtNetPacket.IPAddressFrom) {
				SendDiag("8. Source matches both buffers, this shouldn't be happening!", ARTNET_DP_LOW);
				return;
			} else if (ipA != m_ArtNetPacket.IPAddressFrom && ipB != m_ArtNetPacket.IPAddressFrom) {
				SendDiag("9. More than two sources, discarding data", ARTNET_DP_LOW);
				return;
			} else {
				SendDiag("0. No cases matched, this shouldn't happen!", ARTNET_DP_LOW);
				return;
			}

			if (sendNewData || m_bDirectUpdate) {
				if (!m_State.IsSynchronousMode) {
#ifdef SENDDIAG
					SendDiag("Send new data", ARTNET_DP_LOW);
#endif
					m_pLightSet->SetData(i, m_OutputPorts[i].data, m_OutputPorts[i].nLength);

					if(!m_IsLightSetRunning) {
						m_pLightSet->Start();
						m_IsLightSetRunning = true;
					}
				} else {
#ifdef SENDDIAG
					SendDiag("DMX data pending", ARTNET_DP_LOW);
#endif
					m_OutputPorts[i].IsDataPending = true;
				}
			} else {
#ifdef SENDDIAG
				SendDiag("Data not changed", ARTNET_DP_LOW);
#endif
			}
			// Discontinue the loop. No need to return for other ports
			return;
		}
	}
}

void ArtNetNode::HandleSync(void) {
	m_State.IsSynchronousMode = true;
	m_State.ArtSyncTime = Hardware::Get()->GetTime();

	for (unsigned i = 0; i < ARTNET_MAX_PORTS; i++) {
		if (m_OutputPorts[i].IsDataPending) {
#ifdef SENDDIAG
			SendDiag("Send pending data", ARTNET_DP_LOW);
#endif
			m_pLightSet->SetData(i, m_OutputPorts[i].data, 	m_OutputPorts[i].nLength);
			if(!m_IsLightSetRunning) {
				m_pLightSet->Start();
				m_IsLightSetRunning = true;
			}
			m_OutputPorts[i].IsDataPending = false;
		}
	}
}

void ArtNetNode::HandleAddress(void) {
	const struct TArtAddress *packet = (struct TArtAddress *) &(m_ArtNetPacket.ArtPacket.ArtAddress);
	bool bClearCommand = false;

	m_State.reportCode = ARTNET_RCPOWEROK;

	if ((packet->ShortName[0] != PROGRAM_NO_CHANGE) && (packet->ShortName[0] != PROGRAM_DEFAULTS)) {
		SetShortName((const char *)packet->ShortName);
		m_State.reportCode = ARTNET_RCSHNAMEOK;
	} else if (packet->ShortName[0] == PROGRAM_DEFAULTS) {
		SetShortName((const char *)NODE_DEFAULT_SHORT_NAME);
		m_State.reportCode = ARTNET_RCSHNAMEOK;
	}

	if ((packet->LongName[0] != PROGRAM_NO_CHANGE) && (packet->LongName[0] != PROGRAM_DEFAULTS)) {
		SetLongName((const char *)packet->LongName);
		m_State.reportCode = ARTNET_RCLONAMEOK;
	} else if (packet->LongName[0] == PROGRAM_DEFAULTS) {
		SetLongName((const char *)NODE_DEFAULT_LONG_NAME);
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

	for (unsigned i = 0; i < ARTNET_MAX_PORTS; i++) {
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
		for (unsigned i = 0; i < ARTNET_MAX_PORTS; i++) {
			m_OutputPorts[i].port.nStatus = m_OutputPorts[i].port.nStatus & ~GO_OUTPUT_IS_MERGING;
		}
#ifdef SENDDIAG
		SendDiag("Leaving Merging Mode", ARTNET_DP_LOW);
#endif
		break;
	case ARTNET_PC_LED_NORMAL:
		LedBlink::Get()->SetMode(LEDBLINK_MODE_NORMAL);
		break;
	case ARTNET_PC_LED_MUTE:
		LedBlink::Get()->SetMode(LEDBLINK_MODE_OFF);
		break;
	case ARTNET_PC_LED_LOCATE:
		LedBlink::Get()->SetMode(LEDBLINK_MODE_FAST);
		break;
	case ARTNET_PC_MERGE_LTP_O:
		m_OutputPorts[0].mergeMode = ARTNET_MERGE_LTP;
		m_OutputPorts[0].port.nStatus = m_OutputPorts[0].port.nStatus | GO_MERGE_MODE_LTP;
#ifdef SENDDIAG
		SendDiag("Setting Merge Mode LTP_0", ARTNET_DP_LOW);
#endif
		break;
	case ARTNET_PC_MERGE_LTP_1:
		m_OutputPorts[1].mergeMode = ARTNET_MERGE_LTP;
		m_OutputPorts[1].port.nStatus = m_OutputPorts[1].port.nStatus | GO_MERGE_MODE_LTP;
#ifdef SENDDIAG
		SendDiag("Setting Merge Mode LTP_1", ARTNET_DP_LOW);
#endif
		break;
	case ARTNET_PC_MERGE_LTP_2:
		m_OutputPorts[2].mergeMode = ARTNET_MERGE_LTP;
		m_OutputPorts[2].port.nStatus = m_OutputPorts[2].port.nStatus | GO_MERGE_MODE_LTP;
#ifdef SENDDIAG
		SendDiag("Setting Merge Mode LTP_2", ARTNET_DP_LOW);
#endif
		break;
	case ARTNET_PC_MERGE_LTP_3:
		m_OutputPorts[3].mergeMode = ARTNET_MERGE_LTP;
		m_OutputPorts[3].port.nStatus = m_OutputPorts[3].port.nStatus | GO_MERGE_MODE_LTP;
#ifdef SENDDIAG
		SendDiag("Setting Merge Mode LTP_1", ARTNET_DP_LOW);
#endif
		break;
	case ARTNET_PC_MERGE_HTP_0:
		m_OutputPorts[0].mergeMode = ARTNET_MERGE_HTP;
		m_OutputPorts[0].port.nStatus = m_OutputPorts[0].port.nStatus & ~GO_MERGE_MODE_LTP;
#ifdef SENDDIAG
		SendDiag("Setting Merge Mode HTP_0", ARTNET_DP_LOW);
#endif
		break;
	case ARTNET_PC_MERGE_HTP_1:
		m_OutputPorts[1].mergeMode = ARTNET_MERGE_HTP;
		m_OutputPorts[1].port.nStatus = m_OutputPorts[1].port.nStatus & ~GO_MERGE_MODE_LTP;
#ifdef SENDDIAG
		SendDiag("Setting Merge Mode HTP_1", ARTNET_DP_LOW);
#endif
		break;
	case ARTNET_PC_MERGE_HTP_2:
		m_OutputPorts[2].mergeMode = ARTNET_MERGE_HTP;
		m_OutputPorts[2].port.nStatus = m_OutputPorts[2].port.nStatus & ~GO_MERGE_MODE_LTP;
#ifdef SENDDIAG
		SendDiag("Setting Merge Mode HTP_2", ARTNET_DP_LOW);
#endif
		break;
	case ARTNET_PC_MERGE_HTP_3:
		m_OutputPorts[3].mergeMode = ARTNET_MERGE_HTP;
		m_OutputPorts[3].port.nStatus = m_OutputPorts[3].port.nStatus & ~GO_MERGE_MODE_LTP;
#ifdef SENDDIAG
		SendDiag("Setting Merge Mode HTP_3", ARTNET_DP_LOW);
#endif
		break;
	case ARTNET_PC_CLR_0:
		for (unsigned i = 0; i < ARTNET_DMX_LENGTH; i++) {
			m_OutputPorts[0].data[i] = 0;
		}
		m_pLightSet->SetData (0, m_OutputPorts[0].data, m_OutputPorts[0].nLength);
		bClearCommand = true;
		break;
	case ARTNET_PC_CLR_1:
		for (unsigned i = 0; i < ARTNET_DMX_LENGTH; i++) {
			m_OutputPorts[1].data[i] = 0;
		}
		m_pLightSet->SetData (1, m_OutputPorts[1].data, m_OutputPorts[1].nLength);
		bClearCommand = true;
		break;
	case ARTNET_PC_CLR_2:
		for (unsigned i = 0; i < ARTNET_DMX_LENGTH; i++) {
			m_OutputPorts[2].data[i] = 0;
		}
		m_pLightSet->SetData (2, m_OutputPorts[2].data, m_OutputPorts[2].nLength);
		bClearCommand = true;
		break;
	case ARTNET_PC_CLR_3:
		for (unsigned i = 0; i < ARTNET_DMX_LENGTH; i++) {
			m_OutputPorts[3].data[i] = 0;
		}
		m_pLightSet->SetData (3, m_OutputPorts[3].data, m_OutputPorts[3].nLength);
		bClearCommand = true;
		break;
	default:
		break;
	}

	if (bClearCommand && !m_IsLightSetRunning) {
		m_pLightSet->Start();
		m_IsLightSetRunning = true;
	}

	SendPollRelply(true);
}

void ArtNetNode::HandleTimeCode(void) {
	const struct TArtTimeCode *packet = (struct TArtTimeCode *) &(m_ArtNetPacket.ArtPacket.ArtTimeCode);
	m_pArtNetTimeCode->Handler((struct TArtNetTimeCode *)&packet->Frames);
}

void ArtNetNode::SetTimeCodeHandler(ArtNetTimeCode *pArtNetTimeCode) {
	m_pArtNetTimeCode = pArtNetTimeCode;
}

void ArtNetNode::SendTimeCode(const struct TArtNetTimeCode *pArtNetTimeCode) {
	if (pArtNetTimeCode == 0) {
		return;
	}

	if (pArtNetTimeCode->Frames > 29 || pArtNetTimeCode->Hours > 59 || pArtNetTimeCode->Minutes > 59 || pArtNetTimeCode->Seconds > 59 || pArtNetTimeCode->Type > 3 ) {
		return;
	}
	memcpy(&m_TimeCodeData.Frames, pArtNetTimeCode, sizeof (struct TArtNetTimeCode));

	Network::Get()->SendTo((const uint8_t *) &(m_TimeCodeData), (const uint16_t) sizeof(struct TArtTimeCode), m_Node.IPAddressBroadcast, (uint16_t) ARTNET_UDP_PORT);
}

void ArtNetNode::HandleTimeSync(void) {
	struct TArtTimeSync *packet = (struct TArtTimeSync *) &(m_ArtNetPacket.ArtPacket.ArtTimeSync);

	m_pArtNetTimeSync->Handler((struct TArtNetTimeSync *)&packet->tm_sec);

	packet->Prog = (uint8_t) 0;

	Network::Get()->SendTo((const uint8_t *) packet, (const uint16_t) sizeof(struct TArtTimeSync), m_ArtNetPacket.IPAddressFrom, (uint16_t) ARTNET_UDP_PORT);
}

void ArtNetNode::SetTimeSyncHandler(ArtNetTimeSync *pArtNetTimeSync) {
	m_pArtNetTimeSync = pArtNetTimeSync;
}

void ArtNetNode::HandleTodControl(void) {
	const struct TArtTodControl *packet = (struct TArtTodControl *) &(m_ArtNetPacket.ArtPacket.ArtTodControl);
	const uint16_t portAddress = (uint16_t)(packet->Net << 8) | (uint16_t)(packet->Address);

	if ((portAddress == m_OutputPorts[0].port.nPortAddress) && m_OutputPorts[0].bIsEnabled) {
		m_pLightSet->Stop();
		m_IsLightSetRunning = false;

		if (packet->Command == (uint8_t) 0x01) {	// AtcFlush
			m_pArtNetRdm->Full();
		}
		SendTod();

		m_pLightSet->Start();
		m_IsLightSetRunning = true;;
	}
}

void ArtNetNode::HandleTodRequest(void) {
	const struct TArtTodRequest *packet = (struct TArtTodRequest *) &(m_ArtNetPacket.ArtPacket.ArtTodRequest);
	const uint16_t portAddress = (uint16_t)(packet->Net << 8) | (uint16_t)(packet->Address[0]);

	if ((portAddress == m_OutputPorts[0].port.nPortAddress) && m_OutputPorts[0].bIsEnabled) {
		SendTod();
	}
}

void ArtNetNode::SendTod(void) {
	m_pTodData->Net = m_Node.NetSwitch;
	m_pTodData->Address = m_OutputPorts[0].port.nDefaultAddress;

	const uint8_t discovered = m_pArtNetRdm->GetUidCount();

	m_pTodData->UidTotalHi = 0;
	m_pTodData->UidTotalLo = discovered;
	m_pTodData->BlockCount = 0;
	m_pTodData->UidCount = discovered;

	m_pArtNetRdm->Copy((uint8_t *) m_pTodData->Tod);

	const uint16_t length = (uint16_t) sizeof(struct TArtTodData) - (uint16_t) (sizeof m_pTodData->Tod) + (uint16_t) (discovered * 6);

	Network::Get()->SendTo((const uint8_t *) m_pTodData, (const uint16_t) length, m_Node.IPAddressBroadcast, (uint16_t) ARTNET_UDP_PORT);
}

void ArtNetNode::HandleRdm(void) {
	struct TArtRdm *packet = (struct TArtRdm *) &(m_ArtNetPacket.ArtPacket.ArtRdm);
	const uint16_t portAddress = (uint16_t) (packet->Net << 8) | (uint16_t) (packet->Address);

	if ((portAddress == m_OutputPorts[0].port.nPortAddress) && m_OutputPorts[0].bIsEnabled) {

		if (!m_IsRdmResponder) {
			m_pLightSet->Stop();
			m_IsLightSetRunning = false;
		}

		const uint8_t *response = (uint8_t *) m_pArtNetRdm->Handler(packet->RdmPacket);
		if (response != 0) {
			packet->RdmVer = 0x01;

			const uint8_t nMessageLength = response[2] + 1;
			memcpy((uint8_t *) packet->RdmPacket, &response[1], nMessageLength);

			const uint16_t nLength = (uint16_t) sizeof(struct TArtRdm) - (uint16_t) sizeof(packet->RdmPacket) + nMessageLength;

			Network::Get()->SendTo((const uint8_t *) packet, (const uint16_t) nLength, m_ArtNetPacket.IPAddressFrom, (uint16_t) ARTNET_UDP_PORT);
		} else {
			//printf("\n==> No response <==\n");
		}

		if (!m_IsRdmResponder) {
			m_pLightSet->Start();
			m_IsLightSetRunning = true;
		}
	}
}

void ArtNetNode::SetRdmHandler(ArtNetRdm *pArtNetTRdm, bool IsResponder) {
	m_pArtNetRdm = pArtNetTRdm;
	m_IsRdmResponder = IsResponder;

	if (pArtNetTRdm != 0) {
		m_pTodData = new TArtTodData;
		if (m_pTodData != 0) {
			m_Node.Status1 |= STATUS1_RDM_CAPABLE;
			memset(m_pTodData, 0, sizeof(struct TArtTodData));
			memcpy(m_pTodData->Id, (const char *) NODE_ID, sizeof(m_pTodData->Id));
			m_pTodData->OpCode = OP_TODDATA;
			m_pTodData->ProtVerHi = (uint8_t) 0;// high byte of the Art-Net protocol revision number.
			m_pTodData->ProtVerLo = (uint8_t) ARTNET_PROTOCOL_REVISION;	// low byte of the Art-Net protocol revision number.
			m_pTodData->RdmVer = 0x01;// Devices that support RDM STANDARD V1.0 set field to 0x01.
			m_pTodData->Port = 1;
		}
	}
}

void ArtNetNode::HandleIpProg(void) {
	struct TArtIpProg *packet = (struct TArtIpProg *) &(m_ArtNetPacket.ArtPacket.ArtIpProg);

	m_pArtNetIpProg->Handler((const TArtNetIpProg *)&packet->Command, (TArtNetIpProgReply *)&m_pIpProgReply->ProgIpHi);

	Network::Get()->SendTo((const uint8_t *)m_pIpProgReply, (const uint16_t)sizeof(struct TArtIpProgReply), m_ArtNetPacket.IPAddressFrom, (uint16_t)ARTNET_UDP_PORT);

	memcpy(ip.u8, &m_pIpProgReply->ProgIpHi, ARTNET_IP_SIZE);

	if (ip.u32 != m_Node.IPAddressLocal) {
		// Update Node network details
		m_Node.IPAddressLocal = Network::Get()->GetIp();
		m_Node.IPAddressBroadcast = m_Node.IPAddressLocal | ~(Network::Get()->GetNetmask());
		m_Node.Status2 = (m_Node.Status2 & ~(1 << 1)) | (Network::Get()->IsDhcpUsed() ? STATUS2_IP_DHCP : STATUS2_IP_MANUALY);
		// Update PollReply for new IPAddress
		memcpy(m_PollReply.IPAddress, &m_pIpProgReply->ProgIpHi, ARTNET_IP_SIZE);

		if (m_State.SendArtPollReplyOnChange) {
			SendPollRelply(true);
		}
	}
}

void ArtNetNode::SetIpProgHandler(ArtNetIpProg *pArtNetIpProg) {
	m_pArtNetIpProg = pArtNetIpProg;

	if(pArtNetIpProg != 0) {
		m_pIpProgReply = new TArtIpProgReply;
		if (m_pIpProgReply != 0) {
			memset(m_pIpProgReply, 0, sizeof(struct TArtIpProgReply));
			memcpy(m_pIpProgReply->Id, (const char *) NODE_ID, sizeof(m_pIpProgReply->Id));
			m_pIpProgReply->OpCode = OP_IPPROGREPLY;
			m_pIpProgReply->ProtVerHi = (uint8_t) 0;// high byte of the Art-Net protocol revision number.
			m_pIpProgReply->ProtVerLo = (uint8_t) ARTNET_PROTOCOL_REVISION;	// low byte of the Art-Net protocol revision number.
		} else {
			m_pArtNetIpProg = 0;
		}
	}
}


void ArtNetNode::SetNetworkDataLossCondition(void) {
	if(m_IsLightSetRunning) {
		m_pLightSet->Stop();
		m_IsLightSetRunning = false;

		m_State.IsSynchronousMode = false;

		for (unsigned i = 0; i < ARTNET_MAX_PORTS; i++) {
			m_OutputPorts[i].port.nStatus = m_OutputPorts[i].port.nStatus & (~GO_DATA_IS_BEING_TRANSMITTED);
			m_OutputPorts[i].nLength = (uint16_t) 0;
			m_OutputPorts[i].ipA = (uint32_t) 0;
			m_OutputPorts[i].ipB = (uint32_t) 0;
		}
	}
}

int ArtNetNode::HandlePacket(void) {
	const char *packet = (char *)&(m_ArtNetPacket.ArtPacket);
	uint16_t nForeignPort;

	const int nBytesReceived = Network::Get()->RecvFrom((const uint8_t *)packet, (const uint16_t)sizeof(m_ArtNetPacket.ArtPacket), &m_ArtNetPacket.IPAddressFrom, &nForeignPort) ;

	m_nCurrentPacketTime = Hardware::Get()->GetTime();

	if (nBytesReceived == 0) {
		if ((m_nCurrentPacketTime - m_nPreviousPacketTime) >= m_State.nNetworkDataLossTimeout) {
			SetNetworkDataLossCondition();
		}
		return 0;
	}

	m_ArtNetPacket.length = nBytesReceived;
	m_nPreviousPacketTime = m_nCurrentPacketTime;

	GetType();

	if (m_State.IsSynchronousMode) {
		if ((m_ArtNetPacket.OpCode == OP_DMX) && (m_tOpCodePrevious == OP_DMX)) {
			// WiFi UDP : We have missed the OP_SYNC
			m_State.IsSynchronousMode = false;
			for (unsigned i = 0; i < ARTNET_MAX_PORTS; i++) {
				m_OutputPorts[i].IsDataPending = false;
			}
		} else {
			if (m_nCurrentPacketTime - m_State.ArtSyncTime >= 4) {
				m_State.IsSynchronousMode = false;
			}
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
		break;
	}

	if(m_State.SendArtPollReplyOnChange && m_State.IsChanged) {
		SendPollRelply(false);
		m_State.IsChanged = false;
	}

	m_tOpCodePrevious = m_ArtNetPacket.OpCode;

	return m_ArtNetPacket.length;
}
