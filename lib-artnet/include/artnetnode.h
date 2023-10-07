/**
 * @file artnetnode.h
 *
 */
/**
 * Art-Net Designed by and Copyright Artistic Licence Holdings Ltd.
 */
/* Copyright (C) 2016-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <cstring>
#include <cstdarg>
#include <cstdio>
#include <cassert>

#if !defined(ARTNET_VERSION)
# error ARTNET_VERSION is not defined
#endif

#if (ARTNET_VERSION >= 4) && defined (ARTNET_HAVE_DMXIN)
# if !defined (E131_HAVE_DMXIN)
#  define E131_HAVE_DMXIN
# endif
#endif

#include "artnet.h"
#include "artnettimecode.h"
#include "artnetrdm.h"
#include "artnetstore.h"
#include "artnetdisplay.h"
#include "artnettrigger.h"

#if (ARTNET_VERSION >= 4)
# include "e131bridge.h"
#endif

#include "lightset.h"
#include "hardware.h"
#include "network.h"

#include "debug.h"

namespace artnetnode {
#if !defined(LIGHTSET_PORTS)
# define LIGHTSET_PORTS	0
#endif

#if (LIGHTSET_PORTS == 0)
 static constexpr uint32_t MAX_PORTS = 1;	// ISO C++ forbids zero-size array
#else
 static constexpr uint32_t MAX_PORTS = LIGHTSET_PORTS;
#endif

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

struct ArtPollQueue {
	uint32_t ArtPollMillis;
	uint32_t ArtPollReplyIpAddress;
	struct {
		uint16_t TargetPortAddressTop;
		uint16_t TargetPortAddressBottom;
	} ArtPollReply;
};

struct State {
	uint32_t ArtDiagIpAddress;
	uint32_t ArtPollIpAddress;
	uint32_t ArtPollReplyCount;
	uint32_t ArtPollReplyDelayMillis;
	ArtPollQueue ArtPollReplyQueue[4];
	uint32_t ArtDmxIpAddress;
	uint32_t ArtSyncMillis;				///< Latest ArtSync received time
	ReportCode reportCode;
	Status status;
	bool SendArtPollReplyOnChange;		///< ArtPoll : Flags Bit 1 : 1 = Send ArtPollReply whenever Node conditions change.
	bool SendArtDiagData;				///< ArtPoll : Flags Bit 2 : 1 = Send me diagnostics messages.
	bool IsMultipleControllersReqDiag;	///< ArtPoll : Multiple controllers requesting diagnostics
	bool IsSynchronousMode;				///< ArtSync received
	bool IsMergeMode;
	bool IsChanged;
	bool bDisableMergeTimeout;
	uint8_t nReceivingDmx;
	uint8_t nEnabledOutputPorts;
	uint8_t nEnabledInputPorts;
	uint8_t DiagPriority;				///< ArtPoll : Field 6 : The lowest priority of diagnostics message that should be sent.
};

struct Node {
	uint32_t IPAddressTimeCode;
	bool IsRdmResponder;
	bool bMapUniverse0;										///< Art-Net 4
	struct {
		char ShortName[artnet::SHORT_NAME_LENGTH];
		uint16_t PortAddress;								///< The Port-Address is a 15 bit number composed of Net+Sub-Net+Universe.
		uint8_t DefaultAddress;
		uint8_t NetSwitch;									///< Bits 14-8 of the 15 bit Port-Address are encoded into the bottom 7 bits of this field.
		uint8_t SubSwitch;									///< Bits 7-4 of the 15 bit Port-Address are encoded into the bottom 4 bits of this field.
		lightset::PortDir direction;
		artnet::PortProtocol protocol;						///< Art-Net 4
		bool bLocalMerge;
	} Port[artnetnode::MAX_PORTS];
};

struct Source {
	uint32_t nMillis;	///< The latest time of the data received from port
	uint32_t nIp;		///< The IP address for port
	uint16_t nPhysical;	///< The physical input port from which DMX512 data was input.
};

struct OutputPort {
	Source SourceA;
	Source SourceB;
	uint8_t GoodOutput;
	uint8_t GoodOutputB;
	uint8_t nPollReplyIndex;
	bool IsTransmitting;
	bool IsDataPending;
};

struct InputPort {
	uint32_t nDestinationIp;
	uint32_t nMillis;
	uint8_t nSequenceNumber;
	uint8_t GoodInput;
	uint8_t nPollReplyIndex;
};

inline artnetnode::FailSafe convert_failsafe(const lightset::FailSafe failsafe) {
	const auto fs = static_cast<FailSafe>(static_cast<uint32_t>(failsafe) + static_cast<uint32_t>(FailSafe::LAST));
	DEBUG_PRINTF("failsafe=%u, fs=%u", static_cast<uint32_t>(failsafe), static_cast<uint32_t>(fs));
	return fs;
}

inline lightset::FailSafe convert_failsafe(const artnetnode::FailSafe failsafe) {
	const auto fs = static_cast<lightset::FailSafe>(static_cast<uint32_t>(failsafe) - static_cast<uint32_t>(FailSafe::LAST));
	DEBUG_PRINTF("failsafe=%u, fs=%u", static_cast<uint32_t>(failsafe), static_cast<uint32_t>(fs));
	return fs;
}
}  // namespace artnetnode

#if (ARTNET_VERSION >= 4)
class ArtNetNode: E131Bridge {
#else
class ArtNetNode {
#endif
public:
	ArtNetNode();
	~ArtNetNode();

	void Start();
	void Stop();

	void Run() {
		uint16_t nForeignPort;
		const auto nBytesReceived = Network::Get()->RecvFrom(m_nHandle, const_cast<const void**>(reinterpret_cast<void **>(&m_pReceiveBuffer)), &m_nIpAddressFrom, &nForeignPort);
		m_nCurrentPacketMillis = Hardware::Get()->Millis();

		Process(nBytesReceived);

#if (ARTNET_VERSION >= 4)
		E131Bridge::Run();
#endif
#if defined (LIGHTSET_HAVE_RUN)
		m_pLightSet->Run();
#endif
	}

	uint8_t GetVersion() const {
		return artnet::VERSION;
	}

	void SetFailSafe(const artnetnode::FailSafe failsafe);

	artnetnode::FailSafe GetFailSafe() {
		const auto networkloss = (m_ArtPollReply.Status3 & artnet::Status3::NETWORKLOSS_MASK);
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

#if defined (OUTPUT_HAVE_STYLESWITCH)
	void SetOutputStyle(const uint32_t nPortIndex, lightset::OutputStyle outputStyle) {
		assert(nPortIndex < artnetnode::MAX_PORTS);

		if (outputStyle == GetOutputStyle(nPortIndex)) {
			return;
		}

		if ((m_State.status == artnetnode::Status::ON) && (m_pLightSet != nullptr)) {
			m_pLightSet->SetOutputStyle(nPortIndex, outputStyle);
			outputStyle = m_pLightSet->GetOutputStyle(nPortIndex);
		}

		if (outputStyle == lightset::OutputStyle::CONSTANT) {
			m_OutputPort[nPortIndex].GoodOutputB |= artnet::GoodOutputB::STYLE_CONSTANT;
		} else {
			m_OutputPort[nPortIndex].GoodOutputB &= static_cast<uint8_t>(~artnet::GoodOutputB::STYLE_CONSTANT);
		}

#if defined (OUTPUT_DMX_SEND) || defined (OUTPUT_DMX_SEND_MULTI)
		/**
		 * FIXME I do not like this hack. It should be handled in dmx.cpp
		 */
		if ((m_Node.Port[nPortIndex].direction == lightset::PortDir::OUTPUT)
				&& (outputStyle == lightset::OutputStyle::CONSTANT)
				&& (m_pLightSet != nullptr)) {
			if (m_OutputPort[nPortIndex].IsTransmitting) {
				m_OutputPort[nPortIndex].IsTransmitting = false;
				m_pLightSet->Stop(nPortIndex);
			}
		}
#endif

		m_State.IsSynchronousMode = false;

		if (m_State.status == artnetnode::Status::ON) {
			if (m_pArtNetStore != nullptr) {
				m_pArtNetStore->SaveOutputStyle(nPortIndex, outputStyle);
			}

			artnet::display_outputstyle(nPortIndex, outputStyle);
		}
	}

	lightset::OutputStyle GetOutputStyle(const uint32_t nPortIndex) const {
		assert(nPortIndex < artnetnode::MAX_PORTS);

		const auto isStyleConstant = (m_OutputPort[nPortIndex].GoodOutputB & artnet::GoodOutputB::STYLE_CONSTANT) == artnet::GoodOutputB::STYLE_CONSTANT;
		return isStyleConstant ? lightset::OutputStyle::CONSTANT : lightset::OutputStyle::DELTA;
	}
#endif

	void SetOutput(LightSet *pLightSet) {
		m_pLightSet = pLightSet;
#if (ARTNET_VERSION >= 4)
		E131Bridge::SetOutput(pLightSet);
#endif
	}

	LightSet *GetOutput() const {
		return m_pLightSet;
	}

	uint32_t GetActiveInputPorts() const {
		return m_State.nEnabledInputPorts;
	}

	uint32_t GetActiveOutputPorts() const {
		return m_State.nEnabledOutputPorts;
	}

	void SetShortName(const uint32_t nPortIndex, const char *);
	const char *GetShortName(const uint32_t nPortIndex) const {
		assert(nPortIndex < artnetnode::MAX_PORTS);
		return m_Node.Port[nPortIndex].ShortName;
	}

	void SetLongName(const char *);
	const char *GetLongName() const {
		return reinterpret_cast<const char * >(m_ArtPollReply.LongName);
	}

	void GetLongNameDefault(char *);

	void SetUniverse(const uint32_t nPortIndex, const lightset::PortDir dir, const uint16_t nUniverse);

	lightset::PortDir GetPortDirection(const uint32_t nPortIndex) const {
		assert(nPortIndex < artnetnode::MAX_PORTS);
		return m_Node.Port[nPortIndex].direction;
	}

	bool GetPortAddress(uint32_t nPortIndex, uint16_t& nAddress) const {
		assert(nPortIndex < artnetnode::MAX_PORTS);

		if (m_Node.Port[nPortIndex].direction == lightset::PortDir::DISABLE) {
			return false;
		}

		nAddress = m_Node.Port[nPortIndex].PortAddress;
		return true;
	}

	bool GetPortAddress(const uint32_t nPortIndex, uint16_t& nAddress, lightset::PortDir portDir) const {
		assert(nPortIndex < artnetnode::MAX_PORTS);

		if (portDir == lightset::PortDir::DISABLE) {
			return false;
		}

		nAddress = m_Node.Port[nPortIndex].PortAddress;
		return m_Node.Port[nPortIndex].direction == portDir;
	}

	bool GetOutputPort(const uint16_t nUniverse, uint32_t& nPortIndex) {
		for (nPortIndex = 0; nPortIndex < artnetnode::MAX_PORTS; nPortIndex++) {
			if (m_Node.Port[nPortIndex].direction != lightset::PortDir::OUTPUT) {
				continue;
			}
			if ((m_Node.Port[nPortIndex].protocol == artnet::PortProtocol::ARTNET) && (nUniverse == m_Node.Port[nPortIndex].PortAddress)) {
				return true;
			}
		}
		return false;
	}

	void SetMergeMode(const uint32_t nPortIndex, const lightset::MergeMode mergeMode);
	lightset::MergeMode GetMergeMode(const uint32_t nPortIndex) const {
		assert(nPortIndex < artnetnode::MAX_PORTS);
		if ((m_OutputPort[nPortIndex].GoodOutput & artnet::GoodOutput::MERGE_MODE_LTP) == artnet::GoodOutput::MERGE_MODE_LTP) {
			return lightset::MergeMode::LTP;
		}
		return lightset::MergeMode::HTP;
	}

	void SetRmd(const uint32_t nPortIndex, const bool bEnable);
	bool GetRdm(const uint32_t nPortIndex) const {
		assert(nPortIndex < artnetnode::MAX_PORTS);
		return !((m_OutputPort[nPortIndex].GoodOutputB & artnet::GoodOutputB::RDM_DISABLED) == artnet::GoodOutputB::RDM_DISABLED);
	}

	void SetDisableMergeTimeout(bool bDisable) {
		m_State.bDisableMergeTimeout = bDisable;
#if (ARTNET_VERSION >= 4)
		E131Bridge::SetDisableMergeTimeout(bDisable);
#endif
	}

	bool GetDisableMergeTimeout() const {
		return m_State.bDisableMergeTimeout;
	}

	void SendTimeCode(const struct TArtNetTimeCode *);

	void SetTimeCodeHandler(ArtNetTimeCode *pArtNetTimeCode) {
		m_pArtNetTimeCode = pArtNetTimeCode;
	}

	void SetTimeCodeIp(uint32_t nDestinationIp);

	void SetRdmHandler(ArtNetRdm *, bool isResponder = false);

	void SetArtNetStore(ArtNetStore *pArtNetStore) {
		m_pArtNetStore = pArtNetStore;
	}

	void SetArtNetTrigger(ArtNetTrigger *pArtNetTrigger) {
		m_pArtNetTrigger = pArtNetTrigger;
	}

	void SetDestinationIp(const uint32_t nPortIndex, const uint32_t nDestinationIp) {
		if (nPortIndex < artnetnode::MAX_PORTS) {
			if (Network::Get()->IsValidIp(nDestinationIp)) {
				m_InputPort[nPortIndex].nDestinationIp = nDestinationIp;
			} else {
				m_InputPort[nPortIndex].nDestinationIp = Network::Get()->GetBroadcastIp();
			}

			DEBUG_PRINTF("m_nDestinationIp=" IPSTR, IP2STR(m_InputPort[nPortIndex].nDestinationIp));
		}
	}

	uint32_t GetDestinationIp(const uint32_t nPortIndex) const {
		if (nPortIndex < artnetnode::MAX_PORTS) {
			return m_InputPort[nPortIndex].nDestinationIp;
		}

		return 0;
	}

	/**
	 * LLRP
	 */
	void SetRdmUID(const uint8_t *pUid, bool bSupportsLLRP = false) {
		memcpy(m_ArtPollReply.DefaultUidResponder, pUid, sizeof(m_ArtPollReply.DefaultUidResponder));

		if (bSupportsLLRP) {
			m_ArtPollReply.Status3 |= artnet::Status3::SUPPORTS_LLRP;
		} else {
			m_ArtPollReply.Status3 &= static_cast<uint8_t>(~artnet::Status3::SUPPORTS_LLRP);
		}
	}

	void Print();

	static ArtNetNode* Get() {
		return s_pThis;
	}

#if (ARTNET_VERSION >= 4)
private:
	void SetUniverse4(const uint32_t nPortIndex, const lightset::PortDir portDir);
	void SetLedBlinkMode4(hardware::ledblink::Mode mode);
	void HandleAddress4(const uint8_t nCommand, const uint32_t nPortIndex);
	uint8_t GetGoodOutput4(const uint32_t nPortIndex);

public:
	/**
	 * Art-Net 4
	 */
	void SetMapUniverse0(const bool bMapUniverse0 = false) {
		m_Node.bMapUniverse0 = bMapUniverse0;
	}
	bool IsMapUniverse0() const {
		return m_Node.bMapUniverse0;
	}

	void SetPriority4(const uint32_t nPriority) {
		m_ArtPollReply.AcnPriority = static_cast<uint8_t>(nPriority);

		for (uint32_t nPortIndex = 0; nPortIndex < e131bridge::MAX_PORTS; nPortIndex++) {
			E131Bridge::SetPriority(nPortIndex, static_cast<uint8_t>(nPriority));
		}
	}

	void SetPortProtocol4(const uint32_t nPortIndex, const artnet::PortProtocol portProtocol);
	artnet::PortProtocol GetPortProtocol4(const uint32_t nPortIndex) const {
		assert(nPortIndex < artnetnode::MAX_PORTS);
		return m_Node.Port[nPortIndex].protocol;
	}

	/**
	 * sACN E1.131
	 */
	void SetPriority4(uint32_t nPortIndex, uint8_t nPriority) {
		E131Bridge::SetPriority(nPortIndex, nPriority);
	}
	uint8_t GetPriority4(uint32_t nPortIndex) const {
		return E131Bridge::GetPriority(nPortIndex);
	}

	bool GetUniverse4(uint32_t nPortIndex, uint16_t &nUniverse, lightset::PortDir portDir) const {
		return E131Bridge::GetUniverse(nPortIndex, nUniverse, portDir);
	}

	uint32_t GetActiveOutputPorts4() const {
		return E131Bridge::GetActiveOutputPorts();
	}

	uint32_t GetActiveInputPorts4() const {
		return E131Bridge::GetActiveInputPorts();
	}
#endif

private:
	void SetUniverseSwitch(const uint32_t nPortIndex, const lightset::PortDir dir, const uint8_t nAddress);
	void SetNetSwitch(const uint32_t nPortIndex, const uint8_t nNetSwitch);
	void SetSubnetSwitch(const uint32_t nPortIndex, const uint8_t nSubnetSwitch);

#if defined (ARTNET_ENABLE_SENDDIAG)
# define UNUSED
#else
# define UNUSED	__attribute__((unused))
#endif

	void SendDiag(UNUSED const artnet::PriorityCodes priorityCode, UNUSED const char *format, ...) {
#if defined (ARTNET_ENABLE_SENDDIAG)
		if (!m_State.SendArtDiagData) {
			return;
		}

		if (static_cast<uint8_t>(priorityCode) < m_State.DiagPriority) {
			return;
		}

		m_DiagData.Priority = static_cast<uint8_t>(priorityCode);

		va_list arp;

		va_start(arp, format);

		auto i = vsnprintf(reinterpret_cast<char *>(m_DiagData.Data), sizeof(m_DiagData.Data) - 1, format, arp);

		va_end(arp);

		m_DiagData.Data[sizeof(m_DiagData.Data) - 1] = '\0';	// Just be sure we have a last '\0'
		m_DiagData.LengthLo = static_cast<uint8_t>(i + 1);		// Text length including the '\0'

		const uint16_t nSize = sizeof(struct artnet::ArtDiagData) - sizeof(m_DiagData.Data) + m_DiagData.LengthLo;

		Network::Get()->SendTo(m_nHandle, &m_DiagData, nSize, m_State.ArtDiagIpAddress, artnet::UDP_PORT);
#endif
	}

#if defined (ARTNET_ENABLE_SENDDIAG)
# undef UNUSED
#endif

	void HandlePoll();
	void HandleDmx();
	void HandleSync();
	void HandleAddress();
	void HandleTimeCode();
	void HandleTimeSync();
	void HandleTodControl();
	void HandleTodData();
	void HandleTodRequest();
	void HandleRdm();
	void HandleIpProg();
	void HandleDmxIn();
	void HandleInput();
	void SetLocalMerging();
	void HandleRdmIn();
	void HandleTrigger();

	uint16_t MakePortAddress(const uint16_t nUniverse, const uint32_t nPage);

	void UpdateMergeStatus(const uint32_t nPortIndex);
	void CheckMergeTimeouts(const uint32_t nPortIndex);

	void ProcessPollRelply(const uint32_t nPortIndex, uint32_t& NumPortsInput, uint32_t& NumPortsOutput);
	void SendPollRelply(const uint32_t nBindIndex, const uint32_t nDestinationIp, artnetnode::ArtPollQueue *pQueue = nullptr);

	void SendTod(uint32_t nPortIndex);
	void SendTodRequest(uint32_t nPortIndex);

	void SetNetworkDataLossCondition();

	void FailSafeRecord();
	void FailSafePlayback();

	void Process(const uint16_t);
private:
	int32_t m_nHandle { -1 };
	uint8_t *m_pReceiveBuffer { nullptr };
	uint32_t m_nIpAddressFrom;
	uint32_t m_nCurrentPacketMillis { 0 };
	uint32_t m_nPreviousPacketMillis { 0 };

	LightSet *m_pLightSet { nullptr };

	ArtNetTimeCode *m_pArtNetTimeCode { nullptr };
	ArtNetRdm *m_pArtNetRdm { nullptr };
	ArtNetTrigger *m_pArtNetTrigger { nullptr };
	ArtNetStore *m_pArtNetStore { nullptr };

	artnetnode::Node m_Node;
	artnetnode::State m_State;
	artnetnode::OutputPort m_OutputPort[artnetnode::MAX_PORTS];
	artnetnode::InputPort m_InputPort[artnetnode::MAX_PORTS];

	artnet::ArtPollReply m_ArtPollReply;
#if defined (ARTNET_HAVE_DMXIN)
	artnet::ArtDmx m_ArtDmx;
#endif
#if defined (RDM_CONTROLLER) || defined (RDM_RESPONDER)
	union UArtTodPacket {
		artnet::ArtTodData ArtTodData;
		artnet::ArtTodRequest ArtTodRequest;
		artnet::ArtRdm ArtRdm;
	};
	UArtTodPacket m_ArtTodPacket;
#endif
#if defined (ARTNET_HAVE_TIMECODE)
	artnet::ArtTimeCode m_ArtTimeCode;
#endif
#if defined (ARTNET_ENABLE_SENDDIAG)
	artnet::ArtDiagData m_DiagData;
#endif

	static ArtNetNode *s_pThis;
};

#endif /* ARTNETNODE_H_ */
