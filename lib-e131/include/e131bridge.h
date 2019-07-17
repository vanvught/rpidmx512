/**
 * @file e131bridge.h
 *
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

#ifndef E131BRIDGE_H_
#define E131BRIDGE_H_

#include <stdint.h>

#include "e131.h"
#include "e131packets.h"

#include "lightset.h"

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
	uint32_t SynchronizationTime;
	uint32_t DiscoveryTime;
	uint16_t DiscoveryPacketLength;
	uint16_t nSynchronizationAddressSourceA;
	uint16_t nSynchronizationAddressSourceB;
	uint8_t nActivePorts;
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
	TE131Merge mergeMode;
	bool IsDataPending;
	bool bIsEnabled;
	bool IsTransmitting;
	bool IsMerging;
	struct TSource sourceA;
	struct TSource sourceB;
};

class E131Bridge {
public:
	E131Bridge(void);
	~E131Bridge(void);

	void SetOutput(LightSet *pLightSet);

	const uint8_t *GetSoftwareVersion(void);

	void SetUniverse(uint8_t nPortIndex, TE131PortDir dir, uint16_t nUniverse);
	bool GetUniverse(uint8_t nPortIndex, uint16_t &nUniverse) const;

	void SetMergeMode(uint8_t nPortIndex, TE131Merge tE131Merge);
	TE131Merge GetMergeMode(uint8_t nPortIndex) const;

	uint8_t GetActiveOutputPorts(void) {
		return m_State.nActivePorts;
	}

	void SetDirectUpdate(bool bDirectUpdate) {
		m_bDirectUpdate = bDirectUpdate;
	}
	bool GetDirectUpdate(void) {
		return m_bDirectUpdate;
	}

	bool IsTransmitting(uint8_t nPortIndex) const;
	bool IsMerging(uint8_t nPortIndex) const;
	bool IsStatusChanged(void);

	void SetDisableNetworkDataLossTimeout(bool bDisable = true) {
		m_State.bDisableNetworkDataLossTimeout = bDisable;
	}
	bool GetDisableNetworkDataLossTimeout(void) {
		return m_State.bDisableNetworkDataLossTimeout;
	}

	void SetDisableMergeTimeout(bool bDisable = true) {
		m_State.bDisableMergeTimeout = bDisable;
	}
	bool GetDisableMergeTimeout(void) {
		return m_State.bDisableMergeTimeout;
	}

	void SetEnableDataIndicator(bool bEnable = true) {
		m_bEnableDataIndicator = bEnable;
	}
	bool GetEnableDataIndicator(void) {
		return m_bEnableDataIndicator;
	}

	void Clear(uint8_t nPortIndex);

	void Start(void);
	void Stop(void);

	int Run(void);

	void Print(void);

private:
	bool IsValidRoot(void);
	bool IsValidDataPacket(void);

	void SetNetworkDataLossCondition(bool bSourceA = true, bool bSourceB = true);

	void SetSynchronizationAddress(bool bSourceA, bool bSourceB, uint16_t nSynchronizationAddress);

	void CheckMergeTimeouts(uint8_t nPortIndex);
	bool IsPriorityTimeOut(uint8_t nPortIndex);
	bool isIpCidMatch(const struct TSource *);
	bool IsDmxDataChanged(uint8_t nPortIndex, const uint8_t *pData, uint16_t nLength);
	bool IsMergedDmxDataChanged(uint8_t nPortIndex, const uint8_t *pData, uint16_t nLength);

	void HandleDmx(void);
	void HandleSynchronization(void);

	uint32_t UniverseToMulticastIp(uint16_t nUniverse) const;
	void LeaveUniverse(uint8_t nPortIndex, uint16_t nUniverse);

private:
	int32_t m_nHandle;

	LightSet *m_pLightSet;

	bool m_bDirectUpdate;
	bool m_bEnableDataIndicator;

	uint32_t m_nCurrentPacketMillis;
	uint32_t m_nPreviousPacketMillis;

	struct TE131BridgeState m_State;
	struct TE131OutputPort m_OutputPort[E131_MAX_PORTS];
	struct TE131 m_E131;
};

#endif /* E131BRIDGE_H_ */
