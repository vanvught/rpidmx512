/**
 * @file artnetnode.h
 *
 */
/**
 * Art-Net Designed by and Copyright Artistic Licence Holdings Ltd.
 *
 * Art-Net 3 Protocol Release V1.4 Document Revision 1.4bk 23/1/2016
 *
 */
/* Copyright (C) 2016, 2017 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#ifndef ARTNETNODE_H_
#define ARTNETNODE_H_

#include <stdint.h>

#if defined (__circle__)
#include <circle/time.h>
#else
#include <time.h>
#endif

#include "artnet.h"
#include "packets.h"
#include "common.h"

#include "lightset.h"
#include "ledblink.h"

#include "artnettimecode.h"
#include "artnettimesync.h"
#include "artnetrdm.h"

/**
 * ArtPollReply packet, Field 12
 */
enum TStatus1 {
	STATUS1_INDICATOR_MASK = (3 << 6),			///< 0b11 bit 7-6, Indicator state.
	STATUS1_INDICATOR_LOCATE_MODE = (1 << 6),	///< 0b01 Indicators in Locate Mode.
	STATUS1_INDICATOR_MUTE_MODE = (2 << 6),		///< 0b10 Indicators in Mute Mode.
	STATUS1_INDICATOR_NORMAL_MODE = (3 << 6),	///< 0b11 Indicators in Normal Mode.
	STATUS1_PAP_MASK = (3 << 4),				///< 0b11 bit 5-4, Port Address Programming Authority
	STATUS1_PAP_UNKNOWN = (0 << 4),				///< 0b00 Port Address Programming Authority unknown.
	STATUS1_PAP_FRONT_PANEL = (1 << 4),			///< 0b01 All Port Address set by front panel controls.
	STATUS1_PAP_NETWORK = (2 << 4),				///< 0b10 All or part of Port Address programmed by network or Web browser.
	STATUS1_PAP_NOTUSED = (3 << 4),				///< 0b11 Not used.
	STATUS1_NORMAL_FIRMWARE_BOOT = (0 << 2),	///< 0 = Normal firmware boot (from flash).
	STATUS1_ROM_BOOT = (1 << 2),				///< 1 = Booted from ROM.
	STATUS1_RDM_CAPABLE = (1 << 1),				///< 1 = Capable of Remote Device Management
	STATUS1_UBEA_PRESENT = (1 << 0)				///< 1 = UBEA present
};

/**
 * ArtPollReply packet, Field 40
 */
enum TStatus2 {
	STATUS2_WEB_BROWSER_SUPPORT = (1 << 0),		///< Bit 0, Set = Product supports web browser configuration
	STATUS2_IP_MANUALY = (0 << 1),				///< Bit 1, Clr = Node’s IP is manually configured.
	STATUS2_IP_DHCP = (1 << 1),					///< Bit 1, Set = Node’s IP is DHCP configured.
	STATUS2_DHCP_NOT_CAPABLE = (0 << 2),		///< Bit 2, Clr = Node is not DHCP capable.
	STATUS2_DHCP_CAPABLE = (1 << 2),			///< Bit 2, Set = Node is DHCP capable.
	STATUS2_PORT_ADDRESS_8BIT = (0 << 3),		///< Bit 3, Clr = Node supports 8 bit Port-Address (Art-Net II).
	STATUS2_PORT_ADDRESS_15BIT = (1 << 3),		///< Bit 3, Set = Node supports 15 bit Port-Address (Art-Net 3 or 4).
	STATUS2_SACN_NO_SWITCH = (0 << 4),			///< Bit 4, Clr = Node not able to switch between Art-Net and sACN.
	STATUS2_SACN_ABLE_TO_SWITCH = (1 << 4)		///< Bit 4, Set = Node is able to switch between Art-Net and sACN.
};

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

/**
 *
 */
enum TTalkToMe {
	TTM_SEND_ARP_ON_CHANGE = (1 << 1),			///< Bit 1 set : Send ArtPollReply whenever Node conditions change.
	TTM_SEND_DIAG_MESSAGES = (1 << 2),			///< Bit 2 set : Send me diagnostics messages.
	TTM_SEND_DIAG_UNICAST = (1 << 3)			///< Bit 3 : 0 = Diagnostics messages are broadcast. (if bit 2).
												///< Bit 3 : 1 = Diagnostics messages are unicast. (if bit 2).
};

/**
 *
 */
struct TArtNetNodeState {
	bool SendArtPollReplyOnChange;				///< ArtPoll : TalkToMe Bit 1 : 1 = Send ArtPollReply whenever Node conditions change.
	uint32_t ArtPollReplyCount;					///< ArtPollReply : NodeReport : decimal counter that increments every time the Node sends an ArtPollResponse.
	bool SendArtDiagData;						///< ArtPoll : TalkToMe Bit 2 : 1 = Send me diagnostics messages.
	uint8_t Priority;							///< ArtPoll : Field 6 : The lowest priority of diagnostics message that should be sent.
	uint32_t IPAddressDiagSend;					///< ArtPoll : Destination IPAddress for the ArtDiag
	uint32_t IPAddressArtPoll;					///< ArtPoll : IPAddress for the ArtPoll package
	bool IsMultipleControllersReqDiag;			///< ArtPoll : Multiple controllers requesting diagnostics
	TArtNetNodeReportCode reportCode;			///< See \ref TArtNetNodeReportCode
	TNodeStatus status;							///< See \ref TNodeStatus
	bool IsSynchronousMode;						///< ArtSync received
	time_t ArtSyncTime;							///< Latest ArtSync received time
	bool IsMergeMode;							///< Is the Node in merging mode?
	bool IsChanged;								///< Is the DMX changed? Update output DMX
	uint8_t nActivePorts;						///< Number of active ports
};

/**
 *
 */
struct TArtNetNode {
	uint8_t MACAddressLocal[ARTNET_MAC_SIZE];		///< The local MAC Address
	uint32_t IPAddressLocal;						///< Local IP Address
	uint32_t IPAddressBroadcast;					///< The broadcast IP Address
	uint32_t IPSubnetMask;							///< The subnet mask
	uint8_t  NetSwitch;								///< Bits 14-8 of the 15 bit Port-Address are encoded into the bottom 7 bits of this field.
	uint8_t  SubSwitch;								///< Bits 7-4 of the 15 bit Port-Address are encoded into the bottom 4 bits of this field.
	uint8_t  ShortName[ARTNET_SHORT_NAME_LENGTH];	///< The array represents a null terminated short name for the Node.
	uint8_t  LongName[ARTNET_LONG_NAME_LENGTH];		///< The array represents a null terminated long name for the Node.
	uint32_t IPAddressRemote;						///< The remote IP Address
	uint8_t  TalkToMe;								///< Behavior of Node
	uint8_t  Status1;								///< General Status register
	uint8_t  Status2;
};

/**
 *
 */
struct TGenericPort {
	uint16_t nPortAddress;		///< One of the 32,768 possible addresses to which a DMX frame can be directed. The Port-Address is a 15 bit number composed of Net+Sub-Net+Universe.
	uint8_t nDefaultAddress;	///< the address set by the hardware
	uint8_t nStatus;			///<
};

/**
 * struct to represent an output port
 *
 */
struct TOutputPort {
	uint8_t data[ARTNET_DMX_LENGTH];	///< Data sent
	uint16_t nLength;					///< Length of sent DMX data
	uint8_t dataA[ARTNET_DMX_LENGTH];	///< The data received from Port A
	time_t timeA;						///< The latest time of the data received from Port A
	uint32_t ipA;						///< The IP address for port A
	uint8_t dataB[ARTNET_DMX_LENGTH];	///< The data received from Port B
	time_t timeB;						///< The latest time of the data received from Port B
	uint32_t ipB;						///< The IP address for Port B
	TMerge mergeMode;					///< \ref TMerge
	bool IsDataPending;					///< ArtDMX received and waiting for ArtSync
	bool bIsEnabled;					///< Is the port enabled ?
	TGenericPort port;					///< \ref TGenericPort
};

class ArtNetNode {
public:
	ArtNetNode(void);
	~ArtNetNode(void);

	void SetOutput(LightSet *);
	void SetLedBlink(LedBlink *);

	void SetTimeCodeHandler(ArtNetTimeCode *);
	void SetTimeSyncHandler(ArtNetTimeSync *);
	void SetRdmHandler(ArtNetRdm *);

	const uint8_t *GetSoftwareVersion(void);

	void Start(void);
	void Stop(void);

	void SetDirectUpdate(bool);
	const bool GetDirectUpdate(void);

	const char *GetShortName(void);
	void SetShortName(const char *);
	const char *GetLongName(void);
	void SetLongName(const char *);
	const uint8_t GetUniverseSwitch(uint8_t);
	int SetUniverseSwitch(const uint8_t, const TArtNetPortDir, const uint8_t);
	const uint8_t GetSubnetSwitch(void);
	void SetSubnetSwitch(const uint8_t);
	const uint8_t GetNetSwitch(void);
	void SetNetSwitch(const uint8_t);

	const uint8_t GetActiveOutputPorts(void);
	const uint8_t GetActiveInputPorts(void);

	void SendDiag(const char *, TPriorityCodes);
	void SendTimeCode(const struct TArtNetTimeCode *);

	int HandlePacket(void);

private:
	void GetType(void);

	void FillPollReply(void);
	void FillDiagData(void);
	void FillTimeCodeData(void);

	void HandlePoll(void);
	void HandleDmx(void);
	void HandleSync(void);
	void HandleAddress(void);
	void HandleTimeCode(void);
	void HandleTimeSync(void);
	void HandleTodRequest(void);
	void HandleTodControl(void);
	void HandleRdm(void);

	bool IsMergedDmxDataChanged(const uint8_t, const uint8_t *, const uint16_t);
	void CheckMergeTimeouts(const uint8_t);
	bool IsDmxDataChanged(const uint8_t, const uint8_t *, const uint16_t);

	void SendPollRelply(bool);
	void SendTod(void);

	uint16_t MakePortAddress(const uint16_t);

private:
	LightSet    			*m_pLightSet;		///<
	LedBlink				*m_pLedBlink;		///<

	ArtNetTimeCode			*m_pArtNetTimeCode;	///<
	ArtNetTimeSync			*m_pArtNetTimeSync;	///<
	ArtNetRdm				*m_pArtNetRdm;		///<

	struct TArtNetNode		m_Node;				///< Struct describing the node
	struct TArtNetNodeState m_State;			///< The current state of the node

	struct TArtNetPacket 	m_ArtNetPacket;		///< The received Art-Net package
	struct TArtPollReply	m_PollReply;		///<
	struct TArtDiagData		m_DiagData;			///<
	struct TArtTimeCode		m_TimeCodeData;		///<
	struct TArtTodData		*m_pTodData;		///<

	struct TOutputPort		m_OutputPorts[ARTNET_MAX_PORTS];	///<

	bool					m_bDirectUpdate;

	time_t 					m_nCurrentPacketTime;
	TOpCodes				m_tOpCodePrevious;
};

#endif /* ARTNETNODE_H_ */
