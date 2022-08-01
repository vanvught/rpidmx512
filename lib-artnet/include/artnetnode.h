/**
 * @file artnetnode.h
 *
 */
/**
 * Art-Net Designed by and Copyright Artistic Licence Holdings Ltd.
 */
/* Copyright (C) 2016-2022 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <cstdint>
#include <cassert>

#include "artnet.h"
#include "packets.h"
#include "artnettimecode.h"
#include "artnetrdm.h"
#include "artnetstore.h"
#include "artnetdisplay.h"
#include "artnetdmx.h"
#include "artnettrigger.h"
#include "artnet4handler.h"

#include "lightset.h"
#include "ledblink.h"

namespace artnetnode {
#if !defined(ARTNET_PAGE_SIZE)
 static constexpr uint32_t PAGE_SIZE = 4;
#else
 static constexpr uint32_t PAGE_SIZE = ARTNET_PAGE_SIZE;
#endif

static_assert((PAGE_SIZE == 4) || (PAGE_SIZE == 1) , "ARTNET_PAGE_SIZE");

#if !defined(LIGHTSET_PORTS)
# error LIGHTSET_PORTS not defined
#endif

static constexpr uint32_t PAGES = ((LIGHTSET_PORTS + (PAGE_SIZE - 1)) / PAGE_SIZE);
static constexpr auto MAX_PORTS = PAGE_SIZE * PAGES > LIGHTSET_PORTS ? LIGHTSET_PORTS : PAGE_SIZE * PAGES;

enum class FailSafe : uint8_t {
	LAST = 0x08, OFF= 0x09, ON = 0x0a, PLAYBACK = 0x0b, RECORD = 0x0c
};

/**
 * Table 3 – NodeReport Codes
 * The NodeReport code defines generic error, advisory and status messages for both Nodes and Controllers.
 * The NodeReport is returned in ArtPollReply.
 */
enum class ReportCode : uint8_t {
	RCDEBUG,
	RCPOWEROK,
	RCPOWERFAIL,
	RCSOCKETWR1,
	RCPARSEFAIL,
	RCUDPFAIL,
	RCSHNAMEOK,
	RCLONAMEOK,
	RCDMXERROR,
	RCDMXUDPFULL,
	RCDMXRXFULL,
	RCSWITCHERR,
	RCCONFIGERR,
	RCDMXSHORT,
	RCFIRMWAREFAIL,
	RCUSERFAIL
};

enum class Status : uint8_t {
	OFF, STANDBY, ON
};

struct State {
	uint32_t ArtPollReplyCount;			///< ArtPollReply : NodeReport : decimal counter that increments every time the Node sends an ArtPollResponse.
	uint32_t IPAddressDiagSend;			///< ArtPoll : Destination IPAddress for the ArtDiag
	uint32_t IPAddressArtPoll;			///< ArtPoll : IPAddress for the ArtPoll package
	uint32_t nArtSyncMillis;			///< Latest ArtSync received time
	ReportCode reportCode;
	Status status;
	bool SendArtPollReplyOnChange;		///< ArtPoll : TalkToMe Bit 1 : 1 = Send ArtPollReply whenever Node conditions change.
	bool SendArtDiagData;				///< ArtPoll : TalkToMe Bit 2 : 1 = Send me diagnostics messages.
	bool IsMultipleControllersReqDiag;	///< ArtPoll : Multiple controllers requesting diagnostics
	bool IsSynchronousMode;				///< ArtSync received
	bool IsMergeMode;
	bool IsChanged;
	bool bDisableMergeTimeout;
	uint8_t nReceivingDmx;
	uint8_t nActiveOutputPorts;
	uint8_t nActiveInputPorts;
	uint8_t Priority;					///< ArtPoll : Field 6 : The lowest priority of diagnostics message that should be sent.
};

struct Node {
	uint32_t IPAddressLocal;
	uint32_t IPAddressBroadcast;
	uint32_t IPSubnetMask;
	uint32_t IPAddressTimeCode;
	uint8_t MACAddressLocal[artnet::MAC_SIZE];
	uint8_t NetSwitch[artnetnode::PAGES];			///< Bits 14-8 of the 15 bit Port-Address are encoded into the bottom 7 bits of this field.
	uint8_t SubSwitch[artnetnode::PAGES];			///< Bits 7-4 of the 15 bit Port-Address are encoded into the bottom 4 bits of this field.
	char ShortName[artnet::SHORT_NAME_LENGTH];
	char LongName[artnet::LONG_NAME_LENGTH];
	uint8_t TalkToMe;								///< Behavior of Node
	uint8_t Status1;
	uint8_t Status2;
	uint8_t Status3;
	uint8_t DefaultUidResponder[6];					///< RDMnet & LLRP UID
	bool bMapUniverse0;
};

struct Source {
	uint32_t nMillis;	///< The latest time of the data received from port
	uint32_t nIp;		///< The IP address for port
};

struct GenericPort {
	uint16_t nPortAddress;		///< One of the 32,768 possible addresses to which a DMX frame can be directed. The Port-Address is a 15 bit number composed of Net+Sub-Net+Universe.
	uint8_t nDefaultAddress;	///< the address set by the hardware
	uint8_t nStatus;
	bool bIsEnabled;
};

struct OutputPort {
	GenericPort genericPort;
	Source sourceA;
	Source sourceB;
	lightset::MergeMode mergeMode;
	bool isRdmEnabled;
	artnet::PortProtocol protocol;	///< Art-Net 4
	bool IsDataPending;				///< ArtDMX received and waiting for ArtSync
	bool IsTransmitting;
};

struct InputPort {
	GenericPort genericPort;
	uint32_t nDestinationIp;
	uint8_t nSequenceNumber;
};
}  // namespace artnetnode

class ArtNetNode {
public:
	ArtNetNode();
	~ArtNetNode();

	void Start();
	void Stop();

	void Run();

	uint8_t GetVersion() const {
		return artnet::VERSION;
	}

	void SetFailSafe(const artnetnode::FailSafe failsafe);

	artnetnode::FailSafe GetFailSafe() {
		const auto networkloss = (m_Node.Status3 & artnet::Status3::NETWORKLOSS_MASK);
		switch (networkloss) {
			case artnet::Status3::NETWORKLOSS_LAST_STATE:
				return artnetnode::FailSafe::LAST;
				break;
			case artnet::Status3::NETWORKLOSS_OFF_STATE:
				return artnetnode::FailSafe::OFF;
				break;
			case artnet::Status3::NETWORKLOSS_ON_STATE:
				return artnetnode::FailSafe::ON;
				break;
			case artnet::Status3::NETWORKLOSS_PLAYBACK:
				return artnetnode::FailSafe::PLAYBACK;
				break;
			default:
				assert(0);
				__builtin_unreachable();
				break;
		}

		__builtin_unreachable();
		return artnetnode::FailSafe::OFF;
	}

	void SetOutput(LightSet *pLightSet) {
		m_pLightSet = pLightSet;
	}

	LightSet *GetOutput() const {
		return m_pLightSet;
	}

	uint32_t GetActiveInputPorts() const {
		return m_State.nActiveInputPorts;
	}

	uint32_t GetActiveOutputPorts() const {
		return m_State.nActiveOutputPorts;
	}

	void SetShortName(const char *);

	const char *GetShortName() const {
		return m_Node.ShortName;
	}

	void SetLongName(const char *);

	const char *GetLongName() const {
		return m_Node.LongName;
	}

	int SetUniverse(uint32_t nPortIndex, lightset::PortDir dir, uint16_t nUniverse);

	int SetUniverseSwitch(uint32_t nPortIndex, lightset::PortDir dir, uint8_t nAddress);
	bool GetUniverseSwitch(uint32_t nPortIndex, uint8_t &nAddress,lightset::PortDir dir) const;

	void SetNetSwitch(uint8_t nAddress, uint32_t nPage);

	uint8_t GetNetSwitch(uint32_t nPage) const {
		assert(nPage < artnetnode::PAGES);
		return m_Node.NetSwitch[nPage];
	}

	void SetSubnetSwitch(uint8_t nAddress, uint32_t nPage);

	uint8_t GetSubnetSwitch(uint32_t nPage) const {
		assert(nPage < artnetnode::PAGES);
		return m_Node.SubSwitch[nPage];
	}

	bool GetPortAddress(uint32_t nPortIndex, uint16_t& nAddress) const;
	bool GetPortAddress(uint32_t nPortIndex, uint16_t& nAddress, lightset::PortDir dir) const;

	bool GetOutputPort(const uint16_t nUniverse, uint32_t& nPortIndex) {
		for (nPortIndex = 0; nPortIndex < artnetnode::MAX_PORTS; nPortIndex++) {
			if (!m_OutputPort[nPortIndex].genericPort.bIsEnabled) {
				continue;
			}
			if ((m_OutputPort[nPortIndex].protocol == artnet::PortProtocol::ARTNET) && (nUniverse == m_OutputPort[nPortIndex].genericPort.nPortAddress)) {
				return true;
			}
		}
		return false;
	}

	void SetMergeMode(uint32_t nPortIndex, lightset::MergeMode tMergeMode);

	lightset::MergeMode GetMergeMode(uint32_t nPortIndex) const {
		assert(nPortIndex < artnetnode::MAX_PORTS);
		return m_OutputPort[nPortIndex].mergeMode;
	}

	void SetRmd(uint32_t nPortIndex, bool bEnable);
	bool GetRdm(uint32_t nPortIndex) const {
		assert(nPortIndex < artnetnode::MAX_PORTS);
		return m_OutputPort[nPortIndex].isRdmEnabled;
	}

	void SetDisableMergeTimeout(bool bDisable) {
		m_State.bDisableMergeTimeout = bDisable;
	}

	bool GetDisableMergeTimeout() const {
		return m_State.bDisableMergeTimeout;
	}

#if defined ( ENABLE_SENDDIAG )
	void SendDiag(const char *, artnet::PriorityCodes);
#endif

	void SendTimeCode(const struct TArtNetTimeCode *);

	void SetTimeCodeHandler(ArtNetTimeCode *pArtNetTimeCode) {
		m_pArtNetTimeCode = pArtNetTimeCode;
	}

	void SetTimeCodeIp(uint32_t nDestinationIp);

	void SetRdmHandler(ArtNetRdm *, bool isResponder = false);

	void SetArtNetStore(ArtNetStore *pArtNetStore) {
		m_pArtNetStore = pArtNetStore;
	}

	void SetArtNetDisplay(ArtNetDisplay *pArtNetDisplay) {
		m_pArtNetDisplay = pArtNetDisplay;
	}

	void SetArtNetTrigger(ArtNetTrigger *pArtNetTrigger) {
		m_pArtNetTrigger = pArtNetTrigger;
	}

	void SetArtNetDmx(ArtNetDmx *pArtNetDmx) {
		m_pArtNetDmx = pArtNetDmx;
	}

	ArtNetDmx *GetArtNetDmx() const {
		return m_pArtNetDmx;
	}

	void SetDestinationIp(uint32_t nPortIndex, uint32_t nDestinationIp);

	uint32_t GetDestinationIp(uint32_t nPortIndex) const {
		if (nPortIndex < artnetnode::MAX_PORTS) {
			return m_InputPort[nPortIndex].nDestinationIp;
		}

		return 0;
	}

	void SetRdmUID(const uint8_t *pUid, bool bSupportsLLRP = false);

	/**
	 * Art-Net 4
	 */

	void SetArtNet4Handler(ArtNet4Handler *pArtNet4Handler) {
		if (artnet::VERSION >= 4) {
			m_pArtNet4Handler = pArtNet4Handler;
		}
	}

	void SetPortProtocol(uint32_t nPortIndex, artnet::PortProtocol tPortProtocol);

	artnet::PortProtocol GetPortProtocol(uint32_t nPortIndex) const {
		assert(nPortIndex < (artnetnode::MAX_PORTS));
		return m_OutputPort[nPortIndex].protocol;
	}

	void SetMapUniverse0(bool bMapUniverse0 = false) {
		m_Node.bMapUniverse0 = bMapUniverse0;
	}

	bool IsMapUniverse0() {
		return m_Node.bMapUniverse0;
	}

	void Print();

	static ArtNetNode* Get() {
		return s_pThis;
	}

private:
	void FillPollReply();
#if defined ( ENABLE_SENDDIAG )
	void FillDiagData(void);
#endif

	void GetType();

	void HandlePoll();
	void HandleDmx();
	void HandleSync();
	void HandleAddress();
	void HandleTimeCode();
	void HandleTimeSync();
	void HandleTodRequest();
	void HandleTodControl();
	void HandleRdm();
	void HandleIpProg();
	void HandleDmxIn();
	void HandleTrigger();

	uint16_t MakePortAddress(uint16_t nUniverse, uint32_t nPage);

	void UpdateMergeStatus(uint32_t nPortIndex);
	void CheckMergeTimeouts(uint32_t nPortIndex);

	void ProcessPollRelply(uint32_t nPortIndex, uint32_t nPortIndexStart, uint32_t& NumPortsLo);
	void SendPollRelply(bool);
	void SendTod(uint32_t nPortIndex);

	void SetNetworkDataLossCondition();

private:
	int32_t m_nHandle { -1 };

	LightSet *m_pLightSet { nullptr };

	ArtNetTimeCode *m_pArtNetTimeCode { nullptr };
	ArtNetRdm *m_pArtNetRdm { nullptr };
	ArtNetDmx *m_pArtNetDmx { nullptr };
	ArtNetTrigger *m_pArtNetTrigger { nullptr };
	ArtNet4Handler *m_pArtNet4Handler { nullptr };
	ArtNetStore *m_pArtNetStore { nullptr };
	ArtNetDisplay *m_pArtNetDisplay { nullptr };

	artnetnode::Node m_Node;

	TArtNetPacket m_ArtNetPacket;
	TArtPollReply m_PollReply;
	TArtDmx m_ArtDmx;
#if defined ( ENABLE_SENDDIAG )
	TArtDiagData m_DiagData;
#endif

	uint32_t m_nCurrentPacketMillis { 0 };
	uint32_t m_nPreviousPacketMillis { 0 };

	bool m_IsRdmResponder { false };

	char m_aSysName[16];
	char m_aDefaultNodeLongName[artnet::LONG_NAME_LENGTH];

	artnetnode::State m_State;
	artnetnode::OutputPort m_OutputPort[artnetnode::MAX_PORTS];
	artnetnode::InputPort m_InputPort[artnetnode::MAX_PORTS];

	static ArtNetNode *s_pThis;
};

#endif /* ARTNETNODE_H_ */
