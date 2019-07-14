/**
 * @file artnetnode.h
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

#ifndef ARTNETNODE_H_
#define ARTNETNODE_H_

#include <stdint.h>

#if defined (__circle__)
#include <circle/time.h>
#else
#include <stdbool.h>
#include <time.h>
#endif

#include "artnet.h"
#include "packets.h"

#include "lightset.h"
#include "ledblink.h"

#include "artnettimecode.h"
#include "artnettimesync.h"
#include "artnetrdm.h"
#include "artnetipprog.h"
#include "artnetstore.h"

#include "artnet4handler.h"

/**
 * Table 3 – NodeReport Codes
 * The NodeReport code defines generic error, advisory and status messages for both Nodes and Controllers.
 * The NodeReport is returned in ArtPollReply.
 */
enum TArtNetNodeReportCode {
	ARTNET_RCDEBUG,			///<
	ARTNET_RCPOWEROK,		///<
	ARTNET_RCPOWERFAIL,		///<
	ARTNET_RCSOCKETWR1,		///<
	ARTNET_RCPARSEFAIL,		///<
	ARTNET_RCUDPFAIL,		///<
	ARTNET_RCSHNAMEOK,		///<
	ARTNET_RCLONAMEOK,		///<
	ARTNET_RCDMXERROR,		///<
	ARTNET_RCDMXUDPFULL,	///<
	ARTNET_RCDMXRXFULL,		///<
	ARTNET_RCSWITCHERR,		///<
	ARTNET_RCCONFIGERR,		///<
	ARTNET_RCDMXSHORT,		///<
	ARTNET_RCFIRMWAREFAIL,	///<
	ARTNET_RCUSERFAIL     	///<
};

enum TNodeStatus {
	ARTNET_OFF,		///<
	ARTNET_STANDBY,	///<
	ARTNET_ON		///<
};

struct TArtNetNodeState {
	uint32_t ArtPollReplyCount;			///< ArtPollReply : NodeReport : decimal counter that increments every time the Node sends an ArtPollResponse.
	uint32_t IPAddressDiagSend;			///< ArtPoll : Destination IPAddress for the ArtDiag
	uint32_t IPAddressArtPoll;			///< ArtPoll : IPAddress for the ArtPoll package
	TArtNetNodeReportCode reportCode;	///< See \ref TArtNetNodeReportCode
	TNodeStatus status;					///< See \ref TNodeStatus
	time_t nNetworkDataLossTimeout;
	time_t ArtSyncTime;					///< Latest ArtSync received time
	bool SendArtPollReplyOnChange;		///< ArtPoll : TalkToMe Bit 1 : 1 = Send ArtPollReply whenever Node conditions change.
	bool SendArtDiagData;				///< ArtPoll : TalkToMe Bit 2 : 1 = Send me diagnostics messages.
	bool IsMultipleControllersReqDiag;	///< ArtPoll : Multiple controllers requesting diagnostics
	bool IsSynchronousMode;				///< ArtSync received
	bool IsMergeMode;
	bool IsChanged;
	bool bDisableMergeTimeout;
	bool bIsReceivingDmx;
	uint8_t nActivePorts;
	uint8_t Priority;					///< ArtPoll : Field 6 : The lowest priority of diagnostics message that should be sent.
};

struct TArtNetNode {
	uint32_t IPAddressLocal;						///< Local IP Address
	uint32_t IPAddressBroadcast;					///< The broadcast IP Address
	uint32_t IPSubnetMask;							///< The subnet mask
	uint32_t IPAddressRemote;						///< The remote IP Address
	uint8_t MACAddressLocal[ARTNET_MAC_SIZE];		///< The local MAC Address
	uint8_t NetSwitch[ARTNET_MAX_PAGES];			///< Bits 14-8 of the 15 bit Port-Address are encoded into the bottom 7 bits of this field.
	uint8_t SubSwitch[ARTNET_MAX_PAGES];			///< Bits 7-4 of the 15 bit Port-Address are encoded into the bottom 4 bits of this field.
	uint8_t Oem[2];									///< The Oem word describes the equipment vendor and the feature set available.
	uint8_t ShortName[ARTNET_SHORT_NAME_LENGTH];	///< The array represents a null terminated short name for the Node.
	uint8_t LongName[ARTNET_LONG_NAME_LENGTH];		///< The array represents a null terminated long name for the Node.
	uint8_t Esta[ARTNET_ESTA_SIZE];					///< The ESTA manufacturer code.
	uint8_t TalkToMe;								///< Behavior of Node
	uint8_t Status1;								///< General Status register
	uint8_t Status2;
};

struct TGenericPort {
	uint16_t nPortAddress;		///< One of the 32,768 possible addresses to which a DMX frame can be directed. The Port-Address is a 15 bit number composed of Net+Sub-Net+Universe.
	uint8_t nDefaultAddress;	///< the address set by the hardware
	uint8_t nStatus;			///<
};

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
	TPortProtocol tPortProtocol;		///< Art-Net 4
};

class ArtNetNode {
public:
	ArtNetNode(uint8_t nVersion = 3, uint8_t nPages = 1);
	~ArtNetNode(void);

	void Start(void);
	void Stop(void);

	int Run(void);

	uint8_t GetVersion(void) {
		return m_nVersion;
	}

	uint8_t GetPages(void) {
		return m_nPages;
	}

	void SetOutput(LightSet *pLightSet);
	LightSet* GetOutput(void) {
		return m_pLightSet;
	}

	const uint8_t *GetSoftwareVersion(void);

	uint8_t GetActiveInputPorts(void) { return 0; }

	uint8_t GetActiveOutputPorts(void) {
		return m_State.nActivePorts;
	}

	void SetDirectUpdate(bool bDirectUpdate) {
		m_bDirectUpdate = bDirectUpdate;
	}
	bool GetDirectUpdate(void) {
		return m_bDirectUpdate;
	}

	void SetShortName(const char *);
	const char *GetShortName(void) {
		return (const char *) m_Node.ShortName;
	}

	void SetLongName(const char *);
	const char *GetLongName(void) {
		return (const char *) m_Node.LongName;
	}

	int SetUniverseSwitch(uint8_t nPortIndex, TArtNetPortDir dir, uint8_t nAddress);
	bool GetUniverseSwitch(uint8_t nPortIndex, uint8_t &nAddress) const;

	void SetNetSwitch(uint8_t nAddress, uint8_t nPage = 0);
	uint8_t GetNetSwitch(uint8_t nPage = 0) const;

	void SetSubnetSwitch(uint8_t nAddress, uint8_t nPage = 0);
	uint8_t GetSubnetSwitch(uint8_t nPage = 0) const;

	bool GetPortAddress(uint8_t nPortIndex, uint16_t &nAddress) const;

	void SetMergeMode(uint8_t nPortIndex, TMerge tMergeMode);
	TMerge GetMergeMode(uint8_t nPortIndex = 0) const;

	void SetPortProtocol(uint8_t nPortIndex, TPortProtocol tPortProtocol);
	TPortProtocol GetPortProtocol(uint8_t nPortIndex) const;

	void SetManufacturerId(const uint8_t *);
	const uint8_t* GetManufacturerId(void) {
		return m_Node.Esta;
	}

	void SetOemValue(const uint8_t *);
	const uint8_t* GetOemValue(void) {
		return m_Node.Oem;
	}

	void SetNetworkTimeout(time_t);
	time_t GetNetworkTimeout(void) {
		return m_State.nNetworkDataLossTimeout;
	}

	void SetDisableMergeTimeout(bool);
	bool GetDisableMergeTimeout(void) {
		return m_State.bDisableMergeTimeout;
	}

	void SendDiag(const char *, TPriorityCodes);
	void SendTimeCode(const struct TArtNetTimeCode *);

	void SetTimeCodeHandler(ArtNetTimeCode *);
	void SetTimeSyncHandler(ArtNetTimeSync *);
	void SetRdmHandler(ArtNetRdm *, bool isResponder = false);
	void SetIpProgHandler(ArtNetIpProg *);
	void SetArtNetStore(ArtNetStore *pArtNetStore);

	void SetArtNet4Handler(ArtNet4Handler *pArtNet4Handler);

	void Print(void);

private:
	void FillPollReply(void);
#if defined ( ENABLE_SENDDIAG )
	void FillDiagData(void);
#endif

	void GetType(void);

	void HandlePoll(void);
	void HandleDmx(void);
	void HandleSync(void);
	void HandleAddress(void);
	void HandleTimeCode(void);
	void HandleTimeSync(void);
	void HandleTodRequest(void);
	void HandleTodControl(void);
	void HandleRdm(void);
	void HandleIpProg(void);
	//void HandleDirectory(void);

	uint16_t MakePortAddress(uint16_t, uint8_t nPage = 0);

	bool IsMergedDmxDataChanged(uint8_t, const uint8_t *, uint16_t);
	void CheckMergeTimeouts(uint8_t);
	bool IsDmxDataChanged(uint8_t, const uint8_t *, uint16_t);

	void SendPollRelply(bool);
	void SendTod(uint8_t nPortId = 0);

	void SetNetworkDataLossCondition(void);

private:
	uint8_t m_nVersion;
	uint8_t m_nPages;
	int32_t m_nHandle;
	LightSet *m_pLightSet;

	ArtNetTimeCode *m_pArtNetTimeCode;
	ArtNetTimeSync *m_pArtNetTimeSync;
	ArtNetRdm *m_pArtNetRdm;
	ArtNetIpProg *m_pArtNetIpProg;
	ArtNetStore *m_pArtNetStore;

	ArtNet4Handler *m_pArtNet4Handler;

	struct TArtNetNode m_Node;				///< Struct describing the node
	struct TArtNetNodeState m_State;		///< The current state of the node

	struct TArtNetPacket m_ArtNetPacket;	///< The received Art-Net package
	struct TArtPollReply m_PollReply;
#if defined ( ENABLE_SENDDIAG )
	struct TArtDiagData m_DiagData;
#endif
	struct TArtTimeCode *m_pTimeCodeData;
	struct TArtTodData *m_pTodData;
	struct TArtIpProgReply *m_pIpProgReply;

	struct TOutputPort m_OutputPorts[ARTNET_MAX_PORTS * ARTNET_MAX_PAGES];

	bool m_bDirectUpdate;

	time_t m_nCurrentPacketTime;
	time_t m_nPreviousPacketTime;
	TOpCodes m_tOpCodePrevious;

	bool m_IsLightSetRunning[ARTNET_MAX_PORTS * ARTNET_MAX_PAGES];
	bool m_IsRdmResponder;

	alignas(uint32_t) char m_aSysName[16];
	alignas(uint32_t) char m_aDefaultNodeLongName[ARTNET_LONG_NAME_LENGTH];
};

#endif /* ARTNETNODE_H_ */
