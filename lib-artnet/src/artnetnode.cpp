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
/* Copyright (C) 2016 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#include <assert.h>
#include <circle/util.h>
#include <circle/time.h>
#include <circle/timer.h>
#include <circle/net/socket.h>
#include <circle/net/ipaddress.h>
#include <circle/net/in.h>
#include <circle/usb/macaddress.h>
#include <circle/logger.h>
#include <circle/version.h>

//#define SENDDIAG

static const char FromArtNetNode[] = "artnetnode";

#include "blinktask.h"
//#include "dmxsend.h"
#include "artnetnode.h"
#include "packets.h"

#include <lightset.h>

#ifndef min
#define min(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifndef max
#define max(a, b) ((a) > (b) ? (a) : (b))
#endif

#define bytes_to_short(h,l) ( ((h << 8) & 0xff00) | (l & 0x00FF) );

#define ARTNET_PROTOCOL_REVISION	14							///< Art-Net 3 Protocol Release V1.4 Document Revision 1.4bk 23/1/2016

#define NODE_ID						"Art-Net"					///< Array of 8 characters, the final character is a null termination. Value = A r t - N e t 0x00
#define NODE_UDP_PORT				0x1936						///< The Port is always 0x1936

#define NODE_DEFAULT_SHORT_NAME		"AvV Art-Net Node"			///< The array represents a null terminated short name for the Node.
#define NODE_DEFAULT_LONG_NAME		"Raspberry Pi Art-Net 3 Node DMX Out"	///< The array represents a null terminated long name for the Node.
#define NODE_DEFAULT_NET_SWITCH		0							///<
#define NODE_DEFAULT_SUBNET_SWITCH	0							///<
#define NODE_DEFAULT_UNIVERSE		0							///<

static const uint8_t DEVICE_MANUFACTURER_ID[] = { 0x7F, 0xF0 };	///< 0x7F, 0xF0 : RESERVED FOR PROTOTYPING/EXPERIMENTAL USE ONLY
static const uint8_t DEVICE_SOFTWARE_VERSION[] = {0x01, 0x00 };	///<
static const uint8_t DEVICE_OEM_VALUE[] = { 0x20, 0xE0 };		///< OemArtRelay , 0x00FF = developer code

#define ARTNET_MIN_HEADER_SIZE		12							///< \ref TArtPoll \ref TArtSync
#define MERGE_TIMEOUT_SECONDS		10							///<

#define PORT_IN_STATUS_DISABLED_MASK	0x08

static 	CIPAddress IPAddressFrom;
/**
 *
 * @param pNet
 * @param pDmx
 */
ArtNetNode::ArtNetNode(CNetSubSystem *pNet, LightSet *pOutput, CActLED *pActLED) :
		m_pNet(pNet), m_Socket(m_pNet, IPPROTO_UDP), m_IsDHCPUsed(true), m_pLightSet(pOutput) {

	m_pBlinkTask = new CBlinkTask (pActLED, 1);

	memset(&m_Node, 0, sizeof (struct TArtNetNode));

	// Factory node configuration
	SetShortName((const char *)NODE_DEFAULT_SHORT_NAME);
	SetLongName((const char *)NODE_DEFAULT_LONG_NAME);
	SetNetSwitch(NODE_DEFAULT_NET_SWITCH);
	SetSubnetSwitch(NODE_DEFAULT_SUBNET_SWITCH);
	SetUniverseSwitch(0, ARTNET_OUTPUT_PORT, NODE_DEFAULT_UNIVERSE);

	// There is one DMX out port, there are no DMX in port's.

	for (int i = 0; i < ARTNET_NODE_MAX_PORTS; i++) {
		m_OutputPorts[i].port.nStatus = (uint8_t) 0;
		m_OutputPorts[i].port.nPortAddress = (uint16_t) 0;
		m_OutputPorts[i].port.nDefaultAddress = (uint8_t) 0;
		m_OutputPorts[i].mergeMode = ARTNET_MERGE_HTP;
		m_OutputPorts[i].bIsEnabled = false;
		m_OutputPorts[i].nLength = (uint16_t) 0;
		m_OutputPorts[i].ipA = (uint32_t) 0;
		m_OutputPorts[i].ipB = (uint32_t) 0;
	}

	m_Node.Status1 = STATUS1_INDICATOR_NORMAL_MODE | STATUS1_PAP_FRONT_PANEL;
	SetNetworkDetails();					// m_Node.status2 is set here

	BuildPollReply();
	FillDiagData();

	m_State.IsSynchronousMode = false;
	m_State.IsDataPending = false;
	m_State.SendArtDiagData = false;
	m_State.IsMergeMode = false;
	m_State.IsChanged = false;
	m_State.SendArtPollReplyOnChange = false;
	m_State.ArtPollReplyCount = (uint32_t)0;
	m_State.IPAddressArtPoll = (uint32_t)0;
	m_State.IsMultipleControllersReqDiag = false;
	m_State.reportCode = ARTNET_RCPOWEROK;
	m_State.status = ARTNET_STANDBY;
}

/**
 *
 */
ArtNetNode::~ArtNetNode(void) {
	memset(&m_Node, 0, sizeof (struct TArtNetNode));
	memset(&m_PollReply, 0, sizeof (struct TArtPollReply));
	memset(&m_DiagData, 0, sizeof (struct TArtDiagData));
}

const uint8_t *ArtNetNode::GetSoftwareVersion(void) {
	return DEVICE_SOFTWARE_VERSION;
}

/**
 *
 */
void ArtNetNode::Start(void) {
	if (!m_Socket.Bind(NODE_UDP_PORT) < 0) {
		CLogger::Get()->Write(FromArtNetNode, LogPanic, "Cannot bind socket (port %u)", NODE_UDP_PORT);
	} else {
		m_State.status = ARTNET_ON;
	}

	SendPollRelply(false);				// send a reply on startup
}

/**
 *
 */
void ArtNetNode::Stop(void) {
	m_pBlinkTask->SetFrequency(0);
	m_State.status = ARTNET_OFF;
}

/**
 *
 * @param nPortIndex
 * @return
 */
const uint8_t ArtNetNode::GetUniverseSwitch(const uint8_t nPortIndex) {

	if (nPortIndex >= ARTNET_MAX_PORTS) {
		CLogger::Get()->Write(FromArtNetNode, LogError, "Port index out of bounds (%d < 0 || %d > ARTNET_MAX_PORTS)", nPortIndex, nPortIndex);
		return ARTNET_EARG;
	}

	return m_OutputPorts[nPortIndex].port.nDefaultAddress;
}

/**
 *
 * @param id
 * @param dir \ref TArtNetPortDir
 * @param nAddress
 * @return
 */
int ArtNetNode::SetUniverseSwitch(const uint8_t nPortIndex, const TArtNetPortDir dir, const uint8_t nAddress) {

	if (nPortIndex >= ARTNET_MAX_PORTS) {
		CLogger::Get()->Write(FromArtNetNode, LogError, "Port index out of bounds (%d < 0 || %d > ARTNET_MAX_PORTS)", nPortIndex, nPortIndex);
		return ARTNET_EARG;
	}

	if (dir == ARTNET_INPUT_PORT) {
		// Not supported. We have output ports only.
	} else if (dir == ARTNET_OUTPUT_PORT) {
		m_OutputPorts[nPortIndex].bIsEnabled = true;
	} else {
		CLogger::Get()->Write(FromArtNetNode, LogError, "Attempt to set port %d to invalid address %d", nPortIndex, nAddress);
		return ARTNET_EARG;
	}

	m_OutputPorts[nPortIndex].port.nDefaultAddress = nAddress & (uint16_t)0x0F;		// Universe : Bits 3-0
	m_OutputPorts[nPortIndex].port.nPortAddress = MakePortAddress((uint16_t)nAddress);

	return ARTNET_EOK;
}

/**
 *
 * @return
 */
const uint8_t ArtNetNode::GetSubnetSwitch(void) {
	return m_Node.SubSwitch;
}

/**
 * Sub-Net: A group of 16 consecutive universes is referred to as a sub-net. (Not to be confused with the subnet mask).
 *
 * The Sub-Net address is between 0 and 15. If the supplied address is larger than 15,
 * the lower 4 bits will be used in setting the address.
 *
 * @param nAddress
 */
void ArtNetNode::SetSubnetSwitch(const uint8_t nAddress) {

	m_Node.SubSwitch = nAddress;

	for (int i = 0; i < ARTNET_NODE_MAX_PORTS; i++) {
		m_OutputPorts[i].port.nPortAddress = MakePortAddress(m_OutputPorts[i].port.nPortAddress);
	}
}

/**
 *
 * @return
 */
const uint8_t ArtNetNode::GetNetSwitch(void) {
	return m_Node.NetSwitch;
}

/**
 *
 * @param nAddress
 */
void ArtNetNode::SetNetSwitch(const uint8_t nAddress) {
	m_Node.NetSwitch = nAddress;

	for (int i = 0; i < ARTNET_NODE_MAX_PORTS; i++) {
		m_OutputPorts[i].port.nPortAddress = MakePortAddress(m_OutputPorts[i].port.nPortAddress);
	}
}

/**
 *
 * @return
 */
const char *ArtNetNode::GetShortName(void) {
	return (const char *)m_Node.ShortName;
}

/**
 * Sets the short name of the node.
 * The string should be null terminated and a maximum of 18 Characters will be used
 *
 * @param pName the short name of the node.
 */
void ArtNetNode::SetShortName(const char *pName) {
	if (pName == 0) {
		return;
	}

	memset((void *)m_Node.ShortName, 0, ARTNET_SHORT_NAME_LENGTH);
	strncpy((char *) m_Node.ShortName, pName, ARTNET_SHORT_NAME_LENGTH);
	m_Node.ShortName[ARTNET_SHORT_NAME_LENGTH-1] = '\0';
}

const char *ArtNetNode::GetLongName(void) {
	return (const char *)m_Node.LongName;
}

/**
 * Sets the long name of the node.
 * The string should be null terminated and a maximum of 64 Characters will be used
 *
 * @param pName the long name of the node.
 */
void ArtNetNode::SetLongName(const char *pName) {
	if (pName == 0) {
		return;
	}

	memset((void *)m_Node.LongName, 0, ARTNET_LONG_NAME_LENGTH);
	strncpy((char *) m_Node.LongName, pName, ARTNET_LONG_NAME_LENGTH);
	m_Node.ShortName[ARTNET_LONG_NAME_LENGTH-1] = '\0';
}

/**
 *
 */
void ArtNetNode::SetNetworkDetails(void) {
	m_pNet->GetNetDeviceLayer ()->GetMACAddress ()->CopyTo (m_Node.MACAddressLocal);	// the mac address of the node
	m_IsDHCPUsed = m_pNet->GetConfig()->IsDHCPUsed();									// Used for field status2 in PollReply
	m_pNet->GetConfig()->GetIPAddress()->CopyTo(m_Node.IPAddressLocal);					// the IP address of the node
	m_pNet->GetConfig()->GetBroadcastAddress()->CopyTo(m_Node.IPAddressBroadcast);		// broadcast IP address
	m_pNet->GetConfig()->GetDefaultGateway()->CopyTo(m_Node.IPDefaultGateway);			// gateway IP address

	memcpy (m_Node.IPSubnetMask, m_pNet->GetConfig()->GetNetMask(), 4);		// network mask (Art-Net use 'A' network type)
	const u8 bit1 = (m_IsDHCPUsed ? 1 : 0) << 1;	// Bit 1 : Clr = Node’s IP is manually configured , Set = Node’s IP is DHCP configured.
	m_Node.Status2 = bit1 | 1 << 2 | 1 << 3;		// Bit 2 : Set : Node is DHCP capable
													// Bit 3 : Set : Node supports 15 bit Port-Address (Art-Net 3)
}

/**
 *
 * @param nCurrentAddress
 * @return
 */
uint16_t ArtNetNode::MakePortAddress(const uint16_t nCurrentAddress) {
	// PortAddress Bit 15 = 0
	uint16_t newAddress = (m_Node.NetSwitch & 0x7F) << 8;	// Net : Bits 14-8
	newAddress |= (m_Node.SubSwitch & (uint8_t)0x0F) << 4;	// Sub-Net : Bits 7-4
	newAddress |= nCurrentAddress & (uint16_t)0x0F;		// Universe : Bits 3-0

	return newAddress;
}

/**
 *
 */
void ArtNetNode::BuildPollReply(void) {
	memset(&m_PollReply, 0, sizeof (struct TArtPollReply));

	memcpy (m_PollReply.Id, (const char *)NODE_ID, sizeof m_PollReply.Id);
	m_PollReply.OpCode = OP_POLLREPLY;
	memcpy (m_PollReply.IPAddress, m_Node.IPAddressLocal, sizeof m_PollReply.IPAddress);
	m_PollReply.Port = (uint16_t)NODE_UDP_PORT;
	m_PollReply.VersInfoH = DEVICE_SOFTWARE_VERSION[0];
	m_PollReply.VersInfoL = DEVICE_SOFTWARE_VERSION[1];
	m_PollReply.NetSwitch = m_Node.NetSwitch;
	m_PollReply.SubSwitch = m_Node.SubSwitch;
	m_PollReply.OemHi = DEVICE_OEM_VALUE[0];
	m_PollReply.Oem = DEVICE_OEM_VALUE[1];
	m_PollReply.Status1 = m_Node.Status1;
	m_PollReply.EstaMan[0] = DEVICE_MANUFACTURER_ID[1];
	m_PollReply.EstaMan[1] = DEVICE_MANUFACTURER_ID[0];
	memcpy (m_PollReply.ShortName, m_Node.ShortName, sizeof m_PollReply.ShortName);
	memcpy (m_PollReply.LongName, m_Node.LongName, sizeof m_PollReply.LongName);
	// There is one DMX out port, there are no DMX in port's.
	m_PollReply.NumPortsLo = ARTNET_NODE_MAX_PORTS;
	m_PollReply.PortTypes[0] = ARTNET_ENABLE_OUTPUT | ARTNET_PORT_DMX;
	for (int i = 0 ; i < ARTNET_MAX_PORTS; i++) {
		m_PollReply.GoodInput[i] = PORT_IN_STATUS_DISABLED_MASK;
	}
	m_PollReply.GoodOutput[0] = m_OutputPorts[0].port.nStatus;
	m_PollReply.SwOut[0] = m_OutputPorts[0].port.nDefaultAddress;
	m_PollReply.Style = ARTNET_ST_NODE;
	memcpy (m_PollReply.MAC, m_Node.MACAddressLocal, sizeof m_PollReply.MAC);
	m_PollReply.Status2 = m_Node.Status2;
}

/**
 *
 */
void ArtNetNode::FillDiagData(void) {
	memset(&m_DiagData, 0, sizeof (struct TArtDiagData));

	strncpy((char *)m_DiagData.Id, (const char *)NODE_ID, sizeof m_DiagData.Id);
	m_DiagData.OpCode = OP_DIAGDATA;
	m_DiagData.ProtVerHi = (uint8_t) 0;						// high byte of the Art-Net protocol revision number.
	m_DiagData.ProtVerLo = (uint8_t) ARTNET_PROTOCOL_REVISION;	// low byte of the Art-Net protocol revision number.
}

/**
 *
 */
void ArtNetNode::GetType(void) {
	char *data = (char *)&(m_ArtNetPacket.ArtPacket);

	if (m_ArtNetPacket.length < ARTNET_MIN_HEADER_SIZE) {
		m_ArtNetPacket.OpCode = OP_NOT_DEFINED;
	}

	if (memcmp(data, "Art-Net\0", 8) == 0) {
		m_ArtNetPacket.OpCode = (TOpCodes)((data[9] << 8) + data[8]);
	} else {
		m_ArtNetPacket.OpCode = OP_NOT_DEFINED;
	}
}

/**
 *
 * @return
 */
int ArtNetNode::HandlePacket(void) {
	const char *packet = (char *)&(m_ArtNetPacket.ArtPacket);

	u16	nForeignPort;
	const int nBytesReceived = m_Socket.ReceiveFrom ((void *)packet, sizeof m_ArtNetPacket.ArtPacket, MSG_DONTWAIT, &IPAddressFrom, &nForeignPort);

	if (nBytesReceived < 0) {
		CLogger::Get()->Write(FromArtNetNode, LogPanic, "Cannot receive");
		return nBytesReceived;
	}

	m_ArtNetPacket.length = nBytesReceived;

	if (nBytesReceived == 0) {
		m_ArtNetPacket.IPAddressFrom = 0;
		return 0;
	}

	m_ArtNetPacket.IPAddressFrom = IPAddressFrom;

	if (m_State.IsSynchronousMode) {
		const time_t now = CTimer::Get ()->GetTime ();
		if (now - m_State.ArtSyncTime >= 4) {
			m_State.IsSynchronousMode = false;
			SendDiag("Leaving Synchronous Mode", ARTNET_DP_LOW);
		}
	}

	GetType();

	switch (m_ArtNetPacket.OpCode) {
	case OP_POLL:
		HandlePoll();
		break;
	case OP_DMX:
		HandleDmx();
		break;
	case OP_SYNC:
		HandleSync();
		break;
	case OP_ADDRESS:
		HandleAddress();
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

	return m_ArtNetPacket.length;
}

/**
 *
 * @param response
 */
void ArtNetNode::SendPollRelply(const bool bResponse) {

	if (!bResponse && m_State.status == ARTNET_ON) {
		m_State.ArtPollReplyCount++;
	}

	BuildPollReply();

	CString Report;
	Report.Format("%04x [%04d] RPi AvV " CIRCLE_NAME " " CIRCLE_VERSION_STRING, m_State.reportCode, m_State.ArtPollReplyCount);
	strncpy((char *)m_PollReply.NodeReport, (const char *)Report, Report.GetLength() < ARTNET_REPORT_LENGTH ? Report.GetLength() : ARTNET_REPORT_LENGTH); //

	CIPAddress BroadcastIP;
	BroadcastIP.Set (m_Node.IPAddressBroadcast);

	if ((m_Socket.SendTo((const void *)&(m_PollReply), (unsigned)sizeof (struct TArtPollReply), MSG_DONTWAIT, BroadcastIP, (u16)NODE_UDP_PORT)) != sizeof (struct TArtPollReply)) {
		CLogger::Get()->Write(FromArtNetNode, LogPanic, "Cannot send");
	}
}

/**
 *
 * @param text
 * @param priority
 */
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

	if ((m_Socket.SendTo((const void *)&(m_DiagData), size, MSG_DONTWAIT, m_State.IPAddressDiagSend, (u16)NODE_UDP_PORT)) != (int)size) {
		CLogger::Get()->Write(FromArtNetNode, LogError, "Cannot send");
	}
}

/**
 *
 * @param p
 * @param data_length
 * @return
 */
bool ArtNetNode::IsDmxDataChanged(const uint8_t *p, const uint16_t nLength) {
	bool isChanged = false;

	uint8_t *src = (uint8_t *)p;
	uint8_t *dst = (uint8_t *)m_OutputPorts[0].data;

	if (nLength != m_OutputPorts[0].nLength) {
		m_OutputPorts[0].nLength = nLength;
		for (int i = 0 ; i < ARTNET_DMX_LENGTH; i++) {
			*dst++ = *src++;
		}
		return true;
	}

	for (int i = 0; i < ARTNET_DMX_LENGTH; i++) {
		if (*dst != *src) {
			*dst = *src;
			isChanged = true;
		}
		dst++;
		src++;
	}

	return isChanged;
}

/**
 * merge the data from two sources
 * @param p
 * @param data_length
 * @return
 */
bool ArtNetNode::IsMergedDmxDataChanged(const uint8_t *p, const uint16_t nLength) {
	bool isChanged = false;

	if (!m_State.IsMergeMode) {
		m_State.IsMergeMode = true;
		m_State.IsChanged = true;
		uint8_t nStatus = m_OutputPorts[0].port.nStatus;
		m_OutputPorts[0].port.nStatus = nStatus | (1 << 3);	// Bit 3 : Set – Output is merging ArtNet data.
	}


	if (m_OutputPorts[0].mergeMode == ARTNET_MERGE_HTP) {

		if (nLength != m_OutputPorts[0].nLength) {
			m_OutputPorts[0].nLength = nLength;
			for (int i = 0; i < nLength; i++) {
				uint8_t data = max(m_OutputPorts[0].dataA[i], m_OutputPorts[0].dataB[i]);
				m_OutputPorts[0].data[i] = data;
			}
			return true;
		}

		for (int i = 0; i < nLength; i++) {
			uint8_t data = max(m_OutputPorts[0].dataA[i], m_OutputPorts[0].dataB[i]);
			if (data != m_OutputPorts[0].data[i]) {
				m_OutputPorts[0].data[i] = data;
				isChanged = true;
			}
		}
		return isChanged;
	} else {
		return IsDmxDataChanged(p, nLength);
	}
}

/**
 *
 */
void ArtNetNode::CheckMergeTimeouts(void) {
	const time_t now = CTimer::Get()->GetTime();
	const time_t timeOutA = now - m_OutputPorts[0].timeA;
	const time_t timeOutB = now - m_OutputPorts[0].timeB;

	if (timeOutA > (time_t)MERGE_TIMEOUT_SECONDS) {
		m_OutputPorts[0].ipA = 0;
		m_State.IsMergeMode = false;
	}

	if (timeOutB > (time_t)MERGE_TIMEOUT_SECONDS) {
		m_OutputPorts[0].ipB = 0;
		m_State.IsMergeMode = false;
	}

	if (!m_State.IsMergeMode) {
		m_State.IsChanged = true;
		uint8_t nStatus = m_OutputPorts[0].port.nStatus;
		m_OutputPorts[0].port.nStatus = nStatus & ~GO_OUTPUT_IS_MERGING;
#ifdef SENDDIAG
		SendDiag("Leaving Merging Mode", ARTNET_DP_LOW);
#endif
	}
}

/**
 *
 */
void ArtNetNode::HandlePoll(void) {
	const struct TArtPoll *packet = (struct TArtPoll *)&(m_ArtNetPacket.ArtPacket.ArtPoll);

	if (packet->ProtVerLo != (uint8_t) ARTNET_PROTOCOL_REVISION) {
		return;
	}

	if (packet->TalkToMe & TTM_SEND_ARP_ON_CHANGE) {
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
			m_State.IPAddressDiagSend.Set(m_Node.IPAddressBroadcast);
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
			m_State.IPAddressDiagSend.Set(m_ArtNetPacket.IPAddressFrom);
		} else {
			m_State.IPAddressDiagSend.Set(m_Node.IPAddressBroadcast);
		}
	} else {
		m_State.SendArtDiagData = false;
		m_State.IPAddressDiagSend.Set((u32) 0);
	}

	SendPollRelply(true);
}

/**
 *
 */
void ArtNetNode::HandleDmx(void) {
	const struct TArtDmx *packet = (struct TArtDmx *)&(m_ArtNetPacket.ArtPacket.ArtDmx);

	if (packet->ProtVerLo != (uint8_t) ARTNET_PROTOCOL_REVISION) {
		return;
	}

	int data_length = (int) bytes_to_short(packet->LengthHi, packet->Length);
	data_length = min(data_length, ARTNET_DMX_LENGTH);

	if (packet->PortAddress == m_OutputPorts[0].port.nPortAddress) {
		uint32_t ipA = m_OutputPorts[0].ipA;
		uint32_t ipB = m_OutputPorts[0].ipB;

		bool sendNewData = false;

		m_OutputPorts[0].port.nStatus = m_OutputPorts[0].port.nStatus | GO_DATA_IS_BEING_TRANSMITTED;

		if (m_State.IsMergeMode) {
			CheckMergeTimeouts();
		}

		if (ipA == 0 && ipB == 0) {
#ifdef SENDDIAG
			SendDiag("1. first packet recv on this port", ARTNET_DP_LOW);
#endif
			m_OutputPorts[0].ipA = m_ArtNetPacket.IPAddressFrom;
			m_OutputPorts[0].timeA = CTimer::Get ()->GetTime ();

			memcpy(&m_OutputPorts[0].dataA, packet->Data, data_length);

			sendNewData = IsDmxDataChanged(packet->Data, data_length);

		} else if (ipA == m_ArtNetPacket.IPAddressFrom && ipB == 0) {
#ifdef SENDDIAG
			SendDiag("2. continued transmission from the same ip (source A)", ARTNET_DP_LOW);
#endif
			m_OutputPorts[0].timeA = CTimer::Get ()->GetTime ();

			memcpy(&m_OutputPorts[0].dataA, packet->Data, data_length);

			sendNewData = IsDmxDataChanged(packet->Data, data_length);

		} else if (ipA == 0 && ipB == m_ArtNetPacket.IPAddressFrom) {
#ifdef SENDDIAG
			SendDiag("3. continued transmission from the same ip (source B)", ARTNET_DP_LOW);
#endif
			m_OutputPorts[0].timeB = CTimer::Get ()->GetTime ();

			memcpy(&m_OutputPorts[0].dataB, packet->Data, data_length);

			sendNewData = IsDmxDataChanged(packet->Data, data_length);

		} else if (ipA != m_ArtNetPacket.IPAddressFrom && ipB == 0) {
#ifdef SENDDIAG
			SendDiag("4. new source, start the merge", ARTNET_DP_LOW);
#endif
			m_OutputPorts[0].ipB = m_ArtNetPacket.IPAddressFrom;
			m_OutputPorts[0].timeB = CTimer::Get ()->GetTime ();

			memcpy(&m_OutputPorts[0].dataB, packet->Data, data_length);

			// merge, newest data is port B
			sendNewData = IsMergedDmxDataChanged(m_OutputPorts[0].dataB, data_length);

		} else if (ipA == 0 && ipB == m_ArtNetPacket.IPAddressFrom) {
#ifdef SENDDIAG
			SendDiag("5. new source, start the merge", ARTNET_DP_LOW);
#endif
			m_OutputPorts[0].ipA = m_ArtNetPacket.IPAddressFrom;
			m_OutputPorts[0].timeB = CTimer::Get ()->GetTime ();

			memcpy(&m_OutputPorts[0].dataB, packet->Data, data_length);
			// merge, newest data is port A
			sendNewData = IsMergedDmxDataChanged(m_OutputPorts[0].dataA, data_length);

		} else if (ipA == m_ArtNetPacket.IPAddressFrom && ipB != m_ArtNetPacket.IPAddressFrom) {
#ifdef SENDDIAG
			SendDiag("6. continue merge", ARTNET_DP_LOW);
#endif
			m_OutputPorts[0].timeA = CTimer::Get ()->GetTime ();

			memcpy(&m_OutputPorts[0].dataA, packet->Data, data_length);
			// merge, newest data is port A
			sendNewData = IsMergedDmxDataChanged(m_OutputPorts[0].dataA, data_length);

		} else if (ipA != m_ArtNetPacket.IPAddressFrom && ipB == m_ArtNetPacket.IPAddressFrom) {
#ifdef SENDDIAG
			SendDiag("7. continue merge", ARTNET_DP_LOW);
#endif
			m_OutputPorts[0].timeB = CTimer::Get ()->GetTime ();

			memcpy(&m_OutputPorts[0].dataB, packet->Data, data_length);
			 // merge, newest data is port B
			sendNewData = IsMergedDmxDataChanged(m_OutputPorts[0].dataB, data_length);

		} else if (ipA == m_ArtNetPacket.IPAddressFrom && ipB == m_ArtNetPacket.IPAddressFrom) {
			SendDiag("8. Source matches both buffers, this shouldn't be happening!", ARTNET_DP_LOW);
			return;
		} else if (ipA != m_ArtNetPacket.IPAddressFrom && ipB != m_ArtNetPacket.IPAddressFrom) {
			SendDiag("9. More than two sources, discarding data", ARTNET_DP_LOW);
			return;
		} else {
			SendDiag("0. No cases matched, this shouldn't happen!",	ARTNET_DP_LOW);
			return;
		}

		if (sendNewData) {
			if (!m_State.IsSynchronousMode) {
#ifdef SENDDIAG
				SendDiag("Send new data", ARTNET_DP_LOW);
#endif
				//m_DMX->SetData(m_OutputPorts[0].data, m_OutputPorts[0].nLength);
				m_pLightSet->SetData(m_OutputPorts[0].data, m_OutputPorts[0].nLength);
			} else {
#ifdef SENDDIAG
				SendDiag("DMX data pending", ARTNET_DP_LOW);
#endif
				m_State.IsDataPending = true;
			}
		} else {
#ifdef SENDDIAG
			SendDiag("Data not changed", ARTNET_DP_LOW);
#endif
		}
	}
}

/**
 * Handle ArtSync packet
 *
 * When a node receives an ArtSync packet it should transfer to synchronous operation.
 * This means that received ArtDmx packets will be buffered and output when the next ArtSync is received.
 *
 * In order to allow transition between synchronous and non-synchronous modes,
 * a node shall time out to non-synchronous operation if an ArtSync is not received for 4 seconds or more.
 */
void ArtNetNode::HandleSync(void) {
	const struct TArtSync *packet = (struct TArtSync*)&(m_ArtNetPacket.ArtPacket.ArtSync);

	if (packet->ProtVerLo != (uint8_t) ARTNET_PROTOCOL_REVISION) {
		return;
	}

	m_State.IsSynchronousMode = true;
	m_State.ArtSyncTime = CTimer::Get ()->GetTime ();

	if (m_State.IsDataPending) {
#ifdef SENDDIAG
		SendDiag("Send pending data", ARTNET_DP_LOW);
#endif
		//m_DMX->SetData (m_OutputPorts[0].data, m_OutputPorts[0].nLength);
		m_pLightSet->SetData (m_OutputPorts[0].data, m_OutputPorts[0].nLength);
		m_State.IsDataPending = false;
	}
}

/**
 * A Controller or monitoring device on the network can reprogram numerous controls of a node remotely.
 */
void ArtNetNode::HandleAddress(void) {
	const struct TArtAddress *packet = (struct TArtAddress *) &(m_ArtNetPacket.ArtPacket.ArtAddress);

	if (packet->ProtVerLo != (uint8_t) ARTNET_PROTOCOL_REVISION) {
		return;
	}

	m_State.reportCode = ARTNET_RCPOWEROK;

	if ((packet->ShortName[0] != PROGRAM_NO_CHANGE) && (packet->ShortName[0] != PROGRAM_DEFAULTS)) {
		memcpy(m_Node.ShortName, (const char *)packet->ShortName, ARTNET_SHORT_NAME_LENGTH);
		m_State.reportCode = ARTNET_RCSHNAMEOK;
	} else if (packet->ShortName[0] == PROGRAM_DEFAULTS) {
		SetShortName((const char *)NODE_DEFAULT_SHORT_NAME);
		m_State.reportCode = ARTNET_RCSHNAMEOK;
	}

	if ((packet->LongName[0] != PROGRAM_NO_CHANGE) && (packet->LongName[0] != PROGRAM_DEFAULTS)) {
		memcpy(m_Node.LongName, (const char *)packet->LongName, ARTNET_LONG_NAME_LENGTH);
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

	if (packet->SwOut[0] == PROGRAM_DEFAULTS) {
		SetUniverseSwitch(0, ARTNET_OUTPUT_PORT, NODE_DEFAULT_UNIVERSE);
	} else if (packet->SwOut[0] & PROGRAM_CHANGE_MASK) {
		SetUniverseSwitch(0, ARTNET_OUTPUT_PORT, packet->SwOut[0] & ~PROGRAM_CHANGE_MASK);
	}

	switch (packet->Command) {
	case ARTNET_PC_CANCEL:
		// If Node is currently in merge mode, cancel merge mode upon receipt of next ArtDmx packet.
		m_State.IsMergeMode = false;
		m_OutputPorts[0].port.nStatus = m_OutputPorts[0].port.nStatus & ~GO_OUTPUT_IS_MERGING;
#ifdef SENDDIAG
		SendDiag("Leaving Merging Mode", ARTNET_DP_LOW);
#endif
		break;
	case ARTNET_PC_LED_NORMAL:
		m_pBlinkTask->SetFrequency(1);
		break;
	case ARTNET_PC_LED_MUTE:
		m_pBlinkTask->SetFrequency(0);
		break;
	case ARTNET_PC_LED_LOCATE:
		m_pBlinkTask->SetFrequency(3);
		break;
	case ARTNET_PC_MERGE_LTP_O:
		m_OutputPorts[0].mergeMode = ARTNET_MERGE_LTP;
		m_OutputPorts[0].port.nStatus = m_OutputPorts[0].port.nStatus | GO_MERGE_MODE_LTP;
#ifdef SENDDIAG
		SendDiag("Setting Merge Mode LTP", ARTNET_DP_LOW);
#endif
		break;
	case ARTNET_PC_MERGE_HTP_0:
		m_OutputPorts[0].mergeMode = ARTNET_MERGE_HTP;
		m_OutputPorts[0].port.nStatus = m_OutputPorts[0].port.nStatus & ~GO_MERGE_MODE_LTP;
#ifdef SENDDIAG
		SendDiag("Setting Merge Mode HTP", ARTNET_DP_LOW);
#endif
		break;
	case ARTNET_PC_CLR_0:
		for (int i = 0; i < ARTNET_DMX_LENGTH; i++) {
			m_OutputPorts[0].data[i] = 0;
		}
		//m_DMX->SetData (m_OutputPorts[0].data, m_OutputPorts[0].nLength);
		m_pLightSet->SetData (m_OutputPorts[0].data, m_OutputPorts[0].nLength);
		break;
	default:
		break;
	}

	SendPollRelply(true);
}
