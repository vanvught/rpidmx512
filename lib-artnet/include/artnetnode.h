/**
 * @file artnetnode.h
 *
 */
/**
 * Art-Net Designed by and Copyright Artistic Licence Holdings Ltd.
 */
/* Copyright (C) 2016-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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
#include <cstdarg>
#include <cstring>
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

#if defined (NODE_SHOWFILE) && defined (CONFIG_SHOWFILE_PROTOCOL_NODE_ARTNET)
# define ARTNET_SHOWFILE
#endif

#include "artnet.h"
#include "artnetnode_ports.h"
#include "artnettimecode.h"
#include "artnetdisplay.h"
#include "artnettrigger.h"
#if defined (RDM_CONTROLLER)
# include "artnetrdmcontroller.h"
#endif
#if defined (RDM_RESPONDER)
# include "artnetrdmresponder.h"
#endif
#if (ARTNET_VERSION >= 4)
# include "e131bridge.h"
#endif

#if defined(OUTPUT_DMX_SEND) || defined(OUTPUT_DMX_SEND_MULTI)
# if !defined(ARTNET_DISABLE_DMX_CONFIG_UDP)
#  include "dmxconfigudp.h"
# endif
#endif

#include "lightset.h"
#include "hardware.h"
#include "network.h"

#include "panel_led.h"

#include "debug.h"

#ifndef ALIGNED
# define ALIGNED __attribute__ ((aligned (4)))
#endif

namespace artnetnode {
enum class FailSafe : uint8_t {
	LAST = 0x08, OFF= 0x09, ON = 0x0a, PLAYBACK = 0x0b, RECORD = 0x0c
};

struct State {
	uint32_t ArtDiagIpAddress;
	uint32_t ArtPollIpAddress;
	uint32_t ArtPollReplyCount;
	uint32_t ArtPollReplyDelayMillis;
	uint32_t ArtDmxIpAddress;
	uint32_t ArtSyncMillis;				///< Latest ArtSync received time
	artnet::ArtPollQueue ArtPollReplyQueue[4];
	artnet::ReportCode reportCode;
	artnet::Status status;
	bool SendArtPollReplyOnChange;		///< ArtPoll : Flags Bit 1 : 1 = Send ArtPollReply whenever Node conditions change.
	bool SendArtDiagData;				///< ArtPoll : Flags Bit 2 : 1 = Send me diagnostics messages.
	bool IsMultipleControllersReqDiag;	///< ArtPoll : Multiple controllers requesting diagnostics
	bool IsSynchronousMode;				///< ArtSync received
	bool IsMergeMode;
	bool IsChanged;
	bool bDisableMergeTimeout;
	bool DoRecord;
	uint8_t nReceivingDmx;
	uint8_t nEnabledOutputPorts;
	uint8_t nEnabledInputPorts;
	uint8_t DiagPriority;				///< ArtPoll : Field 6 : The lowest priority of diagnostics message that should be sent.
	struct {
		uint32_t nDiscoveryMillis;
		uint32_t nDiscoveryPortIndex;
		bool IsDiscoveryRunning;
		bool IsEnabled;
	} rdm;
};

struct Node {
	struct {
		uint16_t PortAddress;								///< The Port-Address is a 15 bit number composed of Net+Sub-Net+Universe.
		uint8_t DefaultAddress;
		uint8_t NetSwitch;									///< Bits 14-8 of the 15 bit Port-Address are encoded into the bottom 7 bits of this field.
		uint8_t SubSwitch;									///< Bits 7-4 of the 15 bit Port-Address are encoded into the bottom 4 bits of this field.
		lightset::PortDir direction;
		artnet::PortProtocol protocol;						///< Art-Net 4
		bool bLocalMerge;
		char ShortName[artnet::SHORT_NAME_LENGTH] ALIGNED;
	} Port[artnetnode::MAX_PORTS] ALIGNED;

	uint32_t IPAddressTimeCode;
	bool bMapUniverse0;										///< Art-Net 4
};

struct Source {
	uint32_t nMillis;	///< The latest time of the data received from port
	uint32_t nIp;		///< The IP address for port
	uint16_t nPhysical;	///< The physical input port from which DMX512 data was input.
};

struct OutputPort {
	Source SourceA ALIGNED;
	Source SourceB ALIGNED;
	uint32_t nIpRdm;
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
	if (failsafe > lightset::FailSafe::PLAYBACK) {
		return artnetnode::FailSafe::LAST;
	}

	return static_cast<artnetnode::FailSafe>(static_cast<uint32_t>(failsafe) + static_cast<uint32_t>(artnetnode::FailSafe::LAST));
}

inline lightset::FailSafe convert_failsafe(const artnetnode::FailSafe failsafe) {
	if (failsafe > artnetnode::FailSafe::RECORD) {
		return lightset::FailSafe::HOLD;
	}

	return  static_cast<lightset::FailSafe>(static_cast<uint32_t>(failsafe) - static_cast<uint32_t>(artnetnode::FailSafe::LAST));
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
		const auto nBytesReceived = Network::Get()->RecvFrom(m_nHandle, const_cast<const void **>(reinterpret_cast<void **>(&m_pReceiveBuffer)), &m_nIpAddressFrom, &nForeignPort);
		m_nCurrentPacketMillis = Hardware::Get()->Millis();

		Process(nBytesReceived);

#if (ARTNET_VERSION >= 4)
		E131Bridge::Run();
#endif
#if defined (RDM_CONTROLLER)
		if (__builtin_expect((m_State.rdm.IsEnabled), 0)) {
			assert(m_pArtNetRdmController != nullptr);
			m_pArtNetRdmController->Run();

			if (__builtin_expect((!m_State.rdm.IsDiscoveryRunning && ((m_nCurrentPacketMillis - m_State.rdm.nDiscoveryMillis) > (1000 * 60 * 15))), 0)) {
				DEBUG_PUTS("RDM Discovery -> START");
				m_State.rdm.IsDiscoveryRunning = true;
			}

			if (__builtin_expect((m_State.rdm.IsDiscoveryRunning), 0)) {
				m_State.rdm.IsDiscoveryRunning = RdmDiscoveryRun();

				if (!m_State.rdm.IsDiscoveryRunning) {
					DEBUG_PUTS("RDM Discovery -> DONE");
					m_State.rdm.nDiscoveryPortIndex = 0;
					m_State.rdm.nDiscoveryMillis = m_nCurrentPacketMillis;
				}
			} else {
				uint32_t nPortIndex;
				bool bIsIncremental;
				if (m_pArtNetRdmController->IsFinished(nPortIndex, bIsIncremental)) {
					SendTod(nPortIndex);

					DEBUG_PRINTF("TOD sent -> %u", static_cast<unsigned int>(nPortIndex));

					if (m_OutputPort[nPortIndex].IsTransmitting) {
						DEBUG_PUTS("m_pLightSet->Stop/Start");
						m_pLightSet->Stop(nPortIndex);
						m_pLightSet->Start(nPortIndex);
					}
				}
			}
		}
#endif
		if ((m_nCurrentPacketMillis - m_nPreviousLedpanelMillis) > 200) {
			m_nPreviousLedpanelMillis = m_nCurrentPacketMillis;
			for (uint32_t nPortIndex = 0; nPortIndex < artnetnode::MAX_PORTS; nPortIndex++) {
				hal::panel_led_off(hal::panelled::PORT_A_TX << nPortIndex);
#if defined (ARTNET_HAVE_DMXIN)
				hal::panel_led_off(hal::panelled::PORT_A_RX << nPortIndex);
#endif
#if defined(CONFIG_PANELLED_RDM_PORT)
				hal::panel_led_off(hal::panelled::PORT_A_RDM << nPortIndex);
#elif defined(CONFIG_PANELLED_RDM_NO_PORT)
				hal::panel_led_off(hal::panelled::RDM << nPortIndex);
#endif
			}
		}

#if defined (DMXCONFIGUDP_H)
		m_DmxConfigUdp.Run();
#endif
	}

#if defined (ARTNET_SHOWFILE)
	void HandleShowFile(const artnet::ArtDmx *pArtDmx) {
		m_nCurrentPacketMillis = Hardware::Get()->Millis();
		m_nIpAddressFrom = Network::Get()->GetIp();
		m_pReceiveBuffer = reinterpret_cast<uint8_t *>(const_cast<artnet::ArtDmx *>(pArtDmx));
		HandleDmx();
	}
#endif

	void SetRecordShowfile(const bool doRecord) {
		m_State.DoRecord = doRecord;
	}
	bool GetRecordShowfile() const {
		return m_State.DoRecord;
	}

	uint8_t GetVersion() const {
		return artnet::VERSION;
	}

	void SetOutputStyle(const uint32_t nPortIndex, lightset::OutputStyle outputStyle);
	lightset::OutputStyle GetOutputStyle(const uint32_t nPortIndex) const;

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

	void SetRdm(const bool doEnable);
	bool GetRdm() const {
		return m_State.rdm.IsEnabled;
	}

	void SetRdm(const uint32_t nPortIndex, const bool bEnable);
	bool GetRdm(const uint32_t nPortIndex) const {
		assert(nPortIndex < artnetnode::MAX_PORTS);
		return !((m_OutputPort[nPortIndex].GoodOutputB & artnet::GoodOutputB::RDM_DISABLED) == artnet::GoodOutputB::RDM_DISABLED);
	}

	void SetRdmDiscovery(const uint32_t nPortIndex, const bool bEnable);
	bool GetRdmDiscovery(const uint32_t nPortIndex) const {
		assert(nPortIndex < artnetnode::MAX_PORTS);
		return !((m_OutputPort[nPortIndex].GoodOutputB & artnet::GoodOutputB::DISCOVERY_DISABLED) == artnet::GoodOutputB::DISCOVERY_DISABLED);
	}

#if defined (RDM_CONTROLLER)
	void SetRdmController(ArtNetRdmController *pArtNetRdmController, const bool doEnable = true);

	uint32_t RdmCopyWorkingQueue(char *pOutBuffer, const uint32_t nOutBufferSize) {
		if (m_pArtNetRdmController != nullptr) {
			return m_pArtNetRdmController->CopyWorkingQueue(pOutBuffer, nOutBufferSize);
		}

		return 0;
	}

	uint32_t RdmGetUidCount(const uint32_t nPortIndex) {
		if (m_pArtNetRdmController != nullptr) {
			return m_pArtNetRdmController->GetUidCount(nPortIndex);
		}

		return 0;
	}

	uint32_t RdmCopyTod(const uint32_t nPortIndex, char *pOutBuffer, const uint32_t nOutBufferSize) {
		if (m_pArtNetRdmController != nullptr) {
			return m_pArtNetRdmController->CopyTod(nPortIndex, pOutBuffer, nOutBufferSize);
		}

		return 0;
	}

	bool RdmIsRunning(const uint32_t nPortIndex, bool& bIsIncremental) {
		uint32_t nRdmnPortIndex;
		if (m_pArtNetRdmController->IsRunning(nRdmnPortIndex, bIsIncremental)) {
			const auto isRunning = (nRdmnPortIndex == nPortIndex);
			if (isRunning) {
				assert(!((m_OutputPort[nPortIndex].GoodOutputB & artnet::GoodOutputB::DISCOVERY_NOT_RUNNING) == artnet::GoodOutputB::DISCOVERY_NOT_RUNNING));
			}
			return isRunning;
		}

		return false;
	}

#endif

#if defined (RDM_RESPONDER)
	void SetRdmResponder(ArtNetRdmResponder *pArtNetRdmResponder, const bool doEnable = true);
#endif

	void SetDisableMergeTimeout(bool bDisable) {
		m_State.bDisableMergeTimeout = bDisable;
#if (ARTNET_VERSION >= 4)
		E131Bridge::SetDisableMergeTimeout(bDisable);
#endif
	}

	bool GetDisableMergeTimeout() const {
		return m_State.bDisableMergeTimeout;
	}

#if defined (ARTNET_HAVE_TIMECODE)
	void SendTimeCode(const struct artnet::TimeCode *pArtNetTimeCode) {
		assert(pArtNetTimeCode != nullptr);
		assert(pArtNetTimeCode->Frames < 30);
		assert(pArtNetTimeCode->Hours < 60);
		assert(pArtNetTimeCode->Minutes < 60);
		assert(pArtNetTimeCode->Seconds < 60);
		assert(pArtNetTimeCode->Type < 4);

		memcpy(&m_ArtTimeCode.Frames, pArtNetTimeCode, sizeof(struct artnet::TimeCode));
		Network::Get()->SendTo(m_nHandle, &m_ArtTimeCode, sizeof(struct artnet::ArtTimeCode), m_Node.IPAddressTimeCode, artnet::UDP_PORT);
	}

	void SetTimeCodeHandler(ArtNetTimeCode *pArtNetTimeCode) {
		m_pArtNetTimeCode = pArtNetTimeCode;
	}

	void SetTimeCodeIp(const uint32_t nDestinationIp) {
		m_Node.IPAddressTimeCode = nDestinationIp;
	}
#endif

	void SetArtNetTrigger(ArtNetTrigger *pArtNetTrigger) {
		m_pArtNetTrigger = pArtNetTrigger;
	}

	void SetDestinationIp(const uint32_t nPortIndex, const uint32_t nDestinationIp) {
		if (nPortIndex < artnetnode::MAX_PORTS) {
			m_InputPort[nPortIndex].nDestinationIp = nDestinationIp;
			DEBUG_PRINTF("nDestinationIp=" IPSTR, IP2STR(m_InputPort[nPortIndex].nDestinationIp));
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

	void SendDiag([[maybe_unused]] const artnet::PriorityCodes priorityCode, [[maybe_unused]] const char *format, ...) {
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
	void HandleRdmSub();
	void HandleIpProg();
	void HandleDmxIn();
	void HandleInput();
	void SetLocalMerging();
	void HandleRdmIn();
	void HandleTrigger();

	uint16_t MakePortAddress(const uint16_t nUniverse, const uint32_t nPage) {
		return artnet::make_port_address(m_Node.Port[nPage].NetSwitch, m_Node.Port[nPage].SubSwitch, nUniverse);
	}

	void UpdateMergeStatus(const uint32_t nPortIndex);
	void CheckMergeTimeouts(const uint32_t nPortIndex);

	void ProcessPollRelply(const uint32_t nPortIndex, uint32_t& NumPortsInput, uint32_t& NumPortsOutput);
	void SendPollRelply(const uint32_t nBindIndex, const uint32_t nDestinationIp, artnet::ArtPollQueue *pQueue = nullptr);

	void SendTod(uint32_t nPortIndex);
	void SendTodRequest(uint32_t nPortIndex);

	void SetNetworkDataLossCondition();

	void FailSafeRecord();
	void FailSafePlayback();

	void Process(const uint32_t);

#if defined (RDM_CONTROLLER)
	bool RdmDiscoveryRun() {
		if ((GetPortDirection(m_State.rdm.nDiscoveryPortIndex) == lightset::PortDir::OUTPUT)
				&& (GetRdm(m_State.rdm.nDiscoveryPortIndex))
				&& (GetRdmDiscovery(m_State.rdm.nDiscoveryPortIndex)))
		{
			uint32_t nPortIndex;
			bool bIsIncremental;

			if (m_pArtNetRdmController->IsFinished(nPortIndex, bIsIncremental)) {
				assert(m_State.rdm.nDiscoveryPortIndex == nPortIndex);

				SendTod(nPortIndex);

				DEBUG_PUTS("TOD sent");

				if (m_OutputPort[nPortIndex].IsTransmitting) {
					DEBUG_PUTS("m_pLightSet->Stop/Start");
					m_pLightSet->Stop(nPortIndex);
					m_pLightSet->Start(nPortIndex);
				}

				m_OutputPort[m_State.rdm.nDiscoveryPortIndex].GoodOutputB |= artnet::GoodOutputB::DISCOVERY_NOT_RUNNING;

				m_State.rdm.nDiscoveryPortIndex++;
				return (m_State.rdm.nDiscoveryPortIndex != artnetnode::MAX_PORTS);
			}

			if (!m_pArtNetRdmController->IsRunning(nPortIndex, bIsIncremental)) {
				DEBUG_PRINTF("RDM Discovery Incremental -> %u", static_cast<unsigned int>(m_State.rdm.nDiscoveryPortIndex));
				m_pArtNetRdmController->Incremental(m_State.rdm.nDiscoveryPortIndex);
				m_OutputPort[m_State.rdm.nDiscoveryPortIndex].GoodOutputB &= static_cast<uint8_t>(~artnet::GoodOutputB::DISCOVERY_NOT_RUNNING);
			}

			return true;
		}

		m_State.rdm.nDiscoveryPortIndex++;
		return (m_State.rdm.nDiscoveryPortIndex != artnetnode::MAX_PORTS);
	}
#endif

private:
	int32_t m_nHandle { -1 };
	uint8_t *m_pReceiveBuffer { nullptr };
	uint32_t m_nIpAddressFrom;
	uint32_t m_nCurrentPacketMillis { 0 };
	uint32_t m_nPreviousPacketMillis { 0 };
	uint32_t m_nPreviousLedpanelMillis { 0 };

	LightSet *m_pLightSet { nullptr };

	ArtNetTimeCode *m_pArtNetTimeCode { nullptr };
	ArtNetTrigger *m_pArtNetTrigger { nullptr };

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
# if defined (RDM_CONTROLLER)
	ArtNetRdmController *m_pArtNetRdmController;
# endif
# if defined (RDM_RESPONDER)
	ArtNetRdmResponder *m_pArtNetRdmResponder;
# endif
#endif
#if defined (ARTNET_HAVE_TIMECODE)
	artnet::ArtTimeCode m_ArtTimeCode;
#endif
#if defined (ARTNET_ENABLE_SENDDIAG)
	artnet::ArtDiagData m_DiagData;
#endif
#if defined (DMXCONFIGUDP_H_)
	DmxConfigUdp m_DmxConfigUdp;
#endif

	static ArtNetNode *s_pThis;
};

#endif /* ARTNETNODE_H_ */
