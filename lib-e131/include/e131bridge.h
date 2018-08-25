/**
 * @file e131bridge.h
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

#ifndef E131BRIDGE_H_
#define E131BRIDGE_H_

#include <stdint.h>

#include "e131.h"
#include "e131packets.h"

#include "lightset.h"

#define UUID_STRING_LENGTH	36

struct TE131BridgeState {
	uint8_t nPriority;
	bool IsNetworkDataLoss;			///<
	bool IsMergeMode;				///< Is the Bridge in merging mode?
	bool IsTransmitting;			///<
	bool IsSynchronized;			///< “Synchronized” or an “Unsynchronized” state.
	bool IsForcedSynchronized;		///<
	uint32_t SynchronizationTime;	///<
	uint32_t DiscoveryTime;			///<
	uint16_t DiscoveryPacketLength;	///<
};

struct TSource {
	uint32_t time;					///< The latest time of the data received from source
	uint32_t ip;					///< The IP address for source
	uint8_t data[E131_DMX_LENGTH];	///< The data received from source
	uint8_t cid[E131_CID_LENGTH];	///< Sender's CID. Sender's unique ID
	uint8_t sequenceNumberData;
};

struct TOutputPort {
	uint8_t data[E131_DMX_LENGTH];	///< Data sent
	uint16_t length;				///< Length of sent DMX data
	TMerge mergeMode;				///< \ref TMerge
	bool IsDataPending;				///<
	struct TSource sourceA;			///<
	struct TSource sourceB;			///<
};

class E131Bridge {
public:
	E131Bridge(void);
	~E131Bridge(void);

	void SetOutput(LightSet *);

	const uint8_t *GetSoftwareVersion(void);

	uint16_t GetUniverse(void) const;
	void SetUniverse(const uint16_t);

	inline uint32_t GetMulticastIp(void) {
		return m_nMulticastIp;
	}

	TMerge GetMergeMode(void) const;
	void SetMergeMode(TMerge);

	const uint8_t *GetCid(void);
	void SetCid(const uint8_t[E131_CID_LENGTH]);

	const char *GetSourceName(void);
	void SetSourceName(const char *);

	void Start(void);
	void Stop(void);

	int Run(void);

	void Print(void);

private:
	void FillDiscoveryPacket(void);

	bool IsValidRoot(void);
	bool IsValidDataPacket(void);

	void SetNetworkDataLossCondition(void);
	void CheckMergeTimeouts(void);
	bool IsPriorityTimeOut(void);
	bool isIpCidMatch(const struct TSource *);
	bool IsDmxDataChanged(const uint8_t *, uint16_t);
	bool IsMergedDmxDataChanged(const uint8_t *, uint16_t );

	void SendDiscoveryPacket(void);

	void HandleDmx(void);
	void HandleSynchronization(void);

private:
	LightSet *m_pLightSet;
	uint16_t m_nUniverse;
	uint32_t m_nMulticastIp;
	uint8_t m_Cid[E131_CID_LENGTH];
	char m_SourceName[E131_SOURCE_NAME_LENGTH];

	uint32_t m_DiscoveryIpAddress;

	uint32_t m_nCurrentPacketMillis;
	uint32_t m_nPreviousPacketMillis;

	struct TE131BridgeState m_State;
	struct TOutputPort m_OutputPort;

	struct TE131 m_E131;
	struct TE131DiscoveryPacket m_E131DiscoveryPacket;
};

#endif /* E131BRIDGE_H_ */
