/**
 * @file e131bridge.h
 *
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

#ifndef E131BRIDGE_H_
#define E131BRIDGE_H_

#include <cstdint>
#include <cassert>

#include "e131.h"
#include "e131packets.h"
// Handlers
#include "e131sync.h"

#include "lightset.h"
#include "lightsetdata.h"

#include "network.h"
#include "hardware.h"

#include "debug.h"

namespace e131bridge {
#if !defined(LIGHTSET_PORTS)
# error LIGHTSET_PORTS is not defined
#endif

#if (LIGHTSET_PORTS == 0)
 static constexpr uint32_t MAX_PORTS = 1;
#else
 static constexpr uint32_t MAX_PORTS = LIGHTSET_PORTS;
#endif

 enum class Status : uint8_t {
 	OFF, STANDBY, ON
 };

struct State {
	bool IsNetworkDataLoss;
	bool IsMergeMode;
	bool IsSynchronized;
	bool IsForcedSynchronized;
	bool IsChanged;
	bool bDisableMergeTimeout;
	bool bDisableSynchronize;
	uint32_t SynchronizationTime;
	uint32_t DiscoveryTime;
	uint16_t DiscoveryPacketLength;
	uint16_t nSynchronizationAddressSourceA;
	uint16_t nSynchronizationAddressSourceB;
	uint8_t nEnabledInputPorts;
	uint8_t nEnableOutputPorts;
	uint8_t nPriority;
	uint8_t nReceivingDmx;
	lightset::FailSafe failsafe;
	e131bridge::Status status;
};

struct Bridge {
	struct {
		uint16_t nUniverse;
		lightset::PortDir direction;
		bool bLocalMerge;
	} Port[e131bridge::MAX_PORTS];
};

struct Source {
	uint32_t nMillis;
	uint32_t nIp;
	uint8_t cid[e131::CID_LENGTH];
	uint8_t nSequenceNumberData;
};

struct OutputPort {
	Source sourceA;
	Source sourceB;
	lightset::MergeMode mergeMode;
	lightset::OutputStyle outputStyle;
	bool IsMerging;
	bool IsTransmitting;
};

struct InputPort {
	uint32_t nMulticastIp;
	uint32_t nMillis;
	uint8_t nSequenceNumber;
	uint8_t nPriority;
	bool IsDisabled;
};
}  // namespace e131bridge

class E131Bridge {
public:
	E131Bridge();
	~E131Bridge();

	void SetFailSafe(const lightset::FailSafe failsafe) {
		m_State.failsafe = failsafe;
	}
	lightset::FailSafe GetFailSafe() const {
		return m_State.failsafe;
	}

	void SetOutput(LightSet *pLightSet) {
		m_pLightSet = pLightSet;
	}
	LightSet *GetOutput() const {
		return m_pLightSet;
	}

	void SetUniverse(const uint32_t nPortIndex, const lightset::PortDir portDir, const uint16_t nUniverse);
	bool GetUniverse(const uint32_t nPortIndex, uint16_t &nUniverse, lightset::PortDir portDir) const {
		assert(nPortIndex < e131bridge::MAX_PORTS);

		if (portDir == lightset::PortDir::DISABLE) {
			return false;
		}

		nUniverse = m_Bridge.Port[nPortIndex].nUniverse;
		return m_Bridge.Port[nPortIndex].direction == portDir;
	}

	bool GetOutputPort(const uint16_t nUniverse, uint32_t& nPortIndex) {
		for (nPortIndex = 0; nPortIndex < e131bridge::MAX_PORTS; nPortIndex++) {
			if (m_Bridge.Port[nPortIndex].direction != lightset::PortDir::OUTPUT) {
				continue;
			}
			if (m_Bridge.Port[nPortIndex].nUniverse == nUniverse) {
				return true;
			}
		}
		return false;
	}

	void SetMergeMode(uint32_t nPortIndex, lightset::MergeMode mergeMode) {
		assert(nPortIndex < e131bridge::MAX_PORTS);
		m_OutputPort[nPortIndex].mergeMode = mergeMode;
	}
	lightset::MergeMode GetMergeMode(uint32_t nPortIndex) const {
		assert(nPortIndex < e131bridge::MAX_PORTS);
		return m_OutputPort[nPortIndex].mergeMode;
	}

	uint32_t GetActiveOutputPorts() const {
		return m_State.nEnableOutputPorts;
	}

	uint32_t GetActiveInputPorts() const {
		return m_State.nEnabledInputPorts;
	}

	bool IsTransmitting(uint32_t nPortIndex) const {
		assert(nPortIndex < e131bridge::MAX_PORTS);
		return m_OutputPort[nPortIndex].IsTransmitting;
	}

	bool IsMerging(uint32_t nPortIndex) const {
		assert(nPortIndex < e131bridge::MAX_PORTS);
		return m_OutputPort[nPortIndex].IsMerging;
	}

	bool IsStatusChanged() {
		if (m_State.IsChanged) {
			m_State.IsChanged = false;
			return true;
		}

		return false;
	}

	void SetDisableMergeTimeout(bool bDisable) {
		m_State.bDisableMergeTimeout = bDisable;
	}
	bool GetDisableMergeTimeout() const {
		return m_State.bDisableMergeTimeout;
	}

	void SetEnableDataIndicator(bool bEnable) {
		m_bEnableDataIndicator = bEnable;
	}
	bool GetEnableDataIndicator() const {
		return m_bEnableDataIndicator;
	}

	void SetDisableSynchronize(bool bDisableSynchronize) {
		m_State.bDisableSynchronize = bDisableSynchronize;
	}
	bool GetDisableSynchronize() const {
		return m_State.bDisableSynchronize;
	}

	void SetE131Sync(E131Sync *pE131Sync) {
		m_pE131Sync = pE131Sync;
	}

	const uint8_t *GetCid() const {
		return m_Cid;
	}

	void SetSourceName(const char *pSourceName) {
		assert(pSourceName != nullptr);
		strncpy(m_SourceName, pSourceName, e131::SOURCE_NAME_LENGTH - 1);
		m_SourceName[e131::SOURCE_NAME_LENGTH - 1] = '\0';
	}
	const char *GetSourceName() const {
		return m_SourceName;
	}

	void SetPriority(const uint32_t nPortIndex, uint8_t nPriority) {
		assert(nPortIndex < e131bridge::MAX_PORTS);
		if ((nPriority >= e131::priority::LOWEST) && (nPriority <= e131::priority::HIGHEST)) {
			m_InputPort[nPortIndex].nPriority = nPriority;
		}
	}
	uint8_t GetPriority(const uint32_t nPortIndex) const {
		assert(nPortIndex < e131bridge::MAX_PORTS);
		return m_InputPort[nPortIndex].nPriority;
	}

	void SetInputDisabled(const uint32_t nPortIndex, const bool bDisable) {
		assert(nPortIndex < e131bridge::MAX_PORTS);
		m_InputPort[nPortIndex].IsDisabled = bDisable;
	}
	bool GetInputDisabled(const uint32_t nPortIndex) const {
		return m_InputPort[nPortIndex].IsDisabled;
	}

#if defined (OUTPUT_HAVE_STYLESWITCH)
	void SetOutputStyle(const uint32_t nPortIndex, lightset::OutputStyle outputStyle) {
		assert(nPortIndex < e131bridge::MAX_PORTS);

		if ((m_State.status == e131bridge::Status::ON) && (m_pLightSet != nullptr)) {
			m_pLightSet->SetOutputStyle(nPortIndex, outputStyle);
			outputStyle = m_pLightSet->GetOutputStyle(nPortIndex);
		}

		m_OutputPort[nPortIndex].outputStyle = outputStyle;

#if defined (OUTPUT_DMX_SEND) || defined (OUTPUT_DMX_SEND_MULTI)
		/**
		 * FIXME I do not like this hack. It should be handled in dmx.cpp
		 */
		if (m_Bridge.Port[nPortIndex].direction == lightset::PortDir::OUTPUT
				&& (outputStyle == lightset::OutputStyle::CONSTANT)
				&& (m_pLightSet != nullptr)) {
			if (m_OutputPort[nPortIndex].IsTransmitting) {
				m_OutputPort[nPortIndex].IsTransmitting = false;
				m_pLightSet->Stop(nPortIndex);
			}
		}
#endif

	}

	lightset::OutputStyle GetOutputStyle(const uint32_t nPortIndex) const {
		assert(nPortIndex < e131bridge::MAX_PORTS);
		return m_OutputPort[nPortIndex].outputStyle;
	}
#endif

	void Clear(const uint32_t nPortIndex) {
		assert(nPortIndex < e131bridge::MAX_PORTS);

		lightset::Data::OutputClear(m_pLightSet, nPortIndex);

		if ((m_Bridge.Port[nPortIndex].direction == lightset::PortDir::OUTPUT) && !m_OutputPort[nPortIndex].IsTransmitting) {
			m_pLightSet->Start(nPortIndex);
			m_OutputPort[nPortIndex].IsTransmitting = true;
		}

		m_State.IsNetworkDataLoss = false; // Force timeout
	}

	void Start();
	void Stop();

	void Run() {
		uint16_t nForeignPort;

		const auto nBytesReceived = Network::Get()->RecvFrom(m_nHandle, const_cast<const void **>(reinterpret_cast<void **>(&m_pReceiveBuffer)), &m_nIpAddressFrom, &nForeignPort) ;

		m_nCurrentPacketMillis = Hardware::Get()->Millis();

		if (__builtin_expect((nBytesReceived == 0), 1)) {
			if (m_State.nEnableOutputPorts != 0) {
				if ((m_nCurrentPacketMillis - m_nPreviousPacketMillis) >= static_cast<uint32_t>(e131::NETWORK_DATA_LOSS_TIMEOUT_SECONDS * 1000)) {
					if ((m_pLightSet != nullptr) && (!m_State.IsNetworkDataLoss)) {
						SetNetworkDataLossCondition();
					}
				}

				if ((m_nCurrentPacketMillis - m_nPreviousPacketMillis) >= 1000) {
					m_State.nReceivingDmx &= static_cast<uint8_t>(~(1U << static_cast<uint8_t>(lightset::PortDir::OUTPUT)));
				}
			}

#if defined (E131_HAVE_DMXIN)
			HandleDmxIn();
			SendDiscoveryPacket();
#endif

			// The hardware::ledblink::Mode::FAST is for RDM Identify (Art-Net 4)
			if (m_bEnableDataIndicator && (Hardware::Get()->GetMode() != hardware::ledblink::Mode::FAST)) {
				if (m_State.nReceivingDmx != 0) {
					Hardware::Get()->SetMode(hardware::ledblink::Mode::DATA);
				} else {
					Hardware::Get()->SetMode(hardware::ledblink::Mode::NORMAL);
				}
			}

			return;
		}

		if (__builtin_expect((!IsValidRoot()), 0)) {
			return;
		}

		Process();

#if defined (LIGHTSET_HAVE_RUN)
# if (ARTNET_VERSION < 4)
		m_pLightSet->Run();
# endif
#endif
	}

	void Print();

	static E131Bridge* Get() {
		return s_pThis;
	}

private:
	bool IsValidRoot();
	bool IsValidDataPacket();

	void SetNetworkDataLossCondition(bool bSourceA = true, bool bSourceB = true);

	void SetSynchronizationAddress(bool bSourceA, bool bSourceB, uint16_t nSynchronizationAddress);

	void CheckMergeTimeouts(uint32_t nPortIndex);
	bool IsPriorityTimeOut(uint32_t nPortIndex) const;
	bool isIpCidMatch(const e131bridge::Source *const) const;
	void UpdateMergeStatus(const uint32_t nPortIndex);

	void HandleDmx();
	void HandleSynchronization();

	void LeaveUniverse(uint32_t nPortIndex, uint16_t nUniverse);

	void HandleDmxIn();
	void SetLocalMerging();
	void FillDataPacket();
	void FillDiscoveryPacket();
	void SendDiscoveryPacket();

	void Process();
private:
	int32_t m_nHandle { -1 };

	uint32_t m_nCurrentPacketMillis { 0 };
	uint32_t m_nPreviousPacketMillis { 0 };

	TE131DataPacket *m_pE131DataPacket { nullptr };
	TE131DiscoveryPacket *m_pE131DiscoveryPacket { nullptr };
	uint32_t m_DiscoveryIpAddress { 0 };
	uint8_t m_Cid[e131::CID_LENGTH];
	char m_SourceName[e131::SOURCE_NAME_LENGTH];

	e131bridge::State m_State;
	e131bridge::Bridge m_Bridge;
	e131bridge::OutputPort m_OutputPort[e131bridge::MAX_PORTS];
	e131bridge::InputPort m_InputPort[e131bridge::MAX_PORTS];

	bool m_bEnableDataIndicator { true };

	uint8_t *m_pReceiveBuffer;
	uint32_t m_nIpAddressFrom;
	LightSet *m_pLightSet { nullptr };

	// Synchronization handler
	E131Sync *m_pE131Sync { nullptr };

	static E131Bridge *s_pThis;
};

#endif /* E131BRIDGE_H_ */
