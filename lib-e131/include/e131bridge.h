/**
 * @file e131bridge.h
 *
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

#ifndef E131BRIDGE_H_
#define E131BRIDGE_H_

#include <stdint.h>
#include <assert.h>

#include "e131.h"
#include "e131packets.h"

#include "lightset.h"

// Handlers
#include "e131dmx.h"
#include "e131sync.h"

enum {
	E131_MAX_UARTS = 4
};

#define UUID_STRING_LENGTH	36

struct TE131BridgeState {
	bool IsNetworkDataLoss;
	bool IsMergeMode;				///< Is the Bridge in merging mode?
	bool IsSynchronized;			///< “Synchronized” or an “Unsynchronized” state.
	bool IsForcedSynchronized;
	bool IsChanged;
	bool bDisableNetworkDataLossTimeout;
	bool bDisableMergeTimeout;
	bool bIsReceivingDmx;
	bool bDisableSynchronize;
	uint32_t SynchronizationTime;
	uint32_t DiscoveryTime;
	uint16_t DiscoveryPacketLength;
	uint16_t nSynchronizationAddressSourceA;
	uint16_t nSynchronizationAddressSourceB;
	uint8_t nActiveInputPorts;
	uint8_t nActiveOutputPorts;
	uint8_t nPriority;
};

struct TSource {
	uint32_t time;
	uint32_t ip;
	uint8_t data[E131_DMX_LENGTH];
	uint8_t cid[E131_CID_LENGTH];
	uint8_t sequenceNumberData;
};

struct TE131OutputPort {
	uint8_t data[E131_DMX_LENGTH];
	uint16_t length;
	uint16_t nUniverse;
	E131Merge mergeMode;
	bool IsDataPending;
	bool bIsEnabled;
	bool IsTransmitting;
	bool IsMerging;
	struct TSource sourceA;
	struct TSource sourceB;
};

struct TE131InputPort {
	uint16_t nUniverse;
	bool bIsEnabled;
	bool IsTransmitting;
	uint8_t nSequenceNumber;
	uint8_t nPriority;
	uint32_t nMulticastIp;
};

class E131Bridge {
public:
	E131Bridge();
	~E131Bridge();

	void SetOutput(LightSet *pLightSet) {
		m_pLightSet = pLightSet;
	}

	const uint8_t *GetSoftwareVersion();

	void SetUniverse(uint8_t nPortIndex, TE131PortDir dir, uint16_t nUniverse);
	bool GetUniverse(uint8_t nPortIndex, uint16_t &nUniverse, TE131PortDir tDir = E131_OUTPUT_PORT) const;

	void SetMergeMode(uint8_t nPortIndex, E131Merge tE131Merge);
	E131Merge GetMergeMode(uint8_t nPortIndex) const;

	uint8_t GetActiveOutputPorts() const {
		return m_State.nActiveOutputPorts;
	}

	uint8_t GetActiveInputPorts() const {
		return m_State.nActiveInputPorts;
	}

	void SetDirectUpdate(bool bDirectUpdate) {
		m_bDirectUpdate = bDirectUpdate;
	}
	bool GetDirectUpdate() const {
		return m_bDirectUpdate;
	}

	bool IsTransmitting(uint8_t nPortIndex) const;
	bool IsMerging(uint8_t nPortIndex) const;
	bool IsStatusChanged();

	void SetDisableNetworkDataLossTimeout(bool bDisable = true) {
		m_State.bDisableNetworkDataLossTimeout = bDisable;
	}
	bool GetDisableNetworkDataLossTimeout() const {
		return m_State.bDisableNetworkDataLossTimeout;
	}

	void SetDisableMergeTimeout(bool bDisable = true) {
		m_State.bDisableMergeTimeout = bDisable;
	}
	bool GetDisableMergeTimeout() const {
		return m_State.bDisableMergeTimeout;
	}

	void SetEnableDataIndicator(bool bEnable = true) {
		m_bEnableDataIndicator = bEnable;
	}
	bool GetEnableDataIndicator() const {
		return m_bEnableDataIndicator;
	}

	void SetDisableSynchronize(bool bDisableSynchronize = false) {
		m_State.bDisableSynchronize = bDisableSynchronize;
	}
	bool GetDisableSynchronize() const {
		return m_State.bDisableSynchronize;
	}

	void SetE131Dmx(E131Dmx *pE131Dmx) {
		m_pE131DmxIn = pE131Dmx;
	}

	void SetE131Sync(E131Sync *pE131Sync) {
		m_pE131Sync = pE131Sync;
	}

	const uint8_t *GetCid() const {
		return m_Cid;
	}

	void SetSourceName(const char *pSourceName);
	const char *GetSourceName() const {
		return m_SourceName;
	}

	void SetPriority(uint8_t nPriority, uint8_t nPortIndex = 0) {
		assert(nPortIndex < E131_MAX_UARTS);
		if ((nPriority >= E131_PRIORITY_LOWEST) && (nPriority <= E131_PRIORITY_HIGHEST)) {
			m_InputPort[nPortIndex].nPriority = nPriority;
		}
	}
	uint8_t GetPriority(uint8_t nPortIndex = 0) const {
		assert(nPortIndex < E131_MAX_UARTS);
		return m_InputPort[nPortIndex].nPriority;
	}

	void Clear(uint8_t nPortIndex);

	void Start();
	void Stop();

	void Run();

	void Print();

	static E131Bridge* Get() {
		return s_pThis;
	}

private:
	bool IsValidRoot();
	bool IsValidDataPacket();

	void SetNetworkDataLossCondition(bool bSourceA = true, bool bSourceB = true);

	void SetSynchronizationAddress(bool bSourceA, bool bSourceB, uint16_t nSynchronizationAddress);

	void CheckMergeTimeouts(uint8_t nPortIndex);
	bool IsPriorityTimeOut(uint8_t nPortIndex);
	bool isIpCidMatch(const struct TSource *);
	bool IsDmxDataChanged(uint8_t nPortIndex, const uint8_t *pData, uint16_t nLength);
	bool IsMergedDmxDataChanged(uint8_t nPortIndex, const uint8_t *pData, uint16_t nLength);

	void HandleDmx();
	void HandleSynchronization();

	uint32_t UniverseToMulticastIp(uint16_t nUniverse) const;
	void LeaveUniverse(uint8_t nPortIndex, uint16_t nUniverse);

	// Input
	void HandleDmxIn();
	void FillDataPacket();
	void FillDiscoveryPacket();
	void SendDiscoveryPacket();

private:
	int32_t m_nHandle{-1};

	LightSet *m_pLightSet{nullptr};

	bool m_bDirectUpdate{false};
	bool m_bEnableDataIndicator{true};

	uint32_t m_nCurrentPacketMillis{0};
	uint32_t m_nPreviousPacketMillis{0};

	struct TE131BridgeState m_State;
	struct TE131OutputPort m_OutputPort[E131_MAX_PORTS];
	struct TE131InputPort m_InputPort[E131_MAX_UARTS];
	struct TE131 m_E131;

	// Input
	E131Dmx *m_pE131DmxIn{nullptr};
	TE131DataPacket *m_pE131DataPacket{nullptr};
	TE131DiscoveryPacket *m_pE131DiscoveryPacket{nullptr};
	uint32_t m_DiscoveryIpAddress{0};
	uint8_t m_Cid[E131_CID_LENGTH];
	char m_SourceName[E131_SOURCE_NAME_LENGTH];

	// Synchronization handler
	E131Sync *m_pE131Sync{nullptr};

	static E131Bridge *s_pThis;
};

#endif /* E131BRIDGE_H_ */
